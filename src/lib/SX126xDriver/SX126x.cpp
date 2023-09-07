#include "SX126xRegs.h"
#include "SX126xHal.h"
#include "SX126x.h"
#include "RFAMP_hal.h"
#include "logging.h"

SX126xHal hal;
SX126xDriver *SX126xDriver::instance = NULL;

RFAMP_hal RFAMP;
//DEBUG_SX126x_OTA_TIMING

/* Steps for startup

1. If not in STDBY_RC mode, then go to this mode by sending the command:
SetStandby(STDBY_RC)

2. Define the LoRa® packet type by sending the command:
SetPacketType(PACKET_TYPE_LORA)

3. Define the RF frequency by sending the command:
SetRfFrequency(rfFrequency)
The LSB of rfFrequency is equal to the PLL step i.e. 52e6/2^18 Hz. SetRfFrequency() defines the Tx frequency.

4. Indicate the addresses where the packet handler will read (txBaseAddress in Tx) or write (rxBaseAddress in Rx) the first
byte of the data payload by sending the command:
SetBufferBaseAddress(txBaseAddress, rxBaseAddress)
Note:
txBaseAddress and rxBaseAddress are offset relative to the beginning of the data memory map.

5. Define the modulation parameter signal BW SF CR
*/

#if defined(DEBUG_SX126x_OTA_TIMING)
static uint32_t beginTX;
static uint32_t endTX;
#endif

/*
 * Period Base from table 11-24, page 79 datasheet rev 3.2
 * SX126x_RADIO_TICK_SIZE_0015_US = 15625 nanos
 * SX126x_RADIO_TICK_SIZE_0062_US = 62500 nanos
 * SX126x_RADIO_TICK_SIZE_1000_US = 1000000 nanos
 * SX126x_RADIO_TICK_SIZE_4000_US = 4000000 nanos
 */
#define RX_TIMEOUT_PERIOD_BASE SX126x_RADIO_TICK_SIZE_0015_US
#define RX_TIMEOUT_PERIOD_BASE_NANOS 15625

#ifdef USE_SX126x_DCDC
    #ifndef OPT_USE_SX126x_DCDC
        #define OPT_USE_SX126x_DCDC true
    #endif
#else
    #define OPT_USE_SX126x_DCDC false
#endif

SX126xDriver::SX126xDriver(): SX12xxDriverCommon()
{
    instance = this;
    timeout = 0xFFFFFF;
    currOpmode = SX126x_MODE_SLEEP;
    lastSuccessfulPacketRadio = SX12XX_Radio_1;
}

void SX126xDriver::End()
{
    if (currOpmode != SX126x_MODE_SLEEP)
    {
        SetMode(SX126x_MODE_SLEEP, SX12XX_Radio_All);
    }
//    SetMode(SX126x_MODE_SLEEP, SX12XX_Radio_All);
    hal.end();
    RFAMP.TXRXdisable();
    RemoveCallbacks();
//    currFreq = (uint32_t)((double)903000000 / (double)FREQ_STEP);
}

bool SX126xDriver::Begin()
{
    hal.init();
    RFAMP.init();
    hal.IsrCallback_1 = &SX126xDriver::IsrCallback_1;
    hal.IsrCallback_2 = &SX126xDriver::IsrCallback_2;
    hal.reset();
    DBGLN("SX126x Begin MODE STDBY RC");
    SetMode(SX126x_MODE_STDBY_RC, SX12XX_Radio_All); // Put in STDBY_RC mode.  Must be SX126x_MODE_STDBY_RC for SX126x_RADIO_SET_REGULATORMODE to be set.

    uint8_t syncWordMSB = hal.ReadRegister(0x0740, SX12XX_Radio_1);
    uint8_t syncWordLSB = hal.ReadRegister(0x0741, SX12XX_Radio_1);
    uint16_t loraSyncword = ((uint16_t)syncWordMSB << 8) | syncWordLSB;
    DBGLN("Read Vers sx126x #1 loraSyncword (5156): %d", loraSyncword);
    if (loraSyncword != 0x1424) {
        return false;
    }

    if (GPIO_PIN_NSS_2 != UNDEF_PIN)
    {
        syncWordMSB = hal.ReadRegister(RADIOLIB_SX126X_REG_LORA_SYNC_WORD_MSB, SX12XX_Radio_2);
        syncWordLSB = hal.ReadRegister(RADIOLIB_SX126X_REG_LORA_SYNC_WORD_LSB, SX12XX_Radio_2);
        loraSyncword = ((uint16_t)syncWordMSB << 8) | syncWordLSB;
        DBGLN("Read Vers SX126X #2 loraSyncword (5156): %d", loraSyncword);
        if (loraSyncword != 0x1424) {
            return false;
        }
    }
    //set DIO2 as RF control switching
    //hal.WriteCommand(SX126x_RADIO_SET_DIO2_AS_RF_SWITCH_CTRL, 0x01, SX12XX_Radio_All);
    pwrCurrent = PWRPENDING_NONE;
    SetOutputPower(SX126X_POWER_MIN);

#if defined(USE_SX126x_DCDC)
    if (OPT_USE_SX126x_DCDC)
    {
        hal.WriteCommand(SX126x_RADIO_SET_REGULATORMODE, SX126x_USE_DCDC, SX12XX_Radio_All);        // Enable DCDC converter instead of LDO
    }
#endif

    return true;
}

void SX126xDriver::startCWTest(uint32_t freq, SX12XX_Radio_Number_t radioNumber)
{
    uint8_t buffer;         // we just need a buffer for the write command
    SetFrequencyHz(freq, radioNumber);
    CommitOutputPower();
    RFAMP.TXenable(radioNumber);
    hal.WriteCommand(SX126x_RADIO_SET_TXCONTINUOUSWAVE, &buffer, 0, radioNumber);
}

void SX126xDriver::Config(uint8_t bw, uint8_t sf, uint8_t cr, uint32_t regfreq,
                          uint8_t PreambleLength, bool InvertIQ, uint8_t _PayloadLength, uint32_t interval)
{
    PayloadLength = _PayloadLength;
    IQinverted = InvertIQ;
    SetMode(SX126x_MODE_STDBY_RC, SX12XX_Radio_All);
    // config PACKET_TYPE_LORA
    hal.WriteCommand(SX126x_RADIO_SET_PACKETTYPE, (uint8_t)SX126x_PACKET_TYPE_LORA, SX12XX_Radio_All, 20);
    // as per section 15.2: Better Resistance of the LLCC68 Tx to Antenna Mismatch

    uint8_t reg = hal.ReadRegister(RADIOLIB_SX126X_REG_TX_CLAMP_CONFIG, SX12XX_Radio_1);
    hal.WriteRegister(RADIOLIB_SX126X_REG_TX_CLAMP_CONFIG, reg | 0x1E, SX12XX_Radio_1);
    if (GPIO_PIN_NSS_2 != UNDEF_PIN)
    {
        reg = hal.ReadRegister(RADIOLIB_SX126X_REG_TX_CLAMP_CONFIG, SX12XX_Radio_2);
        hal.WriteRegister(RADIOLIB_SX126X_REG_TX_CLAMP_CONFIG, reg | 0x1E, SX12XX_Radio_2);
    }
    DBGLN("Config LoRa freq: %u", regfreq);
    SetFrequencyReg(regfreq);
    uint8_t calFreq[2];
    uint32_t freq = regfreq * FREQ_STEP;
    DBGLN("freq: %u hz", freq)
    if (freq > 900000000)
    {
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    }
    else if (freq > 850000000)
    {
        calFreq[0] = 0xD7;
        calFreq[1] = 0xDB;
    }
    else if (freq > 770000000)
    {
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    }
    else if (freq > 460000000)
    {
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    }
    else if (freq > 425000000)
    {
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    hal.WriteCommand(SX126X_RADIO_SET_CALIBRATE_IMAGE, calFreq, sizeof(calFreq), SX12XX_Radio_All);

    ConfigModParamsLoRa(bw, sf, cr);
#if defined(DEBUG_FREQ_CORRECTION)
    SX126x_RadioLoRaPacketLengthsModes_t packetLengthType = SX126x_LORA_PACKET_VARIABLE_LENGTH;
#else
    SX126x_RadioLoRaPacketLengthsModes_t packetLengthType = SX126x_LORA_PACKET_FIXED_LENGTH;
#endif
    SetPacketParamsLoRa(PreambleLength, packetLengthType, _PayloadLength, InvertIQ);

    SetRxTimeoutUs(interval);
    SetFIFOaddr(0, 0);

    uint8_t dio1Mask = SX126x_IRQ_TX_DONE | SX126x_IRQ_RX_DONE | SX126x_IRQ_RX_TX_TIMEOUT;
    uint8_t irqMask  = SX126x_IRQ_RADIO_ALL;
    SetDioIrqParams(irqMask, dio1Mask, SX126x_IRQ_RADIO_NONE, SX126x_IRQ_RADIO_NONE);

    ClearIrqStatus(SX126x_IRQ_RADIO_ALL, SX12XX_Radio_All);
}

void SX126xDriver::SetRxTimeoutUs(uint32_t interval)
{
    timeout = 0xFFFFFF; // no timeout, continuous mode
    if (interval)
    {
        timeout = interval * 1000 / RX_TIMEOUT_PERIOD_BASE_NANOS; // number of periods for the SX126x to timeout
    }
}

void SX126xDriver::clearTimeout(SX12XX_Radio_Number_t radioNumber)
{
    if (timeout == 0xFFFFFF)
        return;

    hal.WriteRegister(0x0902, 0x00, radioNumber);

    uint8_t tempValue;
    if (radioNumber & SX12XX_Radio_1)
    {
        tempValue = hal.ReadRegister(0x0944, SX12XX_Radio_1);
        hal.WriteRegister(0x0944, tempValue | 0x02, SX12XX_Radio_1);
    }

    if (GPIO_PIN_NSS_2 != UNDEF_PIN)
    {
        if (radioNumber & SX12XX_Radio_2)
        {
            tempValue = hal.ReadRegister(0x0944, SX12XX_Radio_2);
            hal.WriteRegister(0x0944, tempValue | 0x02, SX12XX_Radio_2);
        }
    }

    // Calling FS mode here to set any radios that may be still be in RX mode.  Important for Gemini.
    instance->SetMode(SX126x_MODE_FS, radioNumber);
}

void SX126xDriver::SetOutputPower(int8_t power)
{
    uint8_t pwrNew = constrain(power, SX126X_POWER_MIN, SX126X_POWER_MAX);

    if ((pwrPending == PWRPENDING_NONE && pwrCurrent != pwrNew) || pwrPending != pwrNew)
    {
        pwrPending = pwrNew;
        DBGLN("SetPower: %u", pwrPending);
    }
}

void ICACHE_RAM_ATTR SX126xDriver::CommitOutputPower()
{
    if (pwrPending == PWRPENDING_NONE)
        return;

    pwrCurrent = pwrPending;
    pwrPending = PWRPENDING_NONE;

    if (pwrCurrent > 30) // power range -9 to -1.
    {
        pwrCurrent = 22;
    }
    DBGLN("pwrCurrent: %u", pwrCurrent);
    // PA Operating Modes with Optimal Settings
    uint8_t paparams[4] = {0x04, 0x07, 0x00, 0x01};
    hal.WriteCommand(SX126X_RADIO_SET_PA_CONFIG, paparams, sizeof(paparams), SX12XX_Radio_All);

    uint8_t buf[2] = {pwrCurrent, (uint8_t)SX126x_RADIO_RAMP_02_US};
    hal.WriteCommand(SX126x_RADIO_SET_TXPARAMS, buf, sizeof(buf), SX12XX_Radio_All);
}

//void SX126xDriver::SetOutputPower(int8_t power)
//{
//    // 0 ~ 22
//    if (power < -18) power = -18;
//    else if (13 < power) power = 13;
//    uint8_t buf[2] = {(uint8_t)(power + 18), (uint8_t)SX126x_RADIO_RAMP_04_US};
//    hal.WriteCommand(SX126x_RADIO_SET_TXPARAMS, buf, sizeof(buf), SX12XX_Radio_All);
//    DBGLN("SetPower: %d", buf[0]);
//    return;
//}

void SX126xDriver::SetMode(SX126x_RadioOperatingModes_t OPmode, SX12XX_Radio_Number_t radioNumber)
{
    /*
    Comment out since it is difficult to keep track of dual radios.
    When checking SPI it is also useful to see every possible SPI transaction to make sure it fits when required.
    */
    // if (OPmode == currOpmode)
    // {
    //    return;
    // }

    WORD_ALIGNED_ATTR uint8_t buf[3];
    switch (OPmode)
    {

    case SX126x_MODE_SLEEP:
        hal.WriteCommand(SX126x_RADIO_SET_SLEEP, (uint8_t)0x01, radioNumber);
        break;

    // case SX126x_MODE_CALIBRATION:
    //     break;

    case SX126x_MODE_STDBY_RC:
        hal.WriteCommand(SX126x_RADIO_SET_STANDBY, SX126x_STDBY_RC, radioNumber, 1500);
        break;

    // The DC-DC supply regulation is automatically powered in STDBY_XOSC mode.
    case SX126x_MODE_STDBY_XOSC:
        hal.WriteCommand(SX126x_RADIO_SET_STANDBY, SX126x_STDBY_XOSC, radioNumber, 50);
        break;

    case SX126x_MODE_FS:
//        hal.WriteCommand(SX126x_RADIO_SET_FS, (uint8_t)0x00, radioNumber, 70);
        hal.WriteCommand(SX126x_RADIO_SET_FS, buf, 0, radioNumber, 70);
        break;

    case SX126x_MODE_RX:
        buf[0] = timeout >> 16;
        buf[1] = timeout >> 8;
        buf[2] = timeout & 0xFF;
        hal.WriteCommand(SX126x_RADIO_SET_RX, buf, sizeof(buf), radioNumber, 100);
        break;

    case SX126x_MODE_RX_CONT:
        buf[0] = 0xFF;
        buf[1] = 0xFF;
        buf[2] = 0xFF;
        hal.WriteCommand(SX126x_RADIO_SET_RX, buf, sizeof(buf), radioNumber, 100);
        break;

    case SX126x_MODE_TX:
        //uses timeout Time-out duration = periodBase * periodBaseCount
//        buf[0] = RX_TIMEOUT_PERIOD_BASE;
        buf[0] = 0x00;
        buf[1] = 0x00; // no timeout set for now
        buf[2] = 0x00; // TODO dynamic timeout based on expected onairtime
        hal.WriteCommand(SX126x_RADIO_SET_TX, buf, sizeof(buf), radioNumber, 100);
        break;

    case SX126x_MODE_CAD:
        break;

    default:
        break;
    }

    currOpmode = OPmode;
}

void SX126xDriver::ConfigModParamsLoRa(uint8_t bw, uint8_t sf, uint8_t cr)
{
    //auto FS
    hal.WriteCommand(SX126X_RADIO_SET_RX_TX_FALLBACK_MODE, RADIOLIB_SX126X_RX_TX_FALLBACK_MODE_FS, SX12XX_Radio_All);
    // Rx Boosted gain : 0x96
    hal.WriteRegister(RADIOLIB_SX126X_REG_RX_GAIN, 0x96, SX12XX_Radio_All);
    hal.WriteRegister(RADIOLIB_SX126X_REG_OCP_CONFIGURATION,0x38, SX12XX_Radio_All);

    CommitOutputPower();

    // Care must therefore be taken to ensure that modulation parameters are set using the command
    // SetModulationParam() only after defining the packet type SetPacketType() to be used
    WORD_ALIGNED_ATTR uint8_t rfparams[4] = {sf, bw, cr, 0x00};
    hal.WriteCommand(SX126x_RADIO_SET_MODULATIONPARAMS, rfparams, sizeof(rfparams), SX12XX_Radio_All, 25);

    // set tx
    uint8_t reg = hal.ReadRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, SX12XX_Radio_1);
    if (bw == SX126x_LORA_BW_500)
    {
        hal.WriteRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, reg & 0xFB, SX12XX_Radio_1);
    }
    else
    {
        hal.WriteRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, reg | 0x04, SX12XX_Radio_1);
    }
    if (GPIO_PIN_NSS_2 != UNDEF_PIN)
    {
        reg = hal.ReadRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, SX12XX_Radio_2);
        if (bw == SX126x_LORA_BW_500)
        {
            hal.WriteRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, reg & 0xFB, SX12XX_Radio_2);
        }
        else
        {
            hal.WriteRegister(RADIOLIB_SX126X_REG_SENSITIVITY_CONFIG, reg | 0x04, SX12XX_Radio_2);
        }
    }
}

void SX126xDriver::SetPacketParamsLoRa(uint8_t PreambleLength, SX126x_RadioLoRaPacketLengthsModes_t HeaderType,
                                       uint8_t PayloadLength, uint8_t InvertIQ)
{
    uint8_t buf[6];

    buf[0] = 0x00; // Preamble MSB
    buf[1] = PreambleLength;
    buf[2] = HeaderType;
    buf[3] = PayloadLength;
    buf[4] = SX126x_LORA_CRC_OFF;
    buf[5] = SX126x_LORA_IQ_NORMAL;
    hal.WriteCommand(SX126x_RADIO_SET_PACKETPARAMS, buf, sizeof(buf), SX12XX_Radio_All, 20);

    buf[0] = hal.ReadRegister(RADIOLIB_SX126X_REG_IQ_CONFIG, SX12XX_Radio_1);
    if (InvertIQ) {
        buf[0] &= ~0x04;
    } else {
        buf[0] |= 0x04;
    }
    hal.WriteRegister(RADIOLIB_SX126X_REG_IQ_CONFIG, buf[0], SX12XX_Radio_1);
    if (GPIO_PIN_NSS_2 != UNDEF_PIN)
    {
        buf[0] = hal.ReadRegister(RADIOLIB_SX126X_REG_IQ_CONFIG, SX12XX_Radio_2);
        if (InvertIQ) {
            buf[0] &= ~0x04;
        } else {
            buf[0] |= 0x04;
        }
        hal.WriteRegister(RADIOLIB_SX126X_REG_IQ_CONFIG, buf[0], SX12XX_Radio_2);
    }

    // FEI only triggers in Lora mode when the header is present :(
    modeSupportsFei = HeaderType == SX126x_LORA_PACKET_VARIABLE_LENGTH;

    uint8_t tmo_symbnum = (PayloadLength * 3) >> 2;
    uint8_t mant = (((tmo_symbnum > 248) ? 248 : tmo_symbnum) + 1) >> 1;
    uint8_t exp  = 0;
    uint8_t reg  = 0;
    while (mant > 31) {
        mant = ( mant + 3 ) >> 2;
        exp++;
    }
    reg = mant << (2 * exp + 1);
    hal.WriteCommand(SX126X_RADIO_SET_LORA_SYMB_NUM_TIMEOUT, reg, SX12XX_Radio_All);
    if (tmo_symbnum != 0) {
        reg = exp + (mant << 3);
        hal.WriteRegister(0x0706, reg, SX12XX_Radio_All);
    }
}

void ICACHE_RAM_ATTR SX126xDriver::SetFrequencyHz(uint32_t freq, SX12XX_Radio_Number_t radioNumber)
{
    uint32_t regfreq = (uint32_t)((double)freq / (double)FREQ_STEP);

    SetFrequencyReg(regfreq, radioNumber);
}

void ICACHE_RAM_ATTR SX126xDriver::SetFrequencyReg(uint32_t regfreq, SX12XX_Radio_Number_t radioNumber)
{
    WORD_ALIGNED_ATTR uint8_t buf[4] = {0};

    buf[0] = (uint8_t)((regfreq >> 24) & 0xFF);
    buf[1] = (uint8_t)((regfreq >> 16) & 0xFF);
    buf[2] = (uint8_t)((regfreq >> 8) & 0xFF);
    buf[3] = (uint8_t)(regfreq & 0xFF);

    hal.WriteCommand(SX126x_RADIO_SET_RFFREQUENCY, buf, sizeof(buf), radioNumber);

    currFreq = regfreq;
}

void SX126xDriver::SetFIFOaddr(uint8_t txBaseAddr, uint8_t rxBaseAddr)
{
    uint8_t buf[2];

    buf[0] = txBaseAddr;
    buf[1] = rxBaseAddr;
    hal.WriteCommand(SX126x_RADIO_SET_BUFFERBASEADDRESS, buf, sizeof(buf), SX12XX_Radio_All);
}

void SX126xDriver::SetDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask)
{
    uint8_t buf[8];

    buf[0] = (uint8_t)((irqMask >> 8) & 0x00FF);
    buf[1] = (uint8_t)(irqMask & 0x00FF);
    buf[2] = (uint8_t)((dio1Mask >> 8) & 0x00FF);
    buf[3] = (uint8_t)(dio1Mask & 0x00FF);
    buf[4] = (uint8_t)((dio2Mask >> 8) & 0x00FF);
    buf[5] = (uint8_t)(dio2Mask & 0x00FF);
    buf[6] = (uint8_t)((dio3Mask >> 8) & 0x00FF);
    buf[7] = (uint8_t)(dio3Mask & 0x00FF);

    hal.WriteCommand(SX126x_RADIO_SET_DIOIRQPARAMS, buf, sizeof(buf), SX12XX_Radio_All);
}

uint16_t ICACHE_RAM_ATTR SX126xDriver::GetIrqStatus(SX12XX_Radio_Number_t radioNumber)
{
    uint8_t status[2];

    hal.ReadCommand(SX126x_RADIO_GET_IRQSTATUS, status, 2, radioNumber);
    return status[0] << 8 | status[1];
}

void ICACHE_RAM_ATTR SX126xDriver::ClearIrqStatus(uint16_t irqMask, SX12XX_Radio_Number_t radioNumber)
{
    uint8_t buf[2];

    buf[0] = (uint8_t)(((uint16_t)irqMask >> 8) & 0xFF00);
    buf[1] = (uint8_t)((uint16_t)irqMask & 0x00FF);

    hal.WriteCommand(SX126x_RADIO_CLR_IRQSTATUS, buf, sizeof(buf), radioNumber);
}

void ICACHE_RAM_ATTR SX126xDriver::TXnbISR()
{
    currOpmode = SX126x_MODE_FS; // radio goes to FS after TX
#ifdef DEBUG_SX126x_OTA_TIMING
    endTX = micros();
    DBGLN("TOA: %d", endTX - beginTX);
#endif
    CommitOutputPower();
    TXdoneCallback();
}

void ICACHE_RAM_ATTR SX126xDriver::TXnb(uint8_t * data, uint8_t size, SX12XX_Radio_Number_t radioNumber)
{
    DBGLN("TXnb!");
    if (currOpmode == SX126x_MODE_TX) //catch TX timeout
    {
        DBGLN("catch TX Timeout!");
        SetMode(SX126x_MODE_FS, SX12XX_Radio_All);
        ClearIrqStatus(SX126x_IRQ_RADIO_ALL, SX12XX_Radio_All);
        TXnbISR();
        return;
    }
    if (radioNumber == SX12XX_Radio_NONE)
    {
        radioNumber = lastSuccessfulPacketRadio;
    }

    if (GPIO_PIN_NSS_2 != UNDEF_PIN && radioNumber != SX12XX_Radio_All)
    {
        // Make sure the unused radio is in FS mode and will not receive the tx packet.
        if (radioNumber == SX12XX_Radio_1)
        {
            instance->SetMode(SX126x_MODE_FS, SX12XX_Radio_2);
        }
        else
        {
            instance->SetMode(SX126x_MODE_FS, SX12XX_Radio_1);
        }
    }

    RFAMP.TXenable(radioNumber); // do first to allow PA stablise
    hal.WriteBuffer(0x00, data, size, radioNumber); //todo fix offset to equal fifo addr
    instance->SetMode(SX126x_MODE_TX, radioNumber);

#ifdef DEBUG_SX126x_OTA_TIMING
    beginTX = micros();
#endif
}

bool ICACHE_RAM_ATTR SX126xDriver::RXnbISR(uint16_t irqStatus, SX12XX_Radio_Number_t radioNumber)
{
    // In continuous receive mode, the device stays in Rx mode
    if (timeout != 0xFFFF)
    {
        // From table 11-28, pg 81 datasheet rev 3.2
        // upon successsful receipt, when the timer is active or in single mode, it returns to STDBY_RC
        // but because we have AUTO_FS enabled we automatically transition to state SX126x_MODE_FS
        currOpmode = SX126x_MODE_FS;
    }

    rx_status fail = SX12XX_RX_OK;
    if (fail == SX12XX_RX_OK)
    {
        uint8_t const FIFOaddr = GetRxBufferAddr(radioNumber);
        hal.ReadBuffer(FIFOaddr, RXdataBuffer, PayloadLength, radioNumber);
    }

    return RXdoneCallback(fail);
}

void ICACHE_RAM_ATTR SX126xDriver::RXnb(SX126x_RadioOperatingModes_t rxMode)
{
    RFAMP.RXenable();
    SetMode(rxMode, SX12XX_Radio_All);
}

uint8_t ICACHE_RAM_ATTR SX126xDriver::GetRxBufferAddr(SX12XX_Radio_Number_t radioNumber)
{
    WORD_ALIGNED_ATTR uint8_t status[2] = {0};
    hal.ReadCommand(SX126x_RADIO_GET_RXBUFFERSTATUS, status, 2, radioNumber);
    return status[1];
}

void ICACHE_RAM_ATTR SX126xDriver::GetStatus(SX12XX_Radio_Number_t radioNumber)
{
    uint8_t status = 0;
    hal.ReadCommand(SX126x_RADIO_GET_STATUS, (uint8_t *)&status, 1, radioNumber);
    DBGLN("Status: %x, %x, %x", (0b11100000 & status) >> 5, (0b00011100 & status) >> 2, 0b00000001 & status);
}

bool ICACHE_RAM_ATTR SX126xDriver::GetFrequencyErrorbool()
{
    // Only need the highest bit of the 20-bit FEI to determine the direction
    uint8_t feiMsb = hal.ReadRegister(SX126x_REG_LR_ESTIMATED_FREQUENCY_ERROR_MSB, lastSuccessfulPacketRadio);
    // fei & (1 << 19) and flip sign if IQinverted
    if (feiMsb & 0x08)
        return IQinverted;
    else
        return !IQinverted;
}

int8_t ICACHE_RAM_ATTR SX126xDriver::GetRssiInst(SX12XX_Radio_Number_t radioNumber)
{
    uint8_t status = 0;

    hal.ReadCommand(SX126x_RADIO_GET_RSSIINST, (uint8_t *)&status, 1, radioNumber);
    return -(int8_t)(status / 2);
}

void ICACHE_RAM_ATTR SX126xDriver::GetLastPacketStats()
{
    uint8_t status[2];
    hal.ReadCommand(SX126x_RADIO_GET_PACKETSTATUS, status, 2, processingPacketRadio);
    // LoRa mode has both RSSI and SNR
    LastPacketRSSI = -(int8_t)(status[0] / 2);
    LastPacketSNRRaw = (int8_t)status[1];
    // https://www.mouser.com/datasheet/2/761/DS_SX126x-1_V2.2-1511144.pdf p84
    // need to subtract SNR from RSSI when SNR <= 0;
    int8_t negOffset = (LastPacketSNRRaw < 0) ? (LastPacketSNRRaw / RADIO_SNR_SCALE) : 0;
    LastPacketRSSI += negOffset;
}

void ICACHE_RAM_ATTR SX126xDriver::IsrCallback_1()
{
    instance->IsrCallback(SX12XX_Radio_1);
}

void ICACHE_RAM_ATTR SX126xDriver::IsrCallback_2()
{
    instance->IsrCallback(SX12XX_Radio_2);
}

void ICACHE_RAM_ATTR SX126xDriver::IsrCallback(SX12XX_Radio_Number_t radioNumber)
{
    instance->processingPacketRadio = radioNumber;
    SX12XX_Radio_Number_t irqClearRadio = radioNumber;

    uint16_t irqStatus = instance->GetIrqStatus(radioNumber);

    if (irqStatus & SX126x_IRQ_TX_DONE)
    {
        RFAMP.TXRXdisable();
        instance->ClearIrqStatus(SX126x_IRQ_RADIO_ALL, SX12XX_Radio_All);
        instance->TXnbISR();
        irqClearRadio = SX12XX_Radio_All;
        return;
    }
    else if (irqStatus & SX126x_IRQ_RX_DONE)
    {
        if (instance->RXnbISR(irqStatus, radioNumber))
        {
            instance->lastSuccessfulPacketRadio = radioNumber;
            irqClearRadio = SX12XX_Radio_All; // Packet received so clear all radios and dont spend extra time retrieving data.
        }
        instance->ClearIrqStatus(SX126x_IRQ_RADIO_ALL, irqClearRadio);
        instance->clearTimeout(irqClearRadio);
        return;
    }
    else
    {
        return;
    }
    instance->ClearIrqStatus(SX126x_IRQ_RADIO_ALL, irqClearRadio);
}
