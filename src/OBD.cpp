#include "Project.h"
#include "OBD.h"


COBD obd;
FreematicsESP32 sys;
extern void print_data(char* data, uint32_t len);
extern void clr_buff(char* buf,uint32_t len);
extern void GetCurrentTime(char* isoTime);
extern int GetShiftTime(char* tm1,char* tm2);

char vin[18] = {0};
uint16_t dtc[6] = {0};

static SemaphoreHandle_t xSemaphore_err=NULL;

//Semaphore to synchronize access to ODB
static SemaphoreHandle_t xSemaphore_OBD=NULL;


int GetOBDInfo(uint8_t pid, int* val){
	int aux;
	if (obd.isValidPID(pid)){
		if (obd.readPID(pid,aux)){
			*val=aux;
			return 1;
		}
	}
	return 0;
}

int set_obd(void){
	int res=0;

	if (!obd.init()) {
		#if USE_SERIAL==1
			Serial.println("OBD is not detected");
		#endif
	} else {
		#if USE_SERIAL==1
			Serial.println("OBD was successfully set !");
		#endif
		res=1;
	}
	
	char buf[128];
	if (obd.getVIN(buf, sizeof(buf))) {
		strncpy(vin, buf, sizeof(vin) - 1);
		#if USE_SERIAL==1
			Serial.print("VIN:");
			Serial.println(vin);
		#endif
	}
	int dtcCount = obd.readDTC(dtc, sizeof(dtc) / sizeof(dtc[0]));
	if (dtcCount > 0) {
		#if USE_SERIAL==1
			Serial.print("DTC:");
			Serial.println(dtcCount);
		#endif
	}
	return res;
}

void init_OBD_dev(void){
	xSemaphore_err= xSemaphoreCreateMutex();
	//creating mutex to synchronze the access to OBD
	xSemaphore_OBD= xSemaphoreCreateMutex();
	obd.begin(sys.link);
	int cnt=0;
	while(!set_obd()){
		cnt++;
		delay(200);
		#if USE_OBD==0
			break;
		#endif

		if (cnt==5){
			esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
			esp_deep_sleep_start();
		}
	}

}

void ErrorOBDCheck(void){
	xSemaphoreTake(xSemaphore_err,portMAX_DELAY);
	if (obd.errors >= MAX_OBD_ERRORS) {
		#if USE_SERIAL==1
			Serial.println("ECU OFF");
		#endif
		obd.reset();
		int cnt=0;
		while(!set_obd()){
			#if USE_OBD==0
				break;
			#endif
			if (cnt==5){
				esp_sleep_enable_timer_wakeup(2 * uS_TO_S_FACTOR);
				esp_deep_sleep_start();
			}
			cnt++;
		}
	}
	xSemaphoreGive(xSemaphore_err);

}

void CheckObdSpeed(char* label,char* isoTime,int val, int* res){
	static char lastTimeStamp[time_size]={0};
	static int meanSpeed=0;
	int j;
	
	if (!strcmp(label,"PID_SPEED")){
		if (lastTimeStamp[0]){
			if (GetShiftTime(lastTimeStamp,isoTime)>3){
				if (abs(val-meanSpeed)<5){
					if (val<2){
						#if STOP_SLEEP==1
							esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
							esp_deep_sleep_start();
						#endif
					}
					*res=100;
				}else{
					meanSpeed=val;
					for (j=0;j<time_size;j++){
						lastTimeStamp[j]=isoTime[j];
					}
				}
			}else{
				meanSpeed+=val;
				meanSpeed/=2;
			}
		}else{
			meanSpeed=val;
			for (j=0;j<time_size;j++){
				lastTimeStamp[j]=isoTime[j];
			}
		}
	}
}


int OBD_looping(PID_INFO* obdData, int nbr_elem,int fr_max){
	char buff[buff_SIZE];
	char* p;
	uint32_t nbr;
	char isoTime[time_size]={0};
	int i;
	int j;
	int val;
	int OBD_flag;
	int res=90;
	
	if (!fr_max)
		res=60000;

	clr_buff(buff,buff_SIZE);
	p=buff;
	OBD_flag=0;
	nbr=0;
	clr_buff(isoTime,time_size);
	
	xSemaphoreTake(xSemaphore_OBD,portMAX_DELAY);

	GetCurrentTime(isoTime);
	j=sprintf(p,"%s,,,,,,,,,,,,,,,",isoTime);
	nbr+=j;
	p+=j;
	
	for(i=0; i<nbr_elem;i++){
		if (obdData[i].fr_max==fr_max){
			if(GetOBDInfo(obdData[i].pid, &val)){
				OBD_flag=1;
				j=sprintf(p,",%d",val);
				nbr+=j;
				p+=j;
				CheckObdSpeed(obdData[i].label,isoTime,val, &res);
			}else{
				j=sprintf(p,",");
				nbr+=j;
				p+=j;
			}
		}else{
			j=sprintf(p,",");
			nbr+=j;
			p+=j;
		}
		
	}

	xSemaphoreGive(xSemaphore_OBD);
	if (OBD_flag){
		j=sprintf(p,"\n");
		nbr+=j;
		p+=j;
		if (res!=100)
			print_data(buff,nbr);
	}
	return res;
	
}


