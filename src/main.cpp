#include "adc_esp.h"
#include <Arduino.h>

int voltage;

ESP32_ADC_Config config;

int gpio_pin = 4;  // Example: GPIO4
adc_unit_t unit;
adc_channel_t channel;

void setup() {
    // 1. Initialize ADC unit
    config.unit = ADC_UNIT_2;
    config.width = ADC_WIDTH_BIT_12;
    config.attenuation = ADC_ATTEN_11db;  // For full 0-3.3V range
    config.vref = 1100;  // Default reference voltage (1.1V)
    
    esp32_adc_init(&config);
    
    // 2. Determine ADC channel for GPIO pin
    
    esp32_adc_gpio_to_channel(gpio_pin, &unit, &channel);
    
    // 3. Configure the specific ADC channel
    esp32_adc_config_channel(unit, channel, config.attenuation);
    
    // Now you can read from this ADC pin
    int raw_value = esp32_adc_read_raw(unit, channel);
    voltage = esp32_adc_read_voltage(unit, channel, &config);

    Serial.begin(115200);
    Serial.printf("hello world\n");
    Serial.printf("Raw ADC Value: %d\n", raw_value);
}

void loop() {
  int raw = esp32_adc_read_raw(unit, channel); 
  Serial.printf("Voltage: %d mV\n", voltage);
  Serial.printf("Raw ADC Value: %d\n", raw);
  delay(1000);
  voltage = esp32_adc_read_voltage(unit, channel, &config);
}