# Demo TCA — I2C Scanner for TCA9548A

Minimal I2C scanner for the TCA9548A multiplexer. Scans all 4 ports and prints the address of every detected device on the Serial Monitor.

## Hardware

| Signal | Pin (ESP32-C6) |
|--------|---------------|
| SDA    | 6             |
| SCL    | 7             |

Connect I2C devices to any of the 4 TCA9548A channels (SC0/SD0 … SC3/SD3).

## Configuration

```cpp
#define TCA_ADDR  0x70  // Default multiplexer address (set via DIP switch)
#define TCA_PORTS 4     // Number of ports to scan (max 8)
```

## Getting Started

1. **Clone the repository**
   ```sh
   git clone git@github.com:UNIT-Electronics-MX/unit_devlab_i2c_tca9548a_multiplexer_module.git
   ```

2. **Open the sketch**
   ```
   software/examples/cpp_examples/demo_tca/demo_tca.ino
   ```

3. **Upload** to the board using Arduino IDE.

4. **Open Serial Monitor** at **115200 baud**.

## Expected Output

```
Port #0
  0x3C
Port #1
Port #2
  0x68
Port #3
done
```

Each detected device address is printed under its port. Ports with no devices show only the header line.

## Code Overview

| Function / block | Description |
|-----------------|-------------|
| `tcaselect(port)` | Activates one TCA9548A port via I2C write |
| `setup()` | Initialises I2C on pins 6/7 and Serial at 115200 |
| `loop()` | Iterates ports → scans addresses 0x01–0x7E → prints hits |
