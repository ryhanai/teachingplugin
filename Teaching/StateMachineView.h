#ifndef TEACHING_STATEMACHINE_VIEW_H_INCLUDED
#define TEACHING_STATEMACHINE_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"
#include "CommandDefTypes.h"
#include "ParameterView.h"

#include "NodeEditor/StateMachineEditor.hpp"
#include "NodeEditor/DataModelRegistry.hpp"


using namespace cnoid;
using namespace std;
using namespace QtNodes;

namespace teaching {
class ItemList : public QListWidget {
public:
	ItemList(QString elemType, QWidget* parent = 0);

	void createInitialNodeTarget();
	void createFinalNodeTarget();
	void createDecisionNodeTarget();

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

private:
	QPoint startPos;
	QString elemType_;
};
//////////
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

	inline void updateTargetParam() { grhStateMachine->updateTargetParam(); };

public Q_SLOTS:
  void editClicked();

// R.Hanai
#ifdef __TASK_PARAM_ADJUSTER
  void trainClicked();
#endif
// R.Hanai

private Q_SLOTS :
  void deleteClicked();
  void runClicked();
  void stepClicked();
  void contClicked();
  void bpToggled();

private:
  QLabel* lblTarget;
  ItemList* lstItem;

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

	StateMachineEditor* grhStateMachine;
  vector<CommandDefParam*> commandList_;

  ParameterView* parameterView_;

  bool isExec_;
  void createCommandNodeTarget(int id, QString name);

	void setStyle();
	std::shared_ptr<DataModelRegistry> registerDataModels();
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
	inline void updateTargetParam() { viewImpl->updateTargetParam(); }

private:
  StateMachineViewImpl* viewImpl;
};

}
#endif
