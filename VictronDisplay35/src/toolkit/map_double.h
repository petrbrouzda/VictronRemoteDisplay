#ifndef __TOOLKIT_map_double_H___
#define __TOOLKIT_map_double_H___

/**
Replacement for original Arduino's map() working with doubles, and with better respect for limits.
*/
double map_double(double x, double in_min, double in_max, double out_min, double out_max);

#endif