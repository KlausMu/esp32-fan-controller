// #define MY_LOG_FORMAT(format) "%lu ms: " format "\r\n", millis()
#define MY_LOG_FORMAT(format) "%lu ms: " format, millis()

#include "Arduino.h"

#ifndef _LOGSTREAMCLASS_H_
#define _LOGSTREAMCLASS_H_

class LogStreamClass : public Stream {
public:
  LogStreamClass(void);

  // Stream implementation
  int read();
  int available();
  int peek();
  #ifdef ESP32
  void flush();
  #endif

  // Print implementation
  virtual size_t write(uint8_t val);
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write; // pull in write(str) and write(buf, size) from Print

  size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));
};

extern LogStreamClass Log;

void doLog(void);
#endif
