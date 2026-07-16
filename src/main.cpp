#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Zyntra I2C Scanner starting...");
  Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22
}

void loop() {
  Serial.println("\nScanning I2C bus...");

  byte deviceCount = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);

      // Identify the device
      if (address == 0x57) Serial.println("  --> MAX30102 (HRV sensor)");
      else if (address == 0x5A) Serial.println("  --> MLX90614 (temperature sensor)");
      else if (address == 0x3C) Serial.println("  --> SSD1306 OLED display");
      else if (address == 0x3D) Serial.println("  --> SSD1306 OLED display (alt address)");
      else if (address == 0x68) Serial.println("  --> DS3231/DS1302 RTC");
      else Serial.println("  --> Unknown device");

      deviceCount++;
    }
  }

  if (deviceCount == 0) {
    Serial.println("No I2C devices found. Check your wiring.");
  } else {
    Serial.print("Total devices found: ");
    Serial.println(deviceCount);
  }

  Serial.println("Scan complete. Waiting 3 seconds...");
  delay(3000);
}