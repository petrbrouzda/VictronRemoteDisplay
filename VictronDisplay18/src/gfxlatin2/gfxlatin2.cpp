#include <Arduino.h>
#include "decodeutf8.h"
#include "gfxlatin2.h"

// Define the macro to flag as unmapped those Latin 1 characters
// that have been replaced with Latin 9 characters.

//#define INVALIDATE_OVERWRITTEN_LATIN_1_CHARS

uint16_t recode(uint8_t b) {

  uint16_t ucs2 = decodeUTF8(b);

  if (ucs2 > 0x7F) {
#ifdef INVALIDATE_OVERWRITTEN_LATIN_1_CHARS     
    if (0xA4 <= ucs2 && ucs2 <= 0xBE) {
      switch (ucs2) {
        case 0xa4: 
        case 0xa6:
        case 0xa8:
        case 0xb4:
        case 0xb8:
        case 0xbc:
        case 0xbd:
        case 0xbe: return (showUnmapped) ? 0x7F : 0xFFFF;
      }
    } 
#endif     
    switch (ucs2) {

        //a0
        case 0x0104: return  0xa1  ; break;
        case 0x02D8: return  0xa2  ; break;
        case 0x0141: return  0xa3  ; break;
        case 0x013D: return  0xa5  ; break;
        case 0x015A: return  0xa6  ; break;

        case 0x0160: return  0xa9  ; break;
        case 0x015E: return  0xaa  ; break;
        case 0x0164: return  0xab  ; break;
        case 0x0179: return  0xac  ; break;
        case 0x017D: return  0xae  ; break;
        case 0x017B: return  0xaf  ; break;

        //b0
        case 0x0105: return  0xb1  ; break;
        case 0x02DB: return  0xb2  ; break;
        case 0x0142: return  0xb3  ; break;
        case 0x013E: return  0xb5  ; break;
        case 0x015B: return  0xb6  ; break;
        case 0x02C7: return  0xb7  ; break;

        case 0x0161: return  0xb9  ; break;
        case 0x015F: return  0xba  ; break;
        case 0x0165: return  0xbb  ; break;
        case 0x017A: return  0xbc  ; break;
        case 0x02DD: return  0xbd  ; break;
        case 0x017E: return  0xbe  ; break;
        case 0x017C: return  0xbf  ; break;

        // c0
          case 0x0154: return  0xc0  ; break;
        case 0x0102: return  0xc3  ; break;
        case 0x0139: return  0xc5  ; break;
        case 0x0106: return  0xc6  ; break;

          case 0x010C: return  0xc8  ; break;
        case 0x0118: return  0xca  ; break;
        case 0x011A: return  0xcc  ; break;
        case 0x010E: return  0xdf  ; break;

        // d0
          case 0x0110: return  0xd0  ; break;
        case 0x0143: return  0xd1  ; break;
        case 0x0147: return  0xd2  ; break;
        case 0x0150: return  0xd5  ; break;

          case 0x0158: return  0xd8  ; break;
        case 0x016E: return  0xd9  ; break;
        case 0x0170: return  0xdb  ; break;
        case 0x0162: return  0xde  ; break;
        
        //e0
          case 0x0155: return  0xe0  ; break;
        case 0x0103: return  0xe3  ; break;
        case 0x013A: return  0xe5  ; break;
        case 0x0107: return  0xe6  ; break;

          case 0x010D: return  0xe8  ; break;
        case 0x0119: return  0xea  ; break;
        case 0x011B: return  0xec  ; break;
        case 0x010F: return  0xef  ; break;

        // f0
          case 0x0111: return  0xf0  ; break;
        case 0x0144: return  0xf1  ; break;
        case 0x0148: return  0xf2  ; break;
        case 0x0151: return  0xf5  ; break;

          case 0x0159: return  0xf8  ; break;
        case 0x016F: return  0xf9  ; break;
        case 0x0171: return  0xfb  ; break;
        case 0x0163: return  0xfe  ; break;
        case 0x02D9: return  0xff  ; break;
    }
  }	  
  return ucs2;
}

// Convert String object from UTF8 string to extended ASCII
String utf8tocp(String s) {
  String r="";
  uint16_t ucs2;
  resetUTF8decoder();
  for (int i=0; i<s.length(); i++) {
    ucs2 = recode(s.charAt(i));

    //dbg:: Serial.printf("s[%d]=0x%02x -> 0x%04x\n", i, (int) s.charAt(i), ucs2);    

    if (0x20 <= ucs2 && ucs2 <= 0x7F)
      r += (char) ucs2;
    else if (0xA0 <= ucs2 && ucs2 <= 0xFF)
      r += (char) (ucs2 - 32);
    else if (showUnmapped && 0xFF < ucs2 && ucs2 < 0xFFFF)
      r += (char) 0x7F;        
  }
  return r;
}


// In place conversion of a UTF8 string to extended ASCII string (ASCII is shorter!)
void utf8tocp(char* s) {      
  int k = 0;
  uint16_t ucs2;
  resetUTF8decoder();
  for (int i=0; i<strlen(s); i++) {
    ucs2 = recode(s[i]);

    //D/ Serial.printf("s[%d]=0x%02x -> 0x%04x\n", i, s[i], ucs2 );    

    if (0x20 <= ucs2 && ucs2 <= 0x7F) {
      s[k++] = (char) ucs2;
      //D/Serial.printf("  > s[%d] = %02x (<7f)\n", k-1, s[k-1] );
    } else if (0xA0 <= ucs2 && ucs2 <= 0xFF) {
      s[k++] = (char) (ucs2 - 32);  
      //D/Serial.printf("  > s[%d] = %02x (a0-ff)\n", k-1, s[k-1] );
    } else if (showUnmapped && 0xFF < ucs2 && ucs2 < 0xFFFF) {
      s[k++] = (char) 127;    
      //D/Serial.printf("  > s[%d] = %02x (x)\n", k-1, s[k-1] );
    }
  }
  s[k]=0;
}

