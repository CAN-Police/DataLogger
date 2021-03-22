#include "Project.h"
#include "SD_custom.h"

RTC_DATA_ATTR char FileName[255];

extern void clr_buff(char*,uint32_t);

char* getFileName(void){
	return FileName;
}

int getFileID(File& root)
{
        if (root) {
            File file;
            int id = 0;
            while(file = root.openNextFile()) {
		#if USE_SERIAL==1
                	Serial.println(file.name());
		#endif
                if (!strncmp(file.name(), "/DATA/", 6)) {
                    unsigned int n = atoi(file.name() + 6);
                    if (n > id) id = n;
                }
            }
            return id + 1;
        } 
    
        else 
        {
            return 1;
        }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
	#if USE_SERIAL==1
		Serial.printf("writing in the file... %s\n", path);
	#endif
	File file = fs.open(path, FILE_APPEND);
	if (!file) {
		#if USE_SERIAL==1
			Serial.println("file can not be opened");
		#endif
		return;
	}
	if (file.print(message)) {
		#if USE_SERIAL==1
			Serial.println("file flushed successfully");
		#endif
	} else {
		#if USE_SERIAL==1
			Serial.println("error while modifying file");
		#endif
	}
	file.close();
}

void init_SD_Card(PID_INFO* obdData,int nbr_elem)
{
  SPI.begin();
	
	//connect pin cs of the reader to the pin 5 of GPIO
	if (!SD.begin(5)) {
		#if USE_SERIAL==1
			Serial.println("SD card can not be connected");
		#endif
		
		delay(5000);
		ESP.restart();
	}
	#if USE_SERIAL==1
		Serial.println("SD card was connected successfully");
	#endif
	
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
	
	if (wakeup_reason!=ESP_SLEEP_WAKEUP_TIMER){
		File root = SD.open("/DATA");
		int m_id = getFileID(root);
		SD.mkdir("/DATA");
		sprintf(FileName, "/DATA/%u.CSV", m_id);
		
		char labels[buff_SIZE];
		clr_buff(labels,buff_SIZE);
		sprintf(labels,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",\
						"TIME_STAMP",\
						"GPS_TIME_STAMP",\
						"GPS_ALTITUDE",\
						"GPS_LONGITUDE",\
						"GPS_LATITUDE",\
						"GPS_SLOPE",\
						"GPS_SPEED",\
						"ACC_X,ACC_Y,ACC_Z",\
						"GYR_X,GYR_Y,GYR_Z",\
						"MAG_X,MAG_Y,MAG_Z"\
			);
		appendFile(SD, getFileName(), labels);

		PID_INFO* p=obdData;
		for (int i=0; i<nbr_elem; i++){
			clr_buff(labels,buff_SIZE);
			sprintf(labels,"%s,",p->label);
			p++;
			appendFile(SD, getFileName(), labels);
		}
		clr_buff(labels,buff_SIZE);
		
		sprintf(labels,"%s","\n");
		appendFile(SD, getFileName(), labels);
	}
	#if USE_SERIAL==1
		Serial.print("file path in the SD card: ");
		Serial.println(FileName);
	#endif
}

