/**
Toolkit for WiFi.
Reports WiFi status to application via callbacks.
Handles WiFi start/stop in more robust way (can be started/stopped many times).
*/


long lastWifiStatusChangeMsec, lastConnected;
int wifiLastStatus;
bool wifiPoweredOn = false;


void startWifi()
{
    if( wifiPoweredOn ) return;

    wifiPoweredOn = true;
    lastConnected = millis();

    Serial.printf("* wifi connecting [%s]\n", WIFI_SSID );
    // WiFi.persistent(false);
    WiFi.softAPdisconnect(true);  // https://stackoverflow.com/questions/39688410/how-to-switch-to-normal-wifi-mode-to-access-point-mode-esp8266
                                  // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html#softapdisconnect

    WiFi.disconnect(); 
    WiFi.mode(WIFI_STA); // sezere 50 kB RAM
    WiFi.disconnect(); 
    WiFi.setAutoReconnect(true);
    WiFi.begin( WIFI_SSID, WIFI_PASSWORD );

    // fix for https://github.com/espressif/arduino-esp32/issues/6767
    // WiFi.setTxPower( WIFI_POWER_8_5dBm );

    lastWifiStatusChangeMsec = millis();

    wifiLastStatus = WL_DISCONNECTED;

    wifiStatus_Starting();
}

void stopWifi()
{
    if( ! wifiPoweredOn ) return;
    
    wifiPoweredOn = false;
    wifiOK = false;

    // pro ESP32-C3 se NESMI zavolat vypnuti WiFi, protoze se pak uz nevzbudi
    WiFi.softAPdisconnect(true);
    WiFi.mode( WIFI_OFF );

    lastWifiStatusChangeMsec = millis();
}

bool checkWifiStatus()
{
  int newStatus;
  if( wifiPoweredOn ) {
      newStatus = WiFi.status();
  } else {
      newStatus = WIFI_POWER_OFF;
  }

  // aby nam to pri spousteni neslo pres dva stavy
  if( newStatus == WL_IDLE_STATUS ) {
    newStatus = WL_DISCONNECTED;
  }

  if( wifiLastStatus!=newStatus )
  {
    if( newStatus == WL_CONNECTED) {
      wifiOK = true;
      wifiStatus_Connected( newStatus, ((millis()-lastConnected)/1000L), WiFi.localIP(), WiFi.RSSI() );
    } else {
      wifiOK = false;
      if( wifiLastStatus==WL_CONNECTED ) {
        lastConnected = millis();
      }
      wifiStatus_NotConnected( newStatus, millis()-lastConnected );
    }
    
    wifiLastStatus=newStatus;
  }

  if( newStatus!=WL_CONNECTED && newStatus!=WIFI_POWER_OFF)
  {
    long notConn = (millis()-lastConnected)/1000L;
    if( notConn>WIFI_NOCONN_RESTART_SEC ) {
      // restartovat wifi, kdyz se strasne dlouho nepripojilo
      Serial.printf("* wifi not connected for %d s, restarting for [%s]\n", notConn, WIFI_SSID );

      stopWifi();
      startWifi();
     
      lastConnected = millis();
    }
  }

  return (newStatus == WL_CONNECTED);
}
