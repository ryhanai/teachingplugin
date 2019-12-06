#ifndef TEACHING_TRAJECTORY_VIEW_H_INCLUDED
#define TEACHING_TRAJECTORY_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class TrajectoryViewImpl : public QWidget {
  Q_OBJECT
public:
  TrajectoryViewImpl(QWidget* parent = 0);
  ~TrajectoryViewImpl();

	void setTaskParam(TaskModelParamPtr param);
  void setEditMode(bool canEdit);

private Q_SLOTS:
  void addTrajClicked();
  void deleteTrajClicked();
  void trajectorySelectionChanged();
  void trajectoryItemEdited(QTableWidgetItem *item);

  void subObjClicked();
  void mainObjClicked();
  void subLinkClicked();
  void mainLinkClicked();
  void addClicked();
  void deleteClicked();
  void updateClicked();
  void upClicked();
  void downClicked();
  void itemEdited(QTableWidgetItem *item);
  void postureSelectionChanged();

private:
  QPushButton* btnSubObj;
  QLineEdit* leSubObj;
  QPushButton* btnMainObj;
  QLineEdit* leMainObj;

  QPushButton* btnSubLink;
  QLineEdit* leSubLink;
  QPushButton* btnMainLink;
  QLineEdit* leMainLink;

  QTableWidget* lstTrajectory;
  QTableWidget* lstViaPoint;

  QPushButton* btnAddTraj;
  QPushButton* btnDeleteTraj;

  QPushButton* btnAdd;
  QPushButton* btnUpdate;
  QPushButton* btnDelete;
  QPushButton* btnUp;
  QPushButton* btnDown;

  TaskModelParamPtr targetTask_;
  TaskTrajectoryParamPtr targetTrajectory_;

  bool isSkip_;
  void showTrajectoryGrid();
  void showPostureGrid();

};

class TrajectoryView : public cnoid::View {
public:
  TrajectoryView();
  ~TrajectoryView();

private:
  TrajectoryViewImpl* viewImpl;
};

struct TrajectryComparator {
	int id_;
	TrajectryComparator(int value) {
		id_ = value;
	}
	bool operator()(const TaskTrajectoryParamPtr elem) const {
		return elem->getId() == id_;
	}
};

struct ViaPointComparator {
	int id_;
	ViaPointComparator(int value) {
		id_ = value;
	}
	bool operator()(const ViaPointParamPtr elem) const {
		return elem->getId() == id_;
	}
};

}
#endif
