#ifndef __UTILS_H__
#define __UTILS_H__

#include <Arduino.h>
#include "structs.h"
#include "constants.h"

void init_error_mechanism();
status_t show_error();

#endif // __UTILS_H__