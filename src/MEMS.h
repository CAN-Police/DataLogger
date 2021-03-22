#ifndef MEMS_H
#define customized_MEMS_H

#define ENABLE_ORIENTATION 0
#ifndef ENABLE_MEMS
#define ENABLE_MEMS 1
#endif

#define MOTION_THRESHOLD 0.5 /* G */

typedef struct
{
	MEMS_I2C* mems;
	float acc[3];
	float gyr[3];
	float mag[3];
	float accBias[3];
	float gyrBias[3];
	float magBias[3];

}MEMS_dev;


void calibrateMEMS();
void MEMSData(void);
void init_MEMS_dev(void);
void AddMEMSData(char*,char* ,int*);

#endif