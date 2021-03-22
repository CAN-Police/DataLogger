#include "GPS.h"
#include "MEMS.h"
#include "SD_custom.h"
#include "OBD.h"
#include "Project.h"
#include "DateTime.h"

FreematicsESP32 sys;

RTC_DATA_ATTR MEMS_dev md_dev;

#define STACK_SIZE 1024 // in words (= 4*STACK_SIZE bytes)

//queue where the data will be send to the daemon task
static QueueHandle_t xQueue_print=NULL;

//daemon task
void vTask_Transmit(void *);

//clear a char buffer with certain length
void clr_buff(char*,uint32_t);

//appending char string to CSV file
void print_data(char*,uint32_t);

//task writing GPS Data 
void write_task_1(void*);

//task collecting MEMS data
void write_task_2(void*);

//task writing OBD data which frequency is 1 sample per minute
void write_task_3(void*);


//mapping each PID of OBD
PID_INFO obdData[]={
	{"PID_SPEED",PID_SPEED,1},
	{"PID_AMBIENT_TEMP",PID_AMBIENT_TEMP,0},
	{"PID_RPM",PID_RPM,1},
	{"PID_THROTTLE",PID_THROTTLE,1},
	{"PID_RUNTIME",PID_RUNTIME,0},
	{"PID_BAROMETRIC",PID_BAROMETRIC,0},
	{"PID_DISTANCE",PID_DISTANCE,0},
	{"PID_HYBRID_BATTERY_PERCENTAGE",PID_HYBRID_BATTERY_PERCENTAGE,0}
};

//message structure
typedef struct{
	char* data;
	int len;
	SemaphoreHandle_t completion;
}Irp;

int nbr_elem= sizeof(obdData)/sizeof(PID_INFO);


void setup()
{

 	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, HIGH);
	delay(1000);
	digitalWrite(PIN_LED, LOW);
	Serial.begin(115200);

	//initializations
	while(!sys.begin());

	//initialize GPS device
	init_GPS_dev();

	//initialize SD_Card
  	init_SD_Card(obdData,nbr_elem);
  
  	//initialize MEMS device
  	init_MEMS_dev();

	//initialize OBD device
  	init_OBD_dev();

	//turn on buzzer at 2000Hz frequency 
	sys.buzzer(2000);
	delay(300);
	//turn off buzzer
	sys.buzzer(0);
  
  	//creating queue with a size (25*sizeof(Irp*))
	xQueue_print = xQueueCreate(25,sizeof(Irp*));

	xTaskCreate(write_task_1,"GPS Data Write",STACK_SIZE*5,(void*) 0,tskIDLE_PRIORITY+1,NULL);

	xTaskCreate(write_task_2,"MEMS Data Write",STACK_SIZE*4,(void*) 0,tskIDLE_PRIORITY+2,NULL);

	xTaskCreate(write_task_3,"OBD Data Write",STACK_SIZE*3,(void*) 0,tskIDLE_PRIORITY+1,NULL);

  //create daemon task
	xTaskCreate(vTask_Transmit,"vTask_Transmit",STACK_SIZE*50,(void*) 0,tskIDLE_PRIORITY+3,NULL);
}


void loop()
{

  float motion = 0;
  for (byte n = 0; n < 10; n++) 
  {
    md_dev.mems->read(md_dev.acc);
    for (byte i = 0; i < 3; i++) 
    {
      float m = (md_dev.acc[i] - md_dev.accBias[i]);
      motion += m * m;
      break;
    }
  }

 if (motion > MOTION_THRESHOLD * MOTION_THRESHOLD) 
  {
    Serial.print("\nMotion:");
    Serial.println(motion);
	MEMSData();
	vTaskDelay(1000);
  }
}

void print_data(char* data, uint32_t len){
	//dynamic allocation of SendItem and filling its fields
		Irp* SendItem=(Irp*)pvPortMalloc(sizeof(Irp));
		char* data_aux =(char*) pvPortMalloc(len*sizeof(char));
		for (int i=0;i<len;i++){
			data_aux[i]=data[i];
		}
		SendItem->data=data_aux;
		SendItem->len=len;
		SendItem->completion=xSemaphoreCreateBinary();

	//sending the adress of the pointer SendItem via xQueue_print
		xQueueSend(xQueue_print,&SendItem,portMAX_DELAY);

	//passive wait for the end of transmission
		xSemaphoreTake(SendItem->completion, portMAX_DELAY);

	//free the memory pointed by SendItem
		vSemaphoreDelete(SendItem->completion); 
		vPortFree(SendItem->data);
		vPortFree(SendItem);
}

void write_task_1(void* parameter)
{
  	char buff[buff_SIZE];
  	char isoTime[time_size]={0};
	uint32_t nbr;
	int i;
	char* p;
	
	while(1){
	clr_buff(isoTime,time_size);
	nbr=0;
	p=buff;
	AddGPSData(isoTime,p,&i);
	p+=i;
	nbr+=i;

	sprintf(p,"\n");
    print_data(buff,nbr+1);
	vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelete(NULL); 
}

void write_task_2(void* parameter){
	char isoTime[time_size]={0};
	char buff[buff_SIZE];
	uint32_t nbr=0;
	int i;
	char* p=0;

	while(1){
		
		clr_buff(isoTime,time_size);
		nbr=0;
		p=buff;
		AddMEMSData(isoTime,p,&i);
		p+=i;
		nbr+=i;
		
		sprintf(p,"\n");
      	print_data(buff,nbr+1);
		vTaskDelay(200);
	}
	vTaskDelete(NULL); 

}

void write_task_3(void* parameter)
{
	int sleep_tme;
	while(1){
		sleep_tme=OBD_looping(obdData,nbr_elem,0);
		taskYIELD();
		ErrorOBDCheck();
		vTaskDelay(pdMS_TO_TICKS(sleep_tme));
	}
	vTaskDelete(NULL);
}


void vTask_Transmit(void *pvParameters){
	Irp* ReceiveItem;
 	char buff[buff_SIZE];
	char* p;
	int i=0;
	for(;;){
		clr_buff(buff,buff_SIZE);
		//receiving ReceiveItem via xQueue_print
		xQueueReceive(xQueue_print,&ReceiveItem,portMAX_DELAY);
			p=buff;

			for(i=0;i<ReceiveItem->len;i++)
				*p++=ReceiveItem->data[i];

		//sending  data via USART
		#if USE_SERIAL==1
			Serial.println(buff);
		#endif

		//writing into SD card
		appendFile(SD, getFileName(),buff);

		//free the semaphore
		xSemaphoreGive(ReceiveItem->completion);			
		vTaskDelay(pdMS_TO_TICKS(20));
	}

}
  
void clr_buff(char* buf,uint32_t len){
	for(int i=0;i<len;i++)
		buf[i]='\000';
}

