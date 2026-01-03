#define MINIAUDIO_IMPLEMENTATION
//#define MA_DEBUG_OUTPUT
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_CUSTOM 
// https://miniaud.io/docs/manual/index.html#Definitions

#define STB_VORBIS_HEADER_ONLY 
#include "stb_vorbis.c"

#include "miniaudio.h"
#undef STB_VORBIS_HEADER_ONLY 
