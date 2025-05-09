#include <Arduino.h>
#include <WiFi.h>

#include "DeviceInfo.h"

void formatMemorySize( char * ptr, long msize )
{
  float out;
  char * jednotka;
  if( msize > 1048576 ) {
    out = ((float)msize) / 1048576.0;
    jednotka = (char*)"MB";
  } else if( msize > 1024 ) {
    out = ((float)msize) / 1024.0;
    jednotka = (char*)"kB";
  } else if( msize > 0 ) {
    out = ((float)msize);
    jednotka = (char*)"B";
  } else {
    out = 0;
  }

  if( out>0 ) {
    sprintf( ptr, "%.1f %s", out, jednotka );
  } else {
    strcpy( ptr, "--" );
  }
}


#ifdef ESP32

  #include <ESP.h>
  
  #if ESP_ARDUINO_VERSION_MAJOR == 2 
    extern "C" {
      #include <esp32/spiram.h>
      #include <esp32/himem.h>
    }
  #else
    extern "C" {
      #include <esp_spiram.h>
      #include <esp_himem.h>
    }
  #endif
  
  
  #define LOW_PSRAM_ONLY
  
  void formatDeviceInfo( char * out )
  {
    strcpy( out, "RAM " );
    formatMemorySize( out+strlen(out), ESP.getHeapSize() );
  
    #ifdef LOW_PSRAM_ONLY
    
      strcat( out, "; PSRAM " );
      formatMemorySize( out+strlen(out), ESP.getPsramSize() );
    
    #else
    
      long psramtotal = esp_spiram_get_size();
      strcat( out, "; PSRAM [" );
      formatMemorySize( out+strlen(out), psramtotal );
      if( psramtotal>0 ) {
        strcat( out, ": low: " );
        formatMemorySize( out+strlen(out), ESP.getPsramSize() );
        strcat( out, ", high: " );
        formatMemorySize( out+strlen(out), esp_himem_get_phys_size() );
      }
      strcat( out, "]" );
      
    #endif
  
    sprintf( out+strlen(out), "; %dx %s %d MHz; flash %d MHz", 
            ESP.getChipCores(),
#if defined(CONFIG_IDF_TARGET_ESP32C3)
            "ESP32-C3",
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
            "ESP32-S2",
#elif defined(CONFIG_IDF_TARGET_ESP32S3)            
            "ESP32-S3",
#else
            "ESP32",
#endif            
            ESP.getCpuFreqMHz(), 
            (ESP.getFlashChipSpeed()/1000000)
           );
  }


#endif


/**
 * Vraci preklad wifi statusu na text.
 * Pole 'desc' musi mit alespon 32 byte
 */
const char * getWifiStatusText( int status )
{
  switch ( status ) {
    case WL_IDLE_STATUS:      return "IDLE";                 // ESP32,  = 0
    case WL_NO_SSID_AVAIL:    return "NO_SSID";             // WL_NO_SSID_AVAIL    = 1,
    case WL_SCAN_COMPLETED:   return "SCAN_COMPL";          // WL_SCAN_COMPLETED   = 2,
    case WL_CONNECT_FAILED:   return "CONN_FAIL";           // WL_CONNECT_FAILED   = 4,
    case WL_CONNECTION_LOST:  return "CONN_LOST";           // WL_CONNECTION_LOST  = 5,
    case WL_DISCONNECTED:     return "DISC";                // WL_DISCONNECTED     = 6
    case WL_NO_SHIELD:        return "NO_SHIELD";           // ESP32, = 255
    case WIFI_POWER_OFF:      return "OFF";
    default: return "UNKNOWN";
  }
}
