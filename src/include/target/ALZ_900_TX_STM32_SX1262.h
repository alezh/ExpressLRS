#ifndef DEVICE_NAME
#define DEVICE_NAME "ELRS 900TX"
#endif

#define TARGET_ALZ_900_TX_STM32_PCB

#define BACKPACK_LOGGING_BAUD 420000

#if !defined(USE_OLED_SPI_SMALL)
    #define USE_OLED_SPI
#endif

// GPIO pin definitions
#define GPIO_PIN_NSS                PA15
#define GPIO_PIN_BUSY               PB15
#define GPIO_PIN_DIO1               PB2
#define GPIO_PIN_MOSI               PA7
#define GPIO_PIN_MISO               PA6
#define GPIO_PIN_SCK                PA5
#define GPIO_PIN_RST                PB0

#define GPIO_PIN_TX_ENABLE          PA8  // Works on Lite
#define GPIO_PIN_RX_ENABLE          PB14 // Works on Lite

#define GPIO_PIN_ANT_CTRL           PA9
#define GPIO_PIN_ANT_CTRL_COMPL     PB13

#define GPIO_PIN_RCSIGNAL_RX        PA10 // S.PORT (Only needs one wire )  UART 3
#define GPIO_PIN_RCSIGNAL_TX        PB6  // Needed for CRSF libs but does nothing/not hooked up to JR module.  UART 3

/* WS2812 led */
#define GPIO_PIN_LED_WS2812         PB6
#define GPIO_PIN_LED_WS2812_FAST    PB_6

#define GPIO_PIN_PA_ENABLE          PB11  // https://www.skyworksinc.com/-/media/SkyWorks/Documents/Products/2101-2200/SE2622L_202733C.pdf
#define GPIO_PIN_RF_AMP_DET         PA3  // Voltage detector pin

#define GPIO_PIN_BUZZER             PC13
#define GPIO_PIN_OLED_CS            PC14
#define GPIO_PIN_OLED_RST           PB12
#define GPIO_PIN_OLED_DC            PC15
#define GPIO_PIN_OLED_MOSI          PB5
#define GPIO_PIN_OLED_SCK           PB3
#define GPIO_PIN_JOYSTICK           A0

#define GPIO_PIN_DEBUG_RX           PA3 // WTF! It's used for AMP_DET  UART 1
#define GPIO_PIN_DEBUG_TX           PA2 // UART 1


// #define RADIO_SX126X

// Output Power
#define MinPower                PWR_50mW
#define MaxPower                PWR_1000mW
#define DefaultPower            PWR_50mW
#define POWER_OUTPUT_VALUES     {0,10,14,17,22}

#ifndef JOY_ADC_VALUES
/* Joystick values              {UP, DOWN, LEFT, RIGHT, ENTER, IDLE}*/
#define JOY_ADC_VALUES          {459, 509, 326, 182, 91, 1021}
#endif