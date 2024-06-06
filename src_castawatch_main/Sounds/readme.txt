http://tomeko.net/online_tools/file_to_hex.php?lang=en

Most online converters change the format, but I found a simple one:

// We need this header file to use FLASH as storage with PROGMEM directive
#include <pgmspace.h>
// The following online tool can be used to convert a jpeg to an array:
// http://tomeko.net/online_tools/file_to_hex.php?lang=en
// Copy and paste the output from the online window to the array below.

const uint8_t jpeg_image[] PROGMEM = {
// paste your output here

};