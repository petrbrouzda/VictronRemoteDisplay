#ifndef _TEXTPAINTER___H_
#define _TEXTPAINTER___H_

/**
 * Nástroj pro zobrazení českého textu. 
 * 
 * Umí tisknout textové bloky včetně skoro korektního rozdělování českých slov na konci řádku.
 * Umí tisknout jednořádkové texty ("labely") zarovnané vpravo/na střed/vlevo, s pozadím i bez.
 * 
 * Pracuje s češtinou v UTF-8, kterou mapuje na 8bit fonty dle https://github.com/petrbrouzda/fontconvert8-iso8859-2
 * 
 * Tiskne na jakékoli zařízení implementující Adafruit_GFX rozhraní (LCD displeje, ePaper displeje) nebo do canvasu, 
 * podle toho, co dostane jako parametr.
 * 
 * Ukázka v ExtGfx.ino ve funkcích 
 * - demo1_zakladniTextovyBlok()
 * - demo2_labely()
 * - demo3_zmenaRadkovani()
 * - demo4_upravyRadkovaniFontu2()
 */


#include "ExtGfx.h"

#include "../gfxlatin2/gfxlatin2.h"

/** velikost pracovního bufferu, delší text bude oříznut na tuto délku */
#define BUFFER_SIZE 512

/** maximální délka slova, delší se vždy zalomí */
#define MAX_WORD_SIZE 20

/** takhle krátká a kratší slova se nikdy nezalamují */
#define MAX_DELKA_SLOVA_CO_SE_NEZALAMUJE 4



/** Objekt držící konfiguraci typu písma a nastavení řádkování. */
class TpFontConfig {
    public:
        /**
         * lineHeightOffset = změna velikosti řádkování vůči rámečku největšího znaku (0 = beze změny)
         * charWidthOffset = změna šířky znaku proti šířce mezery (0 = beze změny)
         * firstLineHeightOffset = offset první řádky vůči lineHeightOffset (0 = beze změny)
         */
        TpFontConfig( const GFXfont * font = NULL, int lineHeightOffset = 0, int charWidthOffset = 0, int firstLineHeightOffset = 0 );

        const GFXfont * font;

        /** zatím se nepoužívá, pro info: vzdálenost od účaří k hornímu okraji  */
        int baselineOffset;

        /** zatím se nepoužívá, pro info: délka ocásku dole pod účařím */
        int underBaseline;

        /** výška řádku */
        int lineHeight;

        /** změna šířky znaku proti šířce mezery (0 = beze změny) */
        int charWidthOffset;

        /** offset první řádky vůči lineHeightOffset (0 = beze změny)  */
        int firstLineHeightOffset;
};

#define TP_MAX_SIZE -1


/**
 * Nástroj pro tisk českého textu. 
 * Umí tisknout textové bloky včetně skoro korektního rozdělování českých slov na konci řádku.
 * Umí tisknout jednořádkové texty ("labely") zarovnané vpravo/na střed/vlevo.
 * Tiskne na displej nebo do canvasu, podle toho, co dostane.
 */
class TextPainter {

    public:
        /** 
         * hyphenation = rozdělování slov na konci řádku;
         * convertUtf8to8bit = podpora pro češtinu podle https://github.com/petrbrouzda/fontconvert8-iso8859-2
         */
        TextPainter( EXTGFX_DISPLAY_TYPE* display, bool hyphenation = true, bool convertUtf8to8bit = true );

        /**
         * Změna cíle, kam se tiskne. Může se tisknout i do canvasu, ne jen na fyzický displej.
         */
        void setDisplay( EXTGFX_DISPLAY_TYPE* display );

        /**
         * Vytvoří parametry fontu. Načte jeho velikost vyrenderováním základních písmen.
         */
        void createFontConfig( TpFontConfig * target, const GFXfont * font );


        /* 
        Nastaví použitý font a řádkování.
        Pokud se následně změní odkazovaný objekt fontConfigu, NEMÁ to vliv na nastavení TextPainteru, protože
        potřebná data si vyčte v okamžiku volání funkce setFont().
         */
        void setFont( TpFontConfig * fontConfig );


        /** 
         * Vrátí konfiguraci fontu - odkaz na existující objekt, ne kopii. 
         * Používá se pro dočasné odložení konfigurace, aby pak šla bezbolestně vrátit zpět.
         */
        TpFontConfig * getFont();


        /** Tisk textového pole: nastaví pozici levého horního rohu pole a velikost pole */
        void startText( int x, int y, int width = TP_MAX_SIZE, int height = TP_MAX_SIZE );

        /**
		 * Tisk textového pole: 
         * 
         * Vytiskne UTF-8 (pokud je zapnuto convertUtf8to8bit) text na displej 
         * vcetne skoro korektniho zalamovani slov (pokud je zapnuto wrapWords).
		 * 
         * Pokud je zapnuto convertUtf8to8bit, obsluhuje češtinu v UTF-8. V tom případě
		 * je potreba pouzit zpracovani fontu dle https://github.com/petrbrouzda/fontconvert8-iso8859-2 !!!
		 * 
		 * Prvni radek se tiskne od pozice [X+x_offset, Y], ktera je LEVY HORNI roh prvniho pismene. 
         * (To je odlišnost proti Adafruit GFX!!!)
		 * x_offset se pouzije jen pro prvni radek, dalsi radek se tiskne na [X, Y+vyskaRadku] 
		 * 
		 * Zalamovani radek je omezene nastavenym bounding boxem - pri zavolani startText(x,y) se 
		 * nastavi bounding box.
		 * 
		 * Vraci aktualni pozici X, kterou je mozne pouzit jako x_offset pro dalsi volani (tj. ne absolutní
         * pozici X, ale vzdálenost od levé hrany textového pole).
		 * Necha nastavenou posX = puvodni posX; nastavi posY = posledni radek s textem.
		 */ 
        int printText( const char * text, int x_offset = 0 );

        /** Tisk textového pole: Přeskočí na další řádek */
        void textLf( int offset=0 );

        /** Tisk textového pole: zapíná/vypíná rozdělování slov na konci řádku */
        void setHyphenation( bool hyphenation );        



        /** ekvivalent getTextBounds() pracujici s UTF-8 textem  (pokud je zapnuto convertUtf8to8bit)
			@brief  Helper to determine size of a string with current font/size.
					Pass string and a cursor position, returns UL corner and W,H.
			@param  str  The ASCII string to measure
			@param  x    The current cursor X
			@param  y    The current cursor Y
			@param  x1   The boundary X coordinate, returned by function
			@param  y1   The boundary Y coordinate, returned by function
			@param  w    The boundary width, returned by function
			@param  h    The boundary height, returned by function
		*/
		void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1,
                   int16_t *y1, uint16_t *w, uint16_t *h);


        /** Možná zarovnání pro printLabel() */
        enum HorizontalAlign { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

        /** 
         * Vytiskne jednořádkový text. Y je horní okraj textu. 
         * 
         * Zarovnání vlevo = X je levá hrana textu.
         * Zarovnání doprostřed = X je střed textu.
         * Zarovnání vpravo = X je pravá hrana textu.
         * 
         * Text by neměl být širší než zbývající místo na displeji, knihovna to neřeší!
        */
        void printLabel( TextPainter::HorizontalAlign ha, int x, int y, char * text );

        /**
         * Pod text se vytiskne obdélník v barvě pozadí.
         * Platí jen pro printLabel.
         */
        void fillBackground( int color, int borderSize=1 );

        /**
         * Pod text se nebude tisknout obdélník v barvě pozadí.
         * Platí jen pro printLabel.
         */
        void noBackground();


    private:
        EXTGFX_DISPLAY_TYPE* display;

        /** rozdělování slov na konci řádku */
        bool hyphenation;

        /** konverze češtiny UTF8 -> 8bit */ 
        bool convertUtf8to8bit;

        /** sirka mezery, nastavi se automaticky z fontu, ale je mozne zmenit */
        int sirkaMezery;

        /** vyska radku, nastavi se automaticky z fontu, ale je mozne zmenit */
        int vyskaRadku;

        /** o kolik zacne prvni radek pod Y pozici */
        int vyskaPrvnihoRadku;
        
        /** pozice kurzoru */
        int posX;

        /** pozice kurzoru */
        int posY;

        /** pro tisk textu: zacatek textového boxu */
        int textBoxY;

        /** pro tisk textu: sirka obdelniku, do ktereho se smi vypsat text */
        int boundingBoxWidth;

        /** pro tisk textu: vyska obdelniku, do ktereho se smi vypsat text */
        int textMaxY;

        /** Aktuální font */
        TpFontConfig * fontConfig;

        /** má se tisknout background? */
        bool background;
        /** barva pozadí */
        int bgColor;
        /** přesah pozadí proti textu */
        int bgBorderSize;

        // ++++ položky pro parser textu
        bool jeSamohlaska( char c );
        char samohlasky[60];
        char * curPos;
        char oneWord[MAX_WORD_SIZE+2];
        char delimiter;
        void initParser( char * text );
        boolean getNextWord();
        // --- parser textu
};

#endif