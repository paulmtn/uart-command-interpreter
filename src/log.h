#include "version.h"   // needed because log_version takes this type

enum e_log_subsystem {
    TEST
};

enum e_log_level {
    INFO,
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL,
};

void log_write(enum e_log_subsystem sys, enum e_log_level level, const char *msg);
void log_num(enum e_log_subsystem sys, enum e_log_level level, const char *msg, int num);
void log_set_output_level(enum e_log_subsystem sys, enum e_log_level level);
void log_global_on(void);
void log_global_off(void);
void log_version(const firmware_version_t *v);
