
/**
Replacement for original Arduino's map() working with doubles, and with better respect for limits.
*/
double map_double(double x, double in_min, double in_max, double out_min, double out_max) {
    if( x>in_max ) return out_max;
    if( x<in_min ) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

