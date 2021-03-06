#ifndef TEACHING_TASK_INSTANCE_VIEW_H_INCLUDED
#define TEACHING_TASK_INSTANCE_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "TeachingTypes.h"

#include "SettingDialog.h"

using namespace std;

namespace teaching {

class TaskInstanceViewImpl : public QWidget {
  Q_OBJECT
public:
  TaskInstanceViewImpl(QWidget* parent = 0);
  ~TaskInstanceViewImpl();

  void loadTaskInfo();
  void setButtonEnableMode(bool isEnable);

	void showGrid(const vector<TaskModelParamPtr>& taskList);
	void updateGrid(TaskModelParamPtr& target);
  void setEditMode(bool canEdit);
  void setExecState(bool isActive);
  void clearSelection();


private Q_SLOTS:
  void taskSelectionChanged();
  void searchClicked();
	void modelMasterClicked();
	void settingClicked();
	void realClicked();

  void runTaskClicked();
  void loadTaskClicked();
  void outputTaskClicked();
  void deleteTaskClicked();
  void registNewTaskClicked();
  void registTaskClicked();
  void abortClicked();
  void initPosClicked();

  void widgetClose();

private:
  QPushButton* btnModelMaster;
  QPushButton* btnSetting;
  QCheckBox* chkReal;

  QLineEdit* leCond;
  QLineEdit* leTask;
  SearchList* lstResult;

  QPushButton* btnRunTask;
  QPushButton* btnInitPos;
  QPushButton* btnLoadTask;
  QPushButton* btnOutputTask;
  QPushButton* btnDeleteTask;
  QPushButton* btnRegistNewTask;
  QPushButton* btnRegistTask;
  QPushButton* btnAbort;

  int currentTaskIndex_;

  bool isSkip_;

  int getSelectedId();
};

class TaskInstanceView : public cnoid::View {
public:
  TaskInstanceView();
  ~TaskInstanceView();

private:
  TaskInstanceViewImpl* viewImpl;
};

}
#endif
