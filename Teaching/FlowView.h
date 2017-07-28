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


class TaskInfoDialog : public QDialog {
  Q_OBJECT
public:
  TaskInfoDialog(TaskModelParam* param, ElementNode* elem, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit* txtName;

  TaskModelParam* targetTask_;
  ElementNode* targetElem_;
};

class FlowViewImpl : public TaskExecutionView {
  Q_OBJECT
public:
  FlowViewImpl(QWidget* parent = 0);

  inline void setMetadataView(MetaDataView* view) { this->metadataView = view; }
  inline void setTaskInstanceView(TaskInstanceView* view) { this->taskInstView = view; }

  inline FlowParam* getCurrentFlow() { return this->currentFlow_; }

  void flowSelectionChanged(TaskModelParam* target);
  void setButtonEnableMode(bool isEnable);

public Q_SLOTS:
  void editClicked();

private Q_SLOTS :
  void modeChanged();

  void searchClicked();
  void newFlowClicked();
  void registFlowClicked();
  void deleteTaskClicked();
  void runFlowClicked();
  void runTaskClicked();
  void abortClicked();
  void initPosClicked();
  void exportFlowClicked();

private:
  QLineEdit* leName;
  QLineEdit* leComment;

  QPushButton* btnSearch;
  QPushButton* btnNewFlow;
  QPushButton* btnRegistFlow;

  QPushButton* btnDeleteTask;
  QPushButton* btnRunFlow;
  QPushButton* btnRunTask;
  QPushButton* btnInitPos;
  QPushButton* btnEdit;
  QPushButton* btnExport;
  QPushButton* btnAbort;

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

  inline void setMetadataView(MetaDataView* view) { viewImpl->setMetadataView(view); }
  inline void setParameterView(ParameterView* view) { viewImpl->setParameterView(view); }
  inline void setStateMachineView(StateMachineView* view) { viewImpl->setStateMachineView(view); }
  inline void setTaskInstanceView(TaskInstanceView* view) { viewImpl->setTaskInstanceView(view); }

  inline void setTaskExecutor(TaskExecuteManager* executor) { viewImpl->setTaskExecutor(executor); }
  inline void unloadCurrentModel() { viewImpl->unloadCurrentModel(); }

  inline FlowParam* getCurrentFlow() { return viewImpl->getCurrentFlow(); }
  inline void setButtonEnableMode(bool isEnable) { viewImpl->setButtonEnableMode(isEnable); }

private:
  FlowViewImpl* viewImpl;
};

}
#endif
