#include "log.h"

// basic log
void log_write(enum e_log_subsystem sys, enum e_log_level level, const char *msg);

// log with num
void log_num(enum e_log_subsystem sys, enum e_log_level level, const char *msg, int num);

// sets some sys to another log level. 
// for example, if we set a bunch of logs to DEBUG, we can to try 
// to find a bug, we can change one subsystem to only show errors to
// keep the other messages.
void log_set_output_level(enum e_log_subsystem sys, enum e_log_level level);

// disable and enable the logger incase of timing errors caused by the logger
void log_global_on();
void log_global_off();
