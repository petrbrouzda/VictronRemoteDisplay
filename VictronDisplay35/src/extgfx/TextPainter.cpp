#include "TextPainter.h"

// pokud se povoli, dumpuji se detailni informace o zalamovani
// #define DUMP_DEBUG_INFO



TpFontConfig::TpFontConfig(const GFXfont *font, int lineHeightOffset, int charWidthOffset, int firstLineHeightOffset)
{
    if( font!=NULL ) {
        this->font = font;
        this->charWidthOffset = charWidthOffset;
        this->firstLineHeightOffset = firstLineHeightOffset;
        this->lineHeight = font->yAdvance + lineHeightOffset;
    }
}

TextPainter::TextPainter(EXTGFX_DISPLAY_TYPE *display, bool hyphenation, bool convertUtf8to8bit)
{
    this->display = display;
    this->hyphenation = hyphenation;
    this->convertUtf8to8bit = convertUtf8to8bit;

    this->background = false;

    strcpy(this->samohlasky, (char*)"eyuioaEYUIOAěýáíéúůĚÝÁÍÉÚŮ");
    if( convertUtf8to8bit ) {
        utf8tocp( samohlasky );
    }
}

void TextPainter::setDisplay(EXTGFX_DISPLAY_TYPE *display)
{
    this->display = display;
}

void TextPainter::createFontConfig( TpFontConfig *target, const GFXfont *font )
{
    int16_t x1, y1;
    uint16_t w, h;

    this->display->setFont( font );

    // sample text - characters with most diverse size over and under the textline
    char buffer[] = "MAjg096";
    this->display->getTextBounds( (const char*)buffer, 0, 0, &x1, &y1, &w, &h );

    /* vrácené hodnoty jsou:
    x1 = nejvíc doleva, co znak sahá; pokud je před určeným bodem, má zápornou hodnotu
    y1 = nejvyšší horní bod, co znak sahá, proti účaří
    h = celková výška textu
    
    třeba: [-4,-29], w=83, h=40

    h+y1 je přesah pod účaří
    -y1 + 1 je první řádka
    h by mělo být řádkování

    */

   target->font = font;
   int changeLH = (int)(0.1 * (float)h );
   if( changeLH==0 ) changeLH=1;
   target->lineHeight = h + 1 + changeLH;
   target->firstLineHeightOffset = - (h + y1 - 1);
   target->baselineOffset = -y1;
   target->underBaseline = h+y1;

   target->charWidthOffset = 0;

    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "createFontConfig: [%d,%d], w=%d, h=%d, font yAdv=%d\n", x1, y1, w, h, font->yAdvance );
        Serial.printf( "createFontConfig out: flho=%d lh=%d \n", target->firstLineHeightOffset, target->lineHeight );
    #endif
}

bool TextPainter::jeSamohlaska( char c ) {
  return strchr(this->samohlasky,c)!=NULL;
}

TpFontConfig *TextPainter::getFont()
{
    return this->fontConfig;
}

void TextPainter::startText(int x, int y, int width, int height)
{
    this->posX = x;
    this->textBoxY = y;
    this->posY = y + this->vyskaPrvnihoRadku;

    if( width==TP_MAX_SIZE 
        || width > this->display->width() - this->posX - 1 ) {
        this->boundingBoxWidth = this->display->width() - this->posX - 1;    
    } else {
        this->boundingBoxWidth = width;
    }
    if( height==TP_MAX_SIZE 
        || height > this->display->height() - this->textBoxY - 1 ) {
        this->textMaxY = this->display->height() -1;
    } else {
        this->textMaxY = height + this->textBoxY;
    }
    
    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "startText [%d,%d], w=%d, maxY=%d, 1stlineY=%d\n", this->posX, this->textBoxY, this->boundingBoxWidth, this->textMaxY, this->posY );
    #endif

}

void TextPainter::textLf(int offset)
{
    this->posY += this->vyskaRadku + offset;
}

/**
 * Nastaví font k použití.
 */
void TextPainter::setFont(TpFontConfig *fontConfig)
{
    this->fontConfig = fontConfig;

    this->display->setFont( fontConfig->font );
    this->vyskaRadku = fontConfig->lineHeight;
    this->vyskaPrvnihoRadku = this->vyskaRadku + fontConfig->firstLineHeightOffset;

    // predpoklada, ze mezera je prvni znak
    this->sirkaMezery = fontConfig->font->glyph[0].xAdvance + fontConfig->charWidthOffset;
    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "setFont: mezera %d, radek %d, prvni radek %d\n", this->sirkaMezery, this->vyskaRadku, this->vyskaPrvnihoRadku );
    #endif
}

void TextPainter::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    char buffer[BUFFER_SIZE];
    strncpy( buffer, string, BUFFER_SIZE );
    buffer[BUFFER_SIZE-1] = 0;
    if( this->convertUtf8to8bit ) {
        utf8tocp( buffer );
    }  
    this->display->getTextBounds( (const char*)buffer, x, y, x1, y1, w, h );
}


void TextPainter::printLabel( TextPainter::HorizontalAlign ha, int x, int y, char *text)
{
    int16_t x1, y1;
    uint16_t w, h;

    char buffer[BUFFER_SIZE];
    strncpy( buffer, text, BUFFER_SIZE );
    buffer[BUFFER_SIZE-1] = 0;
    if( this->convertUtf8to8bit ) {
        utf8tocp( buffer );
    }  
    this->display->getTextBounds( (const char*)buffer, 0, 0, &x1, &y1, &w, &h );

    /* vrácené hodnoty jsou:
    x1 = nejvíc doleva, co znak sahá; pokud je před určeným bodem, má zápornou hodnotu
    y1 = nejvyšší horní bod, co znak sahá, proti účaří
    h = celková výška textu
    
    třeba: [-4,-29], w=83, h=40

    h+y1 je přesah pod účaří
    */

    int x0t, x0b;
    if( ha == ALIGN_CENTER ) {
        x0b = x - (w/2);
        x0t = x0b - x1;
    } else if( ha == ALIGN_RIGHT ) {
        x0b = x - w - 1;
        x0t = x0b - x1;
    } else { // left
        x0b = x;
        x0t = x0b - x1;
    }

    if( this->background ) {
        #ifdef DUMP_DEBUG_INFO  
            Serial.printf( "printLabel bg: [%d,%d], w=%d, h=%d, cfg=%d/%d \n", x1, y1, w, h, this->vyskaPrvnihoRadku, this->vyskaRadku );
        #endif

        // this->vyskaPrvnihoRadku je vzdálenost k účaří; h+y1 je přesah pod účaří
        int vyska = this->vyskaPrvnihoRadku + (h+y1);  
        this->display->fillRect( x0b - this->bgBorderSize , y - this->bgBorderSize + 1, 
                        w + this->bgBorderSize + this->bgBorderSize, vyska + this->bgBorderSize + this->bgBorderSize, 
                        this->bgColor );
    }

    this->display->setCursor( x0t, y + this->vyskaPrvnihoRadku );     
    this->display->print( buffer );
}

void TextPainter::fillBackground(int color, int borderSize )
{
    this->background = true;
    this->bgColor = color;
    this->bgBorderSize = borderSize;
}

void TextPainter::noBackground()
{
    this->background = false;
}

void TextPainter::setHyphenation(bool hyphenation)
{
    this->hyphenation = hyphenation;
}

/**
 * Parser textu.
 * Inicializace.
 */
void TextPainter::initParser( char * text ) {
    this->curPos = text;
    this->delimiter = 0;
}

/**
 * Parser textu.
 * Vraci jednotliva slova ze vstupniho retezce.
 * Osetruje nektere specificke pripady - tecka jako desetinna tecka, stupne Celsia.
 */ 
boolean TextPainter::getNextWord() {
    int outPos = 0;

    while(true) {
        char c = *(this->curPos);

        if( c==0 ) {
            return outPos>0;
        }

        curPos++;
        char c1 = *(this->curPos);

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
            this->oneWord[outPos] = c;
            this->oneWord[++outPos] = 0;
            this->delimiter = c;
            break;
        } 

        this->oneWord[outPos] = c;
        this->oneWord[++outPos] = 0;

        if( outPos==MAX_WORD_SIZE) {
            this->oneWord[outPos] = '-';
            this->oneWord[++outPos] = 0;
            this->delimiter = '-';
            break;
        }
    }

    return true;
}

/**
 * Tisk textového bloku
 */
int TextPainter::printText( const char * text, int x_offset  ) {

    char buffer[BUFFER_SIZE];
    strncpy( buffer, text, BUFFER_SIZE );
    buffer[BUFFER_SIZE-1] = 0;
    if( this->convertUtf8to8bit ) {
        utf8tocp( buffer );
    }    
    #ifdef DUMP_DEBUG_INFO  
        Serial.printf( "[%s]\n", buffer );
        Serial.printf( "  pos=%d,%d bbW=%d xo=%d\n", this->posX, this->posY, this->boundingBoxWidth, x_offset );
    #endif

    int x,y;
    x = this->posX + x_offset;
    y = this->posY;

    if( y > this->textMaxY ) {
        #ifdef DUMP_DEBUG_INFO  
            Serial.printf( "  uz jsme za limitem vysky\n" );
        #endif
        return 0;
    }

    this->initParser( buffer );
    while( true ) {
        if( ! this->getNextWord() ) break;
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
              
            if( strlen(oneWord)>=MAX_DELKA_SLOVA_CO_SE_NEZALAMUJE && this->hyphenation ) {
              // brat zprava, hledat samohlasky, a pokud po pridani pomlcky se na radek vejde, rozdelit na dve slova
              // zbytek dame do oneWord a vytiskneme na dalsi radek
              char slovo1[MAX_WORD_SIZE+5];
              // nenechame kratsi zbytek nez 2 znaky
              for( int i = strlen(oneWord)-3; i>=2; i-- ) {
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

            if( y > this->textMaxY  ) {
                #ifdef DUMP_DEBUG_INFO  
                    Serial.printf( "  uz jsme za limitem vysky\n" );
                #endif
                return 0;
            }

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