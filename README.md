# monitoredBees 
is a project for monitoring the weight of a beehive including climate sensors for temperature,humidity and barometric pressure 
we can optional add up to 3 external temperature sensors, so we can monitor the temperature in the beehive

keep in mind, that the max. airtime/node in lora is limited to 30 sec. per day
with sf=7 .... abaout 500 messages
with sf=12  ... max 20 messages per day ! 
... we send the data with an intervall of 5 minutes -> 288 messages per day

## hardware
- heltec cubecell AB01 
- Waveshare BME280 (Environmental Sensor, Temperature, Humidity, Barometric Pressure Detection Module Low Power Consumption I2C/SPI Interface)
- optional:  DS18B20 temperature sensor (1-Wire for external use) 
- 3,7V 1100mAh Lipo Accu 1S 3C
- optional: solar panel 1W 5V 100mA
- this version is tested with the Dragino LG02 LoRa gateway without TTN-Support. If you need TTN support, choose https://github.com/thomasX/monitoredLoraBees

### pinout

|Pin Function |  wiring | cubecell pin |
|----|----|----|
|BME280 SCK | yellow | SCK |
|BME280 SDA | blue | SDA |
|BME280 GND | black | gnd |
|BME280 vcc | red | vext  |
|    |    |    |
|HX711 DO|   |  GPIO4 |
|HX711 CLK|   |  GPIO5 |
|HX711 GND|   |  GND |
|HX711 VCC|   |  vext |
|    |    |    |
| DS18B20 | yello pullup 10k to vext | GPIO0 |
| DS18B20 | red  | vext |
| DS18B20 | black | gnd |
|    |    |    |
| experimental: |
|additional up to 7 HX711| oneWire protokoll| |
|----|
|    |    |    |

## software
we use the thingspeak network 
with the following field-definition it is possible to use the Android and IOS-APP from the honeyPi project (http://www.honey-pi.de ). so we have no extra app to produce     

| Field | Usage |
| ---- | ---- |
| field1 | temperature inside the hive |
| field2 | temperature outside the hive |
| field3 | humidity |
| field4 | air pressure |
| field5 | air quality |
| field6 | weight |
| field7 | unused |
