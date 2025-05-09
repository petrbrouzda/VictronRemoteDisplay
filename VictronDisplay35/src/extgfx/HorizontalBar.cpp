#include "HorizontalBar.h"

// pokud se povoli, dumpuji se detailni informace o zalamovani
// #define DUMP_DEBUG_INFO


HbColorProfile::HbColorProfile(float valueFrom, uint16_t colorBar, uint16_t colorBorder, 
                            uint16_t colorTextOnBar, uint16_t colorBgOnBar, 
                            uint16_t colorTextOnBg, uint16_t bgColor)
{
    this->valueFrom =valueFrom;
    this->colorBar =colorBar;
    this->colorBorder =colorBorder;
    this->colorTextOnBar =colorTextOnBar;
    this->colorBgOnBar =colorBgOnBar;
    this->colorTextOnBg =colorTextOnBg;
    this->bgColor =bgColor;
}


HorizontalBar::HorizontalBar( EXTGFX_DISPLAY_TYPE *display, TextPainter * painter )
{
    this->display = display;
    this->painter = painter;
    this->dirty = true;
    this->font = NULL;
}

void HorizontalBar::setFont(TpFontConfig *font)
{
    this->font = font;
}

void HorizontalBar::setRange(float minVal, float maxVal)
{
    this->minVal = minVal;
    this->maxVal = maxVal;
    this->range = maxVal - minVal;
}

void HorizontalBar::setPosition( int x, int y, int w, int h )
{
    this->x = x;
    this->y = y;
    this->h = h;
    this->w = w;
}

void HorizontalBar::setValue(float val, char *text)
{
    this->currentvalue = val;
    strncpy( this->currentText, text, HB_MAX_TEXT_SIZE );
    this->currentText[HB_MAX_TEXT_SIZE]=0;
    this->dirty = true;
}

void HorizontalBar::setColors(HbColorProfile **colors)
{
    this->colors = colors;
}

void HorizontalBar::draw(bool force)
{
    if( !force && !this->dirty ) {
        // neni duvod prekreslovat
        return;
    }

    float curVal = this->currentvalue;

    HbColorProfile * color = this->colors[0];
    int i=0;
    while(1) {
        if( this->colors[i] == NULL ) {
            // posledni polozka
            break;
        }
        if( this->colors[i]->valueFrom > curVal ) {
            // nalezena vyssi hodnota
            break;
        }
        color = this->colors[i];
        i++;
    }

    if( curVal < this->minVal ) {
        curVal = this->minVal;
    } else if( curVal > this->maxVal ) {
        curVal = this->maxVal;
    } 
    // rozsah 0.0 az 1.0
    float value = (curVal - this->minVal) / this->range;

    // zde v color mam barvy
    this->display->startWrite();
    this->display->writeFastHLine( this->x,  this->y  ,this->w, color->colorBorder );
    this->display->writeFastHLine( this->x,  this->y+h,this->w, color->colorBorder );
    this->display->writeFastVLine( this->x,  this->y,  this->h, color->colorBorder );
    this->display->writeFastVLine( this->x+this->w,this->y,  this->h, color->colorBorder );
    int size1 = (int)((double)(this->w) * value ) - 1;
    if( size1>0 ) {
        this->display->writeFillRect( this->x+1, this->y+1, size1, this->h-1, color->colorBar );
    } else {
        size1 = 0;
    }
    int size2 = this->w - size1 - 1;
    if( size2>0 ) {
        this->display->writeFillRect( this->x+1+size1, this->y+1, size2, this->h-1, color->bgColor );
    }
    this->display->endWrite();

    int16_t x1, y1;
    uint16_t textW, textH;
    this->painter->getTextBounds( (const char*)this->currentText, this->x, this->y, &x1, &y1, &textW, &textH );

    int textX, textY;
    bool textOnBar = false;

    if( textW < size1-20 ) {
        // text doprostred baru
        textX = this->x + size1/2 - textW/2;
        textOnBar = true;
    } else {
        // text hned za bar
        textX = this->x + size1 + 5;
    }

    int textH2 = this->font->lineHeight+this->font->firstLineHeightOffset;
    textY = this->y + ((this->h - textH2)/2) - 1;

#ifdef DUMP_DEBUG_INFO
    Serial.printf( "h=%d, textH2=%d, ((h-textH2)/2)=%d, ", this->h, textH2, ((this->h-textH2)/2) );
    Serial.printf( "fontLHO=%d, textY-y=%d ", this->font->firstLineHeightOffset, textY-this->y );
    Serial.printf( "fontLH=%d \n", this->font->lineHeight );
#endif

    if( textOnBar ) {
        this->display->setTextColor( color->colorTextOnBar );
        if( color->colorBgOnBar != color->colorBar ) {
            this->painter->fillBackground( color->colorBgOnBar, 3 );
        } else {
            this->painter->noBackground();    
        }
    } else {
        this->display->setTextColor( color->colorTextOnBg );
        this->painter->noBackground();
    }

    TpFontConfig * backupFont;
    backupFont = this->painter->getFont();
    this->painter->setFont( this->font );

    this->painter->printLabel( TextPainter::ALIGN_LEFT, textX, textY, this->currentText );

    this->painter->setFont( backupFont );
    this->painter->noBackground();

    this->dirty = false;
}

void HorizontalBar::setDirty()
{
    this->dirty = true;
}

bool HorizontalBar::willRedraw()
{
    return this->dirty;
}
