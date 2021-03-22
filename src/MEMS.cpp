#include "Project.h"
#include "MEMS.h"

extern void GetCurrentTime(char* isoTime);

RTC_DATA_ATTR MEMS_dev md_dev;

void calibrateMEMS()
{
    // MEMS data collected while sleeping
    float temp=0;
	while (!((md_dev.mems)->read(md_dev.accBias, md_dev.gyrBias, md_dev.magBias, &temp))){}
	#if USE_SERIAL==1
		Serial.println("MEMS is calibrated successfully");
	#endif
}

void MEMSData(void)
{
	float temp=0;
	if (((md_dev.mems)->read(md_dev.acc, md_dev.gyr, md_dev.mag, &temp)))
    {
		for (int i=0; i<3; i++){
			md_dev.acc[i]-=md_dev.accBias[i];
			md_dev.gyr[i]-=md_dev.gyrBias[i];
			md_dev.mag[i]-=md_dev.magBias[i];
		}
	}

   	Serial.print("ACC:");
	for (int i=0; i<3; i++){
    Serial.print(md_dev.acc[i]);
    Serial.print('/');}

	Serial.print("\nGYR:");
	for (int i=0; i<3; i++){
    Serial.print(md_dev.gyr[i]);
    Serial.print('/');}

	Serial.print("\nMAG:");
	for (int i=0; i<3; i++){
    Serial.print(md_dev.mag[i]);
	Serial.print('/');}
}

void init_MEMS_dev(void)
{
	#if USE_SERIAL==1
		Serial.print("MEMS:");
	#endif
	md_dev.mems = new MPU9250;
	byte ret = (md_dev.mems)->begin(ENABLE_ORIENTATION);

	if (ret) 
	{
		#if USE_SERIAL==1
			Serial.println("MPU-9250");
		#endif
	} 
	
	else {
		(md_dev.mems)->end();
		delete md_dev.mems;
		md_dev.mems = new ICM_20948_I2C;
		ret = (md_dev.mems)->begin(ENABLE_ORIENTATION);
		if (ret) {
			#if USE_SERIAL==1
				Serial.println("ICM-20948");
			#endif
		} 
		
		else
		{
			#if USE_SERIAL==1
				Serial.println("NO");
			#endif
			while(1);
		}
	}
	
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
	if (wakeup_reason!=ESP_SLEEP_WAKEUP_TIMER)
	{calibrateMEMS();}
}


void AddMEMSData(char* isoTime,char* p,int* nbr){
	*nbr=0;
	int i;
	
	GetCurrentTime(isoTime);
	i=sprintf(p,"%s,,,,,,",isoTime);
		
	i= sprintf(p,",%f,%f,%f,%f,%f,%f,%f,%f,%f\n",\
		md_dev.acc[0],md_dev.acc[1],md_dev.acc[2],\
		md_dev.gyr[0],md_dev.gyrBias[1],md_dev.gyr[2],\
		md_dev.magBias[0],md_dev.mag[1],md_dev.mag[2]\
		);
		*nbr+=i;
		p+=i;
	}
	