#ifndef _CHARTDATASOURCE_H__
#define _CHARTDATASOURCE_H__

/*
Pro RingBufCDS používáme upravenou verzi knihovny 
https://github.com/wizard97/ArduinoRingBuffer 
v lokálním podadresáři.
*/
#include "../RingBuf/RingBuf.h"

/**
 * Datový zdroj pro grafy - interface.
 * Reálné implementace: RingBufCDS, ArrayCDS
 */
class ChartDatasource {
    public:
        /** Načte počet položek. */
        virtual int getNumItems() = 0;
        
        /** Načte položku na dané pozici. Rozsah: 0 .. getNumItems(). Může vrátit NaN, pokud neexistuje. */
        virtual float getItem( int pos ) = 0;
        
        /** Verze. Při každé změně obsahu se musí změnit verze. */
        int version;
};



/**
 * Datový zdroj - ring buffer. 
 * Vkládání položek na začátek automaticky maže položky na konci, takže v poli zůstává 
 * konstantní počet položek počínaje nejnovější.
 */
class RingBufCDS : public ChartDatasource {
    public:
        RingBufCDS( int numItems );
        /** Vloží položku na začátek; pokud je již pole plné, smaže se tím položka na konci. */
        void put( float item );
        
        int getNumItems();
        /** Načte položku na dané pozici. Rozsah: 0 .. getNumItems() */
        float getItem( int pos );

    private:
        RingBuf * data;

};


/**
 * Datový zdroj - pole.
 * Aplikace může zapisovat do libovolného prvku pole na přeskáčku dle potřeby.
 */
class ArrayCDS : public ChartDatasource {
    public:
        ArrayCDS( int numItems );

        /** nastaví položku */
        void setItem( int pos, float item );
        /** maximální počet položek, co se do pole vejde */
        int getMaxItems();

        /** nejvyšší číslo položky, co v poli je (ale mohou být prázdná místa) */
        int getNumItems();
        /** Načte položku na dané pozici. Rozsah: 0 .. getNumItems() */
        float getItem( int pos );

    private:
        float * data;
        int maxItems;
        int items;

};




#endif