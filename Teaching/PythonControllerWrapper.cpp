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
    python::object result = executor.resultObject();
    int value = python::cast<int>(result);
    DDEBUG_V("result %d", value);
  }

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
