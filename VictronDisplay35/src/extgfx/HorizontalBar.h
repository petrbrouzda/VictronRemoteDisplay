#ifndef _HORIZONTALBAR___H_
#define _HORIZONTALBAR___H_

#include "ExtGfx.h"

#include "TextPainter.h"


/**
Draws a bar with text.
Ukázka v ExtGfx.ino v demo5_horizontalBar()

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
	- bgColor - background box under the text

*/

/* maximální délka textu, delší bude ořezán */
#define HB_MAX_TEXT_SIZE 100


/**
 * Barevný profil - sada barev pro jednotlivé zobrazované hodnoty. 
 * Předává se do HorizontalBar.setColors() jako pole pointerů na HbColorProfile.
 * Vysvětlení role jednotlivých barev v prvním komentáři v HorizontalBar.h
 * Demo/vysvětlení v ExtGfx.ino v demo5_horizontalBar()
 */
class HbColorProfile {
    public:
        HbColorProfile( float valueFrom, uint16_t colorBar, uint16_t colorBorder, 
                        uint16_t colorTextOnBar,  uint16_t colorBgOnBar, uint16_t colorTextOnBg, 
                        uint16_t bgColor );

        /** data value limit */
        float valueFrom;

        /** color of bar  */
        uint16_t colorBar;

        /** color of border  */
        uint16_t colorBorder;

        /** If bar is longer than text - text color */
        uint16_t colorTextOnBar;
        /** If bar is longer than text - background box under the text */
        uint16_t colorBgOnBar;
        
        /** If bar is shorter than text - text color */
        uint16_t colorTextOnBg;

        /** color of empty space in right of bar  */
        uint16_t bgColor;

};


/**
 * Kreslí horizontální bar gauge.
 
 If bar is longer than text, text is centered in bar:

    +---border--------------------------------+
    |###########################              |
    |######## 60 % #############              |
    |###########################              |
    +---border--------------------------------+

else it is printer after bar:

    +---border--------------------------------+
    |########                                 |
    |######## 17526 W                         |
    |########                                 |
    +---border--------------------------------+

 */
class HorizontalBar {
    public:
        HorizontalBar( EXTGFX_DISPLAY_TYPE *display, TextPainter * painter );
        /** Font pro text. */
        void setFont( TpFontConfig * font );
        /** Rozsah hodnot, které budou použity pro zobrazení od levé do pravé strany. Min musí být menší než max.*/
        void setRange(float minVal, float maxVal);
        /** Pozice a velikost widgetu. */
        void setPosition( int x, int y, int w, int h );

        /** Nastaví aktuální hodnotu a aktuální text. 
         * Hodnota může být mimo rozsah minVal..maxVal; v takovém případě se zobrazí jako minimum/maximum. */
        void setValue( float val, char * text );

        /** pole barevných konfigurací pro rozsahy hodnot; musi byt zakoncene NULLem; hodnoty musí být seřazené vzestupně */
        void setColors( HbColorProfile **colors );

        /** Vykresli. Pokud se data nezměnila, nekreslí se, pokud není force=true. */
        void draw( bool force = false );

        /** Nastaví prvek k překreslení při příštím draw(), i když se nezměnily data. */
        void setDirty();

        /** vrátí informaci, zda se bude vykreslovat - tj. je dirty */
        bool willRedraw();

    private:
        EXTGFX_DISPLAY_TYPE *display;
        TextPainter * painter;
        HbColorProfile **colors;
        TpFontConfig * font;
        float minVal;
        float maxVal;
        int x; 
        int y; 
        int h; 
        int w;

        float currentvalue;
        char currentText[HB_MAX_TEXT_SIZE+2];

        bool dirty;

        /** vzdalenost mezi minVal a maxVal */
        float range;
};


#endif