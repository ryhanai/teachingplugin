#include "ModelDialog.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "DataBaseManager.h"
#include "MetaDataView.h"

#include <cnoid/BodyBar>
#include <cnoid/EigenUtil>
#include <boost/bind.hpp>

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
  QLabel* lblModel = new QLabel(tr("Model Name:"));
  leModel = new QLineEdit;
  QLabel* lblModelRName = new QLabel(tr("Model ID:"));
  leModelRName = new QLineEdit;
  QLabel* lblFile = new QLabel(tr("Model File:"));
  cmbType = new QComboBox(this);
  cmbType->addItem("Env.");
  cmbType->addItem("E.E.");
  cmbType->addItem("Work");
  leFile = new QLineEdit;

  btnRef = new QPushButton();
  btnRef->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnRef->setToolTip(tr("Ref..."));

  QLabel* lblX = new QLabel(tr("Origin X:"));
  leX = new QLineEdit; leX->setAlignment(Qt::AlignRight);
  QLabel* lblY = new QLabel(tr("Y:"));
  leY = new QLineEdit; leY->setAlignment(Qt::AlignRight);
  QLabel* lblZ = new QLabel(tr("Z:"));
  leZ = new QLineEdit; leZ->setAlignment(Qt::AlignRight);
  QLabel* lblRx = new QLabel(tr("Rx:"));
  leRx = new QLineEdit; leRx->setAlignment(Qt::AlignRight);
  QLabel* lblRy = new QLabel(tr("Ry:"));
  leRy = new QLineEdit; leRy->setAlignment(Qt::AlignRight);
  QLabel* lblRz = new QLabel(tr("Rz:"));
  leRz = new QLineEdit; leRz->setAlignment(Qt::AlignRight);
  //
  QFrame* frmButtons = new QFrame;
  btnAddModel = new QPushButton(tr("Add"));
  btnAddModel->setIcon(QIcon(":/Teaching/icons/Plus.png"));
  btnAddModel->setToolTip(tr("Add new Model"));

  btnDeleteModel = new QPushButton(tr("Delete"));
  btnDeleteModel->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDeleteModel->setToolTip(tr("Delete selected Model"));

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
  QPushButton* btnOK = new QPushButton(tr("OK"));
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

  setWindowTitle(tr("Models"));
  resize(450, 350);
}

void ModelDialog::setTaskModel(TaskModelParam* param) {
  this->targetTask_ = param;
  showGrid();
}

void ModelDialog::changeTaskModel(TaskModelParam* param) {
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
  DDEBUG("MetaDataViewImpl::okClicked")
  updateTaskModelInfo();
  connectionToKinematicStateChanged.disconnect();
	currentBodyItemChangeConnection.disconnect();
  parentView_->closeModelDialog();
}

void ModelDialog::modelSelectionChanged() {
  DDEBUG("MetaDataViewImpl::modelSelectionChanged")
  updateTaskModelInfo();

  if(currentModel_) {
    lstModel->item(currentModelIndex_, 0)->setText(getTypeName(currentModel_->getType()));
    lstModel->item(currentModelIndex_, 1)->setText(currentModel_->getName());
  }

  QTableWidgetItem* item = lstModel->currentItem();
  if(item) {
    currentModelIndex_ = lstModel->currentRow();
    currentModel_ = targetTask_->getModelById(item->data(Qt::UserRole).toInt());
    btnRef->setEnabled(true);
    DDEBUG_V("MetaDataViewImpl::modelSelectionChanged:item %d", currentModelIndex_)

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
    DDEBUG("MetaDataViewImpl::modelSelectionChanged:NO item")
    clearModelDetail();
    currentModelIndex_ = -1;
    currentModel_ = 0;
  }
}

void ModelDialog::refClicked() {
	QString strFName = QFileDialog::getOpenFileName(
			this, tr( "Model VRML File" ), ".",
			tr( "wrl(*.wrl);;all(*.*)" ) );
	if ( strFName.isEmpty() ) return;
  //
  DDEBUG("ModelDialog::refClicked")
  QString strName = QFileInfo(strFName).fileName();
  QString strPath = QFileInfo(strFName).absolutePath();
  leFile->setText(strName);
  if( !currentModel_ ) return;
  //
  QString currFile = currentModel_->getFileName();
  if(strFName==currFile) return;
  //
  if(currentModel_->getModelItem()) {
    connectionToKinematicStateChanged.disconnect();
    ChoreonoidUtil::unLoadModelItem(currentModel_);
    currentModel_->deleteModelDetails();
  }
  currentModel_->setFileName(strName);
  QFile file(strFName);
  file.open(QIODevice::ReadOnly);
  currentModel_->setData(file.readAll());
  if( ChoreonoidUtil::readModelItem(currentModel_, strFName) ) {
    ChoreonoidUtil::loadModelItem(currentModel_);
    ChoreonoidUtil::showAllModelItem();
  }
  //�Q�ƃ��f���̓ǂݍ���
  TeachingUtil::loadModelDetail(strFName, currentModel_);
}

void ModelDialog::addModelClicked() {
  if(targetTask_) {
    int newIdx = DatabaseManager::getInstance().getModelMaxIndex();
    ModelParam* param = new ModelParam(newIdx, 0, "New Model", "", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true);
    targetTask_->addModel(param);

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), param->getId());
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), param->getId());
  }
}

void ModelDialog::deleteModelClicked() {
  DDEBUG("MetaDataViewImpl::deleteModelClicked")
  QTableWidgetItem* item = lstModel->currentItem();
  if(item) {
    int itemIndex = item->data(Qt::UserRole).toInt();
    targetTask_->deleteModelById(itemIndex);
    if(currentModel_ && currentModel_->getModelItem()) {
      ChoreonoidUtil::unLoadModelItem(currentModel_);
      connectionToKinematicStateChanged.disconnect();
    }

    lstModel->removeRow(lstModel->currentRow());
    lstModel->setFocus();
    currentModelIndex_ = lstModel->currentRow();
  }
}

void ModelDialog::modelPositionChanged() {
  if(isWidgetSkip_) return;
  DDEBUG("MetaDataViewImpl::modelPositionChanged")
  if(currentModel_) {
    if( currentModel_->getModelItem() ) {
      double newX = leX->text().toDouble();
      double newY = leY->text().toDouble();
      double newZ = leZ->text().toDouble();
      double newRx = leRx->text().toDouble();
      double newRy = leRy->text().toDouble();
      double newRz = leRz->text().toDouble();
      if(dbl_eq(newX, currentModel_->getPosX())==false
        || dbl_eq(newY, currentModel_->getPosY())==false
        || dbl_eq(newZ, currentModel_->getPosZ())==false
        || dbl_eq(newRx, currentModel_->getRotRx())==false
        || dbl_eq(newRy, currentModel_->getRotRy())==false
        || dbl_eq(newRz, currentModel_->getRotRz())==false ) {
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
  DDEBUG("MetaDataViewImpl::onCurrentBodyItemChanged")
  if(targetTask_ && bodyItem != currentBodyItem_){
    connectionToKinematicStateChanged.disconnect();
    currentBodyItem_ = bodyItem;
    if(currentBodyItem_) {
      for(int index=0; index<targetTask_->getModelList().size(); index++) {
        ModelParam* model = targetTask_->getModelList()[index];
        if(model->getModelItem().get() == currentBodyItem_) {
          selectedModel_ = model;
          break;
        }
      }
    }
    if(!connectionToKinematicStateChanged.connected() && currentBodyItem_){
      connectionToKinematicStateChanged = currentBodyItem_->sigKinematicStateChanged().connect(
      //    bind(&MetaDataViewImpl::updateKinematicState, this, true));
          updateKinematicStateLater);
    }
  }
}

void ModelDialog::updateKinematicState(bool blockSignals) {
  DDEBUG("MetaDataViewImpl::updateKinematicState")
  if(currentBodyItem_ && selectedModel_){
    Link* currentLink = currentBodyItem_->body()->rootLink();
    selectedModel_->setPosX(currentLink->p()[0]);
    selectedModel_->setPosY(currentLink->p()[1]);
    selectedModel_->setPosZ(currentLink->p()[2]);

    const Matrix3 R = currentLink->attitude();
    const Vector3 rpy = rpyFromRot(R);
    selectedModel_->setRotRx(degree(rpy[0]));
    selectedModel_->setRotRy(degree(rpy[1]));
    selectedModel_->setRotRz(degree(rpy[2]));

    if(selectedModel_==currentModel_) {
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
  DDEBUG("MetaDataViewImpl::showGrid")
  lstModel->setRowCount(0);

  for(int index=0; index<targetTask_->getModelList().size(); index++) {
    ModelParam* param = targetTask_->getModelList()[index];
    if( param->getMode()==DB_MODE_DELETE || param->getMode()==DB_MODE_IGNORE) continue;

    int row = lstModel->rowCount();
    lstModel->insertRow(row);
    UIUtil::makeTableItemWithData(lstModel, row, 0, getTypeName(param->getType()), param->getId());
    UIUtil::makeTableItemWithData(lstModel, row, 1, param->getName(), param->getId());
  }
}

void ModelDialog::updateTaskModelInfo() {
  if(currentModel_) {
    QString strModel = leModel->text();
    if( currentModel_->getName() != strModel) {
      currentModel_->setName(strModel);
    }
    //
    QString strModelRName = leModelRName->text();
    if( currentModel_->getRName() != strModelRName) {
      currentModel_->setRName(strModelRName);
    }
    //
    QString strFile = leFile->text();
    if( currentModel_->getFileName() != strFile) {
      currentModel_->setFileName(strFile);
    }
    //
    int selectedType = cmbType->currentIndex();
    if( currentModel_->getType() != selectedType) {
      currentModel_->setType(selectedType);
    }
    //
    string strPosX = leX->text().toUtf8().constData();
    double posX = std::atof(strPosX.c_str());
    if( currentModel_->getPosX() != posX) {
      currentModel_->setPosX(posX);
    }
    //
    string strPosY = leY->text().toUtf8().constData();
    double posY = std::atof(strPosY.c_str());
    if( currentModel_->getPosY() != posY) {
      currentModel_->setPosY(posY);
    }
    //
    string strPosZ = leZ->text().toUtf8().constData();
    double posZ = std::atof(strPosZ.c_str());
    if( currentModel_->getPosZ() != posZ) {
      currentModel_->setPosZ(posZ);
    }
    //
    string strRotRx = leRx->text().toUtf8().constData();
    double rotRx = std::atof(strRotRx.c_str());
    if( currentModel_->getRotRx() != rotRx) {
      currentModel_->setRotRx(rotRx);
    }
    //
    string strRotRy = leRy->text().toUtf8().constData();
    double rotRy = std::atof(strRotRy.c_str());
    if( currentModel_->getRotRy() != rotRy) {
      currentModel_->setRotRy(rotRy);
    }
    //
    string strRotRz = leRz->text().toUtf8().constData();
    double rotRz = std::atof(strRotRz.c_str());
    if( currentModel_->getRotRz() != rotRz) {
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

  switch(source) {
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
