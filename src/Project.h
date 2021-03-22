#ifndef Project_H
#define Project_H


#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <SPI.h>
#include <FreematicsPlus.h>
#include <FreematicsGPS.h>
#include <httpd.h>
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "telelogger.h"
#include "telemesh.h"
#include "teleclient.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/list.h>
#include <freertos/queue.h>
#include <freertos/portmacro.h>
#include <freertos/semphr.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define SLEEP_STOP 0
#define time_size 100
#define USE_OBD 1
#define USE_SERIAL 1
#define MAX_OBD_ERRORS 3 // maximum consecutive OBD access errors before entering standby
#define USE_SERIAL 1
#define buff_SIZE 500
#define NBR_GPS_POINTS 1

#endif


