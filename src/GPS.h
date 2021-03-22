#ifndef GPS_H
#define GPS_H

//initialize GPS device
void init_GPS_dev(void);

//check for GPS Data
int isDataGPSReady(int);

//return -1 if GPS coordinates are incorrect accroding to speed
int FilterGpsData(char*, double*);

int AddGPSData(char*,char*,int*);

#endif