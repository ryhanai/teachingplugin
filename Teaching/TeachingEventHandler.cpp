#include "TeachingEventHandler.h"

#include "TeachingDataHolder.h"
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"

#include <QMessageBox>
#include <cnoid/BodyBar>
#include <boost/bind.hpp>

#include "ArgumentDialog.h"
#include "DecisionDialog.h"
#include "ExecEnvDialog.h"
#include "TaskExecutor.h"
#include "ControllerManager.h"
#include "DataBaseManager.h"

#include "NodeEditor/models.hpp"

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

	//�^�X�N��`�t�@�C���̓ǂݍ���
	vector<TaskModelParamPtr> taskInstList;
	vector<ModelMasterParamPtr> masterList;
  QString errMessage;
	if (TeachingUtil::importTask(strFName, taskInstList, masterList, errMessage) == false) {
		QMessageBox::warning(tiv_, _("Task Load Error"), errMessage);
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
  //���f���}�X�^�̃`�F�b�N
  for (ModelMasterParamPtr master : masterList) {
    QString txtData = QString::fromUtf8(master->getData());
    QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
    int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
    DDEBUG_V("ModelMaster:%s, %d", master->getName().toStdString().c_str(), ret);

    if (0 < ret) {
      master->setDelete();
      master->setId(ret);
    } else {
      master->setHash(strHash);
    }
  }
	//�^�X�N�̕ۑ�
	if (TeachingDataHolder::instance()->saveImportedTaskModel(taskInstList, masterList) == false) {
		QMessageBox::warning(tiv_, _("Task Import"), TeachingDataHolder::instance()->getErrorStr());
		return false;
	}

	vector<TaskModelParamPtr> taskList = TeachingDataHolder::instance()->getTaskList();
	tiv_->showGrid(taskList);

	QMessageBox::information(tiv_, _("Task Import"), _("target TASK imported"));

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

void TeachingEventHandler::tiv_RegistTaskClicked(int selectedId, QString strTask) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::tiv_RegistTaskClicked()");

	stv_->setStepStatus(false);

  //�폜����Transition���������Ă��܂�����
  //if (0 <= selectedId) {
  //  tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
  //  com_CurrentTask_ = tiv_CurrentTask_;
  //  updateComViews(tiv_CurrentTask_);
  //}
  if (tiv_CurrentTask_ == 0) return;

	for (ModelParamPtr model : tiv_CurrentTask_->getActiveModelList()) {
		if (model->isChangedPosition() == false) continue;
		//
		QMessageBox::StandardButton ret = QMessageBox::question(tiv_, _("Confirm"),
			_("Model Position was changed. Continue?"),
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::No) return;
		break;
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
	} else {
		QMessageBox::warning(tiv_, _("Save Task"), TeachingDataHolder::instance()->getErrorStr());
	}
}

void TeachingEventHandler::tiv_RegistNewTaskClicked(int selectedId, QString strTask, QString strCond) {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::tiv_RegistNewTaskClicked()");

	stv_->setStepStatus(false);

  if (0 <= selectedId) {
    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }
  if (tiv_CurrentTask_ == 0) return;

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
	} else {
		QMessageBox::warning(tiv_, _("Database Error"), TeachingDataHolder::instance()->getErrorStr());
	}
}

//FlowView
void TeachingEventHandler::flv_Loaded(FlowViewImpl* view) {
	this->flv_ = view;
}

void TeachingEventHandler::flv_NewFlowClicked() {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_NewFlowClicked()");

	stv_->setStepStatus(false);

	flv_CurrentFlow_.reset(new FlowParam(NULL_ID, "", "", "", ""));
	flv_CurrentFlow_->setNew();
	flv_->dispView(flv_CurrentFlow_);
}

void TeachingEventHandler::flv_SearchClicked(bool canEdit) {
	DDEBUG("TeachingEventHandler::flv_SearchClicked");
	if (checkPaused()) return;

	stv_->setStepStatus(false);

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
	unloadTaskModelItems();

	com_CurrentTask_ = target;

	if (com_CurrentTask_) {
		updateComViews(com_CurrentTask_, false);

	} else {
		mdv_->clearTaskParam();
		stv_->clearTaskParam();
		prv_->clearTaskParam();
	}
  tiv_CurrentTask_ = 0;
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

  //���f���}�X�^�̃`�F�b�N
  for (ModelMasterParamPtr master : masterList) {
    QString txtData = QString::fromUtf8(master->getData());
    QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
    int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
    if (0 < ret) {
      master->setDelete();
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

	for (ElementStmParamPtr state : flv_CurrentFlow_->getStmElementList()) {
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
	for (ElementStmParamPtr state : flv_CurrentFlow_->getStmElementList()) {
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
  if (flv_->updateTargetFlowParam() == false) {
    QMessageBox::warning(prd_, _("FlowView"), _("Incorrect flow parameter connection."));
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
				if (param->getName() != strCmd) continue;
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
void TeachingEventHandler::tev_stm_RunClicked(bool isReal, ElementStmParamPtr target) {
	DDEBUG("TeachingEventHandler::tev_stm_RunClicked()");

	if (target == NULL) return;

	prv_SetInputValues();
	if (target->getType() != ELEMENT_COMMAND) {
		QMessageBox::warning(stv_, _("Run Command"), _("Please select Command Element."));
		return;
	}
	//
	executor_->setCurrentTask(com_CurrentTask_);
	executor_->setCurrentElement(target);
	if (executor_->runSingleCommand() == false) {
		QMessageBox::information(stv_, _("Run Command"), _("Target Command FAILED."));
	}
}

void TeachingEventHandler::tev_stm_StepClicked() {
	DDEBUG("TeachingEventHandler::tev_stm_StepClicked()");
	if (executor_->doTaskOperationStep() == ExecResult::EXEC_BREAK) {
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

void TeachingEventHandler::tev_RunTaskClicked(int selectedId) {
	DDEBUG("TeachingEventHandler::tev_RunTaskClicked()");
	stv_->updateTargetParam();
  prv_SetInputValues();

  if (0 <= selectedId) {
    tiv_CurrentTask_ = TeachingDataHolder::instance()->getTaskInstanceById(selectedId);
    com_CurrentTask_ = tiv_CurrentTask_;
    updateComViews(tiv_CurrentTask_);
  }

	executor_->setCurrentTask(com_CurrentTask_);
	executor_->setCurrentElement(com_CurrParam_);
	executor_->runSingleTask();
}

void TeachingEventHandler::tev_AbortClicked() {
	DDEBUG("TeachingEventHandler::tev_AbortClicked");
	executor_->abortOperation();
}

void TeachingEventHandler::flv_RunFlowClicked() {
	DDEBUG("TeachingEventHandler::flv_RunFlowClicked()");
  stv_->updateTargetParam();
  if (flv_->updateTargetFlowParam() == false) {
    QMessageBox::warning(prd_, _("FlowView"), _("Incorrect flow parameter connection."));
    return;
  }

  executor_->setCurrentTask(com_CurrentTask_);
	executor_->runFlow(flv_CurrentFlow_);
	com_CurrentTask_ = executor_->getCurrentTask();
}

void TeachingEventHandler::flv_InitPosClicked() {
	if (checkPaused()) return;
	DDEBUG("TeachingEventHandler::flv_InitPosClicked()");

	if (!flv_CurrentFlow_) return;
	stv_->setStepStatus(false);

  for (FlowParameterParamPtr targetParam : flv_CurrentFlow_->getFlowParamList()) {
    targetParam->setInitialValue();
  }
	for (ElementStmParamPtr targetState : flv_CurrentFlow_->getStmElementList()) {
		if (targetState->getType() == ELEMENT_COMMAND) {
			TaskModelParamPtr task = targetState->getTaskParam();
			for (ModelParamPtr model : task->getModelList()) {
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

  vector<ElementStmParamPtr> stateList = flv_CurrentFlow_->getStmElementList();
  vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
  if (targetElem == stateList.end()) return false;
  TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();
  DDEBUG_V("Task Name : %s", taskParam->getName().toStdString().c_str());

  //���f���p�����[�^�̏ꍇ
  if (sourceNode->nodeDataModel()->name() == "Model Param") {
    NodeDataType dataType = sourceNode->nodeDataModel()->dataType(PortType::Out, sourcePortIndex);
    DDEBUG_V("dataType : %s", dataType.id.toStdString().c_str());
    //���f���|�[�g�̏ꍇ
    if (dataType.id == "modeldata") {
      //FlowModelParameter�̌���
      vector<FlowModelParamPtr> modelList = flv_CurrentFlow_->getActiveModelList();
      vector<FlowModelParamPtr>::iterator modelElem = find_if(modelList.begin(), modelList.end(), FlowModelParamComparator(sourceId));
      if (modelElem == modelList.end()) return false;
      DDEBUG_V("Master Id : %d", (*modelElem)->getMasterId());
      int masterId = (*modelElem)->getMasterId();

      ModelParamPtr model = taskParam->getModelParamById(id);
      DDEBUG_V("Model Name : %s", model->getRName().toStdString().c_str());
      vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterList();
      vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList.begin(), modelMasterList.end(), ModelMasterComparator(masterId));
      if (masterParamItr != modelMasterList.end()) {
        ChoreonoidUtil::replaceMaster(model, *masterParamItr);
      }
    }
  } else if (sourceNode->nodeDataModel()->name().startsWith("Flow Param")) {
    //�t���[�p�����[�^�̏ꍇ
    //FlowParameter�̌���
    vector<FlowParameterParamPtr> paramList = flv_CurrentFlow_->getActiveFlowParamList();
    vector<FlowParameterParamPtr>::iterator paramElem = find_if(paramList.begin(), paramList.end(), FlowParameterParamComparator(sourceId));
    if (paramElem == paramList.end()) return false;

    ParameterParamPtr param = taskParam->getParameterById(id);
    DDEBUG_V("Flow Param Type:%d, Task Param Type(id=%d):%d %s", (*paramElem)->getType(), id, param->getParamType(), param->getName().toStdString().c_str());
    if (param->getParamType() != (*paramElem)->getType()) {
      QMessageBox::warning(prd_, _("Flow Parameter"), _("The type of the parameter of the connection destination does not match."));
      return false;
    }
    //
    param->setFlowParam(*paramElem);
  }
  return true;
}

void TeachingEventHandler::flv_Disconnected(QtNodes::Connection& target) {
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
  if (!sourceNode) return;
  int sourceId = sourceNode->getParamId();
  int sourcePortIndex = target.getPortIndex(PortType::Out);

  vector<ElementStmParamPtr> stateList = flv_CurrentFlow_->getStmElementList();
  vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator(targetId));
  if (targetElem == stateList.end()) return;
  TaskModelParamPtr taskParam = (*targetElem)->getTaskParam();
  if (!taskParam) return;
  DDEBUG_V("Task Name : %s", taskParam->getName().toStdString().c_str());

  //���f���p�����[�^�̏ꍇ
  if (sourceNode->nodeDataModel()->name() == "Model Param") {
    NodeDataType dataType = sourceNode->nodeDataModel()->dataType(PortType::Out, sourcePortIndex);
    DDEBUG_V("dataType : %s", dataType.id.toStdString().c_str());
    //���f���|�[�g�̏ꍇ
    if (dataType.id == "modeldata") {
      ModelParamPtr model = taskParam->getModelParamById(id);
      DDEBUG_V("Model Name : %s", model->getRName().toStdString().c_str());
      bool isLoaded = model->isLoaded();
      ChoreonoidUtil::unLoadModelItem(model);
      model->restoreModelMaster();
      if (isLoaded) {
        ChoreonoidUtil::loadModelItem(model);
        ChoreonoidUtil::showAllModelItem();
      }
    }
  } else if (sourceNode->nodeDataModel()->name().startsWith("Flow Param")) {
    //�t���[�p�����[�^�̏ꍇ
    ParameterParamPtr param = taskParam->getParameterById(id);
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
	return false;
}

//FlowSearchDialog
void TeachingEventHandler::fsd_Loaded(FlowSearchDialog* dialog) {
	this->fsd_ = dialog;

	vector<FlowParamPtr> flowList = TeachingDataHolder::instance()->getFlowList();
	fsd_->showGrid(flowList);
}

void TeachingEventHandler::fsd_SeachClicked(QString condition) {
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
	      if (model->getModelMaster()->getModelItem().get() == mdd_BodyItem_) {
					mdd_selectedModel_ = model;
	        break;
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
	if (!com_CurrentTask_) return;

	this->prd_ = dialog;
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	prd_->showModelInfo(modelList);
	vector<ParameterParamPtr> paramList = com_CurrentTask_->getActiveParameterList();
	prd_->showParamInfo(paramList);
	prd_->setTaskName(com_CurrentTask_->getName());

	prd_CurrentParam_ = 0;
}

void TeachingEventHandler::prd_ParamSelectionChanged(int newId, QString name, QString id, int type, int paramType, QString unit, int model_id, int model_param_id, int hide) {
  DDEBUG_V("TeachingEventHandler::prd_ParamSelectionChanged %d", newId);

  prd_UpdateParam(name, id, type, paramType, unit, model_id, model_param_id, hide);
  prd_CurrentParam_ = com_CurrentTask_->getParameterById(newId);
  prd_->updateContents(prd_CurrentParam_);
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
  DDEBUG_V("TeachingEventHandler::prd_ModelSelectionChanged %d", selectedId);
  vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
  for (ModelParamPtr model : modelList) {
    if (model->getId() == selectedId) {
      vector<ModelParameterParamPtr> paramList = model->getModelMaster()->getActiveModelParamList();
      prd_->showModelParamInfo(paramList);
      return;
    }
  }
}

vector<ModelParameterParamPtr> TeachingEventHandler::prd_ModelSelectionChanged(int selectedId) {
  vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
  for (ModelParamPtr model : modelList) {
    if (model->getId() == selectedId) {
      return model->getModelMaster()->getActiveModelParamList();
    }
  }
  vector<ModelParameterParamPtr> result;
  return result;
}
//ModelMasterDialog
void TeachingEventHandler::mmd_Loaded(ModelMasterDialog* dialog) {
	DDEBUG("TeachingEventHandler::mmd_Loaded");

	this->mmd_ = dialog;

	mmd_CurrentId_ = NULL_ID; mmd_CurrentModel_ = 0;

	vector<ModelMasterParamPtr> modelMasterList = TeachingDataHolder::instance()->getModelMasterListFromDB();
	this->mmd_->showGrid(modelMasterList);
}

void TeachingEventHandler::mmd_ModelSelectionChanged(int newId, QString name, QString fileName) {
	DDEBUG_V("TeachingEventHandler::mmd_ModelSelectionChanged %d", newId);

	if (mmd_CurrentId_ !=NULL_ID && mmd_CurrentId_ != newId) {
		TeachingDataHolder::instance()->updateModelMaster(mmd_CurrentId_, name, fileName);
	}
	mmd_CurrentId_ = newId;
	ModelMasterParamPtr model = TeachingDataHolder::instance()->getModelMasterById(newId);

	if (mmd_CurrentModel_) {
		ChoreonoidUtil::unLoadModelMasterItem(mmd_CurrentModel_);
	}
	ChoreonoidUtil::loadModelMasterItem(model);
	ChoreonoidUtil::showAllModelItem();
	mmd_CurrentModel_ = model;

  QImage targetImage = model->getImage();
	this->mmd_->updateContents(model->getName(), model->getFileName(), model->getImageFileName(), &targetImage);
	this->mmd_->showParamGrid(model->getActiveModelParamList());
	mmd_CurrentParam_ = 0;
}

void TeachingEventHandler::mmd_ModelParameterSelectionChanged(int newId, QString name, QString desc) {
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

void TeachingEventHandler::mmd_RefClicked() {
	if (!mmd_CurrentModel_) return;

	QString strFName = QFileDialog::getOpenFileName(0, "VRML File", ".", "wrl(*.wrl);;all(*.*)");
	if (strFName.isEmpty()) return;

	QString strName = QFileInfo(strFName).fileName();
	QString strPath = QFileInfo(strFName).absolutePath();

	QString currFile = mmd_CurrentModel_->getFileName();
	if (strFName == currFile) return;

	if (mmd_CurrentModel_->getModelItem()) {
	  ChoreonoidUtil::unLoadModelMasterItem(mmd_CurrentModel_);
	}
	mmd_CurrentModel_->setFileName(strName);

	QFile file(strFName);
	file.open(QIODevice::ReadOnly);
	mmd_CurrentModel_->setData(file.readAll());
	if (ChoreonoidUtil::readModelItem(mmd_CurrentModel_, strFName)) {
	  ChoreonoidUtil::loadModelMasterItem(mmd_CurrentModel_);
	  ChoreonoidUtil::showAllModelItem();
	}
	//�Q�ƃ��f���̓ǂݍ���
	TeachingUtil::loadModelDetail(strFName, mmd_CurrentModel_);

  QImage targetImage = mmd_CurrentModel_->getImage();
	this->mmd_->updateContents(mmd_CurrentModel_->getName(), mmd_CurrentModel_->getFileName(), mmd_CurrentModel_->getImageFileName(), &targetImage);
}

void TeachingEventHandler::mmd_RefImageClicked() {
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

void TeachingEventHandler::mmd_DeleteImageClicked() {
  if (!mmd_CurrentModel_) return;

  QImage targetImage;
  mmd_CurrentModel_->setImage(targetImage);
  mmd_CurrentModel_->setImageFileName("");

  this->mmd_->updateImage("", targetImage);
}

void TeachingEventHandler::mmd_AddModelClicked() {
	ModelMasterParamPtr model = TeachingDataHolder::instance()->addModelMaster();
	mmd_CurrentModel_ = model;
	mmd_CurrentId_ = model->getId();

	this->mmd_->addModel(mmd_CurrentModel_->getId(), mmd_CurrentModel_->getName());
}

void TeachingEventHandler::mmd_DeleteModelClicked(int id) {
	ModelMasterParamPtr model = TeachingDataHolder::instance()->getModelMasterById(id);
	model->setDelete();
	model->deleteModelDetails();

	if (mmd_CurrentModel_ && mmd_CurrentModel_->getModelItem()) {
	  ChoreonoidUtil::unLoadModelMasterItem(mmd_CurrentModel_);
	}
}

void TeachingEventHandler::mmd_AddModelParamClicked() {
	if (!mmd_CurrentModel_) return;
	TeachingDataHolder::instance()->addModelMasterParam(mmd_CurrentModel_);
	this->mmd_->showParamGrid(mmd_CurrentModel_->getActiveModelParamList());
}

void TeachingEventHandler::mmd_DeleteModelParamClicked() {
	if (!mmd_CurrentModel_ || !mmd_CurrentParam_) return;
	mmd_CurrentParam_->setDelete();
}

bool TeachingEventHandler::mmd_OkClicked(QString name, QString fileName, QString& errMessage) {
	if (mmd_CurrentId_ != NULL_ID) {
		TeachingDataHolder::instance()->updateModelMaster(mmd_CurrentId_, name, fileName);
	}
	return TeachingDataHolder::instance()->saveModelMaster(errMessage);
}

bool TeachingEventHandler::mmd_Check() {
  if (mmd_CurrentModel_->getMode() != DB_MODE_INSERT) return false;
  QString txtData = QString::fromUtf8(mmd_CurrentModel_->getData());
  QString strHash = TeachingUtil::getSha1Hash(txtData.toStdString().c_str(), txtData.toStdString().length());
  int ret = DatabaseManager::getInstance().checkModelMaster(strHash);
  return 0 <= ret;
}

//ArgumentDialog
void TeachingEventHandler::agd_ModelSelectionChanged(int selectedId) {
	vector<ModelParamPtr> modelList = com_CurrentTask_->getActiveModelList();
	for (ModelParamPtr model : modelList) {
		if (model->getId() == selectedId) {
			vector<ModelParameterParamPtr> paramList = model->getModelMaster()->getActiveModelParamList();
			agd_->showModelParamInfo(paramList);
			return;
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
				if (handler->checkSyntax(com_CurrentTask_, param->getValueDesc(), strError) == false) {
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

void TeachingEventHandler::updateComViews(TaskModelParamPtr targetTask, bool canEdit) {
	DDEBUG("TeachingEventHandler::updateComViews()");

	TeachingUtil::loadTaskDetailData(targetTask);
	bool isUpdateTree = ChoreonoidUtil::loadTaskModelItem(targetTask);

	mdv_->setTaskParam(targetTask);
	stv_->setTaskParam(targetTask);
	prv_->setTaskParam(targetTask, canEdit);

	//���X�V���s���ƃG���[�ɂȂ��Ă��܂�����
	if (isUpdateTree) {
		ChoreonoidUtil::showAllModelItem();
	}
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

}