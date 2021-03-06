/*************************************************************************
* Base class for Freematics telematics products
* Distributed under BSD license
* Visit http://freematics.com/products/freematics-one for more information
* (C)2017-18 Stanley Huang <support@freematics.com.au>
*************************************************************************/

#ifndef FREEMATICS_BASE
#define FREEMATICS_BASE

#include <Arduino.h>

// non-OBD/custom PIDs (no mode number)
#define PID_GPS_LATITUDE 0xA
#define PID_GPS_LONGITUDE 0xB
#define PID_GPS_ALTITUDE 0xC
#define PID_GPS_SPEED 0xD
#define PID_GPS_HEADING 0xE
#define PID_GPS_SAT_COUNT 0xF
#define PID_GPS_TIME 0x10
#define PID_GPS_DATE 0x11
#define PID_ACC 0x20
#define PID_GYRO 0x21
#define PID_COMPASS 0x22
#define PID_MEMS_TEMP 0x23
#define PID_BATTERY_VOLTAGE 0x24
#define PID_ORIENTATION 0x25

// custom PIDs for calculated data
#define PID_TRIP_DISTANCE 0x30
#define PID_DATA_SIZE 0x80
#define PID_CSQ 0x81
#define PID_DEVICE_TEMP 0x82
#define PID_DEVICE_HALL 0x83

typedef struct {
    uint32_t date;
    uint32_t time;
    int32_t lat;
    int32_t lng;
    int16_t alt; /* meter */
    uint16_t speed; /* 1/100 Knot */
    int16_t heading;
    uint8_t sat;
} GPS_DATA;

class CFreematics
{
public:
	virtual byte begin() { return 0; }
	// start xBee UART communication
	virtual bool xbBegin(unsigned long baudrate = 115200L) = 0;
	// read data to xBee UART
	virtual int xbRead(char* buffer, int bufsize) = 0;
	// send data to xBee UART
	virtual void xbWrite(const char* cmd) = 0;
	// receive data from xBee UART (returns 0/1/2)
	virtual int xbReceive(char* buffer, int bufsize, unsigned int timeout = 1000, const char** expected = 0, byte expectedCount = 0) = 0;
	// purge xBee UART buffer
	virtual void xbPurge() = 0;
	// toggle xBee module power
	virtual void xbTogglePower() = 0;
};

#endif
