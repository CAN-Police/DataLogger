#include "Project.h"

FreematicsESP32* p_sys;
static SemaphoreHandle_t xSemaphore_Time=NULL;

time_t StringToDatetime(char *str) 
{ 
  tm tm_;
  int year, month, day, hour, minute,second; 
  sscanf(str,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second); 
  tm_.tm_year = year-1900; 
  tm_.tm_mon  = month-1; 
  tm_.tm_mday = day; 
  tm_.tm_hour = hour; 
  tm_.tm_min  = minute; 
  tm_.tm_sec  = second; 
  tm_.tm_isdst = 0; 
 
  time_t t_ = mktime(&tm_); // Has been lost 8 A time zone  
  return t_; // A second time  
} 

void UpdateTime(void){
	GPS_DATA* gd = 0;
	unsigned int GPS_Year;
	unsigned int GPS_Month;
	unsigned int GPS_Day;
	unsigned int GPS_Hour;
	unsigned int GPS_Minutes;
	unsigned int GPS_Seconds;
	unsigned int GPS_mSeconds;
	
	(*p_sys).getCurrentUTC(&gd);
	
	GPS_Year= (unsigned int) (gd->date % 100) + 2000;
	GPS_Month= (unsigned int) (gd->date / 100) % 100;
	GPS_Day= (unsigned int) (gd->date / 10000);
	GPS_Hour = (unsigned int) gd->time / 1000000;
	GPS_Minutes = (unsigned int)(gd->time % 1000000) / 10000;
	GPS_Seconds = (unsigned int)(gd->time % 10000) / 100;
	GPS_mSeconds = (gd->time % 100)/10;
	
	char isoTime[100]={0};
	sprintf(isoTime,"%04u-%02u-%02u %02u:%02u:%02u",\
			GPS_Year,\
			GPS_Month,\
			GPS_Day,\
			GPS_Hour,\
			GPS_Minutes,\
			GPS_Seconds);
	time_t t_=StringToDatetime(isoTime);
	struct timeval tv;
	tv.tv_sec=t_;
	tv.tv_usec=GPS_mSeconds*100000;
	settimeofday(&tv,NULL);
}

void SetSysInstance(FreematicsESP32* sys){
	p_sys=(FreematicsESP32*)sys;
	xSemaphore_Time= xSemaphoreCreateMutex();
	#if USE_SERIAL==1
		Serial.println("Time is set correctly");
	#endif
}

void GetCurrentTime(char* isoTime){
	xSemaphoreTake(xSemaphore_Time,portMAX_DELAY);
	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
	sprintf(isoTime,"%s.%06ld", tmbuf, tv.tv_usec);
	xSemaphoreGive(xSemaphore_Time);
}


void GetCurrentGPSTime(char* isoGPSTime){
	GPS_DATA* gd = 0;
	char* p;
	unsigned int GPS_Year;
	unsigned int GPS_Month;
	unsigned int GPS_Day;
	unsigned int GPS_Hour;
	unsigned int GPS_Minutes;
	unsigned int GPS_Seconds;
	unsigned int GPS_mSeconds;
	
	(*p_sys).getCurrentUTC(&gd);
	
	GPS_Year= (unsigned int) (gd->date % 100) + 2000;
	GPS_Month= (unsigned int) (gd->date / 100) % 100;
	GPS_Day= (unsigned int) (gd->date / 10000);
	GPS_Hour = (unsigned int) gd->time / 1000000;
	GPS_Minutes = (unsigned int)(gd->time % 1000000) / 10000;
	GPS_Seconds = (unsigned int)(gd->time % 10000) / 100;
	GPS_mSeconds = (gd->time % 100)/10;
	p=isoGPSTime;
	p+=sprintf(isoGPSTime,"%04u-%02u-%02u %02u:%02u:%02u",\
			GPS_Year,\
			GPS_Month,\
			GPS_Day,\
			GPS_Hour,\
			GPS_Minutes,\
			GPS_Seconds);
	sprintf(p,".%06u", GPS_mSeconds*100000);
}

int GetShiftTime(char* tm1,char* tm2){
	time_t t1=StringToDatetime(tm1);
	time_t t2=StringToDatetime(tm2);
	return t2-t1;
}