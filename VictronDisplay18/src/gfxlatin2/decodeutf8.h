#ifndef DECODEUTF8_H
#define DECODEUTF8_H

#include "stdint.h"

// If this macro is defined, the UTF-8 decoder will skip the following 
// 3 bytes when it encounters the first byte of a 4 byte encoding
//#define SKIP_4_BYTE_ENCODINGS

// Replace code points not found in the GFX Latin 1 font with 0x7F 
extern bool showUnmapped;

// To reset the decodeUTF8 state machine. Could also parse a single ASCII 
// character as in decodeUTFU(' ').
void resetUTF8decoder(void);

// Returns Unicode code point in the 0 - 0xFFFE range.  0xFFFF is used to signal 
// that more bytes are needed to complete decoding a multi-byte UTF-8 encoding
//   
// This is just a serial decoder, it does not check to see if the code point is 
// actually assigned to a character in Unicode.
//
uint16_t decodeUTF8(uint8_t c);

#endif