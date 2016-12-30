#include <cnoid/Plugin>
#include <cnoid/ViewManager>

#include "TaskInstanceView.h"
#include "MetaDataView.h"
#include "FlowView.h"
#include "ParameterView.h"
#include "StateMachineView.h"
#include "TaskExecuteManager.h"

#include <cnoid/MessageView>

using namespace cnoid;
using namespace boost;
using namespace teaching;

class TeachingPlugin : public Plugin {

public:
  TeachingPlugin() : Plugin("Teaching") { 
  }
  
  virtual bool initialize() {
		viewManager().registerClass<MetaDataView>("MetaDataView", "MetaData", ViewManager::SINGLE_DEFAULT);
		viewManager().registerClass<ParameterView>("ParameterView", "Parameter", ViewManager::SINGLE_DEFAULT);
		viewManager().registerClass<FlowView>("FlowView", "FlowModel", ViewManager::SINGLE_DEFAULT);
		viewManager().registerClass<TaskInstanceView>("TaskInstanceView", "TaskInstance", ViewManager::SINGLE_DEFAULT);
		viewManager().registerClass<StateMachineView>("StateMachineView", "StateMachine", ViewManager::SINGLE_DEFAULT);

		MetaDataView* metadataView = viewManager().findView<MetaDataView>("MetaData");
		ParameterView* parameterView = viewManager().findView<ParameterView>("Parameter");
		FlowView* flowView = viewManager().findView<FlowView>("FlowModel");
		TaskInstanceView* taskView = viewManager().findView<TaskInstanceView>("TaskInstance");
		StateMachineView* statemachineView = viewManager().findView<StateMachineView>("StateMachine");
		//
    flowView->setMetadataView(metadataView);
    flowView->setParameterView(parameterView);
    flowView->setStateMachineView(statemachineView);
    flowView->setTaskInstanceView(taskView);

    taskView->setFlowView(flowView);
    taskView->setMetadataView(metadataView);
    taskView->setParameterView(parameterView);
    taskView->setStateMachineView(statemachineView);

		statemachineView->setParameterView(parameterView);

		TaskExecuteManager* executor = new TaskExecuteManager();
		executor->setTaskInstanceView(taskView);
		executor->setFlowView(flowView);
		executor->setParameterView(parameterView);
		executor->setStateMachineView(statemachineView);
		executor->setMetadataView(metadataView);

		flowView->setTaskExecutor(executor);
		taskView->setTaskExecutor(executor);
		statemachineView->setTaskExecutor(executor);

		metadataView->bringToFront();

		return true;
  }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(TeachingPlugin);
