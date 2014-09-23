#include "log.h"
#include "display.h"
#include <stdarg.h>

#define LOG "../infect.log"
#define MAX 5000

//logs a message to a log file.

void logMsg(const char* format, ...)
{
	FILE *file = NULL;
	file = fopen(LOG, "a");
	
	if (file == NULL) {
		printError("Could not open log file\n");
	} 
        
        else {
		char buffer[MAX];

		va_list args;
		va_start(args,format);

		vsprintf(buffer,format,args);

		fputs(buffer,file);
		fclose(file);

		va_end(args);
	}
	return;
}

