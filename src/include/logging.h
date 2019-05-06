#ifndef _LOGGINNG_H
#define _LOGGINNG_H
	void setLogFile(const char * path_to_logfile);
	void logMsg(const char *fmt, ...);
	void logInFile(const char *fmt, ...);
#endif
