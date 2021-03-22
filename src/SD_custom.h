#include "OBD.h"

#ifndef SD_custom_H
#define SD_custom_H


int getFileID(File& root);
void appendFile(fs::FS &fs, const char * path, const char * message);
void init_SD_Card(PID_INFO*,int);
char* getFileName(void);

#endif