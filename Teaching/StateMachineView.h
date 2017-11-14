#ifndef TEACHING_STATEMACHINE_VIEW_H_INCLUDED
#define TEACHING_STATEMACHINE_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"
#include "CommandDefTypes.h"
#include "ParameterView.h"
#include "ActivityEditor.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class StateMachineViewImpl : public QWidget {
  Q_OBJECT
public:
  StateMachineViewImpl(QWidget* parent = 0);
  inline void setParameterView(ParameterView* view) { this->parameterView_ = view; }

  void setTaskParam(TaskModelParamPtr param);
  void clearTaskParam();
  void createStateCommands();

  void setBPStatus(bool isActive, bool isSet);
  void setStepStatus(bool isActive);
  void setButtonEnableMode(bool isEnable);

public Q_SLOTS:
  void editClicked();

// R.Hanai
#ifdef __TASK_PARAM_ADJUSTER
  void trainClicked();
#endif
// R.Hanai

private Q_SLOTS :
  void setClicked();
  void modeChanged();
  void deleteClicked();
  void runClicked();
  void stepClicked();
  void contClicked();
  void bpToggled();

private:
  QLabel* lblTarget;
  QPushButton* btnTrans;
  ItemList* lstItem;

  QFrame* frmGuard;
  QRadioButton* rdTrue;
  QRadioButton* rdFalse;
  QPushButton* btnSet;
  QPushButton* btnDelete;
  QPushButton* btnEdit;

// R.Hanai
  QPushButton* btnTrain;
  template<class T> void samplePoses(std::vector<T>& samples);
  template<class T> void samplePosesRandom(std::vector<T>& samples);
// R.Hanai

  QPushButton* btnRun;
  QPushButton* btnBP;
  QPushButton* btnStep;
  QPushButton* btnCont;

  ActivityEditor* grhStateMachine;
  vector<CommandDefParam*> commandList_;

  ParameterView* parameterView_;

  bool isExec_;
  void createCommandNodeTarget(int id, QString name);
};

class StateMachineView : public cnoid::View {
public:
  StateMachineView();
  ~StateMachineView();
  inline void setParameterView(ParameterView* view) { viewImpl->setParameterView(view); }

  void setTaskParam(TaskModelParamPtr param) { this->viewImpl->setTaskParam(param); }
  void clearTaskParam() { this->viewImpl->clearTaskParam(); }

  void setStepStatus(bool isActive) { this->viewImpl->setStepStatus(isActive); }
  inline void setButtonEnableMode(bool isEnable) { viewImpl->setButtonEnableMode(isEnable); }

private:

  StateMachineViewImpl* viewImpl;
};

}
#endif
