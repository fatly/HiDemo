#ifndef __LIBPITCH_SHIFT_CONFIG_H__
#define __LIBPITCH_SHIFT_CONFIG_H__

//16-bits sample type
#define INTEGER_SAMPLES		1
#define INTERPOLATE_LINEAR  1

//32-bits sample type
//#define FLOAT_SAMPLES		0

#if defined(FLOAT_SAMPLES) && defined(INTERPOLATE_LINEAR)
#define INTERPOLATE_CUBIC	1
#endif

#endif