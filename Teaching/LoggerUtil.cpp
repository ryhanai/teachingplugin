#include "LoggerUtil.h"
#include <cnoid/MessageView>  /* modified by qtconv.rb 0th rule*/  

using namespace cnoid;

namespace teaching {

LoggerUtil::LoggerUtil() {
}

LoggerUtil::~LoggerUtil() {
}

void LoggerUtil::writeLog(const string contents) {
  MessageView::mainInstance()->cout() << contents << endl;
}

void LoggerUtil::writeLogWithArgs(const char *format, ...) {
  va_list argp;
  va_start(argp, format);
  char allocatedBuffer[1024];
  int size = vsprintf(allocatedBuffer, format, argp);
  va_end(argp);

  MessageView::mainInstance()->cout() << allocatedBuffer << endl;
};

}
