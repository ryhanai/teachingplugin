#ifndef TEACHING_SEARCH_VIEW_H_INCLUDED
#define TEACHING_SEARCH_VIEW_H_INCLUDED

#include <sstream> 
#include <cnoid/View>
#include <QObject>
#include <QtGui>
#include "DataBaseManager.h"

#include "FlowView.h"
#include "MetaDataView.h"

#include <cnoid/BodyItem>
#include <cnoid/RootItem>

#include "SettingDialog.h"
#include "TaskExecutionView.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class FlowView;

class TaskInstanceViewImpl : public TaskExecutionView {
  Q_OBJECT
public:
  TaskInstanceViewImpl(QWidget* parent = 0);

  inline void setFlowView(FlowView* view) { this->flowView_ = view; }
  inline void setMetadataView(MetaDataView* view) { this->metadataView_ = view; }

private Q_SLOTS:
  void taskSelectionChanged();
  void searchClicked();
  void settingClicked();

  void runTaskClicked();
  void loadTaskClicked();
  void outputTaskClicked();
  void deleteTaskClicked();
  void registNewTaskClicked();
  void registTaskClicked();
  void initPosClicked();

  void widgetClose();

private:
  QLineEdit* leCond;
  SearchList* lstResult;
  QCheckBox* chkReal;

  int currentTaskIndex_;

  bool isSkip_;
  vector<TaskModelParam*> taskList_;

  FlowView* flowView_;
  MetaDataView* metadataView_;

  void showGrid();
  void updateCurrentInfo();

  BodyItemPtr m_item_;
};

class TaskInstanceView : public cnoid::View {
public:
    TaskInstanceView();
    ~TaskInstanceView();

  inline void setFlowView(FlowView* view) { viewImpl->setFlowView(view); }
  inline void setMetadataView(MetaDataView* view ) { viewImpl->setMetadataView(view); }
  inline void setParameterView(ParameterView* view ) { viewImpl->setParameterView(view); }
  inline void setStateMachineView(StateMachineView* view ) { viewImpl->setStateMachineView(view); }
  inline void unloadCurrentModel() { viewImpl->unloadCurrentModel(); }

private:
  TaskInstanceViewImpl* viewImpl;
};

}
#endif
