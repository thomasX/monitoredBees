# monitoredBees 
is a project for monitoring the weight of a beehive including climate sensors for temperature,humidity and barometric pressure 
we can optional add up to 3 external temperature sensors, so we can monitor the temperature in the beehive

## hardware
- heltec cubecell AB01 
- Waveshare BME280 (Environmental Sensor, Temperature, Humidity, Barometric Pressure Detection Module Low Power Consumption I2C/SPI Interface)
- optional:  DS18B20 temperature sensor (1-Wire for external use) 
- 3,7V 1100mAh Lipo Accu 1S 3C
- optional: solar panel 1W 5V 100mA

## pinout

|Pin Function |  wiring | cubecell pin |
|----|----|----|
|BME280 SCK | | SCK |
|BME280 SDA | | SDA |
|    |    |    |
|HX711 DO|   |  GPIO2 |
|HX711 CLK|   |  GPIO3 |
|    |    |    |
| DS18B20 | pullup 10k to vcc | GPIO4 |
|    |    |    |
|----|
| experimental: |
|additional up to 7 HX711| oneWire protokoll| |
