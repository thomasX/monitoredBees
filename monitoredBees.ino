/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a CubeCell device over hardware 
 * 
 * 
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/ASR650x-Arduino
 * */

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)
#define USEIIC 1
#define BME280_ADDRESS (0x76)
Adafruit_BME280 bme280; // I2C


/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY                                868200000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
//#define BUFFER_SIZE                                 30 // Define the payload size here
#define BUFFER_SIZE                                 80 // Define the payload size here


// ---- toms paramater
#define timetillwakeup 300000
//#define timetillwakeup 10000







static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=0;



const char DEVICE_ID[] PROGMEM = "4567";
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

double insideTemp;
double outsideTemp;
double humidity;
double airPressure;
double airPressureHPA;
double airQuality;
double unusedField;
double voltage;



int16_t rssi,rxSize;
void  DoubleToString( char *str, double double_num,unsigned int len);
unsigned BME280detected;


typedef enum
{
    READSENSORDATA,
    TX,
    WAITING
}States_t;

States_t state;

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  delay(500);
  lowpower=1;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}

void onWakeUp()
{
  Serial.printf("Woke up\r\n");
  lowpower=0;
  delay(500);
  state=READSENSORDATA;
}


void sendLoraData()
{
  //1turnOnRGB(COLOR_RECEIVED,0); //change rgb color  ... funktioniert nicht mit lowpower und sensor
  generateDataPacket();  
  //1turnOnRGB(COLOR_SEND,0); //change rgb color    ... funktioniert nicht mit lowpower und sensor

  Serial.printf("sending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
  state=WAITING;
  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
  Serial.printf("STATE: SENDING\r\n");
  //delay(500);
  //1turnOffRGB();    ... funktioniert nicht mit lowpower und sensor
}

void generateDataPacket( void )
{
    sprintf(txpacket,"<%s>field1=",DEVICE_ID);
    DoubleToString(txpacket,insideTemp,3);
    sprintf(txpacket,"%s&field2=",txpacket);
    DoubleToString(txpacket,outsideTemp,3);
    sprintf(txpacket,"%s&field3=",txpacket);
    DoubleToString(txpacket,humidity,3);
    sprintf(txpacket,"%s&field4=",txpacket);
    DoubleToString(txpacket,airPressureHPA,3);  
    //DoubleToString(txpacket,airPressure,3);
    sprintf(txpacket,"%s&field5=",txpacket);
 //   DoubleToString(txpacket,airQuality,3);
    DoubleToString(txpacket,voltage,3);
    sprintf(txpacket,"%s&field6=",txpacket);
    DoubleToString(txpacket,unusedField,3);
}

void readSensorValues( void )
{
  Serial.printf("bin im readSensoValues\r\n");
    digitalWrite(Vext, LOW);
    BME280detected=bme280.begin();
  if (BME280detected) {
    Serial.printf("bin da\r\n");
     delay(100);
    insideTemp=0.0;
    Serial.print("oooo   :");
    Serial.println(bme280.readTemperature());
    outsideTemp=bme280.readTemperature();
    humidity=bme280.readHumidity();
    airPressure=bme280.readPressure();
    airPressureHPA=(airPressure / 100.0);
    airQuality=0;
    unusedField=bme280.readAltitude(SEALEVELPRESSURE_HPA);
    Wire.end();
    uint16_t voltageRAW = getBatteryVoltage();
    voltage = static_cast<double>(voltageRAW);
    voltage = (voltage / 1000.0);
  }
  state=TX;
  Serial.printf("STATE: TX\r\n");
}

void OnTxDone( void )
{
  Serial.printf("TX done!\r\n");
  delay(500);
  state=WAITING;
  onSleep();
  //turnOnRGB(0,0);  ... funktioniert nicht mit lowpower und sensor
}

void OnTxTimeout( void )
{
    //Radio.Sleep( );
    Serial.printf("TX Timeout......\r\n");
    state=READSENSORDATA;
    Serial.printf("STATE: READSENSORDATA\r\n");
}

/**
  * @brief  Double To String
  * @param  str: Array or pointer for storing strings
  * @param  double_num: Number to be converted
  * @param  len: Fractional length to keep
  * @retval None
  */
void  DoubleToString( char *str, double double_num,unsigned int len) { 
  double fractpart, intpart;
  fractpart = modf(double_num, &intpart);
  fractpart = fractpart * (pow(10,len));
  sprintf(str + strlen(str),"%d", (int)(intpart)); //Integer part
  sprintf(str + strlen(str), ".%d", (int)(fractpart)); //Decimal part
}


void setup() {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(500);
    Serial.begin(115200);
    delay(2000);
    while(!Serial);    // time to get serial running
    Serial.print("monitoredBees is initializing \r\n");
    BME280detected=bme280.begin();
    if (BME280detected) Serial.printf("BME280 initialized= %s \r\n","true");
    if (!BME280detected)Serial.printf("BME280 initialized= %s \r\n","false");
    //digitalWrite(Vext, LOW);


    insideTemp=0.0;
    outsideTemp=0.0;
    humidity=0.0;
    airPressure=0.0;
    airPressureHPA=0.0;
    airQuality=0;
    unusedField=0.0;
    voltage=0;
    rssi=0;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

    TimerInit( &sleep, onSleep );
    TimerInit( &wakeUp, onWakeUp );
    state=READSENSORDATA;
    Serial.printf("STATE:READSENSORDATA\r\n");
   }



void loop()
{ 
  if (lowpower==1) {
    lowPowerHandler();
  } else {
    switch(state)
    {
      case TX: {
        sendLoraData();
        break;
      }
      case READSENSORDATA:
      {
        readSensorValues();
        break;
      }
      case WAITING:
      {
        Radio.IrqProcess();
        break;
      }
    }
  }
}
