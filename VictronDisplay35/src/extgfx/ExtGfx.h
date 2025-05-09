#ifndef _EXTGFX_H__
#define _EXTGFX_H__

#include <Arduino.h>

#define EXTGFX_CORE_VERSION "2.0.2"

/**
 * Zajistuje kompatibilitu mezi Adafruit_GFX a Arduino_GFX displeji.
 */

#if __has_include(<Arduino_GFX_Library.h>) 
    // Arduino_GFX mode
    #include <Arduino_GFX_Library.h>
    #define EXTGFX_DISPLAY_TYPE Arduino_GFX
    #define EXTGFX_VERSION "ExtGfx-" EXTGFX_CORE_VERSION "-Arduino_GFX"
#else
    // Adafruit_GFX mode
    #include <Adafruit_GFX.h>
    #define EXTGFX_DISPLAY_TYPE Adafruit_GFX
    #define EXTGFX_VERSION "ExtGfx-" EXTGFX_CORE_VERSION "-Adafruit_GFX"
#endif

#endif
