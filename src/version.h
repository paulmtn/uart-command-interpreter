#include <stdint.h>

// we separate fireware_version_t into another file in case we every want to use versioning for any other system but do not want the logging functions. Of course, I wont really be doing particularly complicated things so this could just live in log.h
typedef struct firmware_version {
    uint8_t major;
    uint8_t minor;
    uint16_t build;
} firmware_version_t;
