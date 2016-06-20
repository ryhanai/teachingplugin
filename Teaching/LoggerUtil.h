#ifndef TEACHING_LOGGER_UTIL_H_INCLUDED
#define TEACHING_LOGGER_UTIL_H_INCLUDED

#include <iostream>
#include <string>

using namespace std;

namespace teaching {

class LoggerUtil {
public:
  static void writeLog(const string contents);
  static void writeLogWithArgs(const char *format, ...);

private:
  LoggerUtil();
  ~LoggerUtil();

};

#define LOG_OUT

#ifdef LOG_OUT
#define DDEBUG(s) { \
  LoggerUtil::writeLog(s); \
}
#define DDEBUG_V(...) { \
  LoggerUtil::writeLogWithArgs(__VA_ARGS__); \
}
#else
#define DDEBUG(s)
#define DDEBUG_V(...)
#endif

}
#endif
