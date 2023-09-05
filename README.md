![Banner](https://github.com/ExpressLRS/ExpressLRS-Hardware/blob/master/img/banner.png?raw=true)

<center>

[![Release](https://img.shields.io/github/v/release/ExpressLRS/ExpressLRS?style=flat-square)](https://github.com/ExpressLRS/ExpressLRS/releases)
[![Build Status](https://img.shields.io/github/actions/workflow/status/ExpressLRS/ExpressLRS/build.yml?logo=github&style=flat-square)](https://github.com/ExpressLRS/ExpressLRS/actions)
[![License](https://img.shields.io/github/license/ExpressLRS/ExpressLRS?style=flat-square)](https://github.com/ExpressLRS/ExpressLRS/blob/master/LICENSE)
[![Stars](https://img.shields.io/github/stars/ExpressLRS/ExpressLRS?style=flat-square)](https://github.com/ExpressLRS/ExpressLRS/stargazers)
[![Chat](https://img.shields.io/discord/596350022191415318?color=%235865F2&logo=discord&logoColor=%23FFFFFF&style=flat-square)](https://discord.gg/expresslrs)

</center>

## Support ExpressLRS
You can support ExpressLRS by contributing code, testing new features, sharing your ideas, or helping others get started. We are exceptionally grateful for those who donate their time to our passion.

If you don't have time to lend a hand in that way but still want to have an impact, consider donating. Donations are used for infrastructure costs and to buy test equipment needed to further the project and make it securely accessible. ExpressLRS accepts donations through Open Collective, which provides recognition of donors and transparency on how that support is utilized.

[![Open Collective backers](https://img.shields.io/opencollective/backers/expresslrs?label=Open%20Collective%20backers&style=flat-square)](https://opencollective.com/expresslrs)

We appreciate all forms of contribution and hope you will join us on Discord!

## Website
For general information on the project please refer to our guides on the [website](https://www.expresslrs.org/), and our [FAQ](https://www.expresslrs.org/2.0/faq/)

## About

ExpressLRS is an open source Radio Link for Radio Control applications. Designed to be the best FPV Racing link, it is based on the fantastic Semtech **SX127x**/**SX1280** LoRa hardware combined with an Espressif or STM32 Processor. Using LoRa modulation as well as reduced packet size it achieves best in class range and latency. It achieves this using a highly optimized over-the-air packet structure, giving simultaneous range and latency advantages. It supports both 900 MHz and 2.4 GHz links, each with their own benefits. 900 MHz supports a maximum of 200 Hz packet rate, with higher penetration. 2.4 GHz supports a blistering fast 1000 Hz on [EdgeTX](http://edgetx.org/). With over 60 different hardware targets and 13 hardware manufacturers, the choice of hardware is ever growing, with different hardware suited to different requirements.

## Configurator
To configure your ExpressLRS hardware, the ExpressLRS Configurator can be used, which is found here:

https://github.com/ExpressLRS/ExpressLRS-Configurator/releases/

## Community
We have both a [Discord Server](https://discord.gg/expresslrs) and [Facebook Group](https://www.facebook.com/groups/636441730280366), which have great support for new users and constant ongoing development discussion

## Features

ExpressLRS has the following features:

- 1000 Hz Packet Rate 
- Telemetry (Betaflight Lua Compatibility)
- Wifi Updates
- Bluetooth Sim Joystick
- Oled & TFT Displays
- 2.4 GHz or 900 MHz RC Link
- Ceramic Antenna - allows for easier installation into micros
- VTX and VRX Frequency adjustments from the Lua
- Bind Phrases - no need for button binding

with many more features on the way!

## Supported Hardware

ExpressLRS currently supports hardware from the following manufacturers: AxisFlying, BETAFPV, Flywoo, FrSky, HappyModel, HiYounger, HGLRC, ImmersionRC, iFlight, JHEMCU, Jumper, Matek, NamimnoRC, QuadKopters and SIYI.

For an exhaustive list of hardware targets and their user guides, check out the [Supported Hardware](https://www.expresslrs.org/2.0/hardware/supported-hardware/) and [Receiver Selection](https://www.expresslrs.org/2.0/hardware/receiver-selection/) pages on the website. We do not manufacture any of our hardware, so we can only provide limited support on defective hardware.

## Developers

If you are a developer and would like to contribute to the project, feel free to join the [discord](https://discord.gg/expresslrs) and chat about bugs and issues. You can also look for issues at the [GitHub Issue Tracker](https://github.com/ExpressLRS/ExpressLRS/issues). The best thing to do is to submit a Pull Request to the GitHub Repository. 

![](https://github.com/ExpressLRS/ExpressLRS-Hardware/blob/master/img/community.png?raw=true)

```text
Initialised devices
Setting FCC915 Mode
Number of FHSS frequencies = 40
Sync channel = 21
21 23 14 34 7 18 30 1 32 13 
3 5 16 37 29 8 35 33 28 39 
22 24 9 38 27 36 26 31 15 20 
2 4 0 19 12 25 6 11 10 17 
21 23 15 4 12 0 32 19 28 37 
30 1 31 27 5 8 24 38 6 13 
22 9 29 26 14 10 16 2 7 3 
36 20 35 17 18 34 39 33 11 25 
21 6 0 36 14 37 10 22 5 34 
12 35 29 7 39 13 19 30 11 4 
28 31 20 26 2 24 3 23 38 18 
8 25 17 9 15 1 27 16 33 32 
21 15 27 6 31 4 36 2 29 11 
24 37 34 5 30 33 19 39 13 1 
25 3 8 0 35 26 32 9 16 10 
20 22 18 38 17 14 28 12 23 7 
21 26 9 31 30 8 17 23 20 25 
2 13 22 3 19 12 11 10 35 16 
0 33 34 18 24 32 28 29 15 36 
4 38 6 1 14 27 7 5 37 39 
21 27 35 32 20 10 13 0 5 38 
23 1 24 33 26 39 12 36 6 8 
18 7 22 11 31 14 30 28 16 37 
17 15 3 34 4 9 25 29 2 19 

ExpressLRS TX Module Booted...
Config version 7
Hal Init
SX126x Reset
SX126x Ready!
SX126x Begin
RFAMP_hal Init
Use TX pin: 12
Use RX pin: 13
Read Vers sx126x #1 loraSyncword (5156): 5156
SetPower: 247
pwrCurrent: 10
SetPower: 0
set rate 0
Config LoRa freq: 951566587
freq: 907484614 hz
Adjusted max packet size 64-72
hwTimer Init
About to start CRSF task...$X
<ding get backpack version command
$X<ding get backpack version command
Too many bad UART RX packets!
UART WDT: Switch to: 400000 baud
Adjusted max packet size 64-64
UART STATS Bad:Good = 0:0
$X<ding get backpack version command
Too many bad UART RX packets!
autobaud: low 19, high 19
UART WDT: Switch to: 3750000 baud
Adjusted max packet size 64-88
UART STATS Bad:Good = 0:0
CRSF UART Connected
hwTimer resume
pwrCurrent: 0
UART STATS Bad:Good = 0:64
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
$X<ding get backpack version command
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:249
UART STATS Bad:Good = 0:251
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:249
UART STATS Bad:Good = 0:251
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
Set Lua [Bind]=1
hwTimer stop
set rate 3
hwTimer interval: 20000
Config LoRa freq: 951566587
freq: 907484614 hz
Adjusted max packet size 64-128
hwTimer resume
Entered binding mode at freq = 951566587
$X<     et rate 0
hwTimer interval: 4000
Config LoRa freq: 951566587
freq: 907484614 hz
Adjusted max packet size 64-88
Exiting binding mode
UART STATS Bad:Good = 0:190
UART STATS Bad:Good = 0:250
Set Lua [Bind]=6
UART STATS Bad:Good = 0:249
UART STATS Bad:Good = 0:251
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
UART STATS Bad:Good = 0:250
Set Lua [Bind]=1
hwTimer stop
```