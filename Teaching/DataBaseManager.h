#ifndef TEACHING_DATABASE_MANAGER_H_INCLUDED
#define TEACHING_DATABASE_MANAGER_H_INCLUDED

#include <vector>
#include <string>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "TeachingTypes.h"

using namespace cnoid;
using namespace std;

namespace teaching {

class DatabaseManager {
private:
  DatabaseManager();

public:
  static DatabaseManager& getInstance(void) {
    static DatabaseManager singleton;
    return singleton;
  }

  bool connectDB();
  bool reConnectDB();
  void closeDB();

  vector<FlowParamPtr> searchFlowList(vector<string>& condList, bool isOr);
	FlowParamPtr getFlowParamById(const int id);

	vector<TaskModelParamPtr> getAllTask();
	TaskModelParamPtr getTaskModelById(const int taskId);
  vector<TaskModelParamPtr> searchTaskModels(vector<string>& condList, bool isOr);

	TaskModelParamPtr getTaskModel(int index);
  vector<ElementStmActionParamPtr> getStmActionList(int taskId, int stateId);
	vector<ArgumentParamPtr> getArgumentParams(int taskId, int stateId);
	vector<ParameterParamPtr> getParameterParams(int instId);

  void getDetailParams(TaskModelParamPtr target);

  bool saveTaskModel(TaskModelParamPtr source);
	bool saveTaskModelsForLoad(vector<TaskModelParamPtr>& source);
  bool saveImportedTaskModel(vector<TaskModelParamPtr>& source, vector<ModelMasterParamPtr>& modelMasterList);
  bool saveFlowModel(FlowParamPtr source);
  bool deleteFlowModel(int id);
  bool deleteTaskModel(int task_inst_id);

  bool saveTaskParameter(TaskModelParamPtr source);
  bool saveStateParameter(int taskId, ElementStmParamPtr source);

  int getModelMaxIndex();

	vector<ModelMasterParamPtr> getModelMasterList();
	bool saveModelMasterList(vector<ModelMasterParamPtr> target);

  inline QString getErrorStr() const { return this->errorStr_; }

private:
  QSqlDatabase db_;
  vector<TaskModelParamPtr> taskModelList_;
  QString errorStr_;

  vector<ModelParamPtr> getModelParams(int id);
  vector<ImageDataParamPtr> getFigureParams(int id);
  vector<FileDataParamPtr> getFileParams(int id);
  vector<ElementStmParamPtr> getStateParams(int instId);
  vector<ConnectionStmParamPtr> getTransParams(int instId);
  vector<ElementStmParamPtr> getViaPointParams(int taskId, int transId);

  bool saveTaskInstanceData(TaskModelParamPtr source, bool updateDate);
  bool saveTaskParameterData(int taskId, ParameterParamPtr source);
  bool saveElementStmData(int parentId, ElementStmParamPtr source);
  bool saveElementStmActionData(int taskId, int stateId, ElementStmActionParamPtr source);
  bool saveTransitionStmData(int parentId, ConnectionStmParamPtr source);
  bool saveArgumentData(int taskId, int stateId, ArgumentParamPtr source);

  bool saveViaPointData(int taskId, ConnectionStmParamPtr source);

  vector<ElementStmParamPtr> getFlowStateParams(int flow_id);
  vector<ConnectionStmParamPtr> getFlowTransParams(int flow_id);
  vector<ElementStmParamPtr> getFlowViaPointParams(int flow_id, int trans_id);
	vector<ParameterParamPtr> getFlowParameterParams(int flowId);

  bool saveFlowStmData(int parentId, ElementStmParamPtr source);
  bool saveFlowTransactionStmData(int parentId, ConnectionStmParamPtr source);
  bool saveFlowViaPointData(int parentId, ConnectionStmParamPtr source);
	bool saveFlowParameter(FlowParamPtr source);
	bool saveFlowParameterData(int flowId, ParameterParamPtr source);

  /////
  bool saveFlowData(FlowParamPtr source);

  bool saveDetailData(TaskModelParamPtr source);
  bool saveModelData(int parentId, ModelParamPtr source);
  bool saveModelDetailData(int modelId, ModelDetailParamPtr source);
  bool saveImageData(int parentId, ImageDataParamPtr source);
  bool saveFileData(int parentId, FileDataParamPtr source);

	bool deleteDataById(QString tableName, QString strKey, int id);

  QByteArray image2DB(QString& name, const QImage& source);
};

}
#endif
