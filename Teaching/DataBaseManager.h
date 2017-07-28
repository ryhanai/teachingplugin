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
  vector<FlowParam*> searchFlowList(vector<string>& condList, bool isOr);

  TaskModelParam* getTaskModelById(const int taskId);
  vector<TaskModelParam*> searchTaskModels(vector<string>& condList, bool isOr);
  TaskModelParam* getTaskParamById(int id);
  FlowParam* getFlowParamById(const int id);

  TaskModelParam* getTaskModel(int index);
  vector<ElementStmActionParam*> getStmActionList(int stateId);
  vector<ParameterParam*> getParameterParams(int instId);

  void getDetailParams(TaskModelParam* target);

  bool saveTaskModel(TaskModelParam* source);
  bool saveTaskModelsForLoad(vector<TaskModelParam*>& source);
  bool saveFlowModel(FlowParam* source);
  bool deleteFlowModel(int id);
  bool deleteTaskModel(TaskModelParam* target);

  bool saveTaskParameter(TaskModelParam* source);
  bool saveStateParameter(ElementStmParam* source);

  int getModelMaxIndex();

  inline QString getErrorStr() const { return this->errorStr_; }

private:
  QSqlDatabase db_;
  vector<TaskModelParam*> taskModelList_;
  vector<FlowParam*> flowList_;
  QString errorStr_;

  vector<ArgumentParam*> getArgumentParams(int stateId);
  vector<ModelParam*> getModelParams(int id);
  vector<ImageDataParam*> getFigureParams(int id);
  vector<FileDataParam*> getFileParams(int id);
  vector<ElementStmParam*> getStateParams(int instId);
  vector<ConnectionStmParam*> getTransParams(int instId);
  vector<ElementStmParam*> getViaPointParams(int connId);

  bool saveTaskInstanceData(TaskModelParam* source, bool updateDate);
  bool saveTaskParameterData(int taskId, ParameterParam* source);
  bool saveElementStmData(int parentId, ElementStmParam* source);
  bool saveElementStmActionData(int stateId, ElementStmActionParam* source);
  bool saveTransitionStmData(int parentId, ConnectionStmParam* source);
  bool saveArgumentData(int stateId, ArgumentParam* source);

  bool saveViaPointData(ConnectionStmParam* source);

  vector<ElementStmParam*> getFlowStateParams(int instId);
  vector<ConnectionStmParam*> getFlowTransParams(int instId);
  vector<ElementStmParam*> getFlowViaPointParams(int connId);

  bool saveFlowStmData(int parentId, ElementStmParam* source);
  bool saveFlowTransactionStmData(int parentId, ConnectionStmParam* source);
  bool saveFlowViaPointData(ConnectionStmParam* source);

  /////
  bool saveFlowData(FlowParam* source);

  bool saveDetailData(TaskModelParam* source);
  bool saveModelData(int parentId, ModelParam* source);
  bool saveModelDetailData(int modelId, ModelDetailParam* source);
  bool saveImageData(int parentId, ImageDataParam* source);
  bool saveFileData(int parentId, FileDataParam* source);

  QByteArray image2DB(QString& name, const QImage& source);
};

}
#endif
