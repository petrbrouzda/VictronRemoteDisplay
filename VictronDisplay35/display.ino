

/**
 * All display rendering is done here.
 * Code is prepared for "TENSTAR ROBO 3.5 Inch 320x480" display.
 * 
 * Texts are rendered via canvas, so redraw flickering is minimised.
 * Canvas can't be used for whole screen, as 320x480 16bit canvas is 307 200 byte,
 * and ESP32-C3 I'm using has no PSRAM and has about 220 kB of free RAM.
 * So only a small canvas for three lines of text is used.
 * 
 */


// profile for battery status
HbColorProfile c11( 0.0, EG_RED, EG_RED, EG_WHITE, EG_RED, EG_RED, EG_BLACK );
HbColorProfile c12( 25.0, EG_YELLOW, EG_YELLOW, EG_BLACK, EG_YELLOW, EG_YELLOW, EG_BLACK );
HbColorProfile c13( 75.0, EG_GREEN, EG_GREEN, EG_BLACK, EG_GREEN, EG_GREEN, EG_BLACK );
// musi byt zakoncene NULLem; hodnoty musí být seřazené vzestupně
HbColorProfile *batteryColors[] = { &c11, &c12, &c13, NULL };

// profile for solar panel production
HbColorProfile c21( 0.0, EG_WHITE, EG_WHITE, EG_BLACK, EG_WHITE, EG_WHITE, EG_BLACK );
// musi byt zakoncene NULLem; hodnoty musí být seřazené vzestupně
HbColorProfile *solarColors[] = { &c21, NULL };

// profile for consumption
HbColorProfile c31( 0.0,  EG_WHITE, EG_WHITE, EG_BLACK, EG_WHITE, EG_WHITE, EG_BLACK);
HbColorProfile c32( 0.5 * INVERTER_POWER_W, EG_YELLOW, EG_YELLOW, EG_BLACK, EG_YELLOW, EG_YELLOW, EG_BLACK );
HbColorProfile c33( 0.75 * INVERTER_POWER_W, EG_MAGENTA, EG_MAGENTA, EG_WHITE, EG_MAGENTA, EG_MAGENTA, EG_BLACK );
HbColorProfile c34( INVERTER_POWER_W, EG_RED, EG_RED, EG_WHITE, EG_RED, EG_RED, EG_BLACK );
// musi byt zakoncene NULLem; hodnoty musí být seřazené vzestupně
HbColorProfile *inverterColors[] = { &c31, &c32, &c33, &c34, NULL };

// profile for consumption history
ChColorProfile chc1( 0.0, EG_WHITE );
ChColorProfile chc2( 0.5 * INVERTER_POWER_W, EG_YELLOW );
ChColorProfile chc3( 0.75 * INVERTER_POWER_W, EG_MAGENTA );
ChColorProfile chc4( INVERTER_POWER_W, EG_RED );
// musi byt zakoncene NULLem; hodnoty musí být seřazené vzestupně
ChColorProfile *consColors[] = { &chc1, &chc2, &chc3, &chc4, NULL };


#define BAR_HEIGHT1 28
#define BAR_HEIGHT2 22

#define BAR_X_POS 30
#define BAR_WIDTH (319-BAR_X_POS)

// "accu" section
#define BAR_ACCU_Y 0

// "solar" section
#define BAR_SOLAR_Y 65
#define CHART_SOLAR_Y 138
#define CHART_SOLAR2_Y 210

// "consumption" section
#define BAR_CONS_Y 288
#define CHART_CONS_Y 318

// "info" section 
#define ICO_BLOCK_Y 400

#define CHART_X 60
#define CHART_HEIGHT 50
#define CHART_WIDTH (319-CHART_X-30)

#define CANVAS_WIDTH 320
#define CANVAS_HEIGH 80

#define WARNING_COLOR EG_LIGHT_RED
#define BG_COLOR EG_BLACK

GFXcanvas16 * canvas;

HorizontalBar * hbAccu;
HorizontalBar * hbSolar;
HorizontalBar * hbCons;
SmallChart * bchSolar;
SmallChart * bchSolarPerMin;
SmallChart * bchCons;

void initDisplay() {

  // canvas for smooth redraw of texts 
  canvas = new GFXcanvas16( CANVAS_WIDTH, CANVAS_HEIGH );

  hbAccu = new HorizontalBar( display, painter );
  hbAccu->setRange( 0, 100 );
  hbAccu->setPosition( BAR_X_POS, BAR_ACCU_Y, BAR_WIDTH, BAR_HEIGHT1 );
  hbAccu->setColors( (HbColorProfile**)&batteryColors );
  hbAccu->setFont( &tucnePismo );  

  hbSolar = new HorizontalBar( display, painter );
  hbSolar->setRange( 0, SOLAR_PANELS_POWER_WP );
  hbSolar->setPosition( BAR_X_POS, BAR_SOLAR_Y, BAR_WIDTH, BAR_HEIGHT2 );
  hbSolar->setColors( (HbColorProfile**)&solarColors );
  hbSolar->setFont( &tucnePismo );

  hbCons = new HorizontalBar( display, painter );
  hbCons->setRange( 0, INVERTER_POWER_W );
  hbCons->setPosition( BAR_X_POS, BAR_CONS_Y, BAR_WIDTH, BAR_HEIGHT2 );
  hbCons->setColors( (HbColorProfile**)&inverterColors );
  hbCons->setFont( &tucnePismo );

  bchSolarPerMin = new SmallChart( display );
  bchSolarPerMin->setPosition( CHART_X, CHART_SOLAR_Y, CHART_WIDTH, CHART_HEIGHT );
  bchSolarPerMin->setDatasource( data.minuteProd );
  bchSolarPerMin->setSimpleColors( EG_BLACK, EG_GREEN, EG_WHITE );
  bchSolarPerMin->setOptions( SmallChart::CHART_MODE_BAR | SmallChart::CHART_LEFT_BORDER 
                        | SmallChart::CHART_BOTTOM_BORDER  );


  bchSolar = new SmallChart( display );
  bchSolar->setPosition( CHART_X, CHART_SOLAR2_Y, CHART_WIDTH, CHART_HEIGHT );
  bchSolar->setDatasource( data.dailyYield );
  bchSolar->setSimpleColors( EG_BLACK, EG_GREEN, EG_WHITE );
  bchSolar->setResolution(5,2);
  bchSolar->setOptions( SmallChart::CHART_MODE_BAR | SmallChart::CHART_LEFT_BORDER 
                        | SmallChart::CHART_BOTTOM_BORDER  );

  bchCons = new SmallChart( display );
  bchCons->setPosition( CHART_X, CHART_CONS_Y, CHART_WIDTH, CHART_HEIGHT );
  bchCons->setDatasource( data.minuteCons );
  bchCons->setColors( EG_BLACK, EG_GREEN, consColors );
  bchCons->setOptions( SmallChart::CHART_MODE_BAR | SmallChart::CHART_LEFT_BORDER 
                      | SmallChart::CHART_BOTTOM_BORDER | SmallChart::CHART_COLORS_HBAR );
  
}


void setDirty()
{
  hbAccu->setDirty();
  hbSolar->setDirty();
  hbCons->setDirty();
  bchSolar->setDirty();
  bchSolarPerMin->setDirty();
}


/**
  0 waiting for wifi
  1 waiting for MQTT
  2 reading Victron system ID
  3 regular mode
*/
int appMode = -2;

int prevMode = -1;
bool modeChange = false;
bool someDataAlreadyReceived = false;


/**
 * Which display mode will be used? Fills "appMode" variable based on system conditions.
 */
void computeAppMode()
{
  if( !wifiOK ) {
    appMode = 0;
  } else if( !mqttOK ) {
      if( prevMode==4 && someDataAlreadyReceived ) {
        // MQTT uz bylo spojene a dostali jsme data = jen prekreslime obrazovku
        data.updateTime = millis();
      } else {
        appMode = 1;
      }
  } else if( victronDeviceId[0]==0 ) {
    appMode = 2;
  } else if( data.updateTime==0 ) {
    appMode = 3;
  } else {
    appMode = 4;
  }

  if( prevMode!=appMode ) {
    modeChange = true;
    prevMode=appMode;
    setDirty();
  } else {
    modeChange = false;
  }
}

long lastUpdate = 0;

void printWifiInfo()
{
  char tmp[50];
  sprintf( tmp, "WiFi: %d dBm", WiFi.RSSI() );
  display->setTextColor( EG_WHITE );
  painter->setFont( &vetsiPismo );
  painter->printLabel( TextPainter::ALIGN_CENTER , 160, 80, tmp );
}

/**
Main display function, called from Tasker
*/
void doDisplay() 
{
  computeAppMode();
  if( modeChange ) {
    display->fillScreen(EG_BLACK);
    display->setTextColor( EG_YELLOW );
    painter->setFont( &tucnePismo );

    if( appMode==0 ) {
      // no WiFi
      painter->printLabel( TextPainter::ALIGN_CENTER , 160, 50, (char*)"Čekám na WiFi..." );
    } else if( appMode==1 ) {
      // WiFi connected, no MQTT connection
      painter->printLabel( TextPainter::ALIGN_CENTER , 160, 50, (char*)"Připojuji k FVE..." );
      printWifiInfo();
    } else if( appMode==2 ) {
      // WiFi and MQTT connected, waiting for message with Victron system ID
      painter->printLabel( TextPainter::ALIGN_CENTER , 160, 50, (char*)"Identifikuji FVE..." );
      printWifiInfo();
    } else if( appMode==3 && data.updateTime==0 ) {
      // WiFi and MQTT connected, system ID received, keep-alive message sent, waiting for some data
      painter->printLabel( TextPainter::ALIGN_CENTER , 160, 50, (char*)"Čekám na data..." );
      printWifiInfo();
    }
  
  } 
  
  /*
    regular mode + some data has been received in the past and:
    - new data has been received
    or
    - last statistics has been printed before more than 60 sec
    
    Will be used also when MQTT is not connected
  */
  if( appMode==4 
      && ( data.updateTime!=lastUpdate || millis()-data.msgsStatusTime>60000 ) ) {
    someDataAlreadyReceived = true;
    displayStatus();
    lastUpdate = data.updateTime;
  }
}


/** icon X offset  */
#define ICO_OFFSET 0
/** icon height */
#define ICON_SPACE 30

#define ACCU_TEXT_HEIGHT 24



void addRemainingTime( char * target, float remainingHrs ) {
  if( remainingHrs>3 ) {
    sprintf( target, "%.0f h", remainingHrs );
  } else {
    int hrs = remainingHrs;
    float partHr = remainingHrs - hrs;
    int mi = (partHr * 60.0);
    sprintf( target, "%d:%02d h", hrs, mi );
  }
}


void displayAccu()
{
  char tmp[128];
  int offsetX;

  int yPos = BAR_ACCU_Y + BAR_HEIGHT1 + 3;

  display->fillRect( 0, BAR_ACCU_Y, BAR_X_POS-1, ICON_SPACE, BG_COLOR );

  if( data.accuP > 0 ) {
    // nabijime
    display->setTextColor( EG_GREEN );  
    strcpy( tmp, "0" );
  } else {
    display->setTextColor( EG_YELLOW );  
    strcpy( tmp, "1" );
  }
  painter->setFont( &icons );
  painter->printLabel( TextPainter::ALIGN_LEFT , 0, BAR_ACCU_Y+ICO_OFFSET, tmp );

  sprintf( tmp, "%.0f %%", data.accuUsableSoc() );
  hbAccu->setValue(data.accuUsableSoc(), tmp);
  hbAccu->draw();

  // text will be printed to canvas for smooth redraw
  canvas->fillScreen( BG_COLOR );
  canvas->setTextColor( EG_WHITE );
  painter->setDisplay( canvas );

  if( data.accuP > 0 ) {
    sprintf( tmp, "nabíjí %.0f W ~ máme %.1f kWh", data.accuP, data.accuCapacity() );
    painter->setFont( &velkePismo );
    painter->printLabel( TextPainter::ALIGN_LEFT, 0, 0, tmp );

  } else {

    sprintf( tmp, "vybíjí %.0f W ~ zbývá %.1f", data.accuP, data.accuCapacity() );
    painter->startText( 0, 0 );
    painter->setFont( &velkePismo );    
    offsetX = painter->printText( tmp );
    painter->setFont( &malePismo );
    offsetX = painter->printText( (const char*)"kWh", offsetX );


    if( data.accuP < -20.0 ) {
        painter->setFont( &velkePismo );    
        offsetX = painter->printText( " / ", offsetX );

        if( data.accuRemainTimeHr() < 1.5 ) {
          canvas->setTextColor( WARNING_COLOR );
        } else {
          canvas->setTextColor( EG_WHITE );
        }
        addRemainingTime( tmp, data.accuRemainTimeHr() );
        offsetX = painter->printText( tmp, offsetX );
        canvas->setTextColor( EG_WHITE );
    }

  }

  display->drawRGBBitmap(BAR_X_POS, yPos, canvas->getBuffer(), canvas->width(), ACCU_TEXT_HEIGHT );
  painter->setDisplay( display );
  // end of canvas operation  

  return;
}


#define SOLAR_TEXT_HEIGHT 24
#define SOLAR_TEXT_SPACING 21 

#define TICKSIZE (5+2)

void displaySolar()
{
  char tmp[128];
  int offsetX;

  int yPos = BAR_SOLAR_Y + BAR_HEIGHT2 + 2;

  display->fillRect( 0, BAR_SOLAR_Y, BAR_X_POS-1, ICON_SPACE, BG_COLOR );

  if( data.getSolarP() > 0 ) {
    // sviti
    display->setTextColor( EG_GREEN );  
    strcpy( tmp, "2" );
  } else {
    display->setTextColor( EG_YELLOW );  
    strcpy( tmp, "3" );
  }
  painter->setFont( &icons );
  painter->printLabel( TextPainter::ALIGN_LEFT , 0, BAR_SOLAR_Y+ICO_OFFSET-3, tmp );

  sprintf( tmp, "%.0f W", data.getSolarP() );
  hbSolar->setValue(data.getSolarP(), tmp);
  hbSolar->draw();

  // text will be printed to canvas for smooth redraw
  canvas->fillScreen( BG_COLOR );
  canvas->setTextColor( EG_WHITE );
  painter->setDisplay( canvas );

  sprintf( tmp, "%.0f V ~ dnes %.1f ", data.solarU, data.dailyYield->getItem(0)  );
  painter->startText( 0, 0 );
  painter->setFont( &velkePismo );
  offsetX = painter->printText( tmp );
  painter->setFont( &malePismo );
  offsetX = painter->printText( (const char*)"kWh", offsetX );

  sprintf( tmp, " / max %.0f W", data.getMaxSolarP()  );
  painter->setFont( &velkePismo );
  offsetX = painter->printText( tmp, offsetX );
  
  sprintf( tmp, "%.1f / %.1f / %.1f ", 
                data.dailyYield->getItem(1), data.dailyYield->getItem(2), data.dailyYield->getItem(3) );
  painter->startText( 0, SOLAR_TEXT_SPACING );
  painter->setFont( &velkePismo );
  offsetX = painter->printText( tmp );
  painter->setFont( &malePismo );
  offsetX = painter->printText( (const char*)"kWh", offsetX );

  sprintf( tmp, " ~ měsíc: %.1f ", 
                data.monthYield() );
  painter->setFont( &velkePismo );
  offsetX = painter->printText( tmp, offsetX );
  painter->setFont( &malePismo );
  offsetX = painter->printText( (const char*)"kWh", offsetX );

  display->drawRGBBitmap(BAR_X_POS, yPos, canvas->getBuffer(), canvas->width(), SOLAR_TEXT_SPACING+SOLAR_TEXT_HEIGHT );
  painter->setDisplay( display );
  // end of canvas operation


  display->setTextColor( EG_WHITE );

  if( bchSolarPerMin->willRedraw() ) {

    display->fillRect( 0, CHART_SOLAR_Y, CHART_X-1, CHART_HEIGHT, BG_COLOR );

    // recompute chart limits
    bchSolarPerMin->autoRange( true );
    bchSolarPerMin->draw();

    sprintf( tmp, "%.0f", bchSolarPerMin->getMaxVal() );
    painter->setFont( &velkePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_SOLAR_Y-2, tmp );
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_SOLAR_Y+velkePismo.lineHeight-4, (char*)"W" );

    display->drawFastVLine( CHART_X + 60,  CHART_HEIGHT+CHART_SOLAR_Y, 4, EG_GREEN );
    display->drawFastVLine( CHART_X + 120,  CHART_HEIGHT+CHART_SOLAR_Y, 4, EG_GREEN );
    display->drawFastVLine( CHART_X + 180,  CHART_HEIGHT+CHART_SOLAR_Y, 4, EG_GREEN );
    for( int i = 1; i<16; i++ ) {
      display->drawFastVLine( CHART_X + 15*i,  CHART_HEIGHT+CHART_SOLAR_Y, 2, EG_GREEN );
    }
    
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 60, CHART_HEIGHT+CHART_SOLAR_Y+3, (char*)"1" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 120, CHART_HEIGHT+CHART_SOLAR_Y+3, (char*)"2" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 180, CHART_HEIGHT+CHART_SOLAR_Y+3, (char*)"3" );
    painter->printLabel( TextPainter::ALIGN_LEFT ,    CHART_X + (32*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR_Y+3, (char*)"hod" );
    painter->setFont( &vetsiPismo );
  }

  if( bchSolar->willRedraw() ) {

    display->fillRect( 0, CHART_SOLAR2_Y, CHART_X-1, CHART_HEIGHT, BG_COLOR );

    // recompute chart limits
    bchSolar->autoRange( true );
    bchSolar->draw();

    sprintf( tmp, "%.1f", bchSolar->getMaxVal() );
    painter->setFont( &velkePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_SOLAR2_Y-2, tmp );
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_SOLAR2_Y+velkePismo.lineHeight-4, (char*)"kWh" );
    
    for( int i = 1; i<31; i++ ) {
      int size = 2;
      if( i!=0 && (i%7 == 0) ) { size = 4; }
      display->drawFastVLine( CHART_X + (i*TICKSIZE),  CHART_HEIGHT+CHART_SOLAR2_Y, size, EG_GREEN );
    }
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + (8*TICKSIZE)  - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR2_Y+3, (char*)"7" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + (15*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR2_Y+3, (char*)"14" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + (22*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR2_Y+3, (char*)"21" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + (29*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR2_Y+3, (char*)"28" );
    painter->printLabel( TextPainter::ALIGN_LEFT ,    CHART_X + (32*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_SOLAR2_Y+3, (char*)"dní" );
    painter->setFont( &vetsiPismo );
  }

  return;
}


#define CONS_BLOCK_HEIGHT 25

void displayConsumer()
{
  char tmp[128];

  display->fillRect( 0, BAR_CONS_Y, BAR_X_POS-1, ICON_SPACE, BG_COLOR );

  display->setTextColor( EG_GREEN );  
  painter->setFont( &icons );
  painter->printLabel( TextPainter::ALIGN_LEFT , 0, BAR_CONS_Y+ICO_OFFSET-3, (char*)"4" );

  painter->setFont( &vetsiPismo );

  sprintf( tmp, "%.0f W", data.getConsP() );
  hbCons->setValue(data.getConsP(), tmp);
  hbCons->draw();

  if( bchCons->willRedraw() ) {

    display->fillRect( 0, CHART_CONS_Y, CHART_X-1, CHART_HEIGHT, BG_COLOR );

    // recompute chart limits
    bchCons->autoRange( true );
    bchCons->draw();

    sprintf( tmp, "%.0f", bchCons->getMaxVal() );
    painter->setFont( &velkePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_CONS_Y-2, tmp );
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_RIGHT , CHART_X-3, CHART_CONS_Y+velkePismo.lineHeight-4, (char*)"W" );
    

    display->drawFastVLine( CHART_X + 60,  CHART_HEIGHT+CHART_CONS_Y, 4, EG_GREEN );
    display->drawFastVLine( CHART_X + 120,  CHART_HEIGHT+CHART_CONS_Y, 4, EG_GREEN );
    display->drawFastVLine( CHART_X + 180,  CHART_HEIGHT+CHART_CONS_Y, 4, EG_GREEN );
    for( int i = 1; i<16; i++ ) {
      display->drawFastVLine( CHART_X + 15*i,  CHART_HEIGHT+CHART_CONS_Y, 2, EG_GREEN );
    }
    painter->setFont( &malePismo );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 60, CHART_HEIGHT+CHART_CONS_Y+3, (char*)"1" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 120, CHART_HEIGHT+CHART_CONS_Y+3, (char*)"2" );
    painter->printLabel( TextPainter::ALIGN_CENTER ,  CHART_X + 180, CHART_HEIGHT+CHART_CONS_Y+3, (char*)"3" );
    painter->printLabel( TextPainter::ALIGN_LEFT ,    CHART_X + (32*TICKSIZE) - (TICKSIZE/2), CHART_HEIGHT+CHART_CONS_Y+3, (char*)"hod" );
    painter->setFont( &vetsiPismo );
  }

  return;
}


#define ICO_POS_X 0
#define ICO_SPACING 8
#define ICO_TEXT_SPACING 21


void addUptime( char * target, int minutes  )
{
  if( minutes < 60 ) {
    sprintf( target, "%d m", minutes );
  } else if( minutes < 600 ) {
    int mi = minutes % 60;
    int hr = minutes / 60;
    sprintf( target, "%d:%02d h", hr, mi );
  } else {
    int hr = minutes / 60;
    sprintf( target, "%d h", hr );
  }
}


void displayIco()
{
  char tmp[128];

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


  // text will be printed to canvas for smooth redraw
  canvas->fillScreen( BG_COLOR );
  painter->setDisplay( canvas );
  painter->setFont( &velkePismo );


  int xOffset = 0;

  painter->startText( ICO_POS_X, 0  );
  sprintf( tmp, "Teplota baterie %d °C,", data.temperature );
  if( data.temperature > BATTERY_TEMP_LIMIT ) {
    canvas->setTextColor( WARNING_COLOR );
  } else {
    canvas->setTextColor( EG_WHITE );
  }
  xOffset = painter->printText( tmp );
  sprintf( tmp, " okolí %d °C", data.temperatureEnv );
  canvas->setTextColor( EG_WHITE );
  xOffset = painter->printText( tmp, xOffset );


  painter->startText( ICO_POS_X, ICO_TEXT_SPACING );
  sprintf( tmp, "WiFi: %d dBm ~ %d/m ~ uptime: ", WiFi.RSSI(), throughput  );
  addUptime( tmp+strlen(tmp), millis()/1000/60 );
  canvas->setTextColor( EG_WHITE );
  xOffset = painter->printText( tmp );


  if( !mqttOK ) {
    painter->setFont( &malePismo );
    xOffset = painter->printText( (char*)" ~ ", xOffset );
    canvas->setTextColor( WARNING_COLOR );
    painter->printText( (char*)"!mqtt", xOffset );
    painter->setFont( &velkePismo );
  }


  xOffset = 0;
  painter->startText( ICO_POS_X, ICO_TEXT_SPACING + ICO_TEXT_SPACING );

  if( data.ledLowBattery ) {
    canvas->setTextColor( WARNING_COLOR );
    xOffset = ICO_SPACING + painter->printText( (char*)"Slabá baterie!", xOffset );
  }

  if( data.ledOverload ) {
    canvas->setTextColor( WARNING_COLOR );
    xOffset = ICO_SPACING + painter->printText( (char*)"Přetížení!", xOffset );
  }

  if( data.ledTemperature ) {
    canvas->setTextColor( WARNING_COLOR );
    xOffset = ICO_SPACING + painter->printText( (char*)"Přehřátí!", xOffset );
  }

  if( ! data.ledInverter ) {
    canvas->setTextColor( WARNING_COLOR );
    xOffset = ICO_SPACING + painter->printText( (char*)"Střídač:vyp", xOffset );
  }

  if( xOffset==0 ) {
    if( data.ledInverter ) {
      sprintf( tmp, "Střídač:zap" );
      canvas->setTextColor( EG_GREEN );
      xOffset = ICO_SPACING + painter->printText( tmp, xOffset );
    }  

    if( data.ledAbsorption ) {
      sprintf( tmp, "Accu:abs" );
      canvas->setTextColor( EG_WHITE );
      xOffset = ICO_SPACING + painter->printText( tmp, xOffset );
    }

    if( data.ledBulk ) {
      sprintf( tmp, "Accu:blk" );
      canvas->setTextColor( EG_WHITE );
      xOffset = ICO_SPACING + painter->printText( tmp, xOffset );
    }

    if( data.ledFloat ) {
      sprintf( tmp, "Accu:flt" );
      canvas->setTextColor( EG_WHITE );
      xOffset = ICO_SPACING + painter->printText( tmp, xOffset );
    }
  }

  display->drawRGBBitmap(0, ICO_BLOCK_Y, canvas->getBuffer(), canvas->width(), ICO_TEXT_SPACING+ICO_TEXT_SPACING+SOLAR_TEXT_HEIGHT  );
  painter->setDisplay( display );
  // end of canvas operation

  return ;
}


/**
Display for regular run mode  - ie. when everything works
*/
void displayStatus()
{
  displayAccu();
  displaySolar();
  displayConsumer();
  displayIco();
}





