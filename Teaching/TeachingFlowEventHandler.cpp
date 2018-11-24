#include "TeachingEventHandler.h"

#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
//
#include "DecisionDialog.h"
#include "TaskExecutor.h"
#include "DataBaseManager.h"

#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;

namespace teaching {

//FlowView
void TeachingEventHandler::flv_Loaded(FlowViewImpl* view) {
	this->flv_ = view;
}

void TeachingEventHandler::flv_NewFlowClicked() {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_NewFlowClicked()");

	stv_->setStepStatus(false);
  if(allModelDisp_) flv_HideAllModels();

	flv_CurrentFlow_.reset(new FlowParam(NULL_ID, "", "", "", ""));
	flv_CurrentFlow_->setNew();

  ElementStmParamPtr newInitParam =
    std::make_shared<ElementStmParam>(flv_CurrentFlow_->getMaxStateId(), ELEMENT_START, "Initial", "Initial", 0, 0, "");
  newInitParam->setNew();
  flv_CurrentFlow_->addStmElement(newInitParam);

  ElementStmParamPtr newFinalParam =
    std::make_shared<ElementStmParam>(flv_CurrentFlow_->getMaxStateId(), ELEMENT_FINAL, "Final", "Final", 300, 0, "");
  newFinalParam->setNew();
  flv_CurrentFlow_->addStmElement(newFinalParam);

	flv_->dispView(flv_CurrentFlow_);
}

void TeachingEventHandler::flv_SearchClicked(bool canEdit) {
	DDEBUG("TeachingEventHandler::flv_SearchClicked");
	if (checkPaused()) return;

	stv_->setStepStatus(false);
  if(allModelDisp_) flv_HideAllModels();

	FlowSearchDialog dialog(canEdit, flv_);
	int ret = dialog.exec();
	//
	if (isFlowDeleted_) {
		if (flv_CurrentFlow_) {
			unloadTaskModelItems();
			flv_->clearView();
			com_CurrentTask_ = 0;
			com_CurrParam_ = 0;
			mdv_->clearTaskParam();
			stv_->clearTaskParam();
			prv_->clearTaskParam();
		}
		isFlowDeleted_ = false;
		tiv_CurrentTask_ = 0;
		flv_CurrentId_ = NULL_ID; flv_CurrentFlow_ = 0;
		com_CurrentTask_ = 0;
		return;
	}
	if (ret != QDialog::Accepted) {
		DDEBUG("TeachingEventHandler::flv_SearchClicked Canceled");
		return;
	}
	DDEBUG_V("selected : %d", flv_CurrentId_);

	unloadTaskModelItems();
	if (flv_CurrentFlow_) {
		com_CurrentTask_ = 0;
	}

	flv_CurrentFlow_ = TeachingDataHolder::instance()->getFlowById(flv_CurrentId_);
	if (flv_CurrentFlow_ == 0) {
		QMessageBox::warning(flv_, _("Flow"), _("FAILED to Open Flow."));
		return;
	}
	flv_->dispView(flv_CurrentFlow_);
	mdv_->clearTaskParam();
	stv_->clearTaskParam();
	prv_->clearTaskParam();
	//
	com_CurrentTask_ = 0;
	com_CurrParam_ = 0;
}

void TeachingEventHandler::flv_SelectionChanged(TaskModelParamPtr target) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_SelectionChanged");
	stv_->setStepStatus(false);

	stv_->updateTargetParam();
  if(allModelDisp_) {
    flv_HideAllModels();
  } else {
    unloadTaskModelItems();
  }

	com_CurrentTask_ = target;

	if (com_CurrentTask_) {
		updateComViews(com_CurrentTask_, true);

	} else {
		mdv_->clearTaskParam();
		stv_->clearTaskParam();
		prv_->clearTaskParam();
	}
  tiv_CurrentTask_ = 0;
	DDEBUG("TeachingEventHandler::flv_SelectionChanged End");
}

void TeachingEventHandler::flv_FlowExportClicked(QString name, QString comment) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_FlowExportClicked()");
	stv_->setStepStatus(false);

	if (!flv_CurrentFlow_) {
		QMessageBox::warning(flv_, _("Export Flow"), _("Please select target FLOW"));
		return;
	}

	QFileDialog::Options options;
	QString strSelectedFilter;
	QString strFName = QFileDialog::getSaveFileName(
		flv_, _("FlowModel File"), ".", _("YAML(*.yaml);;all(*.*)"), &strSelectedFilter, options);
	if (strFName.isEmpty()) return;
	//
	if (flv_CurrentFlow_->getName() != name) {
		flv_CurrentFlow_->setName(name);
	}
	if (flv_CurrentFlow_->getComment() != comment) {
		flv_CurrentFlow_->setComment(comment);
	}
	bool ret = TeachingUtil::exportFlow(strFName, flv_CurrentFlow_);
	if (ret == false) {
		QMessageBox::warning(flv_, _("Export Flow"), _("target FLOW export FAILED"));
	} else {
		QMessageBox::information(flv_, _("Export Flow"), _("target FLOW exported"));
	}
}

void TeachingEventHandler::flv_FlowImportClicked() {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_FlowImportClicked()");

	stv_->setStepStatus(false);
  if(allModelDisp_) flv_HideAllModels();

	QString strFName = QFileDialog::getOpenFileName(
		flv_, "TaskFlow File", ".", "YAML(*.yaml);;all(*.*)");
	if (strFName.isEmpty()) return;

	vector<FlowParamPtr> flowModelList;
	vector<ModelMasterParamPtr> masterList;
  QString errMessage;
  if (TeachingUtil::importFlow(strFName, flowModelList, masterList, errMessage) == false) {
		QMessageBox::warning(flv_, _("Import Flow"), errMessage);
		return;
	}
	if (flowModelList.size() == 0) {
		QMessageBox::warning(flv_, _("Import Flow"), _("FLOW import FAILED"));
		return;
	}

  //モデルマスタのチェック
  for (ModelMasterParamPtr master : masterList) {
    QString txtData = QString::fromUtf8(master->getData());
    QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
    int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
    if (0 < ret) {
      master->setIgnore();
      master->setId(ret);
      for(ModelDetailParamPtr detail : master->getModelDetailList() ) {
        detail->setIgnore();
      }
      for(ModelParameterParamPtr param : master->getModelParameterList() ) {
        param->setIgnore();
      }

      for (int idxFlow = 0; idxFlow < flowModelList.size(); idxFlow++) {
        DDEBUG_V("idxFlow : %d", idxFlow);
        FlowParamPtr targetFlow = flowModelList[idxFlow];
        for (ElementStmParamPtr state : targetFlow->getActiveStateList()) {
          TaskModelParamPtr task = state->getTaskParam();;
          if (task) {
            for (ModelParamPtr model : task->getModelList()) {
              if (model->getMasterId() == master->getId()) {
                model->setMasterId(ret);
              }
            }
          }
        }
      }
    } else {
      master->setHash(strHash);
    }
  }
  if (TeachingDataHolder::instance()->saveModelMasterList(masterList) == false) {
		QMessageBox::warning(flv_, _("Import Flow"), _("FLOW save FAILED"));
		return;
	}
	TeachingDataHolder::instance()->updateModelMaster();
	flv_CurrentFlow_ = flowModelList[0];

	for (ElementStmParamPtr state : flv_CurrentFlow_->getActiveStateList()) {
		TaskModelParamPtr task = state->getTaskParam();
		if (task) {
			for (ModelParamPtr model : task->getModelList()) {
				for (ModelMasterParamPtr master : masterList) {
					if (master->getOrgId() == model->getMasterId()) {
						model->setMasterId(master->getId());
						break;
					}
				}
			}
		}
	}
  for (FlowModelParamPtr model : flv_CurrentFlow_->getActiveModelList()) {
    for (ModelMasterParamPtr master : masterList) {
      if (master->getOrgId() == model->getMasterId()) {
        model->setMasterId(master->getId());
        break;
      }
    }
  }

	if (TeachingDataHolder::instance()->saveFlowModel(flv_CurrentFlow_) == false) {
		QMessageBox::warning(flv_, _("Import Flow"), _("FLOW save FAILED"));
	  return;
	 }
	flv_CurrentFlow_ = TeachingDataHolder::instance()->reGetFlowById(flv_CurrentFlow_->getId());
	flv_->dispView(flv_CurrentFlow_);

	QMessageBox::information(flv_, _("Import Flow"), _(" FLOW imported"));
}

void TeachingEventHandler::flv_RegistFlowClicked(QString name, QString comment) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_RegistFlowClicked()");

	stv_->setStepStatus(false);
	if (!flv_CurrentFlow_) return;

	if (flv_CurrentFlow_->getName() != name) {
		flv_CurrentFlow_->setName(name);
	}
	if (flv_CurrentFlow_->getComment() != comment) {
		flv_CurrentFlow_->setComment(comment);
	}
	//
	bool isChanged = false;
	for (ElementStmParamPtr state : flv_CurrentFlow_->getActiveStateList()) {
		if (state->getType() != ELEMENT_COMMAND) continue;
		TaskModelParamPtr task = state->getTaskParam();
		if (task) {
			for (ModelParamPtr model : task->getModelList()) {
				if (model->isChangedPosition() == false) continue;
				isChanged = true;
				break;
			}
			if (isChanged) break;
		}
	}
	if (isChanged) {
		QMessageBox::StandardButton ret = QMessageBox::question(flv_, _("Confirm"),
			_("Model Position was changed. Continue?"),
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::No) return;
	}

	stv_->updateTargetParam();
  QString errMessage;
  if (flv_->updateTargetFlowParam(errMessage) == false) {
    QMessageBox::warning(prd_, _("FlowView"), errMessage);
    return;
  }

	if (TeachingDataHolder::instance()->saveFlowModel(flv_CurrentFlow_)) {
		flv_CurrentFlow_ = TeachingDataHolder::instance()->reGetFlowById(flv_CurrentFlow_->getId());
		QMessageBox::information(flv_, _("Save Flow"), _("Target flow saved"));
		flv_->createStateMachine(flv_CurrentFlow_);

	} else {
		QMessageBox::warning(flv_, _("Save Flow"), TeachingDataHolder::instance()->getErrorStr());
	}
}

void TeachingEventHandler::flv_DeleteTaskClicked() {
	stv_->setStepStatus(false);
	flv_CurrentFlow_->setUpdate();
}

void TeachingEventHandler::flv_RunFlowClicked() {
  DDEBUG("TeachingEventHandler::flv_RunFlowClicked()");

  stv_->updateTargetParam();
  QString errMessage;
  if (flv_->updateTargetFlowParam(errMessage) == false) {
    QMessageBox::warning(prd_, _("FlowView"), errMessage);
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }
  //コマンド,マスタチェック
  vector<CommandDefParam*>commandList = TaskExecutor::instance()->getCommandDefList();
  vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
  QStringList errorList;
  for(ElementStmParamPtr flowElem : flv_CurrentFlow_->getActiveStateList()) {
    if (flowElem->getType() != ELEMENT_COMMAND) continue;
    TaskModelParamPtr targetTask = flowElem->getTaskParam();
    if(targetTask==0) continue;
    //
    DDEBUG("Check Command");
    for(ElementStmParamPtr state : targetTask->getActiveStateList()) {
      if(state->getType() != ELEMENT_COMMAND) continue;
      bool isExist = false;
      for(CommandDefParam* command : commandList) {
        if(command->getName()==state->getCmdName()) {
          isExist = true;
          break;
        }
      }
      if(isExist==false) {
        errorList.append(flowElem->getCmdDspName() + ":" + state->getCmdName());
      }
    }
    DDEBUG("Check Command End");
    //
    DDEBUG("Check Feature");
    vector<ModelParamPtr> modelList = targetTask->getActiveModelList();
    for(ParameterParamPtr param : targetTask->getActiveParameterList() ) {
      if (param->getType() == PARAM_KIND_NORMAL) continue;
      int modelId = param->getModelId();
      vector<ModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), ModelParamComparator(modelId));
      if (modelElem == modelList.end()) {
        errorList.append("Model Param. NOT EXIST :" + param->getName());
        continue;
      }
      int masterId = (*modelElem)->getMasterId();
      vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
      if (masterParamItr == modelMasterList.end()) {
        errorList.append("Model Master NOT EXIST :" + param->getName());
        continue;
      }
      int modelParamId = param->getModelParamId();
      DDEBUG_V("modelParamId %d", modelParamId);
      if (0 <= modelParamId) {
        vector<ModelParameterParamPtr> masterParamList = (*masterParamItr)->getModelParameterList();
        vector<ModelParameterParamPtr>::iterator featureItr = find_if(masterParamList.begin(), masterParamList.end(), ModelMasterParamComparator(modelParamId));
        if (featureItr == masterParamList.end()) {
          errorList.append("Feature NOT EXIST :" + param->getName());
        }
      }
    }
    DDEBUG("Check Feature End");
  }
  if (0 < errorList.size()) {
    QString errMsg = _("The following commands can not be executed.\n");
    for (int index = 0; index < errorList.size(); index++) {
      if (0<index) {
        errMsg.append("\n");
      }
      errMsg.append(errorList.at(index));
    }

    QMessageBox::warning(prd_, _("Flow"), errMsg);
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }
  //
  executor_->setCurrentTask(com_CurrentTask_);
	executor_->runFlow(flv_CurrentFlow_);
	com_CurrentTask_ = executor_->getCurrentTask();
  TeachingEventHandler::instance()->updateExecState(true);
}

void TeachingEventHandler::flv_InitPosClicked() {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_InitPosClicked()");

	if (!flv_CurrentFlow_) return;
	stv_->setStepStatus(false);

  for (FlowParameterParamPtr targetParam : flv_CurrentFlow_->getActiveFlowParamList()) {
    targetParam->setInitialValue();
  }
	for (ElementStmParamPtr targetState : flv_CurrentFlow_->getActiveStateList()) {
		if (targetState->getType() == ELEMENT_COMMAND) {
			TaskModelParamPtr task = targetState->getTaskParam();
			for (ModelParamPtr model : task->getActiveModelList()) {
        model->setInitialPos();
			}
		}
	}
	executor_->detachAllModelItem();
}

void TeachingEventHandler::flv_EditClicked(ElementStmParamPtr target) {
  if (target) {
    if ((target->getType() == ELEMENT_COMMAND || target->getType() == ELEMENT_DECISION) == false) {
      QMessageBox::warning(flv_, _("TaskInstance"), _("Please select Task Instance Node or Decision Node. : ") + QString::number(target->getType()));
      return;
    }
    if (target->getType() == ELEMENT_DECISION) {
      for (FlowParameterParamPtr target : flv_CurrentFlow_->getActiveFlowParamList()) {
        target->updateParamInfo();
      }
      FlowDesisionDialog dialog(flv_CurrentFlow_, target, flv_);
      dialog.exec();
    } else {
      TaskInfoDialog dialog(target, flv_);
      dialog.exec();
    }
  }
}

void TeachingEventHandler::flv_ModelParamChanged(int flowModelId, ModelMasterParamPtr masterParam) {
  DDEBUG_V("TeachingEventHandler::flv_ModelParamChanged : %d, %d", flowModelId, masterParam->getId());
  unloadTaskModelItems();
  if (masterParam->getModelItem() == 0) {
    if (ChoreonoidUtil::makeModelItem(masterParam) == 0) {
      masterParam->setModelItem(0);
    }
  }
  flv_->modelParamUpdated(flowModelId, masterParam);
  if (com_CurrentTask_) {
    bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(com_CurrentTask_);
    if (isUpdateTree) {
      ChoreonoidUtil::showAllModelItem();
    }
  }
  DDEBUG("TeachingEventHandler::flv_ModelParamChanged : End");
}

bool TeachingEventHandler::flv_Connected(QtNodes::Connection& target) {
  DDEBUG("TeachingEventHandler::flv_Connected()");

  Node* taskNode = target.getNode(PortType::In);
  int portIndex = target.getPortIndex(PortType::In);
  if (portIndex <= 0) return true;

  int targetId = taskNode->getParamId();
  if (!taskNode->nodeDataModel()) return false;
  if (taskNode->nodeDataModel()->portNames.size() <= portIndex - 1) return true;
  int id = taskNode->nodeDataModel()->portNames[portIndex - 1].id_;
  DDEBUG_V("portIndex : %d, id : %d", portIndex, id);

  Node* sourceNode = target.getNode(PortType::Out);
  int sourceId = sourceNode->getParamId();
  int sourcePortIndex = target.getPortIndex(PortType::Out);
  DDEBUG_V("sourcePortIndex : %d", sourcePortIndex);

  vector<ElementStmParamPtr> stateList = flv_CurrentFlow_->getActiveStateList();
  vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
  if (targetElem == stateList.end()) return false;
  TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();
  DDEBUG_V("Task Name : %s", taskParam->getName().toStdString().c_str());

  //モデルパラメータの場合
  if (sourceNode->nodeDataModel()->name() == "Model Param") {
    NodeDataType dataType = sourceNode->nodeDataModel()->dataType(PortType::Out, sourcePortIndex);
    DDEBUG_V("dataType : %s", dataType.id.toStdString().c_str());
    //FlowModelParameterの検索
    vector<FlowModelParamPtr> modelList = flv_CurrentFlow_->getActiveModelList();
    vector<FlowModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(sourceId));
    if (modelElem == modelList.end()) return false;
    DDEBUG_V("Master Id : %d", (*modelElem)->getMasterId());

    if (dataType.id == "modelshape") {
      //モデルポートの場合
      int masterId = (*modelElem)->getMasterId();

      ModelParamPtr model = taskParam->getModelParamById(id);
      DDEBUG_V("Model Name : %s", model->getRName().toStdString().c_str());
      vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
      vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
      if (masterParamItr != modelMasterList.end()) {
        ChoreonoidUtil::replaceMaster(model, *masterParamItr);
      }
      ///////////////
      DDEBUG("Connect Origin");
      QString sourcePort = "origin";
	    for (ParameterParamPtr targetParam : taskParam->getActiveParameterList()) {
		    if (targetParam->getType() != PARAM_KIND_MODEL) continue;
        if (targetParam->getModelId() != model->getId()) continue;
        QString targetPort = targetParam->getName() + ":Mdl";
        flv_->connectModelToTask(sourceNode, sourcePort, taskNode, targetPort);
	    }
      ///////////////

    } else if (dataType.id == "modeldata") {
      //データポートの場合
      QString portName = sourceNode->nodeDataModel()->portNames[sourcePortIndex].name_;
      DDEBUG_V("portName : %s", portName.toStdString().c_str());
      if(portName=="origin") {
        ParameterParamPtr paramTask = taskParam->getParameterById(id);
        if(paramTask->getType()==PARAM_KIND_NORMAL) {
          QMessageBox::warning(flv_, _("Flow Parameter"), _("The type of the parameter of the connection destination does not match."));
          isFlowSkip_ = true;
          return false;
        }
        DDEBUG_V("Param Name : %s", paramTask->getName().toStdString().c_str());
        ModelParamPtr model = taskParam->getModelParamById(paramTask->getModelId());
        DDEBUG_V("Model Name : %s", model->getRName().toStdString().c_str());
        if( (*modelElem)->getPosture()==0) {
          (*modelElem)->setPosture(model->getPosture());
        } else {
          model->setPosture((*modelElem)->getPosture());
        }
      }
    }

  } else if (sourceNode->nodeDataModel()->name().startsWith("Flow Param")) {
    //フローパラメータの場合
    //FlowParameterの検索
    vector<FlowParameterParamPtr> paramList = flv_CurrentFlow_->getActiveFlowParamList();
    vector<FlowParameterParamPtr>::iterator paramElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(sourceId));
    if (paramElem == paramList.end()) return false;

    ParameterParamPtr param = taskParam->getParameterById(id);
    DDEBUG_V("Flow Param Type:%d, Task Param Type(id=%d):%d %s", (*paramElem)->getType(), id, param->getParamType(), param->getName().toStdString().c_str());
    if (param->getParamType() != (*paramElem)->getType()) {
      QMessageBox::warning(flv_, _("Flow Parameter"), _("The type of the parameter of the connection destination does not match."));
      return false;
    }
    //
    param->setFlowParam(*paramElem);
    if ((*paramElem)->isFirst()) {
      (*paramElem)->setWidgetValue(param->getDBValues());
    }
  }
  return true;
}

void TeachingEventHandler::flv_Disconnected(QtNodes::Connection& target) {
  if(isFlowSkip_) {
    isFlowSkip_ = false;
    return;
  }
  DDEBUG("TeachingEventHandler::flv_Disconnected()");

  Node* taskNode = target.getNode(PortType::In);
  if (!taskNode) return;
  int portIndex = target.getPortIndex(PortType::In);
  if (portIndex <= 0) {
    DDEBUG("TeachingEventHandler::flv_Disconnected() End");
    return;
  }
  int targetId = taskNode->getParamId();
  if (!taskNode->nodeDataModel()) return;
  if (taskNode->nodeDataModel()->portNames.size() <= portIndex - 1) return;
  int id = taskNode->nodeDataModel()->portNames[portIndex - 1].id_;
  DDEBUG_V("portIndex : %d, id : %d", portIndex, id);

  Node* sourceNode = target.getNode(PortType::Out);
  if (!sourceNode || !sourceNode->nodeDataModel()) return;
  int sourceId = sourceNode->getParamId();
  int sourcePortIndex = target.getPortIndex(PortType::Out);

  vector<ElementStmParamPtr> stateList = flv_CurrentFlow_->getActiveStateList();
  vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
  if (targetElem == stateList.end()) return;
  TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();
  if (!taskParam) return;
  DDEBUG_V("Task Name : %s", taskParam->getName().toStdString().c_str());

  //モデルパラメータの場合
  if (sourceNode->nodeDataModel()->name() == "Model Param") {
    NodeDataType dataType = sourceNode->nodeDataModel()->dataType(PortType::Out, sourcePortIndex);
    DDEBUG_V("dataType : %s", dataType.id.toStdString().c_str());
    if (dataType.id == "modelshape") {
      //モデルポートの場合
      if (taskParam->IsKeepMaster() == false) {
        ModelParamPtr model = taskParam->getModelParamById(id);
        if (!model) return;
        DDEBUG_V("Model Name : %s", model->getRName().toStdString().c_str());
        bool isLoaded = model->isLoaded();
        ChoreonoidUtil::unLoadModelItem(model);
        model->restoreModelMaster();
        if (isLoaded) {
          ChoreonoidUtil::loadModelItem(model);
          ChoreonoidUtil::showAllModelItem();
        }
      }
    } else if (dataType.id == "modeldata") {
      //データポートの場合
      QString portName = sourceNode->nodeDataModel()->portNames[sourcePortIndex].name_;
      DDEBUG_V("portName : %s", portName.toStdString().c_str());
      if(portName=="origin") {
        ParameterParamPtr paramTask = taskParam->getParameterById(id);
        if (!paramTask) return;
        ModelParamPtr model = taskParam->getModelParamById(paramTask->getModelId());
        if (!model) return;
        model->clearPosture();
        //
        //FlowParam側の処理
        if( flv_->checkOutConnection(sourceId, sourcePortIndex)==false) {
          DDEBUG("Clear FlowParam");
          vector<FlowModelParamPtr> modelList = flv_CurrentFlow_->getActiveModelList();
          vector<FlowModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(sourceId));
          if (modelElem == modelList.end()) return;
          (*modelElem)->setPosture(0);
        }
      }
    }

  } else if (sourceNode->nodeDataModel()->name().startsWith("Flow Param")) {
    //フローパラメータの場合
    ParameterParamPtr param = taskParam->getParameterById(id);
    if (!param) return;
    DDEBUG_V("Task Param Type(id=%d):%d %s", id, param->getParamType(), param->getName().toStdString().c_str());
    param->restoreParameter();
  }
  DDEBUG("TeachingEventHandler::flv_Disconnected() End");
}

void TeachingEventHandler::flv_PortDispSetting(bool isActive) {
  if (isActive == false) {
    QMessageBox::warning(flv_, _("Port Disp"), _("Please select target TASK"));
    return;
  }
  if (!com_CurrentTask_) return;

  PortDispDialog dialog(com_CurrentTask_, flv_);
  dialog.exec();

  ElementStmParamPtr targetState = 0;
  for (ElementStmParamPtr state : flv_CurrentFlow_->getActiveStateList()) {
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

void TeachingEventHandler::flv_AllModelDisp(bool checked) {
  allModelDisp_ = checked;
  if (!flv_CurrentFlow_) return;

  if(checked) {
    unloadTaskModelItems();

    for(ElementStmParamPtr state : flv_CurrentFlow_->getActiveStateList()) {
      if(state->getType()!=ELEMENT_COMMAND) continue;
      DDEBUG_V("State : %s", state->getCmdDspName().toStdString().c_str());
  		TaskModelParamPtr task = state->getTaskParam();
	    TeachingUtil::loadTaskDetailData(task);
	    ChoreonoidUtil::loadTaskModelItem(task);
    }
		ChoreonoidUtil::showAllModelItem();

  } else {
    flv_HideAllModels();
  }
}

void TeachingEventHandler::flv_HideAllModels() {
  DDEBUG("TeachingEventHandler::flv_HideAllModels");
  ChoreonoidUtil::deselectTreeItem();
  for(ElementStmParamPtr state : flv_CurrentFlow_->getActiveStateList()) {
    if(state->getType()!=ELEMENT_COMMAND) continue;
  	TaskModelParamPtr task = state->getTaskParam();
	  ChoreonoidUtil::unLoadTaskModelItem(task);
  }
  flv_->cancelAllModel();
}

  bool TeachingEventHandler::flv_RenameNode(QString currentName, QString newName) {
    return this->flv_->renameNode(currentName, newName);
  }

  void TeachingEventHandler::flv_GetNodeByName(QString name) {
    this->flv_->getNodeByName(name);
  }

  bool TeachingEventHandler::flv_ConnectNodes(QString from, QString fromPort, QString to, QString toPort) {
    return this->flv_->connectNodes(from, fromPort, to, toPort);
  }

  bool TeachingEventHandler::flv_CreateNode(QString modelName, QPoint pos) {
    return this->flv_->createNode(modelName, pos);
  }

  bool TeachingEventHandler::tst_DeleteAllFlows() {
    vector<string> condList;
    bool isOr = false;
    condList.push_back("");
    TeachingDataHolder::instance()->searchFlow(condList, isOr);
    for (auto f : TeachingDataHolder::instance()->getFlowList()) {
      if (!TeachingDataHolder::instance()->deleteFlow(f->getId())) {
        return false;
      }
    }
    return true;
  }

  bool TeachingEventHandler::tst_DeleteAllTasks() {
    for (auto tmi : TeachingDataHolder::instance()->getTaskList()) {
      if (!TeachingDataHolder::instance()->deleteTaskModel(tmi->getId())) {
        return false;
      }
    }
    return true;
  }

  void TeachingEventHandler::tst_ClearFlowScene() {
    this->flv_->clearFlowScene();
  }


}
