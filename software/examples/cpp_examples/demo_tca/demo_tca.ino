// I2C Scanner - TCA9548A Multiplexer
#include <Wire.h>

#define TCA_ADDR 0x70
#define TCA_PORTS 4

void tcaselect(uint8_t port) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << port);
  Wire.endTransmission();
}

void setup() {
  Wire.begin(6, 7);
  Serial.begin(115200);
}

void loop() {
  for (byte t = 0; t < TCA_PORTS; t++) {
    tcaselect(t);
    Serial.print("Port #"); Serial.println(t);
    for (byte addr = 1; addr < 127; addr++) {
      if (addr == TCA_ADDR) continue;
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0)
        Serial.printf("  0x%02X\n", addr);
    }
  }
  Serial.println("done\n");
  delay(1000);
}