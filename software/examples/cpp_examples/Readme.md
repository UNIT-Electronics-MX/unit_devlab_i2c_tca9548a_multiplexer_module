# TCA9845A Arduino Code Explanation

This document explain the example code test for work with the I2C Multiplexor, with a test with the `I2C FRAM FM24W256 Unit Module` and the `I2C FT24C32A EEPROM UNIT Module` with conection to the `UNIT Pulsar ESP32-C6`.

## Included Libraries and Initialization
```cpp
#include <Wire.h>

#define MUX_ADDR 0x70

#define SDA_PIN 6
#define SCL_PIN 7

#define EEPROM_PAGE_SIZE 32


```
* `Wire.h`: Enables I2C communication
* `MUX_ADDR`: Configure the default address for the multiplexor, configurable by the dip switch on the board.
* `SDA_PIN`: Default Pin for the SDA connector.
* `SCL_PIN`: Default Pin fot the SCL connector.
* `EEPROM_PAGE_SIZE`: Default setting for the size of page for EEPROM memories.

### Functions Included 

* `muxSelectChannel()`: Activate a specific channel for communication
* `muxCloseChannels()`: Close communication with all channels
* `rawWrite()`: Send a byte of a specific data to a specific channel
* `rawRead()`: Read a quantity of bytes in a specific channel and an specific address
* `eepromWaitReady()`: Waits for the device to be ready after a write.
* `eepromReadSeq()`, `eepromWritePaged()`: Read/write memory, handling page boundaries.
* `handleCommand()`: Parses and executes user commands from the serial monitor.
* `cmdScan()`, `cmdRead()`, `cmdWrite()`,`cmdSetChannel()`: Command handlers.

## Prerequisites

- Ensure you have the required hardware and cables.
- Install necessary software tools (see [Installation](#installation) below).

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/UNIT-Electronics-MX/unit_devlab_i2c_tca9548a_multiplexer_module/tree/main
    ```
2. Open the folder inside of the next direction software/examples/cpp_examples/TCA9548A
3. Test the TCA9548.ino in Arduino IDE 

## Usage

1. Connect the I2C Devices to the TCA9548A multiplexor with the QWIIC connector.
2. Connect the TCA9548A module to the UNIT PULSAR ESP32 C6
3. Put the option of the CDC in enabled for can interact with the serial terminal, like in the image listed below ![Setting Up CDC](../../img/cdcenabled.png)
4. Upload the code example via USB or appropiate conecction to your board.
5. Open the serial monitor or terminal.
6. Use commands like `scan`, `capacity`, `read <channel> <addr> <len>`, `write <channel> <addr> <text>`, `setchannel <channel> <address>` to interact with the connected devices
- Build and flash the firmware as described in the [User Guide](docs/USER_GUIDE.md).

## Summary 
The firmware provides a robust, interactive way totes, use and debug devices connected to the `TCA9548A UNIT Module` on Arduino IDE platform. It handles device detection, setting up of channels listed in the device via specific address of the devices attached to this, making it ideal for expand the devices that we can connect to a microcontroller with a few pins and for learning, prototyping, or diagnostics.
## Support

For questions or issues, please open an issue on GitHub.