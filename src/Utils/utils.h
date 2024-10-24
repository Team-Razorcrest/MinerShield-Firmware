#ifndef __UTILS_H__
#define __UTILS_H__

#include "constants.h"
#include <Arduino.h>
#include "structs.h"

void init_error_mechanism();
status_t show_error();
bool readFallDetection();
int readMethane();
void readDHT(int arr[2]);

#endif // __UTILS_H__