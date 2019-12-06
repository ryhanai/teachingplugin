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
  : targetTask_(0), targetTrajectory_(0),
    isSkip_(false), QWidget(parent) {

  lstTrajectory = UIUtil::makeTableWidget(1, false, true);
  lstTrajectory->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  lstTrajectory->setColumnWidth(0, 150);
  lstTrajectory->setHorizontalHeaderLabels(QStringList() << "Name");
  lstTrajectory->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  btnAddTraj = new QPushButton(_("Add"));
  btnDeleteTraj = new QPushButton(_("Del"));

  QFrame* frmButtonTraj = new QFrame;
	QVBoxLayout* buttonTrajLayout = new QVBoxLayout(frmButtonTraj);
  buttonTrajLayout->setContentsMargins(2, 2, 2, 2);
	buttonTrajLayout->addWidget(btnAddTraj);
	buttonTrajLayout->addWidget(btnDeleteTraj);

  QFrame* frmListTraj = new QFrame;
	QHBoxLayout* listTrajLayout = new QHBoxLayout(frmListTraj);
  listTrajLayout->setContentsMargins(2, 2, 2, 2);
	listTrajLayout->addWidget(lstTrajectory);
	listTrajLayout->addWidget(frmButtonTraj);
  /////
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
  btnSubLink = new QPushButton("Base Link");
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

  QFrame* settingFrame = new QFrame;
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
  lstViaPoint = UIUtil::makeTableWidget(7, true, true);
  lstViaPoint->setColumnWidth(0, 80);
  lstViaPoint->setColumnWidth(1, 80);
  lstViaPoint->setColumnWidth(2, 80);
  lstViaPoint->setColumnWidth(3, 80);
  lstViaPoint->setColumnWidth(4, 80);
  lstViaPoint->setColumnWidth(5, 80);
  lstViaPoint->setColumnWidth(6, 80);
  lstViaPoint->setHorizontalHeaderLabels(QStringList() << "X" << "Y" << "Z" << "R" << "P" << "Y" << "Time");
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
	listLayout->addWidget(lstViaPoint);
	listLayout->addWidget(frmButton);
  
  QFrame* frmViaPoint = new QFrame;
  QVBoxLayout* viaLayout = new QVBoxLayout(frmViaPoint);
  viaLayout->setContentsMargins(2, 0, 2, 0);
  viaLayout->addWidget(settingFrame);
  viaLayout->addWidget(frmList);
  /////
  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->addWidget(frmListTraj);
  mainLayout->addWidget(frmViaPoint);
  setLayout(mainLayout);
  //
  connect(btnAddTraj, SIGNAL(clicked()), this, SLOT(addTrajClicked()));
  connect(btnDeleteTraj, SIGNAL(clicked()), this, SLOT(deleteTrajClicked()));
  connect(lstTrajectory, SIGNAL(itemSelectionChanged()), this, SLOT(trajectorySelectionChanged()));
  connect(lstTrajectory, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(trajectoryItemEdited(QTableWidgetItem*)));

  connect(btnSubObj, SIGNAL(clicked()), this, SLOT(subObjClicked()));
  connect(btnMainObj, SIGNAL(clicked()), this, SLOT(mainObjClicked()));
  connect(btnSubLink, SIGNAL(clicked()), this, SLOT(subLinkClicked()));
  connect(btnMainLink, SIGNAL(clicked()), this, SLOT(mainLinkClicked()));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(upClicked()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(downClicked()));
  connect(lstViaPoint, SIGNAL(itemSelectionChanged()), this, SLOT(postureSelectionChanged()));
  connect(lstViaPoint, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemEdited(QTableWidgetItem*)));

	TeachingEventHandler::instance()->trv_Loaded(this);
}

TrajectoryViewImpl::~TrajectoryViewImpl() {
}

void TrajectoryViewImpl::setTaskParam(TaskModelParamPtr param) {
  targetTask_ = param;
  showTrajectoryGrid();
}

void TrajectoryViewImpl::showTrajectoryGrid() {
  isSkip_ = true;
	vector<TaskTrajectoryParamPtr> traList =  targetTask_->getActiveTrajectoryList();
	lstTrajectory->setRowCount(0);

  for (TaskTrajectoryParamPtr param : traList) {
		int row = lstTrajectory->rowCount();
		lstTrajectory->insertRow(row);
		UIUtil::makeTableItemWithData(lstTrajectory, row, 0, param->getName(), param->getId());
  }
  isSkip_ = false;
}

bool compare_seq(const ViaPointParamPtr left, const ViaPointParamPtr right) {
  return left->getSeq() < right->getSeq();
}

void TrajectoryViewImpl::showPostureGrid() {
  isSkip_ = true;
	lstViaPoint->setRowCount(0);

  std::vector<ViaPointParamPtr> postureList = targetTrajectory_->getActiveViaList();
  std::sort(postureList.begin(), postureList.end(), compare_seq);

	for (ViaPointParamPtr param : postureList) {
		int row = lstViaPoint->rowCount();
		lstViaPoint->insertRow(row);
		UIUtil::makeTableItemWithData(lstViaPoint, row, 0, QString::number(param->getPosX(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 1, QString::number(param->getPosY(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 2, QString::number(param->getPosZ(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 3, QString::number(param->getRotRx(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 4, QString::number(param->getRotRy(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 5, QString::number(param->getRotRz(), 'f', 4), param->getId());
		UIUtil::makeTableItemWithData(lstViaPoint, row, 6, QString::number(param->getTime(), 'f', 4), param->getId());
	}
  leSubObj->setText(targetTrajectory_->getBaseObject());
  leSubLink->setText(targetTrajectory_->getBaseLink());
  leMainObj->setText(targetTrajectory_->getTargetObject());
  leMainLink->setText(targetTrajectory_->getTargetLink());

  isSkip_ = false;
}

void TrajectoryViewImpl::addTrajClicked() {
  if (!targetTask_) return;

  int id = targetTask_->getMaxTrajectoryId();
  TaskTrajectoryParamPtr newParam = std::make_shared<TaskTrajectoryParam>(id, "New Trajectory");
  newParam->setNewForce();
  targetTask_->addTrajectory(newParam);
  showTrajectoryGrid();
}

void TrajectoryViewImpl::deleteTrajClicked() {
  if (!targetTask_) return;
  if (!targetTrajectory_) return;

  int currentIndex = lstTrajectory->currentRow();
  targetTrajectory_->setDelete();
  showTrajectoryGrid();
  if ( 0<lstTrajectory->rowCount() ) {
    lstTrajectory->setCurrentCell(currentIndex - 1, 0);
  }
}

void TrajectoryViewImpl::trajectorySelectionChanged() {
  if (isSkip_) return;
  if (!targetTask_) return;

  targetTrajectory_ = 0;
  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstTrajectory->currentItem();
  if (item) {
    selectedId = item->data(Qt::UserRole).toInt();
  	vector<TaskTrajectoryParamPtr> traList =  targetTask_->getActiveTrajectoryList();
  	std::vector<TaskTrajectoryParamPtr>::iterator traItr = std::find_if(traList.begin(), traList.end(), TrajectryComparator(selectedId));
    if (traItr == traList.end()) return;
    targetTrajectory_ = *traItr;
    showPostureGrid();
  }
}

void TrajectoryViewImpl::trajectoryItemEdited(QTableWidgetItem *item) {
  if (isSkip_) return;
  int currentIndex = lstTrajectory->currentRow();
  QString traName = lstTrajectory->item(currentIndex, 0)->text();

  targetTrajectory_->setName(traName);
}

void TrajectoryViewImpl::subObjClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  Item* selectedItem = ItemTreeView::mainInstance()->selectedItem<Item>();
  if (!selectedItem) return;
  BodyItem* subObjItem = (BodyItem*)selectedItem;
  if (!subObjItem) return;
  leSubObj->setText(QString::fromStdString(subObjItem->name()));
  leSubLink->setText("");
  targetTrajectory_->setBaseObject(QString::fromStdString(subObjItem->name()));
  targetTrajectory_->setBaseObjItem(subObjItem);
}

void TrajectoryViewImpl::mainObjClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  Item* selectedItem = ItemTreeView::mainInstance()->selectedItem<Item>();
  if (!selectedItem) return;
  BodyItem* mainObjItem = (BodyItem*)selectedItem;
  if (!mainObjItem) return;
  leMainObj->setText(QString::fromStdString(mainObjItem->name()));
  leMainLink->setText("");
  targetTrajectory_->setTargetObject(QString::fromStdString(mainObjItem->name()));
  targetTrajectory_->setTargetObjItem(mainObjItem);
}

void TrajectoryViewImpl::subLinkClicked() {
  if (!targetTask_ || !targetTrajectory_) return;
  BodyItem* subObjItem = targetTrajectory_->getBaseObjItem();
  if (!subObjItem) return;

  int linkIndex = LinkSelectionView::mainInstance()->selectedLinkIndex();
  if (linkIndex < 0) return;
  Link* subObjLink = subObjItem->body()->link(linkIndex);
  if (!subObjLink) return;
  leSubLink->setText(QString::fromStdString(subObjLink->name()));
  targetTrajectory_->setBaseLink(QString::fromStdString(subObjLink->name()));
  targetTrajectory_->setBaseObjLink(subObjLink);
}

void TrajectoryViewImpl::mainLinkClicked() {
  if (!targetTask_ || !targetTrajectory_) return;
  BodyItem* mainObjItem = targetTrajectory_->getTargetObjItem();
  if (!mainObjItem) return;

  int linkIndex = LinkSelectionView::mainInstance()->selectedLinkIndex();
  if( 0<= linkIndex ) {
    Link* mainObjLink = mainObjItem->body()->link(linkIndex);
    if(mainObjLink) {
      leMainLink->setText(QString::fromStdString(mainObjLink->name()));
      targetTrajectory_->setTargetLink(QString::fromStdString(mainObjLink->name()));
      targetTrajectory_->setTargetObjLink(mainObjLink);
    }
  }
}

void TrajectoryViewImpl::addClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  BodyItem* subObjItem = targetTrajectory_->getBaseObjItem();
  Link* subObjLink = targetTrajectory_->getBaseObjLink();
  BodyItem* mainObjItem = targetTrajectory_->getTargetObjItem();
  Link* mainObjLink = targetTrajectory_->getTargetObjLink();
  if (!subObjItem || !mainObjItem || !mainObjLink || !subObjLink) return;

  Position relPos = subObjLink->position().inverse()*mainObjLink->position();

  double posX = relPos.translation()[0];
  double posY = relPos.translation()[1];
  double posZ = relPos.translation()[2];

  const Matrix3 R = relPos.rotation();
  const Vector3 rpy = rpyFromRot(R);
  double rotR = degree(rpy[0]);
  double rotP = degree(rpy[1]);
  double rotY = degree(rpy[2]);

  int id = targetTrajectory_->getMaxViaPointId();
  int seq = targetTrajectory_->getMaxViaPointSeq();
  ViaPointParamPtr newPos = std::make_shared<ViaPointParam>(id, seq, posX, posY, posZ, rotR, rotP, rotY, 0.0);
  for (int index = 0; index < 12; index++) {
    newPos->addTransMat(relPos.data()[index]);
  }

  targetTrajectory_->addViaPoint(newPos);

  btnSubObj->setEnabled(false);
  btnMainObj->setEnabled(false);
  btnSubLink->setEnabled(false);
  btnMainLink->setEnabled(false);

  showPostureGrid();
}

void TrajectoryViewImpl::deleteClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstViaPoint->currentItem();
  if (!item) return;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;

  ViaPointParamPtr targetPos = *viaItr;
  targetPos->setDelete();

  int currentIndex = lstViaPoint->currentRow();
  showPostureGrid();

  if(lstViaPoint->rowCount() == 0) {
    btnSubObj->setEnabled(true);
    btnMainObj->setEnabled(true);
    btnSubLink->setEnabled(true);
    btnMainLink->setEnabled(true);
  }

  if ( 0<lstViaPoint->rowCount() ) {
    lstViaPoint->setCurrentCell(currentIndex - 1, 0);
  }
}

void TrajectoryViewImpl::updateClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  BodyItem* subObjItem = targetTrajectory_->getBaseObjItem();
  Link* subObjLink = targetTrajectory_->getBaseObjLink();
  BodyItem* mainObjItem = targetTrajectory_->getTargetObjItem();
  Link* mainObjLink = targetTrajectory_->getTargetObjLink();
  if (!subObjItem || !mainObjItem || !mainObjLink || !subObjLink) return;

  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstViaPoint->currentItem();
  if (!item) return;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;

  ViaPointParamPtr targetPos = *viaItr;

  Position relPos = subObjLink->position().inverse()*mainObjLink->position();

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
  targetPos->setRotRx(rotR);
  targetPos->setRotRy(rotP);
  targetPos->setRotRz(rotY);

  targetPos->clearTransMat();
  for (int index = 0; index < 12; index++) {
    targetPos->addTransMat(relPos.data()[index]);
  }

  int currentIndex = lstViaPoint->currentRow();
  showPostureGrid();
  lstViaPoint->setCurrentCell(currentIndex, 0);
}

void TrajectoryViewImpl::upClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  int currentIndex = lstViaPoint->currentRow();
  if (currentIndex <= 0 || lstViaPoint->rowCount() < currentIndex) return;

  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstViaPoint->currentItem();
  if (!item) return;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;
  ViaPointParamPtr targetPos = *viaItr;

  QTableWidgetItem* upItem = lstViaPoint->item(lstViaPoint->currentRow()-1, 0);
  if (!upItem) return;
  selectedId = upItem->data(Qt::UserRole).toInt();
  std::vector<ViaPointParamPtr>::iterator viaUpItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaUpItr == viaList.end()) return;
  ViaPointParamPtr upPos = *viaUpItr;

  int tempSeq = upPos->getSeq();
  upPos->setSeq(targetPos->getSeq());
  targetPos->setSeq(tempSeq);

  showPostureGrid();
  lstViaPoint->setCurrentCell(currentIndex - 1, 0);
}

void TrajectoryViewImpl::downClicked() {
  if (!targetTask_ || !targetTrajectory_) return;

  int currentIndex = lstViaPoint->currentRow();
  if (currentIndex < 0 || lstViaPoint->rowCount() <= currentIndex) return;

  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstViaPoint->currentItem();
  if (!item) return;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;
  ViaPointParamPtr targetPos = *viaItr;

  QTableWidgetItem* downItem = lstViaPoint->item(lstViaPoint->currentRow()+1, 0);
  if (!downItem) return;
  selectedId = downItem->data(Qt::UserRole).toInt();
  std::vector<ViaPointParamPtr>::iterator viaDownItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaDownItr == viaList.end()) return;
  ViaPointParamPtr downPos = *viaDownItr;

  int tempSeq = downPos->getSeq();
  downPos->setSeq(targetPos->getSeq());
  targetPos->setSeq(tempSeq);

  showPostureGrid();
  lstViaPoint->setCurrentCell(currentIndex + 1, 0);
}

void TrajectoryViewImpl::postureSelectionChanged() {
  if (isSkip_) return;
  if (!targetTrajectory_) return;
  DDEBUG("TrajectoryViewImpl::postureSelectionChanged")

  int selectedId = NULL_ID;
  QTableWidgetItem* item = lstViaPoint->currentItem();
  if (!item) return;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;

  ViaPointParamPtr targetPos = *viaItr;
  Position objTrans;
  for (int index = 0; index < 12; index++) {
    objTrans.data()[index] = targetPos->getTransMat()[index];
  }

  Link* subObjLink = targetTrajectory_->getBaseObjLink();
  BodyItem* mainObjItem = targetTrajectory_->getTargetObjItem();
  Link* mainObjLink = targetTrajectory_->getTargetObjLink();

  mainObjLink->R() = subObjLink->R() * objTrans.linear();
  mainObjLink->p() = subObjLink->p() + subObjLink->R() * objTrans.translation();

  mainObjItem->notifyKinematicStateChange(true);
}

void TrajectoryViewImpl::itemEdited(QTableWidgetItem *item) {
  if (isSkip_) return;
  if (!targetTask_ || !targetTrajectory_) return;

  int selectedId = NULL_ID;
  selectedId = item->data(Qt::UserRole).toInt();
  vector<ViaPointParamPtr> viaList = targetTrajectory_->getActiveViaList();
  std::vector<ViaPointParamPtr>::iterator viaItr = std::find_if(viaList.begin(), viaList.end(), ViaPointComparator(selectedId));
  if (viaItr == viaList.end()) return;
  ViaPointParamPtr targetPos = *viaItr;

  int currentIndex = lstViaPoint->currentRow();
  DDEBUG_V("TrajectoryViewImpl::itemEdited %d", currentIndex);
  double posX = lstViaPoint->item(currentIndex, 0)->text().toDouble();
  double posY = lstViaPoint->item(currentIndex, 1)->text().toDouble();
  double posZ = lstViaPoint->item(currentIndex, 2)->text().toDouble();
  double rotR = lstViaPoint->item(currentIndex, 3)->text().toDouble();
  double rotP = lstViaPoint->item(currentIndex, 4)->text().toDouble();
  double rotY = lstViaPoint->item(currentIndex, 5)->text().toDouble();
  double time = lstViaPoint->item(currentIndex, 6)->text().toDouble();

  targetPos->setPosX(posX);
  targetPos->setPosY(posY);
  targetPos->setPosZ(posZ);
  targetPos->setRotRx(rotR);
  targetPos->setRotRy(rotP);
  targetPos->setRotRz(rotY);
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
  lstViaPoint->setCurrentCell(currentIndex, 0);
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

  btnAddTraj->setEnabled(canEdit);
  btnDeleteTraj->setEnabled(canEdit);

  btnAdd->setEnabled(canEdit);
  btnUpdate->setEnabled(canEdit);
  btnDelete->setEnabled(canEdit);
  btnUp->setEnabled(canEdit);
  btnDown->setEnabled(canEdit);
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
