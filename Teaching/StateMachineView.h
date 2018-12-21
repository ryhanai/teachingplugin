#ifndef TEACHING_STATEMACHINE_VIEW_H_INCLUDED
#define TEACHING_STATEMACHINE_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"
#include "CommandDefTypes.h"

#include "NodeEditor/StateMachineEditor.hpp"
#include "NodeEditor/DataModelRegistry.hpp"


using namespace cnoid;
using namespace std;
using namespace QtNodes;

namespace teaching {
class ExecEnvDialog : public QDialog {
  Q_OBJECT
public:
  ExecEnvDialog(TaskModelParamPtr param, QWidget* parent = 0);

private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTextEdit* txtEnv;

	TaskModelParamPtr targetTask_;
};

class ItemList : public QListWidget {
public:
	ItemList(QString elemType, QWidget* parent = 0);

	void createInitialNodeTarget();
	void createFinalNodeTarget();
	void createDecisionNodeTarget();
  void createMergeNodeTarget();

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
  ~StateMachineViewImpl();

  void setTaskParam(TaskModelParamPtr param);
  void clearTaskParam();
  void createStateCommands();

  void setBPStatus(bool isActive, bool isSet);
  void setStepStatus(bool isActive);

	inline void updateTargetParam() { grhStateMachine->updateTargetParam(); };
  void setEditMode(bool canEdit);
  void setExecState(bool isActive);

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
  QFrame* frmItem_;

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

  bool isExec_;
  void createCommandNodeTarget(QString cmdName, QString dispName);

	void setStyle();
	std::shared_ptr<DataModelRegistry> registerDataModels();
};

class StateMachineView : public cnoid::View {
public:
  StateMachineView();
  ~StateMachineView();

  void setTaskParam(TaskModelParamPtr param) { this->viewImpl->setTaskParam(param); }
  void setStepStatus(bool isActive) { this->viewImpl->setStepStatus(isActive); }

private:
  StateMachineViewImpl* viewImpl;
};

}
#endif
