#include "TeachingEventHandler.h"

#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
//
#include <cnoid/BodyBar>
#include <boost/bind.hpp>
//
#include "DecisionDialog.h"
#include "TaskExecutor.h"
#include "ControllerManager.h"
#include "DataBaseManager.h"

#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;

namespace teaching {

TeachingEventHandler::~TeachingEventHandler() {
  DDEBUG("TeachingEventHandler Destruct");
}

TeachingEventHandler* TeachingEventHandler::instance() {
  static TeachingEventHandler* holder = new TeachingEventHandler();
  return holder;
}

//TaskInstanceView
void TeachingEventHandler::tiv_Loaded(TaskInstanceViewImpl* view) {
	this->tiv_ = view;
}
void TeachingEventHandler::tiv_TaskSelectionChanged(int selectedId, QString strTask) {
	DDEBUG_V("TeachingEventHandler::tiv_TaskSelectionChanged %d", selectedId);

	stv_->setStepStatus(false);
	mdv_->updateTaskParam();
	prv_SetInputValues();

  if(allModelDisp_) {
    flv_HideAllModels();
  }
  unloadTaskModelItems();
	if (tiv_CurrentTask_) {
		if (tiv_CurrentTask_->getName() != strTask) {
			tiv_CurrentTask_->setName(strTask);
		}
		tiv_->updateGrid(tiv_CurrentTask_);
	}

	tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
	com_CurrentTask_ = tiv_CurrentTask_;
	//
	if (tiv_CurrentTask_) {
		updateComViews(tiv_CurrentTask_);
	}
	//
	if (mdd_) {
		vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
		vector<ModelParamPtr> modelList = tiv_CurrentTask_->getActiveModelList();
		mdd_->showModelGrid(modelList);
		mdd_->showModelMasterGrid(modelMasterList);

		mdd_CurrentModel_ = 0;
		mdd_selectedModel_ = 0;
		mdd_BodyItem_ = 0;
		mdd_currentBodyItemChangeConnection = BodyBar::instance()->sigCurrentBodyItemChanged().connect(
			bind(&TeachingEventHandler::mdd_CurrentBodyItemChanged, this, _1));
	}
	DDEBUG("TeachingEventHandler::tiv_TaskSelectionChanged End");
}

bool TeachingEventHandler::tiv_DeleteTaskClicked(int selectedId) {
	DDEBUG("TeachingEventHandler::tiv_DeleteTaskClicked");
	if (checkPaused()) return false;

	stv_->setStepStatus(false);

  if (0 <= selectedId) {
    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }
  if (!tiv_CurrentTask_) return false;

	unloadTaskModelItems();

	if (TeachingDataHolder::instance()->deleteTaskModel(tiv_CurrentTask_->getId()) == false) {
		QMessageBox::warning(tiv_, _("Task Delete"), TeachingDataHolder::instance()->getErrorStr());
		return true;
	}
	QMessageBox::information(tiv_, _("Task Delete"), _("Target task deleted"));

	this->mdv_->clearTaskParam();
	this->stv_->clearTaskParam();
	this->prv_->clearTaskParam();

	tiv_CurrentTask_ = 0;
	com_CurrentTask_ = 0;

	tiv_SearchTaskInstance("");
	DDEBUG("TeachingEventHandler::tiv_DeleteTaskClicked End");

	return true;
}

void TeachingEventHandler::tiv_TaskExportClicked(int selectedId, QString strTask) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::tiv_TaskExportClicked()");

	stv_->setStepStatus(false);

  if (0 <= selectedId) {
    stv_->updateTargetParam();
    mdv_->updateTaskParam();
    prv_SetInputValues();

    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }
  if (!tiv_CurrentTask_) {
		QMessageBox::warning(tiv_, _("Output Task"), _("Please select target TASK"));
		return;
	}

	if (tiv_CurrentTask_->getName() != strTask) {
		tiv_CurrentTask_->setName(strTask);
	}
	tiv_->updateGrid(tiv_CurrentTask_);

	QFileDialog::Options options;
	QString strSelectedFilter;
	QString strFName = QFileDialog::getSaveFileName(
		tiv_, _("TaskModel File"), ".",
		_("YAML(*.yaml);;all(*.*)"),
		&strSelectedFilter, options);
	if (strFName.isEmpty()) return;
	DDEBUG_V("saveTaskClicked : %s", strFName.toStdString().c_str());

	if (TeachingUtil::exportTask(strFName, tiv_CurrentTask_)) {
		QMessageBox::information(tiv_, _("Output Task"), _("target TASK exported"));
	} else {
		QMessageBox::warning(tiv_, _("Output Task"), _("target TASK export FAILED"));
	}
}

bool TeachingEventHandler::tiv_TaskImportClicked() {
  if (checkPaused()) return false;
  DDEBUG("TeachingEventHandler::tiv_TaskImportClicked()");
  stv_->setStepStatus(false);

  QString strFName = QFileDialog::getOpenFileName(
    tiv_, "TaskModel File", ".", "YAML(*.yaml);;all(*.*)");
  if (strFName.isEmpty()) return false;

  return tiv_TaskImport(strFName);
}

  bool TeachingEventHandler::tiv_TaskImport(QString strFName) {
    //タスク定義ファイルの読み込み
    vector<TaskModelParamPtr> taskInstList;
    vector<ModelMasterParamPtr> masterList;
    QString errMessage;
    if (TeachingUtil::importTask(strFName, taskInstList, masterList, errMessage) == false) {
      if (EVENT_HANDLER(checkTest())) {
        QMessageBox::warning(tiv_, _("Task Load Error"), errMessage);
      }
      return false;
    }
    //
    for (TaskModelParamPtr task : taskInstList) {
      for (ModelParamPtr model : task->getModelList()) {
        for (ModelMasterParamPtr master : masterList) {
          if (model->getMasterId() == master->getId()) {
            model->setModelMaster(master);
            break;
          }
        }
      }
    }
    //モデルマスタのチェック
    for (ModelMasterParamPtr master : masterList) {
      QString txtData = QString::fromUtf8(master->getData());
      QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
      int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
      DDEBUG_V("ModelMaster:%s, %d", master->getName().toStdString().c_str(), ret);

      if (0 < ret) {
        master->setIgnore();
        master->setId(ret);
        for(ModelDetailParamPtr detail : master->getModelDetailList() ) {
          detail->setIgnore();
        }
        for(ModelParameterParamPtr param : master->getModelParameterList() ) {
          param->setIgnore();
        }
      } else {
        master->setHash(strHash);
      }
    }
    //タスクの保存
    if (TeachingDataHolder::instance()->saveImportedTaskModel(taskInstList, masterList) == false) {
      if (EVENT_HANDLER(checkTest())) {
        QMessageBox::warning(tiv_, _("Task Import"), TeachingDataHolder::instance()->getErrorStr());
      }
      return false;
    }

    vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->getTaskList();
    tiv_->showGrid(taskList);

    if (EVENT_HANDLER(checkTest())) {
      QMessageBox::information(tiv_, _("Task Import"), _("target TASK imported"));
    }

	return true;
  }

void TeachingEventHandler::tiv_SearchClicked(QString cond) {
	DDEBUG("TeachingEventHandler::tiv_SearchClicked");
	stv_->setStepStatus(false);

	if (ControllerManager::instance()->isExistController() == false) {
		QMessageBox::warning(tiv_, _("Task Search Error"), _("Controller does NOT EXIST."));
		return;
	}
	//
	unloadTaskModelItems();

	mdv_->clearTaskParam();
	stv_->clearTaskParam();
	prv_->clearTaskParam();

	tiv_CurrentTask_ = 0;
	com_CurrentTask_ = 0;

	tiv_SearchTaskInstance(cond);
	DDEBUG("TeachingEventHandler::tiv_SearchClicked End");
}

void TeachingEventHandler::tiv_SearchTaskInstance(QString cond) {
	DDEBUG_V("TeachingEventHandler::tiv_SearchTaskInstance %s", cond.toStdString().c_str());

	vector<string> condList;
	QStringList targetList;
	bool isOr = false;
	if (cond.contains("||")) {
		isOr = true;
		targetList = cond.split("||");
	} else {
		targetList = cond.split(" ");
	}
	for (unsigned int index = 0; index < targetList.size(); index++) {
		QString each = targetList[index].trimmed();
		condList.push_back(each.toStdString());
	}
	vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->searchTaskModels(condList, isOr);
	tiv_->showGrid(taskList);
}

bool TeachingEventHandler::tiv_RegistTaskClicked(int selectedId, QString strTask) {
	if (checkPaused()) return false;
	DDEBUG("TeachingEventHandler::tiv_RegistTaskClicked()");

	stv_->setStepStatus(false);

  if (tiv_CurrentTask_ == 0) return false;

	for (ModelParamPtr model : tiv_CurrentTask_->getActiveModelList()) {
		if (model->isChangedPosition() == false) continue;
		//
		QMessageBox::StandardButton ret = QMessageBox::question(tiv_, _("Confirm"),
			_("Model Position was changed. Continue?"),
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::No) return false;
		break;
	}
  //
  if(TeachingUtil::checkNameStr(strTask)==false) {
		QMessageBox::information(tiv_, _("Save Task"),
      _("Characters that can not be used in names are included."));
    return false;
  }
	//
	if (tiv_CurrentTask_->getName() != strTask) {
		tiv_CurrentTask_->setName(strTask);
	}
	stv_->updateTargetParam();
	mdv_->updateTaskParam();
	prv_SetInputValues();
	tiv_->updateGrid(tiv_CurrentTask_);
	unloadTaskModelItems();

	if (TeachingDataHolder::instance()->saveTaskModel(tiv_CurrentTask_)) {
		tiv_->updateGrid(tiv_CurrentTask_);
		updateComViews(tiv_CurrentTask_);
		QMessageBox::information(tiv_, _("Save Task"), _("Target task saved."));
    return true;
	} else {
		QMessageBox::warning(tiv_, _("Save Task"), TeachingDataHolder::instance()->getErrorStr());
    return false;
	}
}

bool TeachingEventHandler::tiv_RegistNewTaskClicked(int selectedId, QString strTask, QString strCond) {
	if (checkPaused()) return false;
	DDEBUG("TeachingEventHandler::tiv_RegistNewTaskClicked()");

	stv_->setStepStatus(false);

  if (0 <= selectedId) {
    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }
  if (tiv_CurrentTask_ == 0) return false;

  //
  if(TeachingUtil::checkNameStr(strTask)==false) {
		QMessageBox::information(tiv_, _("Save Task"),
      _("Characters that can not be used in names are included."));
    return false;
  }
	//
	if (tiv_CurrentTask_->getName() != strTask) {
		tiv_CurrentTask_->setName(strTask);
	}
	stv_->updateTargetParam();
	mdv_->updateTaskParam();
	unloadTaskModelItems();

	tiv_CurrentTask_->setAllNewData();

	if (TeachingDataHolder::instance()->saveTaskModelasNew(tiv_CurrentTask_)) {
		tiv_SearchClicked("");
		QMessageBox::information(tiv_, _("Database"), _("Database updated"));
    return true;
	} else {
		QMessageBox::warning(tiv_, _("Database Error"), TeachingDataHolder::instance()->getErrorStr());
    return false;
	}
}
//MetaDataView
void TeachingEventHandler::mdv_Loaded(MetaDataViewImpl* view) {
	this->mdv_ = view;
}

void TeachingEventHandler::mdv_ModelClicked() {
	if (com_CurrentTask_) {
		if (mdd_) {
			mdd_->close();
			mdd_ = 0;
		}
		ModelDialog* dialog = new ModelDialog();
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->show();
	}
}

int TeachingEventHandler::mdv_DropEventImage(QString name, QString fileName) {
	return TeachingDataHolder::instance()->addImage(com_CurrentTask_, name, fileName);
}

int TeachingEventHandler::mdv_DropEventFile(QString name, QString fileName) {
	return TeachingDataHolder::instance()->addFile(com_CurrentTask_, name, fileName);
}

void TeachingEventHandler::mdv_FileOutputClicked(int id) {
	FileDataParamPtr target = com_CurrentTask_->getFileById(id);
	QByteArray data = target->getData();
	//
	QFileDialog fileDialog(mdv_);
	fileDialog.setFileMode(QFileDialog::Directory);
	fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
	if (fileDialog.exec() == false) return;
	QStringList strDirs = fileDialog.selectedFiles();
	//
	QString strDir = strDirs[0];
	strDir += QString("/") + target->getName();
	QFile file(strDir);
	file.open(QIODevice::WriteOnly);
	file.write(data);
	file.close();

	QMessageBox::information(mdv_, _("File Output"), _("Target FILE saved"));
}

void TeachingEventHandler::mdv_ImageOutputClicked(int id) {
	ImageDataParamPtr target = com_CurrentTask_->getImageById(id);
	QImage image = target->getData();

	QFileDialog fileDialog(mdv_);
	fileDialog.setFileMode(QFileDialog::Directory);
	fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
	if (fileDialog.exec() == false) return;
	QStringList strDirs = fileDialog.selectedFiles();
	//
	QString strDir = strDirs[0];
	strDir += QString("/") + target->getName();
	image.save(strDir);

	QMessageBox::information(mdv_, _("File Output"), _("Target IMAGE saved"));
}

void TeachingEventHandler::mdv_FileDeleteClicked(int id) {
	TeachingDataHolder::instance()->deleteFile(id, com_CurrentTask_);
}

void TeachingEventHandler::mdv_ImageDeleteClicked(int id) {
	TeachingDataHolder::instance()->deleteImage(id, com_CurrentTask_);
}

void TeachingEventHandler::mdv_ImageShowClicked(int id) {
	ImageDataParamPtr target = com_CurrentTask_->getImageById(id);
	QImage image = target->getData();

	if (!m_FigDialog_) {
		m_FigDialog_ = new FigureDialog(mdv_);
	}
	m_FigDialog_->setImage(image);
	m_FigDialog_->show();
}

FileDataParamPtr TeachingEventHandler::mdv_FileShowClicked(int id) {
	return com_CurrentTask_->getFileById(id);
}

void TeachingEventHandler::mdv_UpdateComment(QString comment) {
	DDEBUG("TeachingEventHandler::mdv_UpdateComment");
	if (com_CurrentTask_) {
		com_CurrentTask_->setComment(comment);
	}
  if (tiv_CurrentTask_) {
    tiv_CurrentTask_->setComment(comment);
  }
}

void TeachingEventHandler::mdv_UpdateFileSeq(int id, int seq) {
	DDEBUG("TeachingEventHandler::mdv_UpdateFileSeq");
	FileDataParamPtr target = com_CurrentTask_->getFileById(id);
	if (target->getSeq() == seq) return;
	target->setSeq(seq);
	target->setUpdate();
}

void TeachingEventHandler::mdv_UpdateImageSeq(int id, int seq) {
	DDEBUG_V("TeachingEventHandler::mdv_UpdateImageSeq id=%d, seq=%d", id, seq);
	ImageDataParamPtr target = com_CurrentTask_->getImageById(id);
	if (target->getSeq() == seq) return;
	target->setSeq(seq);
	target->setUpdate();
}

//StateMachineView
void TeachingEventHandler::stv_Loaded(StateMachineViewImpl* view) {
	this->stv_ = view;
}

void TeachingEventHandler::stv_EditClicked(ElementStmParamPtr target) {
	DDEBUG("TeachingEventHandler::stv_EditClicked()");
	if (target == 0) {
		ExecEnvDialog dialog(com_CurrentTask_, stv_);
		dialog.exec();
		return;
	}
	/////
	agd_Current_Stm_ = target;
	if ((agd_Current_Stm_->getType() == ELEMENT_COMMAND || agd_Current_Stm_->getType() == ELEMENT_DECISION) == false) {
		QMessageBox::warning(stv_, _("Command"), _("Please select Command or Decision Node. : ") + QString::number(agd_Current_Stm_->getType()));
		return;
	}
	//
	if (agd_Current_Stm_->getType() == ELEMENT_DECISION) {
		DesisionDialog dialog(com_CurrentTask_, agd_Current_Stm_, stv_);
		dialog.exec();
		return;
	}
	//
	if (agd_Current_Stm_->getType() == ELEMENT_COMMAND) {
		vector<CommandDefParam*> commandList = TaskExecutor::instance()->getCommandDefList();

		if (agd_Current_Stm_->getArgList().size() == 0) {
			DDEBUG("editClicked : No Arg");
			QString strCmd = agd_Current_Stm_->getCmdName();
			for (int index = 0; index < commandList.size(); index++) {
				CommandDefParam* param = commandList[index];
				if (param->getCmdName() != strCmd) continue;
				vector<ArgumentDefParam*> argList = param->getArgList();
				for (int idxArg = 0; idxArg < argList.size(); idxArg++) {
					ArgumentDefParam* arg = argList[idxArg];
          ArgumentParamPtr argParam = std::make_shared<ArgumentParam>(idxArg + 1, idxArg + 1, QString::fromStdString(arg->getName()), "");
					argParam->setNew();
					agd_Current_Stm_->addArgument(argParam);
				}
			}
		}

		ArgumentDialog dialog(stv_);
		dialog.exec();
		if (dialog.isOK()) {
			agd_Current_Stm_->getRealElem()->nodeDataModel()->setTaskName(agd_Current_Stm_->getCmdDspName());
			agd_Current_Stm_->getRealElem()->nodeGraphicsObject().update();
		}
	}
}

//ParameterView
void TeachingEventHandler::prv_Loaded(ParameterViewImpl* view) {
	this->prv_ = view;
}

void TeachingEventHandler::prv_SetInputValues() {
	DDEBUG("TeachingEventHandler::prv_SetInputValues");
	if (com_CurrentTask_) {
		vector<ParameterParamPtr> paramList = com_CurrentTask_->getParameterList();
    DDEBUG_V("paramList:%d", paramList.size());
    vector<ParameterParamPtr>::iterator itParam = paramList.begin();
		while (itParam != paramList.end()) {
			(*itParam)->saveValues();
			++itParam;
		}
	}
  DDEBUG("TeachingEventHandler::prv_SetInputValues End");
}

//TaskExecutionView
void TeachingEventHandler::tev_stm_RunClicked(ElementStmParamPtr target) {
	DDEBUG("TeachingEventHandler::tev_stm_RunClicked()");
	if (checkPaused()) return;

  if (target == NULL) {
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }

	prv_SetInputValues();
	if (target->getType() != ELEMENT_COMMAND) {
		QMessageBox::warning(stv_, _("Run Command"), _("Please select Command Element."));
    TeachingEventHandler::instance()->updateExecState(true);
		return;
	}
  //コマンドチェック
  vector<CommandDefParam*>commandList = TaskExecutor::instance()->getCommandDefList();
  bool isExist = false;
  for(CommandDefParam* command : commandList) {
    DDEBUG_V("source:%s, target:%s", target->getCmdName().toStdString(), command->getCmdName().toStdString().c_str());
    if(command->getCmdName()==target->getCmdName()) {
      isExist = true;
      break;
    }
  }
  if(isExist==false) {
    QMessageBox::warning(prd_, _("Run Command"), _("This command can not be executed."));
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }
	//
	executor_->setCurrentTask(com_CurrentTask_);
	executor_->setCurrentElement(target);
	if (executor_->runSingleCommand() == false) {
		QMessageBox::information(stv_, _("Run Command"), _("Target Command FAILED."));
	}
  TeachingEventHandler::instance()->updateExecState(true);
}

void TeachingEventHandler::tev_stm_StepClicked() {
	DDEBUG("TeachingEventHandler::tev_stm_StepClicked()");
  ExecResult ret = executor_->doTaskOperationStep();
	if (ret == ExecResult::EXEC_BREAK || ret == ExecResult::EXEC_FINISHED) {
		stv_->setStepStatus(true);
	} else {
		stv_->setStepStatus(false);
	}
}

void TeachingEventHandler::tev_stm_ContClicked() {
	DDEBUG("TeachingEventHandler::tev_stm_ContClicked()");

	tiv_->setButtonEnableMode(false);
	flv_->setButtonEnableMode(false);

	ExecResult result;
	ExecResult ret = executor_->doTaskOperationStep();
	result = ret;
	if (ret == ExecResult::EXEC_BREAK) {
		ExecResult retFlow = executor_->doFlowOperationCont();
		result = retFlow;
		if (retFlow == ExecResult::EXEC_FINISHED) {
			com_CurrentTask_ = executor_->getCurrentTask();
		}
	}
	//
	if (result == ExecResult::EXEC_BREAK) {
		stv_->setStepStatus(true);
	} else {
		stv_->setStepStatus(false);
	}
	tiv_->setButtonEnableMode(true);
	flv_->setButtonEnableMode(true);
}

void TeachingEventHandler::tev_RunTaskClicked(int selectedId, bool isFlow) {
	DDEBUG_V("TeachingEventHandler::tev_RunTaskClicked() %d", selectedId);
  if(isFlow) {
    if (com_CurrentTask_) {
      selectedId = com_CurrentTask_->getId();
    }
  }
  if(selectedId<0) {
    QMessageBox::warning(tiv_, _("Run Task"), _("Please select target TASK"));
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }

	stv_->updateTargetParam();
  prv_SetInputValues();

  if (0 <= selectedId && !isFlow) {
    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }
  //コマンドチェック
  vector<CommandDefParam*>commandList = TaskExecutor::instance()->getCommandDefList();
  QStringList errorList;
  for(ElementStmParamPtr state : com_CurrentTask_->getActiveStateList()) {
    if(state->getType() != ELEMENT_COMMAND) continue;
    bool isExist = false;
    for(CommandDefParam* command : commandList) {
      if(command->getCmdName()==state->getCmdName()) {
        isExist = true;
        break;
      }
    }
    if(isExist==false) {
      errorList.append(state->getCmdName());
    }
  }
  if (0 < errorList.size()) {
    QString errMsg = _("The following commands can not be executed.\n");
    for (int index = 0; index < errorList.size(); index++) {
      if (0<index) {
        errMsg.append("\n");
      }
      errMsg.append(errorList.at(index));
    }

    QMessageBox::warning(prd_, _("Run Task"), errMsg);
    TeachingEventHandler::instance()->updateExecState(true);
    return;
  }
  //
	executor_->setCurrentTask(com_CurrentTask_);
	executor_->setCurrentElement(com_CurrParam_);
	executor_->runSingleTask();
  TeachingEventHandler::instance()->updateExecState(true);
}

void TeachingEventHandler::tev_AbortClicked() {
	DDEBUG("TeachingEventHandler::tev_AbortClicked");
	executor_->abortOperation();
}

void TeachingEventHandler::tiv_InitPosClicked() {
	DDEBUG("TeachingEventHandler::initPosClicked()");

	if (checkPaused()) return;
	if (!tiv_CurrentTask_) return;
	stv_->setStepStatus(false);
	//
	for (ModelParamPtr model : tiv_CurrentTask_->getActiveModelList()) {
		model->setInitialPos();
	}

	executor_->detachAllModelItem();
}

bool TeachingEventHandler::checkPaused() {
	if (executor_->isBreak() == false) return false;
	//
	QMessageBox::StandardButton ret = QMessageBox::question(0, _("Confirm"),
		_("Cancel pausing processing?"),
		QMessageBox::Yes | QMessageBox::No);
	if (ret == QMessageBox::No) return true;
	//
	executor_->setBreak(false);
  stv_->setStepStatus(false);
	return false;
}
///////////
void TeachingEventHandler::unloadTaskModelItems() {
	DDEBUG("TeachingEventHandler::unloadTaskModelItems");
	ChoreonoidUtil::deselectTreeItem();

	if (com_CurrentTask_) {
		ChoreonoidUtil::unLoadTaskModelItem(com_CurrentTask_);
	}
	if (tiv_CurrentTask_) {
		ChoreonoidUtil::unLoadTaskModelItem(tiv_CurrentTask_);
	}

}

void TeachingEventHandler::updateComViews(TaskModelParamPtr targetTask, bool isFlowView) {
	DDEBUG("TeachingEventHandler::updateComViews()");

	TeachingUtil::loadTaskDetailData(targetTask);
	bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(targetTask);

	mdv_->setTaskParam(targetTask);
	stv_->setTaskParam(targetTask);
	prv_->setTaskParam(targetTask, isFlowView);

	//即更新を行うとエラーになってしまうため
	if (isUpdateTree) {
		ChoreonoidUtil::showAllModelItem();
	}
	DDEBUG("TeachingEventHandler::updateComViews() End");
}

void TeachingEventHandler::updateEditState(bool blockSignals) {
  DDEBUG("TeachingEventHandler::updateEditState");
  canEdit_ = SceneView::instance()->sceneWidget()->isEditMode();
  DDEBUG_V("isEditMode %d", canEdit_);
  tiv_->setEditMode(canEdit_);
  stv_->setEditMode(canEdit_);
  prv_->setEditMode(canEdit_);
  mdv_->setEditMode(canEdit_);
  flv_->setEditMode(canEdit_);
}

void TeachingEventHandler::updateExecState(bool isActive) {
  flv_->setExecState(isActive);
  tiv_->setExecState(isActive);
  stv_->setExecState(isActive);
}


}
