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

  void loadTaskInfo();
  void setButtonEnableMode(bool isEnable);

	void showGrid(vector<TaskModelParamPtr>& taskList);
	void updateGrid(TaskModelParamPtr& target);
  void setEditMode(bool canEdit);


private Q_SLOTS:
  void taskSelectionChanged();
  void searchClicked();
	void modelMasterClicked();
	void settingClicked();

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
};

class TaskInstanceView : public cnoid::View {
public:
  TaskInstanceView();
  ~TaskInstanceView();

  inline void setButtonEnableMode(bool isEnable) { viewImpl->setButtonEnableMode(isEnable); }


private:
  TaskInstanceViewImpl* viewImpl;
};

}
#endif
