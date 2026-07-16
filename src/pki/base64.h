#ifndef BASE64_H
#define BASE64_H

#include "../other/globals.h"
#include <stdbool.h>

//lenInput should not include a null terminator
String base64Encode(String inp,bool nullTerminatorOutput);
//lenData should not include null terminator
String base64Decode(String inp,bool nullTerminatorOutput);


#endif