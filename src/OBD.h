#ifndef OBD_H
#define OBD_H

typedef struct{
	char label[255];
	byte pid;
	int fr_max;
}PID_INFO;

int GetOBDInfo(uint8_t, int*);
int set_obd(void);
void init_OBD_dev(void);
void ErrorOBDCheck(void);
void CheckObdSpeed(char* ,char* ,int , int* );
int OBD_looping(PID_INFO* , int,int);

#endif