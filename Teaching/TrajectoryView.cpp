#include "TrajectoryView.h"
#include <cnoid/UTF8>
#include <cnoid/ViewManager>
#include <cnoid/ItemTreeView>
#include <cnoid/LinkSelectionView>

#include "TeachingEventHandler.h"
#include "TeachingUtil.h"
//
#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

TrajectoryViewImpl::TrajectoryViewImpl(QWidget* parent)
  : subObjItem_(0), mainObjItem_(0), subObjLink_(0), mainObjLink_(0),
    isSkip_(false), QWidget(parent) {
  QFrame* settingFrame = new QFrame;

  btnSubObj = new QPushButton("Base Obj.");
  //btnSubObj->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
  //btnSubObj->setToolTip(_("Delete selected element"));
  btnSubObj->setEnabled(false);

  leSubObj = new QLineEdit;
  leSubObj->setReadOnly(true);
  leSubObj->setEnabled(false);

  btnMainObj = new QPushButton("Target Obj.");
  //btnMainObj->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
  //btnMainObj->setToolTip(_("Delete selected element"));
  btnMainObj->setEnabled(false);

  leMainObj = new QLineEdit;
  leMainObj->setReadOnly(true);
  leMainObj->setEnabled(false);
  /////
  btnSubLink = new QPushButton("Basse Link");
  //btnSubLink->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
  //btnSubLink->setToolTip(_("Delete selected element"));
  btnSubLink->setEnabled(false);

  leSubLink = new QLineEdit;
  leSubLink->setReadOnly(true);
  leSubLink->setEnabled(false);

  btnMainLink = new QPushButton("Target Link");
  //btnMainLink->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
  //btnMainLink->setToolTip(_("Delete selected element"));
  btnMainLink->setEnabled(false);

  leMainLink = new QLineEdit;
  leMainLink->setReadOnly(true);
  leMainLink->setEnabled(false);

  QGridLayout* settingLayout = new QGridLayout;
  settingFrame->setLayout(settingLayout);
  settingLayout->addWidget(btnSubObj, 0, 0, 1, 1);
  settingLayout->addWidget(leSubObj, 0, 1, 1, 1);
  settingLayout->addWidget(btnMainObj, 0, 2, 1, 1);
  settingLayout->addWidget(leMainObj, 0, 3, 1, 1);

  settingLayout->addWidget(btnSubLink, 1, 0, 1, 1);
  settingLayout->addWidget(leSubLink, 1, 1, 1, 1);
  settingLayout->addWidget(btnMainLink, 1, 2, 1, 1);
  settingLayout->addWidget(leMainLink, 1, 3, 1, 1);
  //
  lstTrajectory = UIUtil::makeTableWidget(7, true, true);
  lstTrajectory->setColumnWidth(0, 80);
  lstTrajectory->setColumnWidth(1, 80);
  lstTrajectory->setColumnWidth(2, 80);
  lstTrajectory->setColumnWidth(3, 80);
  lstTrajectory->setColumnWidth(4, 80);
  lstTrajectory->setColumnWidth(5, 80);
  lstTrajectory->setColumnWidth(6, 80);
  lstTrajectory->setHorizontalHeaderLabels(QStringList() << "X" << "Y" << "Z" << "R" << "P" << "Y" << "Time");
  //
  btnAdd = new QPushButton(_("Add"));
  btnUpdate = new QPushButton(_("Upd"));
  btnDelete = new QPushButton(_("Del"));
  btnUp = new QPushButton(_("Up"));
  btnDown = new QPushButton(_("Down"));

  QFrame* frmButton = new QFrame;
	QVBoxLayout* buttonLayout = new QVBoxLayout(frmButton);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
	buttonLayout->addWidget(btnAdd);
	buttonLayout->addWidget(btnUpdate);
	buttonLayout->addWidget(btnDelete);
	buttonLayout->addStretch();
	buttonLayout->addWidget(btnUp);
	buttonLayout->addWidget(btnDown);

  QFrame* frmList = new QFrame;
	QHBoxLayout* listLayout = new QHBoxLayout(frmList);
  listLayout->setContentsMargins(2, 2, 2, 2);
	listLayout->addWidget(lstTrajectory);
	listLayout->addWidget(frmButton);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(settingFrame);
  mainLayout->addWidget(frmList);
  setLayout(mainLayout);
  //
  connect(btnSubObj, SIGNAL(clicked()), this, SLOT(subObjClicked()));
  connect(btnMainObj, SIGNAL(clicked()), this, SLOT(mainObjClicked()));
  connect(btnSubLink, SIGNAL(clicked()), this, SLOT(subLinkClicked()));
  connect(btnMainLink, SIGNAL(clicked()), this, SLOT(mainLinkClicked()));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(upClicked()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(downClicked()));
  connect(lstTrajectory, SIGNAL(itemSelectionChanged()), this, SLOT(postureSelectionChanged()));
  connect(lstTrajectory, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemEdited(QTableWidgetItem*)));

  showPostureGrid();
	TeachingEventHandler::instance()->trv_Loaded(this);
}

TrajectoryViewImpl::~TrajectoryViewImpl() {
}

void TrajectoryViewImpl::showPostureGrid() {
  isSkip_ = true;
	lstTrajectory->setRowCount(0);

	for (ViaPointParamPtr param : postureList_) {
		int row = lstTrajectory->rowCount();
		lstTrajectory->insertRow(row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 0, QString::number(param->getPosX(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 1, QString::number(param->getPosY(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 2, QString::number(param->getPosZ(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 3, QString::number(param->getRotR(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 4, QString::number(param->getRotP(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 5, QString::number(param->getRotY(), 'f', 4), row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 6, QString::number(param->getTime(), 'f', 4), row);
	}
  isSkip_ = false;
}

void TrajectoryViewImpl::subObjClicked() {
  Item* selectedItem = ItemTreeView::mainInstance()->selectedItem<Item>();
  if (!selectedItem) return;
  subObjItem_ = (BodyItem*)selectedItem;
  if (!subObjItem_) return;
  leSubObj->setText(QString::fromStdString(subObjItem_->name()));
  leSubLink->setText("");
}

void TrajectoryViewImpl::mainObjClicked() {
  Item* selectedItem = ItemTreeView::mainInstance()->selectedItem<Item>();
  if (!selectedItem) return;
  mainObjItem_ = (BodyItem*)selectedItem;
  if (!mainObjItem_) return;
  leMainObj->setText(QString::fromStdString(mainObjItem_->name()));
  leMainLink->setText("");
}

void TrajectoryViewImpl::subLinkClicked() {
  if (!subObjItem_) return;
  int linkIndex = LinkSelectionView::mainInstance()->selectedLinkIndex();
  if (linkIndex < 0) return;
  subObjLink_ = subObjItem_->body()->link(linkIndex);
  if (!subObjLink_) return;
  leSubLink->setText(QString::fromStdString(subObjLink_->name()));
}

void TrajectoryViewImpl::mainLinkClicked() {
  if (!mainObjItem_) return;
  int linkIndex = LinkSelectionView::mainInstance()->selectedLinkIndex();
  if( 0<= linkIndex ) {
    mainObjLink_ = mainObjItem_->body()->link(linkIndex);
    if(mainObjLink_) {
      leMainLink->setText(QString::fromStdString(mainObjLink_->name()));
    }
  }
}

void TrajectoryViewImpl::addClicked() {
  if (!subObjItem_ || !mainObjItem_ || !mainObjLink_ || !subObjLink_) return;

  Position relPos = subObjLink_->position().inverse()*mainObjLink_->position();

  double posX = relPos.translation()[0];
  double posY = relPos.translation()[1];
  double posZ = relPos.translation()[2];

  const Matrix3 R = relPos.rotation();
  const Vector3 rpy = rpyFromRot(R);
  double rotR = degree(rpy[0]);
  double rotP = degree(rpy[1]);
  double rotY = degree(rpy[2]);

  ViaPointParamPtr newPos = std::make_shared<ViaPointParam>(posX, posY, posZ, rotR, rotP, rotY, 0.0);
  for (int index = 0; index < 12; index++) {
    newPos->addTransMat(relPos.data()[index]);
  }

  postureList_.push_back(newPos);

  btnSubObj->setEnabled(false);
  btnMainObj->setEnabled(false);
  btnSubLink->setEnabled(false);
  btnMainLink->setEnabled(false);

  showPostureGrid();
}

void TrajectoryViewImpl::deleteClicked() {
  int currentIndex = lstTrajectory->currentRow();
  DDEBUG_V("TrajectoryViewImpl::deleteClicked %d", currentIndex);
  if (currentIndex < 0 || postureList_.size() < currentIndex) return;
  ViaPointParamPtr targetPos = postureList_[currentIndex];
  this->postureList_.erase(std::remove(this->postureList_.begin(), this->postureList_.end(), targetPos), this->postureList_.end());

  if(postureList_.size() == 0) {
    btnSubObj->setEnabled(true);
    btnMainObj->setEnabled(true);
    btnSubLink->setEnabled(true);
    btnMainLink->setEnabled(true);
  }

  showPostureGrid();
  if ( 0<lstTrajectory->rowCount() ) {
    lstTrajectory->setCurrentCell(currentIndex - 1, 0);
  }
}

void TrajectoryViewImpl::updateClicked() {
  if (!subObjItem_ || !mainObjItem_ || !mainObjLink_ || !subObjLink_) return;

  int currentIndex = lstTrajectory->currentRow();
  if (currentIndex < 0 || postureList_.size() < currentIndex) return;
  ViaPointParamPtr targetPos = postureList_[currentIndex];

  Position relPos = subObjLink_->position().inverse()*mainObjLink_->position();

  double posX = relPos.translation()[0];
  double posY = relPos.translation()[1];
  double posZ = relPos.translation()[2];

  const Matrix3 R = relPos.rotation();
  const Vector3 rpy = rpyFromRot(R);
  double rotR = degree(rpy[0]);
  double rotP = degree(rpy[1]);
  double rotY = degree(rpy[2]);

  targetPos->setPosX(posX);
  targetPos->setPosY(posY);
  targetPos->setPosZ(posZ);
  targetPos->setRotR(rotR);
  targetPos->setRotP(rotP);
  targetPos->setRotY(rotY);

  targetPos->clearTransMat();
  for (int index = 0; index < 12; index++) {
    targetPos->addTransMat(relPos.data()[index]);
  }

  showPostureGrid();
  lstTrajectory->setCurrentCell(currentIndex, 0);
}

void TrajectoryViewImpl::upClicked() {
  int currentIndex = lstTrajectory->currentRow();
  if (currentIndex <= 0 || postureList_.size() < currentIndex) return;

  swap(postureList_[currentIndex], postureList_[currentIndex - 1]);
  showPostureGrid();
  lstTrajectory->setCurrentCell(currentIndex - 1, 0);
}

void TrajectoryViewImpl::downClicked() {
  int currentIndex = lstTrajectory->currentRow();
  if (currentIndex < 0 || postureList_.size() <= currentIndex) return;

  swap(postureList_[currentIndex], postureList_[currentIndex + 1]);
  showPostureGrid();
  lstTrajectory->setCurrentCell(currentIndex + 1, 0);
}

void TrajectoryViewImpl::postureSelectionChanged() {
  if (isSkip_) return;
  DDEBUG("TrajectoryViewImpl::postureSelectionChanged")
  int currentIndex = lstTrajectory->currentRow();
  ViaPointParamPtr targetPos = postureList_[currentIndex];
  Position objTrans;
  for (int index = 0; index < 12; index++) {
    objTrans.data()[index] = targetPos->getTransMat()[index];
  }

  mainObjLink_->R() = subObjLink_->R() * objTrans.linear();
  mainObjLink_->p() = subObjLink_->p() + subObjLink_->R() * objTrans.translation();

  mainObjItem_->notifyKinematicStateChange(true);
}

void TrajectoryViewImpl::itemEdited(QTableWidgetItem *item) {
  if (isSkip_) return;
  int currentIndex = lstTrajectory->currentRow();
  DDEBUG_V("TrajectoryViewImpl::itemEdited %d", currentIndex);
  double posX = lstTrajectory->item(currentIndex, 0)->text().toDouble();
  double posY = lstTrajectory->item(currentIndex, 1)->text().toDouble();
  double posZ = lstTrajectory->item(currentIndex, 2)->text().toDouble();
  double rotR = lstTrajectory->item(currentIndex, 3)->text().toDouble();
  double rotP = lstTrajectory->item(currentIndex, 4)->text().toDouble();
  double rotY = lstTrajectory->item(currentIndex, 5)->text().toDouble();
  double time = lstTrajectory->item(currentIndex, 6)->text().toDouble();

  ViaPointParamPtr targetPos = postureList_[currentIndex];
  targetPos->setPosX(posX);
  targetPos->setPosY(posY);
  targetPos->setPosZ(posZ);
  targetPos->setRotR(rotR);
  targetPos->setRotP(rotP);
  targetPos->setRotY(rotY);
  targetPos->setTime(time);
  /////
  Position T;
  T.linear() = rotFromRpy(radian(rotR), radian(rotP), radian(rotY));
  T.translation() << posX, posY, posZ;

  targetPos->clearTransMat();
  for (int index = 0; index < 12; index++) {
    targetPos->addTransMat(T.data()[index]);
  }
  /////
  showPostureGrid();
  lstTrajectory->setCurrentCell(currentIndex, 0);
}

void TrajectoryViewImpl::setEditMode(bool canEdit) {
  btnSubObj->setEnabled(canEdit);
  leSubObj->setEnabled(canEdit);
  btnMainObj->setEnabled(canEdit);
  leMainObj->setEnabled(canEdit);

  btnSubLink->setEnabled(canEdit);
  leSubLink->setEnabled(canEdit);
  btnMainLink->setEnabled(canEdit);
  leMainLink->setEnabled(canEdit);
}

/////
TrajectoryView::TrajectoryView() : viewImpl(0) {
  setName(_("Trajectory"));
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  viewImpl = new TrajectoryViewImpl(this);
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->addWidget(viewImpl);
  setLayout(vbox);
  setDefaultLayoutArea(View::LEFT_TOP);
}

TrajectoryView::~TrajectoryView() {
};

}
