#ifndef TEACHING_TEACHING_CONTROLLERMANAGER_H_INCLUDED
#define TEACHING_TEACHING_CONTROLLERMANAGER_H_INCLUDED

#include <vector>
#include "ControllerBase.h"
#include "StateMachineView.h"
#include "TaskInstanceView.h"

using namespace std;

namespace teaching {

struct ControllerInfo {
  string controllerName;
  ControllerBase* controller;
};


class CNOID_EXPORT ControllerManager {
public:
  static ControllerManager* instance();
  void registController(string controllerName, ControllerBase* controller);
  ControllerBase* getController(string controllerName);

  ~ControllerManager();

  void setStateMachineView(StateMachineViewImpl* view);
  void setTaskInstanceView(TaskInstanceViewImpl* view);
  vector<string> getControllerNameList();
  void initialize();

  bool isExistController();

private:
  ControllerManager();

  vector<ControllerInfo> controllerList_;
  StateMachineViewImpl* stateView_;
  TaskInstanceViewImpl* taskView_;

};

}
#endif
