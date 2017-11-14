#include "TeachingDataHolder.h"
#include "DataBaseManager.h"
#include "TeachingUtil.h"

#include "LoggerUtil.h"


namespace teaching {

TeachingDataHolder* TeachingDataHolder::instance() {
  static TeachingDataHolder* holder = new TeachingDataHolder();
  return holder;
}

QString TeachingDataHolder::getErrorStr() {
	return DatabaseManager::getInstance().getErrorStr();
}

void TeachingDataHolder::loadData() {
	modelMasterList_ = DatabaseManager::getInstance().getModelMasterList();
	vector<string> condList;
	taskList_ = DatabaseManager::getInstance().searchTaskModels(condList, false);
	flowList_ = DatabaseManager::getInstance().searchFlowList(condList, false);
}

void TeachingDataHolder::updateModelMaster() {
	DDEBUG("TeachingDataHolder::updateModelMaster");
	modelMasterList_ = DatabaseManager::getInstance().getModelMasterList();
}

//T_FLOW
vector<FlowParamPtr> TeachingDataHolder::getFlowList() {
	vector<FlowParamPtr> result;

	for (int index = 0; index < flowList_.size(); index++) {
		FlowParamPtr param = flowList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}

	return result;
}

vector<FlowParamPtr> TeachingDataHolder::searchFlow(vector<string>& condList, bool isOr) {
	flowList_ = DatabaseManager::getInstance().searchFlowList(condList, isOr);
	return flowList_;
}

bool TeachingDataHolder::deleteFlow(int targetId) {
	bool ret = DatabaseManager::getInstance().deleteFlowModel(targetId);
	if (ret) {
		vector<string> condList;
		flowList_ = DatabaseManager::getInstance().searchFlowList(condList, false);
	}
	return ret;
}

FlowParamPtr TeachingDataHolder::getFlowById(int id) {
	FlowParamPtr result;
	for (int index = 0; index < flowList_.size(); index++) {
		FlowParamPtr param = flowList_[index];
		if (param->getId() == id) {
			result = param;
			break;
		}
	}

	if (result) {
		result = DatabaseManager::getInstance().getFlowParamById(result->getId());
	}
	return result;
}

FlowParamPtr TeachingDataHolder::reGetFlowById(int id) {
	return DatabaseManager::getInstance().getFlowParamById(id);
}

bool TeachingDataHolder::saveFlowModel(FlowParamPtr& target) {
	return DatabaseManager::getInstance().saveFlowModel(target);
}

//T_TASK_MODEL_INST
TaskModelParamPtr TeachingDataHolder::getTaskInstanceById(int id) {
	std::vector<TaskModelParamPtr>::iterator taskItr = std::find_if(taskList_.begin(), taskList_.end(), TaskInstanceComparatorByID(id));
	if (taskItr == taskList_.end()) return 0;
	return *taskItr;
}

vector<TaskModelParamPtr> TeachingDataHolder::searchTaskModels(vector<string>& condList, bool isOr) {
	vector<TaskModelParamPtr> taskList = DatabaseManager::getInstance().searchTaskModels(condList, isOr);
	return taskList;
}

bool TeachingDataHolder::deleteTaskModel(int task_inst_id) {
	taskList_.erase(remove_if(taskList_.begin(), taskList_.end(), TaskInstanceComparatorByID(task_inst_id)), taskList_.end());
	return DatabaseManager::getInstance().deleteTaskModel(task_inst_id);
}

bool TeachingDataHolder::saveImportedTaskModel(vector<TaskModelParamPtr>& source, vector<ModelMasterParamPtr>& modelMasterList) {
	if (DatabaseManager::getInstance().saveImportedTaskModel(source, modelMasterList) == false) return false;

	updateModelMaster();

	for (int index = 0; index < source.size(); index++) {
		TaskModelParamPtr task = source[index];
		TaskModelParamPtr newTask = DatabaseManager::getInstance().getTaskModelById(task->getId());
		TeachingUtil::loadTaskDetailData(newTask);
		if (newTask) {
			addTaskData(newTask);
		}
	}

	return true;
}

bool TeachingDataHolder::saveTaskModel(TaskModelParamPtr source) {
	if (DatabaseManager::getInstance().saveTaskModel(source) == false) return false;

	source->clearDetailParams();
	DatabaseManager::getInstance().getDetailParams(source);
	source->setNormal();
	source->setLoaded(false);

	return true;
}

bool TeachingDataHolder::saveTaskModelasNew(TaskModelParamPtr source) {
	vector<TaskModelParamPtr> taskList;
	taskList.push_back(source);
	if (DatabaseManager::getInstance().saveTaskModelsForLoad(taskList) == false) return false;

	TaskModelParamPtr newTask = DatabaseManager::getInstance().getTaskModelById(source->getId());
	TeachingUtil::loadTaskDetailData(newTask);
	if (newTask) {
		addTaskData(newTask);
	}

	return true;
}

//T_MODEL_INFO
ModelParamPtr TeachingDataHolder::addModel(TaskModelParamPtr& source, ModelMasterParamPtr& master) {
	DDEBUG("TeachingDataHolder::addModel");

	int maxId = -1;
	for (int index = 0; index < source->getModelList().size(); index++ ) {
		ModelParamPtr model = source->getModelList()[index];
		if (maxId < model->getId()) {
			maxId = model->getId();
		}
	}
	maxId++;
	DDEBUG_V("TeachingDataHolder::addModel: %d", maxId);

	ModelParamPtr param = std::make_shared<ModelParam>(maxId, master->getId(), 0, "New Model", "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true);
	param->setModelMaster(master);
	source->addModel(param);

	return param;
}

//T_FILE
int TeachingDataHolder::addFile(TaskModelParamPtr& source, QString name, QString fileName) {
	int maxId = -1;
	for (int index = 0; index < source->getFileList().size(); index++) {
		FileDataParamPtr param = source->getFileList()[index];
		if (maxId < param->getId()) {
			maxId = param->getId();
		}
	}
	maxId++;
	DDEBUG_V("TeachingDataHolder::addFile %d", maxId);
	FileDataParamPtr newParam = std::make_shared<FileDataParam>(maxId, 0, name);
	newParam->setNew();
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	newParam->setData(file.readAll());
	source->addFile(newParam);

	return newParam->getId();
}

void TeachingDataHolder::deleteFile(int id, TaskModelParamPtr& source) {
	FileDataParamPtr target = source->getFileById(id);
	target->setDelete();
}

//T_FIGURE
int TeachingDataHolder::addImage(TaskModelParamPtr& source, QString name, QString fileName) {
	int maxId = -1;
	for (int index = 0; index < source->getImageList().size(); index++) {
		ImageDataParamPtr param = source->getImageList()[index];
		if (maxId < param->getId()) {
			maxId = param->getId();
		}
	}
	maxId++;
	DDEBUG_V("TeachingDataHolder::addImage %d", maxId);
	ImageDataParamPtr newParam = std::make_shared<ImageDataParam>(maxId, 0, name);
	newParam->setNew();
	QImage image(fileName);
	newParam->setData(image);
	source->addImage(newParam);

	return newParam->getId();
}

void TeachingDataHolder::deleteImage(int id, TaskModelParamPtr& source) {
	ImageDataParamPtr target = source->getImageById(id);
	target->setDelete();
}

//T_PARAMETER
vector<ParameterParamPtr> TeachingDataHolder::loadParameter(int id) {
	return DatabaseManager::getInstance().getParameterParams(id);
}

ParameterParamPtr TeachingDataHolder::addParameter(TaskModelParamPtr& source) {
	int maxId = -1;
	for (int index = 0; index < source->getParameterList().size(); index++) {
		ParameterParamPtr param = source->getParameterList()[index];
		if (maxId < param->getId()) {
			maxId = param->getId();
		}
	}
	maxId++;
	DDEBUG_V("TeachingDataHolder::addParameter %d", maxId);
	ParameterParamPtr newParam = std::make_shared<ParameterParam>(maxId, 0, "", 1, "", source->getId(), "New Param", "", "", 0);
	newParam->setNew();
	source->addParameter(newParam);

	return newParam;
}

bool TeachingDataHolder::saveTaskParameter(TaskModelParamPtr& target) {
	return DatabaseManager::getInstance().saveTaskParameter(target);
}

//M_MODEL
vector<ModelMasterParamPtr> TeachingDataHolder::getModelMasterList() {
	vector<ModelMasterParamPtr> result;

	for (int index = 0; index < modelMasterList_.size(); index++) {
		ModelMasterParamPtr param = modelMasterList_[index];
		if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;
		result.push_back(param);
	}

	return result;
};

ModelMasterParamPtr TeachingDataHolder::getModelMasterById(int id) {
	std::vector<ModelMasterParamPtr>::iterator modelItr = std::find_if(modelMasterList_.begin(), modelMasterList_.end(), ModelMasterComparatorByID(id));
	if (modelItr == modelMasterList_.end()) return 0;
	return *modelItr;
}

ModelMasterParamPtr TeachingDataHolder::addModelMaster() {
	int maxId = -1;
	for (int index = 0; index < modelMasterList_.size(); index++) {
		ModelMasterParamPtr master = modelMasterList_[index];
		if (maxId < master->getId()) {
			maxId = master->getId();
		}
	}
	maxId++;
	DDEBUG_V("TeachingDataHolder::addModelMaster %d", maxId);

	ModelMasterParamPtr param = std::make_shared<ModelMasterParam>(maxId, "New Model", "");
	param->setNew();
	modelMasterList_.push_back(param);

	return param;
}

void TeachingDataHolder::updateModelMaster(int id, QString name, QString fileName) {
	ModelMasterParamPtr target = getModelMasterById(id);
	target->setName(name);
	target->setFileName(fileName);
}

bool TeachingDataHolder::saveModelMaster(QString& errMessage) {
	bool ret = DatabaseManager::getInstance().saveModelMasterList(this->modelMasterList_);
	if (ret == false) {
		errMessage = DatabaseManager::getInstance().getErrorStr();
	}
	return ret;
}

bool TeachingDataHolder::saveModelMasterList(vector<ModelMasterParamPtr> masterList) {
	return DatabaseManager::getInstance().saveModelMasterList(masterList);
}

}