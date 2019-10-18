#ifndef TEACHING_TRAJECTORY_VIEW_H_INCLUDED
#define TEACHING_TRAJECTORY_VIEW_H_INCLUDED

#include <cnoid/View>
#include <string>
#include "QtUtil.h"
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class ViaPointParam {
public:
  ViaPointParam(double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz, double time)
    : posX_(posX), posY_(posY), posZ_(posZ), rotR_(rotRx), rotP_(rotRy), rotY_(rotRz), time_(time) {
  };
  ViaPointParam(const ViaPointParam* source)
    : posX_(source->posX_), posY_(source->posY_), posZ_(source->posZ_),
    rotR_(source->rotR_), rotP_(source->rotP_), rotY_(source->rotY_),
    time_(source->time_) {};

  inline double getPosX() const { return this->posX_; }
  inline void setPosX(double value) { this->posX_ = value; }

  inline double getPosY() const { return this->posY_; }
  inline void setPosY(double value) { this->posY_ = value; }

  inline double getPosZ() const { return this->posZ_; }
  inline void setPosZ(double value) { this->posZ_ = value; }

  inline double getRotR() const { return this->rotR_; }
  inline void setRotR(double value) { this->rotR_ = value; }

  inline double getRotP() const { return this->rotP_; }
  inline void setRotP(double value) { this->rotP_ = value; }

  inline double getRotY() const { return this->rotY_; }
  inline void setRotY(double value) { this->rotY_ = value; }

  inline double getTime() const { return this->time_; }
  inline void setTime(double value) { this->time_ = value; }

  inline std::vector<double> getTransMat() const { return this->transMat_; }
  inline void addTransMat(double target){ this->transMat_.push_back(target); }
  inline void clearTransMat(){ this->transMat_.clear(); }


private:
  double posX_, posY_, posZ_;
  double rotR_, rotP_, rotY_;
  double time_;
  std::vector<double> transMat_;
};
typedef std::shared_ptr<ViaPointParam> ViaPointParamPtr;

class TrajectoryViewImpl : public QWidget {
  Q_OBJECT
public:
  TrajectoryViewImpl(QWidget* parent = 0);
  ~TrajectoryViewImpl();

  void setEditMode(bool canEdit);

private Q_SLOTS:
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

  QPushButton* btnAdd;
  QPushButton* btnUpdate;
  QPushButton* btnDelete;
  QPushButton* btnUp;
  QPushButton* btnDown;

  BodyItem* subObjItem_;
  BodyItem* mainObjItem_;
  Link* subObjLink_;
  Link* mainObjLink_;

  std::vector<ViaPointParamPtr> postureList_;
  bool isSkip_;
  void showPostureGrid();

};

class TrajectoryView : public cnoid::View {
public:
  TrajectoryView();
  ~TrajectoryView();

private:
  TrajectoryViewImpl* viewImpl;
};

}
#endif
