#ifndef TEACHING_FLOW_VIEW_H_INCLUDED
#define TEACHING_FLOW_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "DataBaseManager.h"
#include "MetaDataView.h"
#include "TaskInstanceView.h"
#include "TaskExecutionView.h"

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

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

private Q_SLOTS:
  void flowSelectionChanged();
  void searchClicked();
  void newFlowClicked();
  void registFlowClicked();
  void deleteTaskClicked();
  void upTaskClicked();
  void downTaskClicked();
  void runFlowClicked();
  void runTaskClicked();
  void initPosClicked();

private:
  QLineEdit* leName;
  QLineEdit* leComment;
  QListWidget* lstFlow;

  QPushButton* btnRegistFlow;
  QPushButton* btnDeleteTask;
  QPushButton* btnUpTask;
  QPushButton* btnDownTask;
  QPushButton* btnRunFlow;
  QPushButton* btnRunTask;
  QPushButton* btnInitPos;

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
  inline void unloadCurrentModel() { viewImpl->unloadCurrentModel(); }

  inline FlowParam* getCurrentFlow() { return viewImpl->getCurrentFlow(); }

private:
  FlowViewImpl* viewImpl;
};

}
#endif
