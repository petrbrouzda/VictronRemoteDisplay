#include "toolkit.h"

/**
  0 waiting for wifi
  1 waiting for MQTT
  2 reading Victron system ID
  3 regular mode
*/
int appMode = -2;
int prevMode = -1;
bool modeChange = false;

void computeAppMode()
{
  if( !wifiOK ) {
    appMode = 0;
  } else if( !mqttOK ) {
    appMode = 1;
  } else if( victronDeviceId[0]==0 ) {
    appMode = 2;
  } else {
    appMode = 3;
  }

  if( prevMode!=appMode ) {
    modeChange = true;
    prevMode=appMode;
  } else {
    modeChange = false;
  }
}

long lastUpdate = 0;


/**
Main display function, called from Tasker
*/
void doDisplay() 
{
  computeAppMode();
  if( modeChange ) {
    display->fillScreen(ST77XX_BLACK);
    
    extdisplay->setPos( 0, 20 );
    extdisplay->setFont( fnt_text1(), -8, 0 );
    display->setTextColor( ST77XX_YELLOW );
    if( appMode==0 ) {
      extdisplay->printUTF8( "Čekám na WiFi..." );
    } else if( appMode==1 ) {
      extdisplay->printUTF8( "Připojuji k FVE..." );
    } else if( appMode==2 ) {
      extdisplay->printUTF8( "Identifikuji FVE..." );
    } else if( appMode==3 && data.updateTime==0 ) {
      extdisplay->printUTF8( "Čekám na data..." );
    }
  
  } 
  
  /*
    regular mode and some data has been received and:
    - new data has been received
    or
    - last statistics has been printed before more than 60 sec
  */
  if( appMode==3 && data.updateTime!=0 && 
      ( data.updateTime!=lastUpdate || millis()-data.msgsStatusTime>60000 ) ) {
    displayStatus();
    lastUpdate = data.updateTime;
  }
}

#define BG_COLOR ST77XX_BLACK

#define BAR_HEIGHT 16
#define BAR_WIDTH (159-BAR_X_POS)
#define BAR_X_POS 20
#define ICO_OFFSET 15

#define ACCU_TEXT_OFFSET 29
#define ACCU_BLOCK_HEIGHT 35


int  displayAccu( int yPos )
{
  char tmp[128];

  display->fillRect( 0, yPos, extdisplay->width, ACCU_BLOCK_HEIGHT, BG_COLOR );

  if( data.accuP > 0 ) {
    // nabijime
    display->setTextColor( ST77XX_GREEN );  
    strcpy( tmp, "0" );
  } else {
    display->setTextColor( ST77XX_YELLOW );  
    strcpy( tmp, "1" );
  }
  extdisplay->setPos( 0, yPos+ICO_OFFSET );
  extdisplay->setFont( fnt_icons(), -8, 0 );
  extdisplay->printUTF8( tmp );

  extdisplay->setFont( fnt_text1(), -8, 0 );

  uint16_t colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg;

  double val = data.accuUsableSoc();
  sprintf( tmp, "%.0f %%", val );
  if( val < 25.0 ) {
    colorBar = ST77XX_RED;
    colorBorder = ST77XX_RED;
    colorTextOnBar = ST77XX_RED;
    colorTextOnBg = ST77XX_RED;
    colorBgOnBar = ST77XX_BLACK;
    colorBgOnBg = ST77XX_BLACK;
  } else if ( val < 50.0 ) {
    colorBar = ST77XX_YELLOW;
    colorBorder = ST77XX_RED;
    colorTextOnBar = ST77XX_BLACK;
    colorTextOnBg = ST77XX_YELLOW;
    colorBgOnBar = ST77XX_YELLOW;
    colorBgOnBg = ST77XX_BLACK;
  } else {
    colorBar = ST77XX_GREEN;
    colorBorder = ST77XX_GREEN;
    colorTextOnBar = ST77XX_BLACK;
    colorTextOnBg = ST77XX_GREEN;
    colorBgOnBar = ST77XX_GREEN;
    colorBgOnBg = ST77XX_BLACK;
  }
  extdisplay->drawBar( BAR_X_POS, yPos, 
            BAR_WIDTH, BAR_HEIGHT, 
            val, tmp, 
            colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg, BG_COLOR );

  if( data.accuP > 0 ) {
    sprintf( tmp, "%.0f W  %.1f kWh", data.accuP, data.accuCapacity() );
  } else {
    sprintf( tmp, "%.0f W  %.1f kWh  %.1f h", data.accuP, data.accuCapacity(), data.accuRemainTimeHr() );
  }
  extdisplay->setPos( BAR_X_POS, yPos+ACCU_TEXT_OFFSET );
  extdisplay->setFont( fnt_text1(), -8, 0 );
  display->setTextColor( ST77XX_WHITE );
  extdisplay->printUTF8( tmp );

  return yPos + ACCU_BLOCK_HEIGHT + 1;
}


#define SOLAR_TEXT_OFFSET 29
#define SOLAR_TEXT_OFFSET2 42
#define SOLAR_BLOCK_HEIGHT 48

int  displaySolar( int yPos )
{
  char tmp[128];

  display->fillRect( 0, yPos, extdisplay->width, SOLAR_BLOCK_HEIGHT, BG_COLOR );

  if( data.getSolarP() > 0 ) {
    // sviti
    display->setTextColor( ST77XX_GREEN );  
    strcpy( tmp, "2" );
  } else {
    display->setTextColor( ST77XX_YELLOW );  
    strcpy( tmp, "3" );
  }
  extdisplay->setPos( 0, yPos+ICO_OFFSET );
  extdisplay->setFont( fnt_icons(), -8, 0 );
  extdisplay->printUTF8( tmp );

  extdisplay->setFont( fnt_text1(), -8, 0 );

  uint16_t colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg;

  double val = data.getSolarP() / SOLAR_PANELS_POWER_WP * 100.0;
  sprintf( tmp, "%.0f W", data.getSolarP() );
  colorBar = ST77XX_WHITE;
  colorBorder = ST77XX_WHITE;
  colorTextOnBar = ST77XX_BLACK;
  colorTextOnBg = ST77XX_WHITE;
  colorBgOnBar = ST77XX_WHITE;
  colorBgOnBg = ST77XX_BLACK;
  extdisplay->drawBar( BAR_X_POS, yPos, 
            BAR_WIDTH, BAR_HEIGHT, 
            val, tmp, 
            colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg, BG_COLOR );

  
  sprintf( tmp, "%.0f V  %.1f kWh  %.0f W", data.solarU, data.day0.solarYield, data.day0.solarMaxPower );
  extdisplay->setPos( BAR_X_POS, yPos+SOLAR_TEXT_OFFSET );
  extdisplay->setFont( fnt_text1(), -8, 0 );
  display->setTextColor( ST77XX_WHITE );
  extdisplay->printUTF8( tmp );

  sprintf( tmp, "(%.1f / %.1f / %.1f kWh)", data.day1.solarYield, data.day2.solarYield, data.day3.solarYield );
  extdisplay->setPos( BAR_X_POS, yPos+SOLAR_TEXT_OFFSET2 );
  extdisplay->setFont( fnt_text1(), -8, 0 );
  display->setTextColor( ST77XX_WHITE );
  extdisplay->printUTF8( tmp );

  return yPos + SOLAR_BLOCK_HEIGHT + 1;
}


#define CONS_BLOCK_HEIGHT 25

int  displayConsumer( int yPos )
{
  char tmp[128];

  display->fillRect( 0, yPos, extdisplay->width, CONS_BLOCK_HEIGHT, BG_COLOR );

  display->setTextColor( ST77XX_GREEN );  
  strcpy( tmp, "4" );
  extdisplay->setPos( 0, yPos+ICO_OFFSET );
  extdisplay->setFont( fnt_icons(), -8, 0 );
  extdisplay->printUTF8( tmp );

  extdisplay->setFont( fnt_text1(), -8, 0 );

  uint16_t colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg;

  double val = data.consP / INVERTER_POWER_W * 100.0;
  sprintf( tmp, "%.0f W", data.consP );
  if( data.consP > 0.9 * INVERTER_POWER_W ) {
    colorBar = ST77XX_RED;
    colorBorder = ST77XX_RED;
    colorTextOnBar = ST77XX_WHITE;
    colorTextOnBg = ST77XX_RED;
    colorBgOnBar = ST77XX_RED;
    colorBgOnBg = ST77XX_BLACK;
  } else if( data.consP > 0.6 * INVERTER_POWER_W ) {
    colorBar = ST77XX_YELLOW;
    colorBorder = ST77XX_YELLOW;
    colorTextOnBar = ST77XX_BLACK;
    colorTextOnBg = ST77XX_YELLOW;
    colorBgOnBar = ST77XX_YELLOW;
    colorBgOnBg = ST77XX_BLACK;
  } else {
    colorBar = ST77XX_WHITE;
    colorBorder = ST77XX_WHITE;
    colorTextOnBar = ST77XX_BLACK;
    colorTextOnBg = ST77XX_WHITE;
    colorBgOnBar = ST77XX_WHITE;
    colorBgOnBg = ST77XX_BLACK;
  }
  extdisplay->drawBar( BAR_X_POS, yPos, 
            BAR_WIDTH, BAR_HEIGHT, 
            val, tmp,  
            colorBar, colorBorder, colorTextOnBar, colorTextOnBg, colorBgOnBar, colorBgOnBg, BG_COLOR );

  return yPos + CONS_BLOCK_HEIGHT + 1;
}

#define ICO_BLOCK_HEIGHT 20
#define ICO_TEXT_OFFSET 18
#define ICO_POS_X 0
#define ICO_SPACING 8

int  displayIco( int yPos )
{
  char tmp[128];

  display->fillRect( 0, yPos, extdisplay->width, CONS_BLOCK_HEIGHT, BG_COLOR );

  extdisplay->setFont( fnt_text1(), -8, 0 );
  int xOffset = 0;

  extdisplay->setPos( ICO_POS_X, yPos+ICO_TEXT_OFFSET );

  if( data.ledLowBattery ) {
    sprintf( tmp, "LowBat" );
    display->setTextColor( ST77XX_RED );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  if( data.ledOverload ) {
    sprintf( tmp, "OvrLd" );
    display->setTextColor( ST77XX_RED );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  if( data.ledTemperature ) {
    sprintf( tmp, "Temp" );
    display->setTextColor( ST77XX_RED );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  if( data.ledInverter ) {
    sprintf( tmp, "inv" );
    display->setTextColor( ST77XX_GREEN );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }  

  if( data.ledAbsorption ) {
    sprintf( tmp, "abs" );
    display->setTextColor( ST77XX_WHITE );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  if( data.ledBulk ) {
    sprintf( tmp, "blk" );
    display->setTextColor( ST77XX_WHITE );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  if( data.ledFloat ) {
    sprintf( tmp, "flt" );
    display->setTextColor( ST77XX_WHITE );
    xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  }

  // msg throughput
  long throughput = 0;
  long t = millis() - data.msgsStatusTime;
  if( t<60000 ) {
    // jeste neni cela minuta
    if( data.prevThoughput == -1 ) {
      // jeste neni prvni minuta
      throughput = data.msgs;
    } else {
      // uz nejake statistiky mame
      throughput = data.prevThoughput;
    }
  } else {
    throughput = data.msgs;
    data.prevThoughput = throughput;
    data.msgsStatusTime = millis();
    data.msgs = 0;
  }
  sprintf( tmp, "%d/m", throughput );
  display->setTextColor( ST77XX_WHITE );
  xOffset = ICO_SPACING + extdisplay->printUTF8( tmp, xOffset );
  

  return yPos + ICO_BLOCK_HEIGHT + 1;
}


/**
Display for regular run mode  - ie. when everything works
*/
void displayStatus()
{
  // display->fillScreen(ST77XX_BLACK);
  int posY = 0;
  posY = displayAccu( posY );
  posY = displaySolar( posY );
  posY = displayConsumer( posY );
  displayIco( extdisplay->height - ICO_BLOCK_HEIGHT - 1 );
}





