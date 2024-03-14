#include "fonts.h"

#include "FrederickatheGreat-Regular40pt8b.h"
#include "PragatiNarrow-Regular16pt8b.h"
#include "PragatiNarrow-Regular20pt8b.h"
#include "icons.h"

const GFXfont * fnt_hodiny() {
    return &FrederickatheGreat_Regular40pt8b;
}  

const GFXfont * fnt_icons() {
    return &Icons;
}  

const GFXfont * fnt_text1() {
    return &PragatiNarrow_Regular16pt8b;
}  

const GFXfont * fnt_text2() {
    return &PragatiNarrow_Regular20pt8b;
}  
