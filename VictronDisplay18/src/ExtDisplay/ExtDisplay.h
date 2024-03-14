/**
 * Extension pro Adafruit GFX>
 * - tisk UTF-8cestiny
 * - skoro korektni zalamovani slov v cestine
 * - tisk baru (procentualni indikator)
 */ 

#ifndef _EXTDISP___H_
#define _EXTDISP___H_

#include <Arduino.h>
#include <Adafruit_GFX.h>

#define EXTDISPLAY_VERSION "3.1"

class ExtDisplay {
    public:
    
    	/* ****************** Initialisation **************** */

		/** initialisation */    
        void init( Adafruit_GFX* display );

		
		/* ****************** Display properties, to be read only **************** */

        /** sirka displeje, nemeni se */
        int width;
        /** vyska displeje, nemeni se */
        int height;
        /** odkaz na display */
        Adafruit_GFX* display;

        /** Aktualni pozice pro vykreslovani textu - X. Lepe menit pres setPos().  */
        int posX;
        /** Aktualni pozice pro vykreslovani textu - Y. Lepe menit pres setPos().  */
        int posY;
        
        /* **************** Text functions ********************************/
        
        /** pro tisk textu: nastavi pozici a pro danou pozici nastavi bounding box az do okraje displeje */
        void setPos( int x, int y );

        /** pro tisk textu: sirka obdelniku, do ktereho se smi vypsat text */
        int boundingBoxWidth;
        
        /** pro tisk textu: nastavi maximalni sirku vykreslovaciho obdelniku pro aktualni pozici X */
        void setBbFullWidth();
        
        /** pro tisk textu: nastavi sirku vykreslovaciho obdelniku pro aktualni pozici X a urceny pravy okraj */
        void setBbRightMargin( int right );
        
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
        int printUTF8( const char * text, int x_offset = 0 );

        /** preskoci na dalsi radek */
        void lf( int offset=0 );
        
        /** nastavi font a pripadne zmeni jeho vysku radku a sirku znaku o dany offset */
        void setFont( const GFXfont * font, int yOffset = 0, int xOffset = 0 );
        
        /** sirka mezery, nastavi se automaticky z fontu, ale je mozne zmenit */
        int sirkaMezery;

        /** vyska radku, nastavi se automaticky z fontu, ale je mozne zmenit */
        int vyskaRadku;
        
        

        /* **************** Graphics ********************************/
        
		/**
		Draws a bar with text.
		value should be 0-100 (as per cent)
		
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
		void drawBar( int x, int y, int w, int h, double value, const char * text,  
					uint16_t colorBar, uint16_t colorBorder, 
		            uint16_t colorTextOnBar, uint16_t colorTextOnBg, 
		            uint16_t colorBgOnBar, uint16_t colorBgOnBg, uint16_t bgColor );



    private:


};

#endif