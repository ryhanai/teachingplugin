#include "TeachingMasterEventHandler.h"

#include "TeachingDataHolder.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
//
#include "DataBaseManager.h"
//
#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;

namespace teaching {

TeachingMasterEventHandler::~TeachingMasterEventHandler() {
  DDEBUG("TeachingMasterEventHandler Destruct");
}

TeachingMasterEventHandler* TeachingMasterEventHandler::instance() {
  static TeachingMasterEventHandler* holder = new TeachingMasterEventHandler();
  return holder;
}

//ModelMasterDialog
void TeachingMasterEventHandler::mmd_Loaded(ModelMasterDialog* dialog) {
	DDEBUG("TeachingEventHandler::mmd_Loaded");

	this->mmd_ = dialog;

	mmd_CurrentId_ = NULL_ID; mmd_CurrentModel_ = 0;

	vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterListFromDB();
	this->mmd_->showGrid(modelMasterList);
}

void TeachingMasterEventHandler::mmd_ModelSelectionChanged(int newId, QString name, QString fileName) {
	DDEBUG_V("TeachingEventHandler::mmd_ModelSelectionChanged %d", newId);

	if (mmd_CurrentId_ !=NULL_ID && mmd_CurrentId_ != newId) {
		TeachingDataHolder::instance()->updateModelMaster(mmd_CurrentId_, name, fileName);
	}
	mmd_CurrentId_ = newId;
	mmd_CurrentModel_ = TeachingDataHolder::instance()->getModelMasterById(newId);

  QImage targetImage = mmd_CurrentModel_->getImage();
	this->mmd_->updateContents(mmd_CurrentModel_->getName(), mmd_CurrentModel_->getFileName(),
                             mmd_CurrentModel_->getImageFileName(), &targetImage);
	this->mmd_->showParamGrid(mmd_CurrentModel_->getActiveModelParamList());
	mmd_CurrentParam_ = 0;
}

void TeachingMasterEventHandler::mmd_ModelParameterSelectionChanged(int newId, QString name, QString desc) {
	DDEBUG_V("TeachingEventHandler::mmd_ModelParameterSelectionChanged %d,  %s, %s", newId, name.toStdString().c_str(), desc.toStdString().c_str());
	if (mmd_CurrentParam_) {
		mmd_CurrentParam_->setName(name);
		mmd_CurrentParam_->setValueDesc(desc);
	}
	mmd_CurrentParam_ = 0;
	//
	for (ModelParameterParamPtr param : mmd_CurrentModel_->getModelParameterList()) {
		if (newId == param->getId()) {
			mmd_CurrentParam_ = param;
			break;
		}
	}
	if (mmd_CurrentParam_) {
		this->mmd_->updateParamContents(mmd_CurrentParam_->getName(), mmd_CurrentParam_->getValueDesc());
	}
}

bool TeachingMasterEventHandler::mmd_RefClicked() {
	if (!mmd_CurrentModel_) return false;

	QString strFName = QFileDialog::getOpenFileName(0, "VRML File", ".", "wrl(*.wrl);;all(*.*)");
	if (strFName.isEmpty()) return false;

	QString strName = QFileInfo(strFName).fileName();
	QString strPath = QFileInfo(strFName).absolutePath();

	if (strFName == mmd_CurrentModel_->getFileName()) return true;
	mmd_CurrentModel_->setFileName(strName);

  QFile file(strFName);
  if (file.open(QIODevice::ReadOnly) == false) return false;
  mmd_CurrentModel_->setData(file.readAll());
  //ŽQÆƒ‚ƒfƒ‹‚Ì“Ç‚Ýž‚Ý
  if (TeachingUtil::loadModelDetail(strFName, mmd_CurrentModel_) == false) return false;
  /////
  QImage targetImage = mmd_CurrentModel_->getImage();
	this->mmd_->updateContents(mmd_CurrentModel_->getName(), mmd_CurrentModel_->getFileName(), mmd_CurrentModel_->getImageFileName(), &targetImage);
  return true;
}

void TeachingMasterEventHandler::mmd_RefImageClicked() {
  if (!mmd_CurrentModel_) return;

  QString strFName = QFileDialog::getOpenFileName(0, "Image File", ".", "png(*.png);;jpg(*.jpg);;jpg(*.jpeg);;all(*.*)");
  if (strFName.isEmpty()) return;

  QImage targetImage;
  if (!targetImage.load(strFName)) return;

  QString strName = QFileInfo(strFName).fileName();
  mmd_CurrentModel_->setImage(targetImage);
  mmd_CurrentModel_->setImageFileName(strName);

  this->mmd_->updateImage(strName, targetImage);
}

void TeachingMasterEventHandler::mmd_DeleteImageClicked() {
  if (!mmd_CurrentModel_) return;

  QImage targetImage;
  mmd_CurrentModel_->setImage(targetImage);
  mmd_CurrentModel_->setImageFileName("");

  this->mmd_->updateImage("", targetImage);
}

void TeachingMasterEventHandler::mmd_AddModelClicked() {
	ModelMasterParamPtr model = TeachingDataHolder::instance()->addModelMaster();
	mmd_CurrentModel_ = model;
	mmd_CurrentId_ = model->getId();

	this->mmd_->addModel(mmd_CurrentModel_->getId(), mmd_CurrentModel_->getName());
}

bool TeachingMasterEventHandler::mmd_DeleteModelClicked() {
	if (!mmd_CurrentModel_) return false;

  QStringList errorList;

  vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->getTaskList();
  for (TaskModelParamPtr task : taskList) {
    if (task->getMode() == DB_MODE_DELETE || task->getMode() == DB_MODE_IGNORE) continue;
    for (ModelParamPtr model : task->getActiveModelList()) {
      if (model->getMasterId() == mmd_CurrentModel_->getId()) {
        QString error = "  TaskName : [" + task->getName() + "], ModelName : [" + model->getRName() + "]";
        errorList.append(error);
      }
    }
  }
  vector<FlowParamPtr> flowList = TeachingDataHolder::instance()->getFlowList();
  DDEBUG_V("Check Flow : %d", flowList.size());
  for (FlowParamPtr flow : flowList) {
    flow = TeachingDataHolder::instance()->reGetFlowById(flow->getId());
    for(FlowModelParamPtr flowModel : flow->getActiveModelList()) {
      DDEBUG_V("FlowModel MasterId : %d", flowModel->getMasterId());
      if( flowModel->getMasterId() == mmd_CurrentModel_->getId()) {
        QString error = "  FlowName : [" + flow->getName() + "]";
        errorList.append(error);
      }
    }
  }
  if (0 < errorList.size()) {
    QString errMsg = _("CANNOT delete the master because it is referenced from the following models.\n");
    for (int index = 0; index < errorList.size(); index++) {
      if (0<index) {
        errMsg.append("\n");
      }
      errMsg.append(errorList.at(index));
    }
    QMessageBox::warning(mmd_, _("Model Master"), errMsg);
    return false;
  }
  //
	ModelMasterParamPtr model = TeachingDataHolder::instance()->getModelMasterById(mmd_CurrentModel_->getId());
	model->setDelete();
	model->deleteModelDetails();

	if (mmd_CurrentModel_ && mmd_CurrentModel_->getModelItem()) {
	  ChoreonoidUtil::unLoadModelMasterItem(mmd_CurrentModel_);
	}
  return true;
}

void TeachingMasterEventHandler::mmd_AddModelParamClicked() {
	if (!mmd_CurrentModel_) return;
	TeachingDataHolder::instance()->addModelMasterParam(mmd_CurrentModel_);
	this->mmd_->showParamGrid(mmd_CurrentModel_->getActiveModelParamList());
}

bool TeachingMasterEventHandler::mmd_DeleteModelParamClicked() {
	if (!mmd_CurrentModel_ || !mmd_CurrentParam_) return false;

  QStringList errorList;
  vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->getTaskList();
  for (TaskModelParamPtr task : taskList) {
    if (task->getMode() == DB_MODE_DELETE || task->getMode() == DB_MODE_IGNORE) continue;
    for (ModelParamPtr model : task->getActiveModelList()) {
      if (model->getMasterId() == mmd_CurrentModel_->getId()) {
        for(ParameterParamPtr param : task->getActiveParameterList()) {
          if(param->getType() != PARAM_KIND_MODEL) continue;
          if(param->getModelId()==model->getId() && param->getModelParamId()==mmd_CurrentParam_->getId()) {
            QString error = " TaskName:" + task->getName() + ", ParamName:" + param->getName();
            errorList.append(error);
          }
        }
      }
    }
  }
  if (0 < errorList.size()) {
    QString errMsg = _("CANNOT delete the master feature because it is referenced from the following parameters.\n");
    for (int index = 0; index < errorList.size(); index++) {
      if (0<index) {
        errMsg.append("\n");
      }
      errMsg.append(errorList.at(index));
    }
    QMessageBox::warning(mmd_, _("Model Master"), errMsg);
    return false;
  }

	mmd_CurrentParam_->setDelete();

  return true;
}

bool TeachingMasterEventHandler::mmd_OkClicked(QString name, QString fileName, QString& errMessage) {
  DDEBUG("TeachingMasterEventHandler::mmd_OkClicked");
	if (mmd_CurrentId_ != NULL_ID) {
		TeachingDataHolder::instance()->updateModelMaster(mmd_CurrentId_, name, fileName);
	}
	return TeachingDataHolder::instance()->saveModelMaster(errMessage);
}

bool TeachingMasterEventHandler::mmd_Check() {
  if (mmd_CurrentModel_->getMode() != DB_MODE_INSERT) return false;
  QString txtData = QString::fromUtf8(mmd_CurrentModel_->getData());
  QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
  int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
  return 0 <= ret;
}

void TeachingMasterEventHandler::mmd_Close() {
	if (mmd_CurrentModel_) {
		ChoreonoidUtil::unLoadModelMasterItem(mmd_CurrentModel_);
	}
}

}