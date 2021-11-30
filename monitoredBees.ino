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
#include "Seeed_BME280.h"
#include <Wire.h>

BME280 bme280;




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
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=0;
uint8_t isBME280detected=0;


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
double airQuality;
double unusedField;



int16_t rssi,rxSize;
void  DoubleToString( char *str, double double_num,unsigned int len);

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
  turnOnRGB(COLOR_RECEIVED,0); //change rgb color
  generateDataPacket();  
  turnOnRGB(COLOR_SEND,0); //change rgb color

  Serial.printf("sending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
  state=WAITING;
  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
  Serial.printf("STATE: SENDING\r\n");
  delay(500);
  turnOffRGB();
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
    DoubleToString(txpacket,airPressure,3);
    sprintf(txpacket,"%s&field5=",txpacket);
    DoubleToString(txpacket,airQuality,3);
    sprintf(txpacket,"%s&field6=",txpacket);
    DoubleToString(txpacket,unusedField,3);
}

void readSensorValues( void )
{
  Serial.printf("bin im readSensoValues\r\n");
  if (isBME280detected) {
    insideTemp=0.0;
    outsideTemp=bme280.getTemperature();
    humidity=bme280.getHumidity();
    airPressure=bme280.getPressure();
    airQuality=0;
    unusedField=bme280.calcAltitude(airPressure);
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
  //turnOnRGB(0,0);
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
    Serial.print("INITIALIZING !!! \r\n");
    if (bme280.init()) isBME280detected=1;
    insideTemp=0.0;
    outsideTemp=0.0;
    humidity=0.0;
    airPressure=0.0;
    airQuality=0;
    unusedField=0.0;
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
