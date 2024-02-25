// This is the basic config file that defines which
// LILYGO product you are compiling for

#define LILYGO_WATCH_2020_V3              // Use T-Watch2020
#define LILYGO_WATCH_LVGL 


// Except T-Watch2020, other versions need to be selected according to the actual situation
#if  !defined(LILYGO_WATCH_2020_V1) && !defined(LILYGO_WATCH_2020_V3)

// T-Watch comes with the default backplane, it uses internal DAC
#define STANDARD_BACKPLANE

// Such as MAX98357A, PCM5102 external DAC backplane
// #define EXTERNAL_DAC_BACKPLANE

#else
// T-Watch2020 uses external DAC
#undef STANDARD_BACKPLANE
#define EXTERNAL_DAC_BACKPLANE

// TWATCH 2020 V3 PDM microphone pin
#define MIC_DATA            2
#define MIC_CLOCK           0
#define BUFFER_SIZE (2*1024)

#endif

#include <LilyGoWatch.h>
