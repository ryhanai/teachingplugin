#include "ModelDialog.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "DataBaseManager.h"
#include "MetaDataView.h"

#include <cnoid/BodyBar>
#include <cnoid/EigenUtil>
#include <boost/bind.hpp>

#include "gettext.h"
#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

ModelDialog::ModelDialog(MetaDataViewImpl* view, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
  targetTask_(0), currentModel_(0), currentModelIndex_(-1), selectedModel_(0),
  isWidgetSkip_(false), parentView_(view),
  currentBodyItem_(0),
  updateKinematicStateLater(bind(&ModelDialog::updateKinematicState, this, true), IDLE_PRIORITY_LOW) {
  lstModel = UIUtil::makeTableWidget(2, false);
  lstModel->setColumnWidth(0, 50);
  lstModel->setColumnWidth(1, 300);
  lstModel->setHorizontalHeaderLabels(QStringList() << "Type" << "Name");

  QFrame* frmTask = new QFrame;
  QLabel* lblModel = new QLabel(_("Model Name:"));
  leModel = new QLineEdit;
  QLabel* lblModelRName = new QLabel(_("Model ID:"));
  leModelRName = new QLineEdit;
  QLabel* lblFile = new QLabel(_("Model File:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Env.");
  cmbType->addItem("E.E.");
  cmbType->addItem("Work");
  leFile = new QLineEdit;

  btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(_("Ref..."));

  QLabel* lblX = new QLabel(_("Origin X:"));
  leX = new QLineEdit; leX->setAlignment(Qt::AlignRight);
  QLabel* lblY = new QLabel(_("Y:"));
  leY = new QLineEdit; leY->setAlignment(Qt::AlignRight);
  QLabel* lblZ = new QLabel(_("Z:"));
  leZ = new QLineEdit; leZ->setAlignment(Qt::AlignRight);
  QLabel* lblRx = new QLabel(_("Rx:"));
  leRx = new QLineEdit; leRx->setAlignment(Qt::AlignRight);
  QLabel* lblRy = new QLabel(_("Ry:"));
  leRy = new QLineEdit; leRy->setAlignment(Qt::AlignRight);
  QLabel* lblRz = new QLabel(_("Rz:"));
  leRz = new QLineEdit; leRz->setAlignment(Qt::AlignRight);
  //
  QFrame* frmButtons = new QFrame;
  btnAddModel = new QPushButton(_("Add"));
  btnAddModel->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAddModel->setToolTip(_("Add new Model"));

  btnDeleteModel = new QPushButton(_("Delete"));
  btnDeleteModel->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteModel->setToolTip(_("Delete selected Model"));

  QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setContentsMargins(2, 0, 2, 0);
  frmTask->setLayout(taskLayout);
  taskLayout->addWidget(lblModel, 1, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModel, 1, 1, 1, 7);
  taskLayout->addWidget(lblModelRName, 2, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leModelRName, 2, 1, 1, 7);
  taskLayout->addWidget(lblFile, 3, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(cmbType, 3, 1, 1, 3);
  taskLayout->addWidget(leFile, 3, 2, 1, 5);
  taskLayout->addWidget(btnRef, 3, 7, 1, 1);

  taskLayout->addWidget(lblX, 4, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leX, 4, 1, 1, 2);
  taskLayout->addWidget(lblY, 4, 3, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leY, 4, 4, 1, 1);
  taskLayout->addWidget(lblZ, 4, 5, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leZ, 4, 6, 1, 2);
  taskLayout->addWidget(lblRx, 5, 0, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRx, 5, 1, 1, 2);
  taskLayout->addWidget(lblRy, 5, 3, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRy, 5, 4, 1, 1);
  taskLayout->addWidget(lblRz, 5, 5, 1, 1, Qt::AlignRight);
  taskLayout->addWidget(leRz, 5, 6, 1, 2);
  //
  QHBoxLayout* buttonLayout = new QHBoxLayout;
  frmButtons->setLayout(buttonLayout);
  buttonLayout->addWidget(btnAddModel);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDeleteModel);

  QFrame* frmBotButtons = new QFrame;
  QPushButton* btnOK = new QPushButton(_("OK"));
  QHBoxLayout* buttonBotLayout = new QHBoxLayout(frmBotButtons);
  buttonBotLayout->setContentsMargins(2, 2, 2, 2);
  buttonBotLayout->addStretch();
  buttonBotLayout->addWidget(btnOK);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(2, 0, 2, 0);
  mainLayout->addWidget(lstModel);
  mainLayout->addWidget(frmTask);
  mainLayout->addWidget(frmButtons);
  mainLayout->addWidget(frmBotButtons);
  setLayout(mainLayout);
  ////
  connect(btnAddModel, SIGNAL(clicked()), this, SLOT(addModelClicked()));
  connect(btnDeleteModel, SIGNAL(clicked()), this, SLOT(deleteModelClicked()));
  connect(lstModel, SIGNAL(itemSelectionChanged()), this, SLOT(modelSelectionChanged()));
  connect(btnRef, SIGNAL(clicked()), this, SLOT(refClicked()));
  connect(btnOK, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(this, SIGNAL(rejected()), this, SLOT(okClicked()));

  connect(leX, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leY, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leZ, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRx, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRy, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));
  connect(leRz, SIGNAL(editingFinished()), this, SLOT(modelPositionChanged()));

  currentBodyItemChangeConnection = BodyBar::instance()->sigCurrentBodyItemChanged().connect(
    bind(&ModelDialog::onCurrentBodyItemChanged, this, _1));

  setWindowTitle(_("Models"));
  resize(450, 350);
}

void ModelDialog::setTaskModel(TaskModelParam* param) {
  DDEBUG("ModelDialog::setTaskModel()");

  this->targetTask_ = param;
  showGrid();
}

void ModelDialog::changeTaskModel(TaskModelParam* param) {
  DDEBUG("ModelDialog::changeTaskModel()");

  updateTaskModelInfo();
  isWidgetSkip_ = true;
  currentModelIndex_ = -1;
  currentModel_ = 0;
  selectedModel_ = 0;
  currentBodyItem_ = 0;
  clearModelDetail();

  this->targetTask_ = param;

  showGrid();
  isWidgetSkip_ = false;
}

void ModelDialog::okClicked() {
  DDEBUG("ModelDialog::okClicked()");

  updateTaskModelInfo();
  connectionToKinematicStateChanged.disconnect();
  currentBodyItemChangeConnection.disconnect();
  parentView_->closeModelDialog();
}

void ModelDialog::modelSelectionChanged() {
  DDEBUG("ModelDialog::modelSelectionChanged()");

  updateTaskModelInfo();

  if (currentModel_) {
    lstModel->item(currentModelIndex_, 0)->setText(getTypeName(currentModel_->getType()));
    lstModel->item(currentModelIndex_, 1)->setText(currentModel_->getName());
  }

  QTableWidgetItem* item = lstModel->currentItem();
  if (item) {
    currentModelIndex_ = lstModel->currentRow();
    currentModel_ = targetTask_->getModelList()[item->data(Qt::UserRole).toInt()];
    btnRef->setEnabled(true);
    DDEBUG_V("MetaDataViewImpl::modelSelectionChanged:item %d", currentModelIndex_);

    ChoreonoidUtil::selectTreeItem(currentModel_);

    leModel->setText(currentModel_->getName());
    leModelRName->setText(currentModel_->getRName());
    cmbType->setCurrentIndex(currentModel_->getType());
    leFile->setText(currentModel_->getFileName());
    leX->setText(QString::number(currentModel_->getPosX(), 'f', 6));
    leY->setText(QString::number(currentModel_->getPosY(), 'f', 6));
    leZ->setText(QString::number(currentModel_->getPosZ(), 'f', 6));
    leRx->setText(QString::number(currentModel_->getRotRx(), 'f', 6));
    leRy->setText(QString::number(currentModel_->getRotRy(), 'f', 6));
    leRz->setText(QString::number(currentModel_->getRotRz(), 'f', 6));

  } else {
    DDEBUG("MetaDataViewImpl::modelSelectionChanged:NO item");
    clearModelDetail();
    currentModelIndex_ = -1;
    currentModel_ = 0;
  }
}

void ModelDialog::refClicked() {
  DDEBUG("ModelDialog::refClicked()");

  QString strFName = QFileDialog::getOpenFileName(
    this, "VRML File", ".", "wrl(*.wrl);;all(*.*)");
  if (strFName.isEmpty()) return;
  //
  QString strName = QFileInfo(strFName).fileName();
  QString strPath = QFileInfo(strFName).absolutePath();
  leFile->setText(strName);
  if (!currentModel_) return;
  //
  QString currFile = currentModel_->getFileName();
  if (strFName == currFile) return;
  //
  if (currentModel_->getModelItem()) {
    connectionToKinematicStateChanged.disconnect();
    ChoreonoidUtil::unLoadModelItem(currentModel_);
    currentModel_->deleteModelDetails();
  }
  currentModel_->setFileName(strName);
  QFile file(strFName);
  file.open(QIODevice::ReadOnly);
  currentModel_->setData(file.readAll());
  if (ChoreonoidUtil::readModelItem(currentModel_, strFName)) {
    ChoreonoidUtil::loadModelItem(currentModel_);
    ChoreonoidUtil::showAllModelItem();
  }
  //ŽQÆƒ‚ƒfƒ‹‚Ì“Ç‚Ýž‚Ý
  TeachingUtil::loadModelDetail(strFName, currentModel_);
}

void ModelDialog::addModelClicked() {
  DDEBUG("ModelDialog::addModelClicked()");

  if (targetTask_) {
    int newIdx = DatabaseManager::getInstance().getModelMaxIndex();
    ModelParam* param = new ModelParam(newIdx, 0, "New Model", "", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true);
    targetTask_->addModel(param);

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), targetTask_->getModelList().size()-1);
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), targetTask_->getModelList().size() - 1);
  }
}

void ModelDialog::deleteModelClicked() {
  DDEBUG("ModelDialog::deleteModelClicked()");

  QTableWidgetItem* item = lstModel->currentItem();
  if (item) {
    int itemIndex = item->data(Qt::UserRole).toInt();
    ModelParam* target = targetTask_->getModelList()[itemIndex];
    target->setDelete();
    target->deleteModelDetails();
    if (currentModel_ && currentModel_->getModelItem()) {
      ChoreonoidUtil::unLoadModelItem(currentModel_);
      connectionToKinematicStateChanged.disconnect();
    }

    lstModel->removeRow(lstModel->currentRow());
    lstModel->setFocus();
    currentModelIndex_ = lstModel->currentRow();
  }
}

void ModelDialog::modelPositionChanged() {
  if (isWidgetSkip_) return;
  DDEBUG("ModelDialog::modelPositionChanged()");

  if (currentModel_) {
    if (currentModel_->getModelItem()) {
      double newX = leX->text().toDouble();
      double newY = leY->text().toDouble();
      double newZ = leZ->text().toDouble();
      double newRx = leRx->text().toDouble();
      double newRy = leRy->text().toDouble();
      double newRz = leRz->text().toDouble();
      if (dbl_eq(newX, currentModel_->getPosX()) == false
        || dbl_eq(newY, currentModel_->getPosY()) == false
        || dbl_eq(newZ, currentModel_->getPosZ()) == false
        || dbl_eq(newRx, currentModel_->getRotRx()) == false
        || dbl_eq(newRy, currentModel_->getRotRy()) == false
        || dbl_eq(newRz, currentModel_->getRotRz()) == false) {
        ChoreonoidUtil::updateModelItemPosition(currentModel_->getModelItem(), newX, newY, newZ, newRx, newRy, newRz);
        currentModel_->setPosX(newX);
        currentModel_->setPosY(newY);
        currentModel_->setPosZ(newZ);
        currentModel_->setRotRx(newRx);
        currentModel_->setRotRy(newRy);
        currentModel_->setRotRz(newRz);
      }
    }
  }
}

void ModelDialog::onCurrentBodyItemChanged(BodyItem* bodyItem) {
  DDEBUG("ModelDialog::onCurrentBodyItemChanged()");

  if (targetTask_ && bodyItem != currentBodyItem_) {
    connectionToKinematicStateChanged.disconnect();
    currentBodyItem_ = bodyItem;
    if (currentBodyItem_) {
      for (int index = 0; index < targetTask_->getModelList().size(); index++) {
        ModelParam* model = targetTask_->getModelList()[index];
        if (model->getModelItem().get() == currentBodyItem_) {
          selectedModel_ = model;
          break;
        }
      }
    }
    if (!connectionToKinematicStateChanged.connected() && currentBodyItem_) {
      connectionToKinematicStateChanged = currentBodyItem_->sigKinematicStateChanged().connect(
        //    bind(&MetaDataViewImpl::updateKinematicState, this, true));
        updateKinematicStateLater);
    }
  }
}

void ModelDialog::updateKinematicState(bool blockSignals) {
  DDEBUG("ModelDialog::updateKinematicState()");

  if (currentBodyItem_ && selectedModel_) {
    Link* currentLink = currentBodyItem_->body()->rootLink();
    selectedModel_->setPosX(currentLink->p()[0]);
    selectedModel_->setPosY(currentLink->p()[1]);
    selectedModel_->setPosZ(currentLink->p()[2]);

    const Matrix3 R = currentLink->attitude();
    const Vector3 rpy = rpyFromRot(R);
    selectedModel_->setRotRx(degree(rpy[0]));
    selectedModel_->setRotRy(degree(rpy[1]));
    selectedModel_->setRotRz(degree(rpy[2]));

    if (selectedModel_ == currentModel_) {
      isWidgetSkip_ = true;
      leX->setText(QString::number(currentModel_->getPosX(), 'f', 6));
      leY->setText(QString::number(currentModel_->getPosY(), 'f', 6));
      leZ->setText(QString::number(currentModel_->getPosZ(), 'f', 6));
      leRx->setText(QString::number(currentModel_->getRotRx(), 'f', 6));
      leRy->setText(QString::number(currentModel_->getRotRy(), 'f', 6));
      leRz->setText(QString::number(currentModel_->getRotRz(), 'f', 6));
      isWidgetSkip_ = false;
    }
  }
}
/////
void ModelDialog::showGrid() {
  lstModel->setRowCount(0);

  for (int index = 0; index < targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), index);
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), index);
  }
}

void ModelDialog::updateTaskModelInfo() {
  if (currentModel_) {
    QString strModel = leModel->text();
    if (currentModel_->getName() != strModel) {
      currentModel_->setName(strModel);
    }
    //
    QString strModelRName = leModelRName->text();
    if (currentModel_->getRName() != strModelRName) {
      currentModel_->setRName(strModelRName);
    }
    //
    QString strFile = leFile->text();
    if (currentModel_->getFileName() != strFile) {
      currentModel_->setFileName(strFile);
    }
    //
    int selectedType = cmbType->currentIndex();
    if (currentModel_->getType() != selectedType) {
      currentModel_->setType(selectedType);
    }
    //
    string strPosX = leX->text().toUtf8().constData();
    double posX = std::atof(strPosX.c_str());
    if (currentModel_->getPosX() != posX) {
      currentModel_->setPosX(posX);
    }
    //
    string strPosY = leY->text().toUtf8().constData();
    double posY = std::atof(strPosY.c_str());
    if (currentModel_->getPosY() != posY) {
      currentModel_->setPosY(posY);
    }
    //
    string strPosZ = leZ->text().toUtf8().constData();
    double posZ = std::atof(strPosZ.c_str());
    if (currentModel_->getPosZ() != posZ) {
      currentModel_->setPosZ(posZ);
    }
    //
    string strRotRx = leRx->text().toUtf8().constData();
    double rotRx = std::atof(strRotRx.c_str());
    if (currentModel_->getRotRx() != rotRx) {
      currentModel_->setRotRx(rotRx);
    }
    //
    string strRotRy = leRy->text().toUtf8().constData();
    double rotRy = std::atof(strRotRy.c_str());
    if (currentModel_->getRotRy() != rotRy) {
      currentModel_->setRotRy(rotRy);
    }
    //
    string strRotRz = leRz->text().toUtf8().constData();
    double rotRz = std::atof(strRotRz.c_str());
    if (currentModel_->getRotRz() != rotRz) {
      currentModel_->setRotRz(rotRz);
    }
  }
}

void ModelDialog::clearModelDetail() {
  leModel->setText("");
  leModelRName->setText("");
  leFile->setText("");
  btnRef->setEnabled(false);
  leX->setText("");
  leY->setText("");
  leZ->setText("");
  leRx->setText("");
  leRy->setText("");
  leRz->setText("");
}

QString ModelDialog::getTypeName(int source) {
  QString result = "";

  switch (source) {
    case 0:
    result = "Env.";
    break;
    case 1:
    result = "E.E.";
    break;
    case 2:
    result = "Work";
    break;
  }

  return result;
}

}
