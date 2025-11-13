#ifndef PCA9554_H
#define PCA9554_H

#include <Wire.h>
#include <Arduino.h>

// PCA9554 I2C GPIO Expander
// Address: 0x20 (hardwired on ESP32-S3-8048S070)
// This chip controls:
// - GPIO0: Display Reset (ST7701)
// - GPIO1: Touch Reset (GT911)
// - GPIO2: Display CS (Chip Select)

#define PCA9554_ADDRESS 0x20
#define PCA9554_INPUT_PORT_REG 0x00
#define PCA9554_OUTPUT_PORT_REG 0x01
#define PCA9554_CONFIG_REG 0x03

// Pin definitions
#define PCA9554_DISPLAY_RST 0  // GPIO0 - Display reset
#define PCA9554_TOUCH_RST 1    // GPIO1 - Touch reset
#define PCA9554_DISPLAY_CS 2   // GPIO2 - Display chip select

bool pca9554_init() {
  Serial.println("Initializing PCA9554 GPIO Expander...");
  
  // Configure all pins as outputs (0 = output, 1 = input)
  // Set config register to 0x00 (all outputs)
  Wire.beginTransmission(PCA9554_ADDRESS);
  Wire.write(PCA9554_CONFIG_REG);
  Wire.write(0x00);  // All pins as outputs
  int result = Wire.endTransmission();
  
  if (result != 0) {
    Serial.print("PCA9554 config write failed with error: ");
    Serial.println(result);
    return false;
  }
  
  delay(10);
  
  // Set initial output states
  // All pins HIGH (inactive state for reset pins, CS inactive high)
  Wire.beginTransmission(PCA9554_ADDRESS);
  Wire.write(PCA9554_OUTPUT_PORT_REG);
  Wire.write(0xFF);  // All pins HIGH
  result = Wire.endTransmission();
  
  if (result != 0) {
    Serial.print("PCA9554 output write failed with error: ");
    Serial.println(result);
    return false;
  }
  
  delay(10);
  
  Serial.println("PCA9554 initialized successfully");
  return true;
}

bool pca9554_set_pin(uint8_t pin, uint8_t level) {
  // Read current output state
  Wire.beginTransmission(PCA9554_ADDRESS);
  Wire.write(PCA9554_OUTPUT_PORT_REG);
  int result = Wire.endTransmission();
  
  if (result != 0) {
    Serial.print("PCA9554 read request failed with error: ");
    Serial.println(result);
    return false;
  }
  
  // Read the output port register
  uint8_t current_state = 0;
  if (Wire.requestFrom(PCA9554_ADDRESS, 1)) {
    current_state = Wire.read();
  } else {
    Serial.println("PCA9554 read failed");
    return false;
  }
  
  // Modify the specific pin
  if (level) {
    current_state |= (1 << pin);   // Set bit
  } else {
    current_state &= ~(1 << pin);  // Clear bit
  }
  
  // Write back the modified state
  Wire.beginTransmission(PCA9554_ADDRESS);
  Wire.write(PCA9554_OUTPUT_PORT_REG);
  Wire.write(current_state);
  result = Wire.endTransmission();
  
  if (result != 0) {
    Serial.print("PCA9554 write failed with error: ");
    Serial.println(result);
    return false;
  }
  
  return true;
}

void pca9554_reset_display() {
  Serial.println("Resetting display (PCA9554 GPIO0)...");
  pca9554_set_pin(PCA9554_DISPLAY_RST, 0);  // LOW
  delay(100);
  pca9554_set_pin(PCA9554_DISPLAY_RST, 1);  // HIGH
  delay(100);
  Serial.println("Display reset complete");
}

void pca9554_reset_touch() {
  Serial.println("Resetting touch (PCA9554 GPIO1)...");
  pca9554_set_pin(PCA9554_TOUCH_RST, 0);   // LOW
  delay(100);
  pca9554_set_pin(PCA9554_TOUCH_RST, 1);   // HIGH
  delay(100);
  Serial.println("Touch reset complete");
}

void pca9554_set_cs_low() {
  pca9554_set_pin(PCA9554_DISPLAY_CS, 0);  // LOW
}

void pca9554_set_cs_high() {
  pca9554_set_pin(PCA9554_DISPLAY_CS, 1);  // HIGH
}

#endif // PCA9554_H
