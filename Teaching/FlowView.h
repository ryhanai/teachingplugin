#ifndef TEACHING_FLOW_VIEW_H_INCLUDED
#define TEACHING_FLOW_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "DataBaseManager.h"
#include "MetaDataView.h"
#include "TaskInstanceView.h"
#include "StateMachineView.h"

#include "TaskExecutionView.h"
#include "FlowActivityEditor.h"

using namespace cnoid;

namespace teaching {

class TaskInstanceView;

class FlowViewImpl : public TaskExecutionView {
  Q_OBJECT
public:
  FlowViewImpl(QWidget* parent = 0);

  inline void setMetadataView(MetaDataView* view) { this->metadataView = view; }
  inline void setTaskInstanceView(TaskInstanceView* view) { this->taskInstView = view; }

  inline FlowParam* getCurrentFlow() { return this->currentFlow_; }

	void flowSelectionChanged(TaskModelParam* target);

	private Q_SLOTS:
	void modeChanged();

	void searchClicked();
  void newFlowClicked();
  void registFlowClicked();
  void deleteTaskClicked();
  void runFlowClicked();
  void runTaskClicked();
  void initPosClicked();

private:
  QLineEdit* leName;
  QLineEdit* leComment;

  QPushButton* btnRegistFlow;
  QPushButton* btnDeleteTask;
  QPushButton* btnRunFlow;
  QPushButton* btnRunTask;
  QPushButton* btnInitPos;

	QPushButton* btnTrans;
	ItemList* lstItem;
	//QFrame* frmGuard;
	//QRadioButton* rdTrue;
	//QRadioButton* rdFalse;
	//QPushButton* btnSet;

	FlowActivityEditor* grhStateMachine;

	MetaDataView* metadataView;
  TaskInstanceView* taskInstView;

  FlowParam* currentFlow_;

  void changeEnables(bool value);
};

class FlowView : public cnoid::View {
public:
    FlowView();
    ~FlowView();

  inline void setMetadataView(MetaDataView* view ) { viewImpl->setMetadataView(view); }
  inline void setParameterView(ParameterView* view ) { viewImpl->setParameterView(view); }
  inline void setStateMachineView(StateMachineView* view ) { viewImpl->setStateMachineView(view); }
  inline void setTaskInstanceView(TaskInstanceView* view ) { viewImpl->setTaskInstanceView(view); }

	inline void setTaskExecutor(TaskExecuteManager* executor) { viewImpl->setTaskExecutor(executor); }
	inline void unloadCurrentModel() { viewImpl->unloadCurrentModel(); }

  inline FlowParam* getCurrentFlow() { return viewImpl->getCurrentFlow(); }

private:
  FlowViewImpl* viewImpl;
};

}
#endif
