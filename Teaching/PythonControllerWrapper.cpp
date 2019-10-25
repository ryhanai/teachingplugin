#include "PythonControllerWrapper.h"

#include <cnoid/PluginManager>
#include <cnoid/PythonExecutor>

#include "LoggerUtil.h"

namespace teaching {

PythonControllerWrapper* PythonControllerWrapper::instance () {
  static PythonControllerWrapper* c = new PythonControllerWrapper();
  return c;
}

std::vector<CommandDefParam*> PythonControllerWrapper::getCommandDefList() {
  DDEBUG("PythonControllerWrapper::getCommandDefList");
  std::vector<CommandDefParam*> result;

  PythonExecutor executor;
  bool ret = executor.execCode("test()");
  if(ret) {
    DDEBUG("OK");
    //python::object result = executor.resultObject();
    std::string result = executor.resultString();
    DDEBUG("1");
    //int value = result.cast<int>();
    //char* value = python::cast<char*>(result);
    DDEBUG_V("result %s", result.c_str());
  }

  DDEBUG("End");
  return result;
}

bool PythonControllerWrapper::executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params, bool isReal) {
  return true;
}

void PythonControllerWrapper::initialize() {
}

cnoid::Link* PythonControllerWrapper::getToolLink(int toolNumber) {
  return 0;
}

}
