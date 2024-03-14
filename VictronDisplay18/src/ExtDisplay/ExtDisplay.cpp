/**
 * Extension pro Adafruit GFX>
 * - tisk UTF-8cestiny
 * - skoro korektni zalamovani slov v cestine
 * - tisk baru (procentualni indikator)
 */

#include "ExtDisplay.h"

#include <stdlib.h>
#include <string.h>

#include "../gfxlatin2/gfxlatin2.h"

char samohlasky[] = "eyuioaěýáíéúůEYUIOAĚÝÁÍÉÚŮ";

bool jeSamohlaska( char c ) {
  return strchr(samohlasky,c)!=NULL;
}


/** pro tisk textu: nastavi maximalni sirku vykreslovaciho obdelniku pro aktualni pozici X */
void ExtDisplay::setBbFullWidth() {
    this->boundingBoxWidth = this->width - this->posX - 1;
}

/** pro tisk textu: nastavi sirku vykreslovaciho obdelniku pro aktualni pozici X a urceny pravy okraj */
void ExtDisplay::setBbRightMargin( int right ) {
    this->boundingBoxWidth = this->width - this->posX - 1 - right;
    if( this->boundingBoxWidth < 10 ) {
        this->boundingBoxWidth = 10;
    }
}

/** pro tisk textu: nastavi pozici a pro danou pozici nastavi bounding box az do okraje displeje */
void ExtDisplay::setPos( int x, int y ) {
    this->posX = x;
    this->posY = y;
    this->setBbFullWidth();
}



void ExtDisplay::init( Adafruit_GFX* display )
{
    // tohle je potřeba pro zalamování textu
    utf8tocp( samohlasky );  

    this->display = display;
    this->width = display->width();
    this->height = display->height();
}


#define BUFFER_SIZE 512
#define MAX_WORD_SIZE 20

char * curPos;
char oneWord[MAX_WORD_SIZE+2];
char delimiter;

void initParser( char * text ) {
    curPos = text;
    delimiter = 0;
}

/**
 * Vraci jednotliva slova ze vstupniho retezce.
 * Osetruje nektere specificke pripady - tecka jako desetinna tecka, stupne Celsia.
 */ 
boolean getNextWord() {
    int outPos = 0;

    while(true) {
        char c = *curPos;

        if( c==0 ) {
            return outPos>0;
        }

        curPos++;
        char c1 = *curPos;

        bool canBeDelimiter = true;

        // pokud je sekvence ".<CISLO>", pak predpokladame desetinnou tecku -  neni delimiter a stava se soucasti slova!
        if( c=='.' && isdigit(c1) ) {
          canBeDelimiter=false;
        } 

        // pokud je sekvence " °", pak predpokladame teplotu: "12 °C" a mezera neni delimiter
        // (neporovnavam se znakem "°", protoze by byl unicode! v mem kodovani jde o 0x90)
        if( c==' ' && c1==0x90 ) {
          canBeDelimiter=false;
        } 

        // hledame delimiter
        if( canBeDelimiter && (c==' ' || c==',' || c=='.' || c==';') ) {
            oneWord[outPos] = c;
            oneWord[++outPos] = 0;
            delimiter = c;
            break;
        } 

        oneWord[outPos] = c;
        oneWord[++outPos] = 0;

        if( outPos==MAX_WORD_SIZE) {
            oneWord[outPos] = '-';
            oneWord[++outPos] = 0;
            delimiter = '-';
            break;
        }
    }

    return true;
}


// pokud se povoli, dumpuji se detailni informace o zalamovani
// #define DUMP_DEBUG_INFO


/**
 * Vytiskne UTF-8 text na displej vcetne skoro korektniho zalamovani slov.
 * 
 * Pro fonty je potreba pouzit zpracovani dle https://github.com/petrbrouzda/fontconvert8-iso8859-2 !!!
 * 
 * Prvni radek se tiskne od pozice [X+x_offset, Y], ktera je LEVY DOLNI roh prvniho pismene. 
 * x_offset se pouzije jen pro prvni radek, dalsi radek se tiskne na [X, Y+vyskaRadku] 
 * 
 * Zalamovani radek je omezene nastavenym bounding boxem - pri zavolani setPos(x,y) se 
 * nastavi bounding box az do plne sirky displeje, stejne jako funkci setBbFullWidth().
 * Dalsi alternativy jsou: setBbRightMargin(), boundingBoxWidth().
 * 
 * Vraci aktualni pozici X, kterou je mozne pouzit jako x_offset pro dalsi volani.
 * Necha nastavenou posX = puvodni posX; nastavi posY = posledni radek s textem.
 */ 
int ExtDisplay::printUTF8( const char * text, int x_offset  ) {
    char buffer[BUFFER_SIZE];
    strncpy( buffer, text, BUFFER_SIZE );
    buffer[BUFFER_SIZE-1] = 0;
    utf8tocp( buffer );
    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "[%s]\n", buffer );
        Serial.printf( "  pos=%d,%d bbW=%d xo=%d\n", this->posX, this->posY, this->boundingBoxWidth, x_offset );
    #endif

    int x,y;
    x = this->posX + x_offset;
    y = this->posY;

    initParser( buffer );
    while( true ) {
        if( ! getNextWord() ) break;
        #ifdef DUMP_DEBUG_INFO  
            Serial.printf( "  # '%s'\n", oneWord );
        #endif

        int16_t x1, y1;
        uint16_t w, h;
        this->display->getTextBounds( (const char*)oneWord, x, y, &x1, &y1, &w, &h );
        #ifdef DUMP_DEBUG_INFO  
            Serial.printf( "  max pos %d,%d size %d,%d\n", x1,y1, w,h );
        #endif

        if( ( x + w ) <= ( this->posX + this->boundingBoxWidth ) ) {
            // slovo se veslo na tenhle radek
            this->display->setCursor( x, y );     
            this->display->print( oneWord );
            x += w;
            #ifdef DUMP_DEBUG_INFO  
                Serial.printf( "  pokracuju, nova souradnice %d,%d\n", x,y );
            #endif
        } else {
            // slovo se nevejde na tento radek

            #ifdef DUMP_DEBUG_INFO  
                Serial.printf( "  @ %d,%d, w=%d\n", x,y, w );
            #endif
            
            /* 
              moznosti:
              - slovo je kratsi nez 4 znaky - zalomit na dalsi radek rovnou
              - slovo je delsi - brat zprava, hledat samohlasky, a pokud po pridani pomlcky se na radek vejde, rozdelit na dve slova 
                (trivialni zalamovani, nepocitame se slabikotvornymi L/R)
            */
              
            if( strlen(oneWord)>=4 ) {
              // brat zprava, hledat samohlasky, a pokud po pridani pomlcky se na radek vejde, rozdelit na dve slova
              // zbytek dame do oneWord a vytiskneme na dalsi radek
              char slovo1[BUFFER_SIZE];
              // nenechame kratsi zbytek nez 2 znaky
              for( int i = strlen(oneWord)-2; i>=2; i-- ) {
                char cminus1 = oneWord[i-2];
                char c = oneWord[i-1];
                char cplus1 = oneWord[i];
                
                #ifdef DUMP_DEBUG_INFO  
                    Serial.printf( "  * zalamuji na %d, predesly znak [%c]\n", i, c );
                #endif
                if( jeSamohlaska(c) && !jeSamohlaska(cplus1)) {
                  // pokud to je samohlaska, muzeme na tom zalomit
                  // nesmi ale nasledovat dalsi samohlaska, protoze OU, AU a jine dvojhlasky
                } else if( jeSamohlaska(cminus1) && !jeSamohlaska(c) && !jeSamohlaska(cplus1) ) {
                  // v sekvenci "lehkou" muzeme rozdelit "leh_kou", ekv. "odpoled_ne"
                  // tj. aktualni NENI samohlaska, predesle JE samohlaska a nasledujici NENI samohlaska
                  // jenze to zase prinese zalomeni v "zam_raceno"
                } else {
                  // tady zalomit nemuzeme
                  continue;
                }

                memcpy( slovo1, oneWord, i );
                slovo1[i] = '-';
                slovo1[i+1] = 0;
                this->display->getTextBounds( (const char*)slovo1, x, y, &x1, &y1, &w, &h );
                if( ( x + w ) <= ( this->posX + this->boundingBoxWidth ) ) {
                  #ifdef DUMP_DEBUG_INFO  
                      Serial.printf( "  @ %d,%d, w=%d, zalomeno [%s]\n", x,y, w, slovo1 );
                  #endif
                  // vejde se to!
                  this->display->setCursor( x, y );     
                  this->display->print( slovo1 );

                  // do oneWord presuneme jen zbytek slova a pujdeme vykreslit druhy radek
                  strcpy( oneWord, oneWord+i );

                  break;
                }
              }
            }

            // zalomime na dalsi radek
            x = this->posX;
            y += this->vyskaRadku;

            // musime znovu, protoze ve vyjimecnem pripade to zalomi text samo a vrati to w treba 392 bodu
            this->display->getTextBounds( (const char*)oneWord, x, y, &x1, &y1, &w, &h );
            #ifdef DUMP_DEBUG_INFO  
                Serial.printf( "  @ %d,%d, w=%d\n", x,y, w );
            #endif

            this->display->setCursor( x, y );     
            this->display->print( oneWord );
            x += w;
            #ifdef DUMP_DEBUG_INFO  
                Serial.printf( "  novy radek, nova souradnice %d,%d\n", x,y );
            #endif
        }
        if( delimiter==' ' ) {
            x += this->sirkaMezery;
        }
    }

    this->posY = y;
    return x - this->posX;
}

/** preskoci na dalsi radek */
void ExtDisplay::lf( int offset ) {
    this->posY += this->vyskaRadku + offset;
}


/** nastavi font a pripadne zmeni jeho vysku radku a sirku znaku o dany offset */
void ExtDisplay::setFont( const GFXfont * font, int yOffset, int xOffset )
{
    this->display->setFont( font );
    this->vyskaRadku = font->yAdvance + yOffset;
    // predpoklada, ze mezera je prvni znak
    this->sirkaMezery = font->glyph[0].xAdvance + xOffset;
    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "setFont: mezera %d, radek %d\n", this->sirkaMezery, this->vyskaRadku );
    #endif
}


/**
Draws a bar with text.
- 'value' means length of the bar; should be 0-100 (as per cent)

Colors:
- colorBar - color of bar
- colorBorder - color of border
- bgColor - color of empty space in right of bar

If bar is longer than text, text is centered in bar:

    +---border--------------------------------+
    |###########################              |
    |######## 60 % #############              |
    |###########################              |
    +---border--------------------------------+

	and colors are used:
	- colorTextOnBar - text color
	- colorBgOnBar - background box under the text

else it is printer after bar:

    +---border--------------------------------+
    |########                                 |
    |######## 17526 W                         |
    |########                                 |
    +---border--------------------------------+

	and colors are used:
	- colorTextOnBg - text color
	- colorBgOnBg - background box under the text

*/
void ExtDisplay::drawBar( int x, int y, int w, int h, double value, const char * text,  
			uint16_t colorBar, uint16_t colorBorder, 
            uint16_t colorTextOnBar, uint16_t colorTextOnBg, 
            uint16_t colorBgOnBar, uint16_t colorBgOnBg, uint16_t bgColor
) {
  if( value<0.0 ) value=0;
  if( value>100.0 ) value=100;

  this->display->startWrite();
  this->display->writeFastHLine( x,  y  ,w, colorBorder );
  this->display->writeFastHLine( x,  y+h,w, colorBorder );
  this->display->writeFastVLine( x,  y,  h, colorBorder );
  this->display->writeFastVLine( x+w,y,  h, colorBorder );
  int size1 = (int)((double)w * value / 100.0) - 1;
  if( size1>0 ) {
    this->display->writeFillRect( x+1, y+1, size1, h-1, colorBar );
  } else {
    size1 = 0;
  }
  int size2 = w - size1 - 1;
  if( size2>0 ) {
    this->display->writeFillRect( x+1+size1, y+1, size2, h-1, bgColor );
  }
  this->display->endWrite();

  // conversion to 8bit encoding - so getTextBounds() will work
  char buffer[BUFFER_SIZE];
  strncpy( buffer, text, BUFFER_SIZE );
  buffer[BUFFER_SIZE-1] = 0;
  utf8tocp( buffer );
  
  int16_t x1, y1;
  uint16_t textW, textH;
  this->display->getTextBounds( (const char*)buffer, x, y, &x1, &y1, &textW, &textH );
  
  int textX, textY;
  bool textOnBar = false;

  if( textW < size1-20 ) {
    // text doprostred baru
    textX = x + size1/2 - textW/2;
    textOnBar = true;
  } else {
    // text hned za bar
    textX = x + size1 + 5;
  }
  textY = y+h-((h-textH)/2);

  if( textOnBar ) {
    this->display->setTextColor( colorTextOnBar );
    this->display->fillRect( textX, textY-textH, textW+1, textH+2, colorBgOnBar );
  } else {
    this->display->setTextColor( colorTextOnBg );
    this->display->fillRect( textX, textY-textH, textW+1, textH+2, colorBgOnBg );
  }

  this->setPos( textX, textY );
  this->setBbFullWidth();
  this->printUTF8( text, 0 );
}