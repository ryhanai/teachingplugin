#ifndef TEACHING_TEACHING_DATAHOLDER_H_INCLUDED
#define TEACHING_TEACHING_DATAHOLDER_H_INCLUDED

#include "TeachingTypes.h"

using namespace std;

namespace teaching {

class TeachingDataHolder {
public:
  static TeachingDataHolder* instance();
	~TeachingDataHolder() {};

	QString getErrorStr();

	void loadData();
	void updateModelMaster();

	vector<FlowParamPtr> getFlowList();
	vector<FlowParamPtr> searchFlow(vector<string>& condList, bool isOr);
	bool deleteFlow(int targetId);
	FlowParamPtr getFlowById(int id);
	FlowParamPtr reGetFlowById(int id);
	bool saveFlowModel(FlowParamPtr& target);

	inline void addTaskData(TaskModelParamPtr param) { taskList_.push_back(param); };
	vector<TaskModelParamPtr> getTaskList() { return taskList_; };
	TaskModelParamPtr getTaskInstanceById(int id);
	vector<TaskModelParamPtr> searchTaskModels(vector<string>& condList, bool isOr);
	bool deleteTaskModel(int task_inst_id);
	bool saveImportedTaskModel(vector<TaskModelParamPtr>& source, vector<ModelMasterParamPtr>& modelMasterList);
	bool saveTaskModel(TaskModelParamPtr source);
	bool saveTaskModelasNew(TaskModelParamPtr source);

	ModelParamPtr addModel(TaskModelParamPtr& source, ModelMasterParamPtr& master);

	int addFile(TaskModelParamPtr& source, QString name, QString fileName);
	void deleteFile(int id, TaskModelParamPtr& source);

	int addImage(TaskModelParamPtr& source, QString name, QString fileName);
	void deleteImage(int id, TaskModelParamPtr& source);

	vector<ParameterParamPtr> loadParameter(int id);
	ParameterParamPtr addParameter(TaskModelParamPtr& source);
	bool saveTaskParameter(TaskModelParamPtr& target);

	vector<ModelMasterParamPtr> getModelMasterList();
	ModelMasterParamPtr getModelMasterById(int id);
	void updateModelMaster(int id, QString name, QString fileName);
	ModelMasterParamPtr addModelMaster();
	bool saveModelMaster(QString& errMessage);
	bool saveModelMasterList(vector<ModelMasterParamPtr> masterList);


private:
	TeachingDataHolder() {};

	vector<FlowParamPtr> flowList_;
	vector<TaskModelParamPtr> taskList_;
	vector<ModelMasterParamPtr> modelMasterList_;


};

struct TaskInstanceComparatorByID {
	int id_;
	TaskInstanceComparatorByID(int value) {
		id_ = value;
	}
	bool operator()(const TaskModelParamPtr elem) const {
		return elem->getId() == id_;
	}
};

struct ModelComparatorByMasterID {
	int id_;
	ModelComparatorByMasterID(int value) {
		id_ = value;
	}
	bool operator()(const ModelParamPtr elem) const {
		return elem->getMasterId() == id_;
	}
};

struct ModelMasterComparatorByID {
	int id_;
	ModelMasterComparatorByID(int value) {
		id_ = value;
	}
	bool operator()(const ModelMasterParamPtr elem) const {
		return elem->getId() == id_;
	}
};

}
#endif
