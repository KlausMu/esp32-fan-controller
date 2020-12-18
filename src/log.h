#include "esp32-hal.h"
// #define MY_LOG_FORMAT(format) "%u ms [%s:%u] %s(): " format "\r\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__
#define MY_LOG_FORMAT(format) "%u ms: " format "\r\n", millis()

void doLog(void);