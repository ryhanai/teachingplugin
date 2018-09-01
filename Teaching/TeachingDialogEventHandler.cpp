#include "TeachingEventHandler.h"

#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
//
#include <cnoid/BodyBar>
#include <boost/bind.hpp>
//

#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;

namespace teaching {

//FlowSearchDialog
void TeachingEventHandler::fsd_Loaded(FlowSearchDialog* dialog) {
	this->fsd_ = dialog;

	vector<FlowParamPtr> flowList = TeachingDataHolder::instance()->getFlowList();
	fsd_->showGrid(flowList);
}

void TeachingEventHandler::fsd_SearchClicked(QString condition) {
	vector<string> condList;
	QStringList conditionList;
	bool isOr = false;
	if (condition.contains("||")) {
		isOr = true;
		conditionList = condition.split("||");
	} else {
		conditionList = condition.split(" ");
	}
	for (unsigned int index = 0; index < conditionList.size(); index++) {
		condList.push_back(conditionList[index].trimmed().toStdString());
	}

	vector<FlowParamPtr> flowList = TeachingDataHolder::instance()->searchFlow(condList, isOr);
	fsd_->showGrid(flowList);
}

bool TeachingEventHandler::fsd_DeleteClicked(int targetId) {
	bool ret = TeachingDataHolder::instance()->deleteFlow(targetId);
	if (ret) {
		fsd_->close();
	}
	if (targetId == flv_CurrentId_) {
		isFlowDeleted_ = true;
	}
	return ret;
}

void TeachingEventHandler::fsd_OKClicked(int targetId) {
	flv_CurrentId_ = targetId;

	fsd_->close();
}

//ModelDialog
bool TeachingEventHandler::mdd_Loaded(ModelDialog* dialog) {
	DDEBUG("TeachingEventHandler::mdd_Loaded");
	if (!com_CurrentTask_) return false;

	this->mdd_ = dialog;
	vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	mdd_->showModelGrid(modelList);
	mdd_->showModelMasterGrid(modelMasterList);

	mdd_CurrentId_ = NULL_ID; mdd_CurrentModel_ = 0;
	mdd_CurrentMasterId_ = NULL_ID; mdd_CurrentModelMaster_ = 0;
	mdd_selectedModel_ = 0;
	mdd_BodyItem_ = 0;
	mdd_currentBodyItemChangeConnection = BodyBar::instance()->sigCurrentBodyItemChanged().connect(
	  bind(&TeachingEventHandler::mdd_CurrentBodyItemChanged, this, _1));

	return true;
}

void TeachingEventHandler::mdd_ModelSelectionChanged(int newId, QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ, int hide) {
	DDEBUG_V("TeachingEventHandler::mdd_ModelSelectionChanged: %d, %d, %d", newId, mdd_CurrentId_, com_CurrentTask_->getId());
	if (mdd_CurrentModel_) {
		mdd_CurrentModel_->setRName(rname);
		mdd_CurrentModel_->setType(type);
		mdd_CurrentModel_->setPosX(posX);
		mdd_CurrentModel_->setPosY(posY);
		mdd_CurrentModel_->setPosZ(posZ);
		mdd_CurrentModel_->setRotRx(rotX);
		mdd_CurrentModel_->setRotRy(rotY);
		mdd_CurrentModel_->setRotRz(rotZ);
    mdd_CurrentModel_->setHide(hide);
  }

	mdd_CurrentModel_ = 0;
	for (ModelParamPtr model : com_CurrentTask_->getModelList()) {
		if (model->getId() == newId) {
			mdd_CurrentModel_ = model;
			break;
		}
	}
	if (mdd_CurrentModel_) {
		ChoreonoidUtil::selectTreeItem(mdd_CurrentModel_);
	}

	mdd_CurrentId_ = newId;
	mdd_->updateContents(mdd_CurrentModel_);
}

void TeachingEventHandler::mdd_ModelMasterSelectionChanged(int newId) {
	DDEBUG_V("TeachingEventHandler::mdd_ModelMasterSelectionChanged: %d", newId);

	mdd_CurrentMasterId_ = newId;
	mdd_CurrentModelMaster_ = TeachingDataHolder::instance()->getModelMasterById(newId);
}

void TeachingEventHandler::mdd_CurrentBodyItemChanged(BodyItem* bodyItem) {
	DDEBUG("TeachingEventHandler::mdd_CurrentBodyItemChanged");

	if (com_CurrentTask_ && bodyItem != mdd_BodyItem_) {
		mdd_connectionToKinematicStateChanged.disconnect();
		mdd_BodyItem_ = bodyItem;
	  if (mdd_BodyItem_) {
	    for (ModelParamPtr model : com_CurrentTask_->getModelList()) {
        if(model->getModelMaster()) {
	        if (model->getModelMaster()->getModelItem().get() == mdd_BodyItem_) {
					  mdd_selectedModel_ = model;
	          break;
	        }
        }
	    }
	  }
	  if (!mdd_connectionToKinematicStateChanged.connected() && mdd_BodyItem_) {
			mdd_connectionToKinematicStateChanged = mdd_BodyItem_->sigKinematicStateChanged().connect(
	      //    bind(&MetaDataViewImpl::updateKinematicState, this, true));
				mdd_updateKinematicStateLater);
	  }
	}
}

void TeachingEventHandler::mdd_updateKinematicState(bool blockSignals) {
	DDEBUG("TeachingEventHandler::mdd_updateKinematicState");

	if (mdd_BodyItem_ && mdd_selectedModel_) {
	  Link* currentLink = mdd_BodyItem_->body()->rootLink();
		mdd_selectedModel_->setPosX(currentLink->p()[0]);
		mdd_selectedModel_->setPosY(currentLink->p()[1]);
		mdd_selectedModel_->setPosZ(currentLink->p()[2]);

	  const Matrix3 R = currentLink->attitude();
	  const Vector3 rpy = rpyFromRot(R);
		mdd_selectedModel_->setRotRx(degree(rpy[0]));
		mdd_selectedModel_->setRotRy(degree(rpy[1]));
		mdd_selectedModel_->setRotRz(degree(rpy[2]));

	  if (mdd_selectedModel_ == mdd_CurrentModel_) {
			eventSkip_ = true;
			mdd_->updateContents(mdd_CurrentModel_);
			eventSkip_ = false;
	  }
	}
}

void TeachingEventHandler::mdd_ModelPositionChanged(double posX, double posY, double posZ, double rotX, double rotY, double rotZ) {
	if (eventSkip_) return;

	if (mdd_CurrentModel_) {
    if(mdd_CurrentModel_->getModelMaster()) {
	    if (mdd_CurrentModel_->getModelMaster()->getModelItem()) {
	      if (dbl_eq(posX, mdd_CurrentModel_->getPosX()) == false
	        || dbl_eq(posY, mdd_CurrentModel_->getPosY()) == false
	        || dbl_eq(posZ, mdd_CurrentModel_->getPosZ()) == false
	        || dbl_eq(rotX, mdd_CurrentModel_->getRotRx()) == false
	        || dbl_eq(rotY, mdd_CurrentModel_->getRotRy()) == false
	        || dbl_eq(rotZ, mdd_CurrentModel_->getRotRz()) == false) {
	        ChoreonoidUtil::updateModelItemPosition(mdd_CurrentModel_->getModelMaster()->getModelItem(), posX, posY, posZ, rotX, rotY, rotZ);
				  mdd_CurrentModel_->setPosX(posX);
				  mdd_CurrentModel_->setPosY(posY);
				  mdd_CurrentModel_->setPosZ(posZ);
				  mdd_CurrentModel_->setRotRx(rotX);
				  mdd_CurrentModel_->setRotRy(rotY);
				  mdd_CurrentModel_->setRotRz(rotZ);
	      }
	    }
    }
	}
}

bool TeachingEventHandler::mdd_AddModelClicked() {
	DDEBUG("TeachingEventHandler::mdd_AddModelClicked");

	if (!mdd_CurrentModelMaster_) return false;

	ModelParamPtr param = TeachingDataHolder::instance()->addModel(com_CurrentTask_, mdd_CurrentModelMaster_);
	ChoreonoidUtil::loadModelItem(param);
	ChoreonoidUtil::showAllModelItem();

	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	mdd_->showModelGrid(modelList);

	return true;
}

bool TeachingEventHandler::mdd_DeleteModelClicked() {
	if (!mdd_CurrentModel_) return false;

	ChoreonoidUtil::unLoadModelMasterItem(mdd_CurrentModel_->getModelMaster());
	mdd_connectionToKinematicStateChanged.disconnect();
	ChoreonoidUtil::showAllModelItem();

	mdd_CurrentModel_->setDelete();

	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	mdd_->showModelGrid(modelList);

	return true;
}

bool TeachingEventHandler::mdd_CheckModel(QString target) {
  for (ModelParamPtr model : com_CurrentTask_->getModelList()) {
    if (model->getId() == mdd_CurrentModel_->getId()) continue;
    if (model->getRName() == target) return false;
  }
  return true;
}

void TeachingEventHandler::mdd_OkClicked(QString rname, int type, double posX, double posY, double posZ, double rotX, double rotY, double rotZ, int hide) {
  if (0 < rname.length() && mdd_CurrentModel_) {
    mdd_CurrentModel_->setRName(rname);
    mdd_CurrentModel_->setType(type);
    mdd_CurrentModel_->setPosX(posX);
    mdd_CurrentModel_->setPosY(posY);
    mdd_CurrentModel_->setPosZ(posZ);
    mdd_CurrentModel_->setRotRx(rotX);
    mdd_CurrentModel_->setRotRy(rotY);
    mdd_CurrentModel_->setRotRz(rotZ);
    mdd_CurrentModel_->setHide(hide);
  }
	mdd_connectionToKinematicStateChanged.disconnect();
	mdd_currentBodyItemChangeConnection.disconnect();
	mdd_->close();
	mdd_ = 0;
  /////
  if (flv_CurrentFlow_) {
    ElementStmParamPtr targetState = 0;
    for (ElementStmParamPtr state : flv_CurrentFlow_->getStmElementList()) {
      TaskModelParamPtr task = state->getTaskParam();
      if (task) {
        if (task->getId() == com_CurrentTask_->getId()) {
          targetState = state;
          break;
        }
      }
    }
    if (targetState) {
      flv_->paramInfoUpdated(targetState);
    }
  }
}

void TeachingEventHandler::mdd_CancelClicked() {
	mdd_connectionToKinematicStateChanged.disconnect();
	mdd_currentBodyItemChangeConnection.disconnect();
	mdd_->close();
	mdd_ = 0;
}

//ParameterDialog
void TeachingEventHandler::prd_Loaded(ParameterDialog* dialog) {
	DDEBUG("TeachingEventHandler::prd_Loaded");
	if (!com_CurrentTask_) return;

	this->prd_ = dialog;
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	prd_->showModelInfo(modelList);
	vector<ParameterParamPtr> paramList = com_CurrentTask_->getActiveParameterList();
	prd_->showParamInfo(paramList);
	prd_->setTaskName(com_CurrentTask_->getName());

	prd_CurrentParam_ = 0;
	DDEBUG("TeachingEventHandler::prd_Loaded End");
}

void TeachingEventHandler::prd_ParamSelectionChanged(int newId, QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide) {
  DDEBUG_V("TeachingEventHandler::prd_ParamSelectionChanged %d", newId);

  prd_UpdateParam(name, id, type, paramType, unit, model_id, model_param_id, hide);
  prd_CurrentParam_ = com_CurrentTask_->getParameterById(newId);
  if(prd_CurrentParam_->getType()==PARAM_KIND_MODEL) {
    int modelId = prd_CurrentParam_->getModelId();
    ModelParamPtr model = com_CurrentTask_->getModelParamById(modelId);
    if(model) {
      ModelMasterParamPtr master = TeachingDataHolder::instance()->getModelMasterById(model->getMasterId());
      model->setModelMaster(master);
    }
  }
  prd_->updateContents(prd_CurrentParam_);
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	prd_->showModelInfo(modelList);
}

void TeachingEventHandler::prd_AddParamClicked(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide) {
  prd_UpdateParam(name, id, type, paramType, unit, model_id, model_param_id, hide);

  ParameterParamPtr param = TeachingDataHolder::instance()->addParameter(com_CurrentTask_);
  prd_->insertParameter(param);
}

bool TeachingEventHandler::prd_DeleteParamClicked() {
	if (prd_CurrentParam_ == 0) return false;
	prd_CurrentParam_->setDelete();
	return true;
}

bool TeachingEventHandler::prd_OkClicked(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide) {
  DDEBUG("TeachingEventHandler::prd_OkClicked");
  if (!com_CurrentTask_ || !prd_CurrentParam_) return true;

  for (ParameterParamPtr param : com_CurrentTask_->getParameterList()) {
    if (param->getId() == prd_CurrentParam_->getId()) continue;
    if (id == param->getRName()) {
      QMessageBox::warning(prd_, _("Parameter"), _("Duplicate specified ID."));
      return false;
    }
  }
  /////
  prd_UpdateParam(name, id, type, paramType, unit, model_id, model_param_id, hide);
  //
  vector<ParameterParamPtr> paramList = com_CurrentTask_->getActiveParameterList();
  for (int index = 0; index < paramList.size(); index++) {
    ParameterParamPtr param = paramList[index];

    if (param->getName().size() == 0) {
      QMessageBox::warning(prd_, _("Parameter"), _("Please input Parameter Name."));
      return false;
    }
    if (param->getRName().size() == 0) {
      QMessageBox::warning(prd_, _("Parameter"), _("Please input Parameter Id."));
      return false;
    }
    //
    int type = param->getType();
    if (type == PARAM_KIND_MODEL) {
      if (param->getModelId() < 0) continue;
      for (int idxSub = 0; idxSub < paramList.size(); idxSub++) {
        if (idxSub == index) continue;
        ParameterParamPtr paramSub = paramList[idxSub];
        if (paramSub->getType() != PARAM_KIND_MODEL) continue;
        if (param->getModelId() == paramSub->getModelId() && param->getModelParamId() == paramSub->getModelParamId()) {
          QMessageBox::warning(prd_, _("Parameter"), _("Target Model CANNOT duplicate."));
          return false;
        }
      }
    }
  }
  //
  if (flv_CurrentFlow_) {
    ElementStmParamPtr targetState = 0;
    for (ElementStmParamPtr state : flv_CurrentFlow_->getStmElementList()) {
      TaskModelParamPtr task = state->getTaskParam();
      if (task) {
        if (task->getId() == com_CurrentTask_->getId()) {
          targetState = state;
          break;
        }
      }
    }
    if (targetState) {
      flv_->paramInfoUpdated(targetState);
    }
  }
  //
  if (TeachingDataHolder::instance()->saveTaskParameter(com_CurrentTask_) == false) {
    QMessageBox::warning(prd_, _("Save Task Parameter Error"), TeachingDataHolder::instance()->getErrorStr());
    return false;
  }
  //
  com_CurrentTask_->clearParameterList();
  vector<ParameterParamPtr> newParamList = TeachingDataHolder::instance()->loadParameter(com_CurrentTask_->getId());
  for (int index = 0; index < newParamList.size(); index++) {
    com_CurrentTask_->addParameter(newParamList[index]);
  }
  prv_->setTaskParam(com_CurrentTask_, true);
  DDEBUG("TeachingEventHandler::prd_OkClicked End");

  return true;
}

void TeachingEventHandler::prd_UpdateParam(QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide) {
  DDEBUG("TeachingEventHandler::prd_UpdateParam");
  if (prd_CurrentParam_) {
    if (prd_CurrentParam_->getName() != name) {
      prd_CurrentParam_->setName(name);
    }
    if (prd_CurrentParam_->getRName() != id) {
      prd_CurrentParam_->setRName(id);
    }
    if (prd_CurrentParam_->getType() != type) {
      prd_CurrentParam_->setType(type);
    }
    if (prd_CurrentParam_->getHide() != hide) {
      prd_CurrentParam_->setHide(hide);
    }
    //
    if (type == PARAM_KIND_NORMAL) {
      if (prd_CurrentParam_->getUnit() != unit) {
        prd_CurrentParam_->setUnit(unit);
      }
      if (prd_CurrentParam_->getParamType() != paramType) {
        prd_CurrentParam_->setParamType(paramType);
      }
      if (prd_CurrentParam_->getModelId() != NULL_ID) {
        prd_CurrentParam_->setModelId(NULL_ID);
      }
      if (prd_CurrentParam_->getModelParamId() != NULL_ID) {
        prd_CurrentParam_->setModelParamId(NULL_ID);
      }

    } else {
      if (prd_CurrentParam_->getModelId() != model_id) {
        prd_CurrentParam_->setModelId(model_id);
      }
      if (prd_CurrentParam_->getModelParamId() != model_param_id) {
        prd_CurrentParam_->setModelParamId(model_param_id);
      }
      if (prd_CurrentParam_->getUnit().length() != 0) {
        prd_CurrentParam_->setUnit("");
      }
      if (prd_CurrentParam_->getParamType() != PARAM_TYPE_FRAME) {
        prd_CurrentParam_->setParamType(PARAM_TYPE_FRAME);
      }
    }
  }
  DDEBUG("TeachingEventHandler::prd_UpdateParam End");
}

void TeachingEventHandler::prd_ModelTableSelectionChanged(int selectedId) {
  DDEBUG_V("TeachingEventHandler::prd_ModelTableSelectionChanged %d", selectedId);
  vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
  for (ModelParamPtr model : modelList) {
    if (model->getId() == selectedId) {
      if(model->getModelMaster()) {
        vector<ModelParameterParamPtr> paramList = model->getModelMaster()->getActiveModelParamList();
        prd_->showModelParamInfo(paramList);
        return;
      }
    }
  }
}

vector<ModelParameterParamPtr> TeachingEventHandler::prd_ModelSelectionChanged(int selectedId) {
  DDEBUG_V("TeachingEventHandler::prd_ModelSelectionChanged %d", selectedId);
  vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
  for (ModelParamPtr model : modelList) {
    if (model->getId() == selectedId) {
      if( model->getModelMaster()) {
        return model->getModelMaster()->getActiveModelParamList();
      } else {
        break;
      }
    }
  }
  vector<ModelParameterParamPtr> result;
  return result;
}

//ArgumentDialog
void TeachingEventHandler::agd_ModelSelectionChanged(int selectedId) {
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	for (ModelParamPtr model : modelList) {
		if (model->getId() == selectedId) {
      if(model->getModelMaster()) {
			  vector<ModelParameterParamPtr> paramList = model->getModelMaster()->getActiveModelParamList();
			  agd_->showModelParamInfo(paramList);
			  return;
      }
		}
	}
}

void TeachingEventHandler::agd_Loaded(ArgumentDialog* dialog) {
  DDEBUG_V("TeachingEventHandler::agd_Loaded() %d", com_CurrentTask_->getId());

  this->agd_ = dialog;

	agd_Current_Action_ = 0;
	agd_Current_Arg_ = 0;

	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	agd_->showModelInfo(modelList);
	agd_->showParamInfo(com_CurrentTask_->getActiveParameterList(), modelList);
	agd_->showArgInfo(agd_Current_Stm_, agd_Current_Stm_->getActiveArgumentList());
	agd_->showActionInfo(agd_Current_Stm_->getActiveStateActionList());
}

void TeachingEventHandler::agd_ArgSelectionChanged(int selectedId, QString strDef) {
  DDEBUG_V("TeachingEventHandler::argSelectionChanged : %d", selectedId);
  if (selectedId == NULL_ID) return;
  if (agd_Current_Arg_) {
		if (agd_Current_Arg_->getValueDesc() != strDef) {
			agd_Current_Arg_->setValueDesc(strDef);
		}
	}
	agd_Current_Arg_ = agd_Current_Stm_->getArgumentById(selectedId);

	agd_->updateArgument(agd_Current_Arg_->getValueDesc());
}

void TeachingEventHandler::agd_ActionSelectionChanged(int selectedId, QString strAct, QString strModel, QString strTarget) {
	if (selectedId == NULL_ID) return;
	agd_Update(strAct, strModel, strTarget);
	agd_Current_Action_ = agd_Current_Stm_->getStateActionById(selectedId);
	agd_->updateAction(agd_Current_Action_);
}

void TeachingEventHandler::agd_AddClicked(QString strAct, QString strModel, QString strTarget) {
	DDEBUG("TeachingEventHandler::agd_AddClicked");
	agd_Update(strAct, strModel, strTarget);
	//
	int maxId = 0;
	for (unsigned int index = 0; index < agd_Current_Stm_->getActionList().size(); index++) {
		ElementStmActionParamPtr param = agd_Current_Stm_->getActionList()[index];
		if (maxId < param->getId()) {
			maxId = param->getId();
		}
	}
	maxId++;
	DDEBUG_V("id=%d", maxId);

	ElementStmActionParamPtr newAction = std::make_shared<ElementStmActionParam>(maxId, agd_Current_Stm_->getActionList().size(), "attach", "", "", true);
	agd_Current_Stm_->addModelAction(newAction);

	agd_->updateAddAction(newAction);
}

void TeachingEventHandler::agd_DeleteClicked() {
	DDEBUG("TeachingEventHandler::agd_DeleteClicked");
	if (agd_Current_Action_ == 0) return;

	agd_Current_Action_->setDelete();
	agd_Current_Action_ = 0;
}

bool TeachingEventHandler::agd_OKClicked(QString strName, QString strAct, QString strModel, QString strTarget, QString strArgDef) {
	agd_Current_Stm_->setCmdDspName(strName);
	agd_Update(strAct, strModel, strTarget);
	if (agd_Current_Arg_) {
		if (agd_Current_Arg_->getValueDesc() != strArgDef) {
			agd_Current_Arg_->setValueDesc(strArgDef);
		}
	}
	//
	for (unsigned int index = 0; index < agd_Current_Stm_->getActionList().size(); index++) {
		ElementStmActionParamPtr param = agd_Current_Stm_->getActionList()[index];
		if (param->getModel().length() == 0) {
		  QMessageBox::warning(agd_, _("Argument"), _("Error : Model Definition."));
		  return false;
		}
	}
	//
	ArgumentEstimator* handler = EstimatorFactory::getInstance().createArgEstimator(com_CurrentTask_);
	std::stringstream errorMsg;
	bool existError = false;
	for (int index = 0; index < agd_Current_Stm_->getActiveArgumentList().size(); index++) {
		ArgumentParamPtr param = agd_Current_Stm_->getArgList()[index];
		ArgumentDefParam* argDef = agd_Current_Stm_->getCommadDefParam()->getArgList()[index];
		if (argDef->getDirection() == 1) {
			QString targetStr = agd_Current_Stm_->getArgList()[index]->getValueDesc();
			ParameterParamPtr targetParam = NULL;
			for (ParameterParamPtr parmParm : com_CurrentTask_->getParameterList()) {
				if (parmParm->getRName() == targetStr) {
					targetParam = parmParm;
					break;
				}
			}
			if (targetParam == NULL) {
				errorMsg << "[" << param->getName().toStdString() << "] " << "target parameter [" << targetStr.toStdString() << "] NOT Exists." << std::endl;
				existError = true;
			}

		} else {
			if (0 < param->getValueDesc().trimmed().length()) {
				string strError;
				if (handler->checkSyntax(0, com_CurrentTask_, param->getValueDesc(), strError) == false) {
					DDEBUG_V("checkSyntax Error : %s", param->getValueDesc().toStdString().c_str());
					errorMsg << "[" << param->getName().toStdString() << "]" << strError << std::endl;
					existError = true;
				}
			}
			if (existError == false && agd_Current_Stm_->getMode() == DB_MODE_INSERT) {
				param->setNew();
			}
		}
	}
	EstimatorFactory::getInstance().deleteArgEstimator(handler);
	if (existError) {
		QMessageBox::warning(agd_, _("Argument"), QString::fromStdString(errorMsg.str()));
		return false;
	}
	return true;
}

void TeachingEventHandler::agd_CancelClicked() {
	DDEBUG("TeachingEventHandler::agd_CancelClicked()");
	for (ArgumentParamPtr param : agd_Current_Stm_->getArgList()) {
		param->setValueDesc(param->getValueDescOrg());
	}
}

void TeachingEventHandler::agd_Update(QString strAct, QString strModel, QString strTarget) {
	if (agd_Current_Action_) {
		if (agd_Current_Action_->getAction() != strAct) {
			agd_Current_Action_->setAction(strAct);
		}
		if (agd_Current_Action_->getModel() != strModel) {
			agd_Current_Action_->setModel(strModel);
		}
		if (agd_Current_Action_->getTarget() != strTarget) {
			agd_Current_Action_->setTarget(strTarget);
		}
	}
}

void TeachingEventHandler::agd_SetSeq(int selected, int seq) {
	for (unsigned int index = 0; index < agd_Current_Stm_->getActionList().size(); index++) {
		ElementStmActionParamPtr param = agd_Current_Stm_->getActionList()[index];
		if (param->getId() == selected) {
			if (param->getSeq() != seq) {
				param->setSeq(seq);
			}
			break;
		}
	}
}

}
