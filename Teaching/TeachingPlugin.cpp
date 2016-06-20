#include <cnoid/Plugin>  /* modified by qtconv.rb 0th rule*/  
#include "TaskInstanceView.h"
#include "MetaDataView.h"
#include "FlowView.h"
#include "ParameterView.h"
#include "StateMachineView.h"

using namespace std;
using namespace boost;
using namespace cnoid;
using namespace teaching;


class TeachingPlugin : public cnoid::Plugin {

private:
  bool onTimeout() {
    return true;
  }

public:
  TeachingPlugin() : Plugin("Teaching") { 
//    depend("Trajectory");
  }
  
  virtual bool initialize() {
    MetaDataView* metadataView = new MetaDataView();
    ParameterView* parameterView = new ParameterView();
    StateMachineView* statemachineView = new StateMachineView();
    FlowView* flowView = new FlowView();
    TaskInstanceView* taskView = new TaskInstanceView();

    flowView->setMetadataView(metadataView);
    flowView->setParameterView(parameterView);
    flowView->setStateMachineView(statemachineView);
    flowView->setTaskInstanceView(taskView);

    taskView->setFlowView(flowView);
    taskView->setMetadataView(metadataView);
    taskView->setParameterView(parameterView);
    taskView->setStateMachineView(statemachineView);

    addView(taskView);
    addView(metadataView);
    addView(flowView);
    addView(parameterView);
    addView(statemachineView);
    return true;
  }
};


CNOID_IMPLEMENT_PLUGIN_ENTRY(TeachingPlugin);
