// FQBN: esp32:esp32:esp32c3:CPUFreq=80,FlashFreq=40,FlashMode=dio

#include "config.h"

#include <WiFi.h>
#include <PubSubClient.h>

// non-interrupt task planner
// https://github.com/joysfera/arduino-tasker
#include <Tasker.h>

#include <ArduinoJson.h>
DynamicJsonDocument * json;

// pro NAN
#include <math.h>

#include "FveData.h"
FveData data;

#include "DeviceInfo.h"

#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include "Adafruit_ST77xx.h"
#include <SPI.h>
#include "src/ExtDisplay/ExtDisplay.h"
#include "src/fonts/fonts.h"

Adafruit_ST7735 *display;
ExtDisplay * extdisplay;

WiFiClient espClient;
PubSubClient mqtt(espClient);
Tasker tasker;

/** stav spojeni na MQTT */
bool mqttOK = false;

/** stav WiFi */
bool wifiOK = false;

/**
Victron device ID - needed for KeepAlive message
*/
char victronDeviceId[20];

void blinkLed()
{
  pinMode( LED, OUTPUT );
  digitalWrite( LED, LOW );
  delay(500);
  digitalWrite( LED, HIGH );
  delay(500);
  digitalWrite( LED, LOW );
}

/**
It is good to know what application and which version is used in device.
*/
void printStartupInfo() 
{
  Serial.printf( "%s, %s %s\n", __FILE__, __DATE__, __TIME__ );

  char tmp[200];
  formatDeviceInfo( tmp );
  Serial.println( tmp );
}

void setup() {
  victronDeviceId[0] = 0;
  json = new DynamicJsonDocument(2000);
    
  Serial.begin(115200);
  blinkLed();

  pinMode( TFT_BACKLIGHT, OUTPUT );
  digitalWrite( TFT_BACKLIGHT, HIGH );
  SPI.end(); // release standard SPI pins
  SPI.begin(TFT_SCLK, SPI_MISO_UNUSED, TFT_MOSI, TFT_CS); // map and init SPI pins
  display = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
  display->initR(INITR_BLACKTAB);
  display->setRotation(1);
  display->fillScreen(ST77XX_BLACK);

  extdisplay = new ExtDisplay();
  extdisplay->init( display );
  extdisplay->setPos( 0, 50 );
  extdisplay->setFont( fnt_text1(), -8, 0 );
  display->setTextColor( ST77XX_YELLOW );
  extdisplay->printUTF8( "Spouštím..." );

  printStartupInfo();
  
  startWifi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqqtCallback);
  // victron messages are larger than default
  mqtt.setBufferSize(1024);

  tasker.setInterval( raPrintHeap, 60000 );
  tasker.setInterval( doDisplay, 2500 );
}

void loop() {
  checkWifiStatus();
  
  tasker.loop();

  if( wifiOK ) {
    if( !mqttOK ) {
      connectMqtt();   
    } else if( !mqtt.connected() ) {
      connectMqtt();   
    } else {
      mqtt.loop();
    }
  }
}

/**
Called periodically from Tasker
*/
void displayData() 
{
  Serial.println( data.toString() );
}


/** Time of last connect operation to MQTT. To limit maximal throughput of requests. */
long lastConnectionTry = -60000;
#define MQTT_CONN_INTERVAL_MS 10000

void connectMqtt()
{
  if (mqtt.connected()) {
    return;
  }
  mqttOK = false;

  if( millis() - lastConnectionTry < MQTT_CONN_INTERVAL_MS ) {
    return;
  }

  lastConnectionTry = millis();

  String client_id = "esp32-";
  client_id += String(WiFi.macAddress());
  Serial.printf("MQTT client %s connects to %s:%d\n", client_id.c_str(), MQTT_BROKER, MQTT_PORT );
  if (! mqtt.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print("MQTT failed with state ");
    Serial.println(mqtt.state());
    return;
  }

  Serial.println("MQTT Connected");
  mqttOK = true;

  if( victronDeviceId[0]==0 ) {
    // nejprve si subskribneme ID systemu, abychom vedeli, s kym si povidat
    mqtt.subscribe("N/+/system/0/Serial");
  } else {
    mqttSubscribeAll();
  }
}


/**
Subscribe only topics we can parse something from.
*/
void mqttSubscribeAll() 
{
  // mqtt.subscribe("N/#" );
  char tmp[100];
  sprintf( tmp, "N/%s/vebus/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/solarcharger/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Ac/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Dc/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Batteries/#", victronDeviceId );
  mqtt.subscribe( tmp );
}


/**
Called when message is received 
*/
void mqqtCallback(char *topic, byte *payload, unsigned int length) {
  data.msgs++;

  payload[length] = 0;
  Serial.printf("> %s: %s\n", topic, payload);

  if( checkTopicName("N/+/system/0/Serial", topic ) ) {
    const char * v = getValue(payload);
    if( v!=NULL ) {
      mqtt_SystemSerialNumber( v );
    }
  } else if( checkTopicName("N/+/system/0/Dc/Battery/Power", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.accuP = v;
      data.update(); 
    }
  } else if( checkTopicName("N/+/system/0/Ac/Consumption/L1/Power", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.consP = v;
      data.update(); 
    }
  } else if( checkTopicName("N/+/solarcharger/+/Pv/V", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.solarU = v;
      data.update(); 
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/0/Yield", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day0.solarYield = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/1/Yield", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day1.solarYield = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/2/Yield", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day2.solarYield = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/3/Yield", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day3.solarYield = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/0/MaxPower", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day0.solarMaxPower = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/1/MaxPower", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day1.solarMaxPower = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/2/MaxPower", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day2.solarMaxPower = v;
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/3/MaxPower", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day3.solarMaxPower = v;
    }
  }  else if( checkTopicName("N/+/vebus/+/Leds/Absorption", topic ) ) {
    data.ledAbsorption = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Bulk", topic ) ) {
    data.ledBulk = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Float", topic ) ) {
    data.ledFloat = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Inverter", topic ) ) {
    data.ledInverter = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/LowBattery", topic ) ) {
    data.ledLowBattery = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Overload", topic ) ) {
    data.ledOverload = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Temperature", topic ) ) {
    data.ledTemperature = getIntValue(payload);
    data.update(); 
  }  else if( checkTopicName("N/+/system/0/Batteries", topic ) ) {
    /*
      Parses only _first_ battery as I have no more.
      Expected data:

      {"value": 
        [
          {
              "name": "MultiPlus-II 48/3000/35-32", 
              "soc": 92.5, 
              "power": -51, 
              "active_battery_service": true, 
              "instance": 276, 
              "id": "com.victronenergy.vebus.ttyS4", 
              "state": 2, 
              "voltage": 49.779998779296875, 
              "current": -1.600000023841858
          }
        ]
      }
    */
    DeserializationError de = deserializeJson( *json, payload );
    if( !de ) {
      double v = (*json)["value"][0]["soc"];
      data.accuSoc = v;
      v = (*json)["value"][0]["voltage"];
      data.accuU = v;
      v = (*json)["value"][0]["current"];
      data.accuI = v;
      data.update(); 
    } else {
      Serial.printf( "JSON error: %s\n", de.c_str() );
    }
  }
}

/**
Called when Victron system ID is received
*/
void mqtt_SystemSerialNumber( const char * sernum ) 
{
  if( victronDeviceId[0]==0 ) {
    Serial.printf( "System serial ID: %s\n", sernum );
    strcpy( victronDeviceId, sernum );
    mqttSubscribeAll();
    sendKeepAlive();
    tasker.setInterval( sendKeepAlive, 55000 );
    tasker.setInterval( displayData, 60000 );
  }
}


/**
Gets value from basic message:
  {"value": "c0619ab33b2f"}
*/
const char * getValue(unsigned char * payload) {
  DeserializationError de = deserializeJson( *json, payload );
  if( !de ) {
    const char * text1 = (*json)["value"];
    return text1;
  } else {
    Serial.printf( "JSON error: %s\n", de.c_str() );
    return NULL;
  }
}

/**
Gets value from basic message:
  {"value": 11.2 }
*/
double getDoubleValue(unsigned char * payload ) {
  DeserializationError de = deserializeJson( *json, payload );
  if( !de ) {
    double v = (*json)["value"];
    return v;
  } else {
    Serial.printf( "JSON error: %s\n", de.c_str() );
    return NAN;
  }
}


/**
Gets value from basic message:
  {"value": 1}
*/
int getIntValue(unsigned char * payload) {
  DeserializationError de = deserializeJson( *json, payload );
  if( !de ) {
    int v = (*json)["value"];
    return v;
  } else {
    Serial.printf( "JSON error: %s\n", de.c_str() );
    return 0;
  }
}


/**
Checks if topic name equals to a mask.
Mask can contain '+' as folder name wildcard

  "N/+/system/0/Serial" == "N/c0619ab33b2f/system/0/Serial"
*/ 
bool checkTopicName( const char * mask, char * topic ) {
  int mask_i = 0;
  int topic_i = 0;
  while( true ) {
    if( mask[mask_i]==0 && topic[topic_i]==0 ) {
      return true;
    }
    if( mask[mask_i]==0 || topic[topic_i]==0 ) {
      return false;
    }
    if( mask[mask_i]=='+' ) {
      mask_i++;
      // rolovat topic az do /
      while(topic[topic_i]!='/' && topic[topic_i]!=0) {
        topic_i++;
      }
    }
    if( mask[mask_i] != topic[topic_i] ) {
      return false;
    }
    mask_i++;
    topic_i++;
  }
}

/* ******************* Keep alive  *********************/

/**
Must send keep alive request every 55 sec to get any data
Called from tasker.
Tasker set up from mqtt_SystemSerialNumber().
https://community.victronenergy.com/questions/155407/mqtt-local-via-mqtt-broker.html
*/
void sendKeepAlive()
{
  if( mqttOK && mqtt.connected() ) {
    char topic[50];
    sprintf( topic, "R/%s/keepalive", victronDeviceId );
    mqtt.publish( topic, "");
    Serial.println( "* KA" );
  }
}




/*  ******************** Memory info *******************************************  */

int prevFreeHeap = ESP.getFreeHeap();

/**
 * Vola se jednou za minutu a vypise stav heapu
 */
void raPrintHeap()
{
    long freeHeap = ESP.getFreeHeap();
    long delta = freeHeap-prevFreeHeap;
    prevFreeHeap = freeHeap;
    Serial.printf( "~ heap: %d, delta %d; PSRAM: %d\n", freeHeap, delta, ESP.getFreePsram() );
}


/*  ******************** WiFi callbacks *******************************************  */


void wifiStatus_Connected(  int status, int waitTime, IPAddress ip, int rssi  )
{
  Serial.print("* wifi [");
  Serial.print( ip );
  Serial.printf("], %d dBm, %d s\n", rssi, waitTime );

  connectMqtt();
}

void wifiStatus_NotConnected( int status, long msecNoConnTime )
{
  char desc[32];
  getWifiStatusText( status, desc );
  Serial.printf("* no wifi (%s), %d s\n", desc, (msecNoConnTime / 1000L) );
  
  if( mqttOK ) {
    mqttOK = false;
    mqtt.disconnect();
  }
}

void wifiStatus_Starting()
{ 
}


/* ************************** vypis pouzitych knihoven **************************** */

/*

Using library WiFi at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.11\libraries\WiFi 
Using library PubSubClient at version 2.8 in folder: C:\Users\brouzda\Documents\Arduino\libraries\PubSubClient 
Using library Tasker at version 2.0.3 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Tasker 
Using library ArduinoJson at version 6.21.4 in folder: C:\Users\brouzda\Documents\Arduino\libraries\ArduinoJson 
Using library Adafruit ST7735 and ST7789 Library at version 1.10.0 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Adafruit_ST7735_and_ST7789_Library 
Using library Adafruit GFX Library at version 1.11.8 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Adafruit_GFX_Library 
Using library Adafruit BusIO at version 1.14.4 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Adafruit_BusIO 
Using library Wire at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.11\libraries\Wire 
Using library SPI at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.11\libraries\SPI 

*/


