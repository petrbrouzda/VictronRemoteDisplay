// FQBN: esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashMode=dio

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

#include "src/toolkit/DeviceInfo.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7796S.h>

#include "fonts/PragatiNarrow-Regular16pt8b.h"
#include "fonts/PragatiNarrow-Regular20pt8b.h"
#include "fonts/PragatiNarrow-Regular22pt8b.h"
#include "fonts/PragatiNarrow-Bold22pt8b.h"
#include "fonts/icons.h"

#include "src/extgfx/TextPainter.h"
#include "src/extgfx/HorizontalBar.h"
#include "src/extgfx/SmallChart.h"
#include "src/extgfx/BasicColors.h"

#include "src/toolkit/map_double.h"

#ifdef USE_BRIGHTNESS_SENSOR
  #include <ESP32AnalogRead.h>
  ESP32AnalogRead adc;
#endif

Adafruit_ST7796S *display;
 

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

TextPainter * painter;

TpFontConfig malePismo;
TpFontConfig vetsiPismo;
TpFontConfig velkePismo;
TpFontConfig tucnePismo;
TpFontConfig icons;

void blinkLed()
{
  pinMode( LED, OUTPUT );
  digitalWrite( LED, LOW );
  delay(500);
  digitalWrite( LED, HIGH );
  delay(500);
  digitalWrite( LED, LOW );
  delay(500);
  digitalWrite( LED, HIGH );
  delay(500);
}

/**
It is good to know what application and which version is used in device.
*/
void printStartupInfo() 
{
  Serial.printf( "%s, %s %s\n", __FILE__, __DATE__, __TIME__ );
  Serial.println( EXTGFX_VERSION );

  char tmp[200];
  formatDeviceInfo( tmp );
  Serial.println( tmp );
}

void setup() {
  
  pinMode( LED_UNUSED, OUTPUT );
  digitalWrite( LED_UNUSED, LOW );

  victronDeviceId[0] = 0;
  json = new DynamicJsonDocument(2000);
    
  Serial.begin(115200);
  blinkLed();
  printStartupInfo();

  #ifdef USE_BRIGHTNESS_SENSOR
    adc.attach(BRIGHTNESS_SENSOR_PIN);
  #endif

  // aby se objekt vytvoril vcas
  data.accuP=0;

   // ++++ zde si nastavte svuj displej

    pinMode( TFT_RST, OUTPUT );
    digitalWrite( TFT_RST, RST_VALUE );
    pinMode( TFT_BACKLIGHT, OUTPUT );
    digitalWrite( TFT_BACKLIGHT, TFT_BACKLIGHT_ON );
    
    // nastavení pro TENSTAR ROBO 3.5 Inch 320x480
    
    SPI.end(); // release standard SPI pins
    SPI.begin(TFT_SCLK, SPI_MISO_UNUSED, TFT_MOSI, TFT_CS); // map and init SPI pins
    display = new Adafruit_ST7796S(TFT_CS, TFT_DC, TFT_RST);

    display->init(320, 480, 0, 0, ST7796S_BGR );
    display->invertDisplay(true);
    display->setRotation(2);
    display->fillScreen(ST77XX_BLACK);

  // ----- nastaveni displeje

  painter = new TextPainter( display );
  painter->createFontConfig( &malePismo, &PragatiNarrow_Regular16pt8b );
  painter->createFontConfig( &vetsiPismo, &PragatiNarrow_Regular20pt8b );
  painter->createFontConfig( &velkePismo, &PragatiNarrow_Regular22pt8b );
  painter->createFontConfig( &tucnePismo, &PragatiNarrow_Bold22pt8b );
  painter->createFontConfig( &icons, &Icons );
  
  painter->setFont( &tucnePismo );
  display->setTextColor( EG_YELLOW );
  painter->printLabel( TextPainter::ALIGN_CENTER , 160, 50, (char*)"Spouštím..." );

  startWifi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqqtCallback);
  // victron messages are larger than default
  mqtt.setBufferSize(1024);

  initDisplay();

  tasker.setInterval( raPrintHeap, 60000 );
  tasker.setInterval( doDisplay, 2500 );
  tasker.setInterval( minuteData, 60000 );

  #ifdef USE_BRIGHTNESS_SENSOR
    tasker.setInterval( setBrightness, 5000 );
  #endif

}

void minuteData()
{
  data.minuteCons->put( data.minuteMaxConsP );
  data.minuteProd->put( data.getSolarP() );
  data.clearMinuteMaxConsP();
}

#ifdef USE_BRIGHTNESS_SENSOR
  void setBrightness() {
    float v = adc.readVoltage();
    float svetlo = map_double( v, SENSOR_U_MIN, SENSOR_U_MAX, BL_MAX, BL_MIN );
    analogWrite( TFT_BACKLIGHT, (int)svetlo );
    Serial.printf( "adc = %.2f V -> backlight %.0f = %.0f %% \n", v, svetlo, (svetlo/2.55) );
  }
#endif

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
  // sprintf( tmp, "N/%s/solarcharger/#", victronDeviceId );
  sprintf( tmp, "N/%s/solarcharger/+/Pv/V", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/solarcharger/+/History/Daily/+/Yield", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/solarcharger/+/History/Daily/0/MaxPower", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Ac/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Dc/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/system/+/Batteries/#", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/battery/+/Dc/0/Temperature", victronDeviceId );
  mqtt.subscribe( tmp );
  sprintf( tmp, "N/%s/temperature/+/Temperature", victronDeviceId );
  mqtt.subscribe( tmp );

  // alternative: N/%s/system/+/Dc/Pv/Power
  sprintf( tmp, "N/%s/solarcharger/+/Yield/Power", victronDeviceId );
  mqtt.subscribe( tmp );
}


/**
Called when message is received 
*/
void mqqtCallback(char *topic, byte *payload, unsigned int length) {
  data.msgs++;

  digitalWrite( LED, HIGH );

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
      data.setConsP(v);
      data.update(); 
    }
  } else if( checkTopicName("N/+/solarcharger/+/Pv/V", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.solarU = v;
      data.update(); 
    }
  } else if( checkTopicName("N/+/solarcharger/+/Yield/Power", topic) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.solarP = v;
      data.update(); 
    }
  } else if( checkTopicName("N/+/vebus/+/Leds/Absorption", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledAbsorption = v; } 
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Bulk", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledBulk  = v; }
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Float", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledFloat  = v; }
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Inverter", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledInverter  = v; }
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/LowBattery", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledLowBattery  = v; }
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Overload", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledOverload  = v; }
    data.update(); 
  }  else if( checkTopicName("N/+/vebus/+/Leds/Temperature", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.ledTemperature  = v; }
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
      if( (*json)["value"]!=NULL && (*json)["value"][0]!=NULL ) {
        double v = (*json)["value"][0]["soc"];
        if( v!=NAN) { data.accuSoc = v; }
        v = (*json)["value"][0]["voltage"];
        if( v!=NAN) { data.accuU = v; }
        v = (*json)["value"][0]["current"];
        if( v!=NAN) { data.accuI = v; }
        data.update(); 
      } 
    } else {
      Serial.printf( "JSON error: %s\n", de.c_str() );
    }
    
  } else if( checkTopicName("N/+/temperature/+/Temperature", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.temperatureEnv=v; } 
    data.update(); 
  } else if( checkTopicName("N/+/battery/+/Dc/0/Temperature", topic ) ) {
    int v = getIntValue(payload);
    if( v!=-999 ) { data.temperature=v; } 
    data.update(); 
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/+/Yield", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      // N/c0619ab33b2f/solarcharger/280/History/Daily/1/Yield
      #define NEEDLE "/History/Daily/"
      #define NEEDLE_LEN 15
      char * p = strstr( topic, NEEDLE );
      int dayNr = atoi( p+NEEDLE_LEN );
      data.dailyYield->setItem( dayNr, v );
      // Serial.printf( "  dailyYield[%d] = %.1f\n", dayNr, v );
    }
  } else if( checkTopicName("N/+/solarcharger/+/History/Daily/0/MaxPower", topic ) ) {
    double v = getDoubleValue(payload);
    if( v!=NAN ) {
      data.day0solarMaxPower = v;
    }
  }

  digitalWrite( LED, LOW );
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
    return -999;
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
  Serial.printf("* no wifi (%s), %d s\n", getWifiStatusText(status), (msecNoConnTime / 1000L) );
  
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
Using library WiFi at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.17\libraries\WiFi 
Using library PubSubClient at version 2.8 in folder: E:\dev.moje\arduino\libraries\PubSubClient 
Using library Tasker at version 2.0.3 in folder: E:\dev.moje\arduino\libraries\Tasker 
Using library ArduinoJson at version 6.21.5 in folder: E:\dev.moje\arduino\libraries\ArduinoJson 
Using library Adafruit GFX Library at version 1.12.0 in folder: E:\dev.moje\arduino\libraries\Adafruit_GFX_Library 
Using library Adafruit BusIO at version 1.17.0 in folder: E:\dev.moje\arduino\libraries\Adafruit_BusIO 
Using library Wire at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.17\libraries\Wire 
Using library SPI at version 2.0.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.17\libraries\SPI 
Using library Adafruit ST7735 and ST7789 Library at version 1.11.0 in folder: E:\dev.moje\arduino\libraries\Adafruit_ST7735_and_ST7789_Library 
Using library ESP32AnalogRead at version 0.3.0 in folder: E:\dev.moje\arduino\libraries\ESP32AnalogRead 
*/


