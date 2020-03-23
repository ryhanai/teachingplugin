#ifndef TEACHING_PYTHON_CONTOROLLER_WAPPER_H_INCLUDED
#define TEACHING_PYTHON_CONTOROLLER_WAPPER_H_INCLUDED

#include "ControllerBase.h"
#include "ArgumentEstimator.h"

using namespace std;

namespace teaching {

class PythonControllerWrapper : public ControllerBase {
public:
  static PythonControllerWrapper* instance ();

  PythonControllerWrapper() {};
  ~PythonControllerWrapper() {};

  std::vector<CommandDefParam*> getCommandDefList();
  bool executeCommand(const std::string& commandName, TaskModelParamPtr taskParam, ElementStmParamPtr targetParam, bool isReal = false) override;

  void initialize();
  cnoid::Link* getToolLink(int toolNumber);
};

}
#endif
