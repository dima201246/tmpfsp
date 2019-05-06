#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "include/logging.h"

char out_buf[650];
char const *path_logfile = NULL;

char *genLogLine(const char *log_str) {
	time_t temp_time;
	struct tm* time_now;

	temp_time = time(NULL);
	time_now = localtime(&temp_time);

	snprintf(out_buf, 650, "[%d.%.2d.%.2d %.2d:%.2d:%.2d] %s", (time_now->tm_year + 1900), (time_now->tm_mon + 1), time_now->tm_mday,\
		time_now->tm_hour, time_now->tm_min, time_now->tm_sec, log_str);

	return out_buf;
}

void setLogFile(const char * path_to_logfile) {
	path_logfile = path_to_logfile;
}

void logMsg(const char *fmt, ...) {
	char str_buf[512];
	FILE *log_file;

	va_list ap;
	va_start(ap, fmt);
	vsprintf(str_buf, fmt, ap);
	va_end(ap);

	printf("%s", genLogLine(str_buf));

	if ((path_logfile != NULL) && ((log_file = fopen(path_logfile, "a")) != NULL))
	{
		fprintf(log_file, "%s", genLogLine(str_buf));
		fclose(log_file);
	}
}

void logInFile(const char *fmt, ...) {
	FILE *log_file;
	char str_buf[512];

	va_list ap;
	va_start(ap, fmt);
	vsprintf(str_buf, fmt, ap);
	va_end(ap);

	if ((path_logfile != NULL) && (log_file = fopen(path_logfile, "a")) != NULL)
	{
		fprintf(log_file, "%s", genLogLine(str_buf));
		fclose(log_file);
	}

	return;
}
