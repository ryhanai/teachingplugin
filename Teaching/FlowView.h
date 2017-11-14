#ifndef TEACHING_FLOW_VIEW_H_INCLUDED
#define TEACHING_FLOW_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "TeachingTypes.h"

#include "StateMachineView.h"
#include "FlowActivityEditor.h"

using namespace cnoid;

namespace teaching {

class TaskInstanceView;


class TaskInfoDialog : public QDialog {
  Q_OBJECT
public:
  TaskInfoDialog(TaskModelParamPtr param, ElementNode* elem, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit* txtName;

	TaskModelParamPtr targetTask_;
  ElementNode* targetElem_;
};

class FlowViewImpl : public QWidget {
  Q_OBJECT
public:
  FlowViewImpl(QWidget* parent = 0);

  void flowSelectionChanged(TaskModelParamPtr target);
  void setButtonEnableMode(bool isEnable);
	void clearView();
	void dispView(FlowParamPtr& target);
	void createStateMachine(FlowParamPtr& target);

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
  void importFlowClicked();

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
  QPushButton* btnImport;
  QPushButton* btnAbort;

  QPushButton* btnTrans;
  ItemList* lstItem;
  //QFrame* frmGuard;
  //QRadioButton* rdTrue;
  //QRadioButton* rdFalse;
  //QPushButton* btnSet;

  FlowActivityEditor* grhStateMachine;

  void changeEnables(bool value);
};

class FlowView : public cnoid::View {
public:
  FlowView();
  ~FlowView();

  inline void setButtonEnableMode(bool isEnable) { viewImpl->setButtonEnableMode(isEnable); }

private:
  FlowViewImpl* viewImpl;
};

}
#endif
