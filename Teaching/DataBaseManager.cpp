#include "DataBaseManager.h"
#include <QVariant>
#include <QBuffer>
#include <qsqlerror.h>
#include "TeachingUtil.h"
#include "ChoreonoidUtil.h"
#include "TaskExecutor.h"
#include <cnoid/UTF8>

#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

DatabaseManager::DatabaseManager() {
}

bool DatabaseManager::connectDB() {
  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(QString::fromStdString(SettingManager::getInstance().getDatabase()));
  if(db_.open()==false) {
    return false;
  }
  return true;
}

bool DatabaseManager::reConnectDB() {
  closeDB();
  //
  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(QString::fromStdString(SettingManager::getInstance().getDatabase()));
  if(db_.open()==false) {
    return false;
  }
  return true;
}

void DatabaseManager::closeDB() {
  if(db_.isOpen()) {
    db_.close();
  }
}
/////
vector<FlowParam*> DatabaseManager::searchFlowList(vector<string>& condList, bool isOr) {
  flowList_.clear();

  string strQuery = "SELECT "; 
  strQuery += "flow_id, name, comment, created_date, last_updated_date ";
  strQuery += "FROM T_FLOW ";
  string strQueryCond = "WHERE ( ";
  for(unsigned int index=0; index<condList.size(); index++) {
    if(0<index) {
      if(isOr) strQueryCond += " Or ";
      else     strQueryCond += " And ";
    }
    strQueryCond += "((name || ' ' || comment) like \"%" + condList[index] + "%\")";
  }
  if(0<condList.size()) {
    strQueryCond += " ) ";
    strQuery += strQueryCond;
  }
  strQuery += " ORDER BY flow_id";

  QSqlQuery taskQuery(db_);
  taskQuery.exec(strQuery.c_str());
  while (taskQuery.next()) {
    int id = taskQuery.value(0).toInt();
    QString name = taskQuery.value(1).toString();
    QString comment = taskQuery.value(2).toString();
    QString createdDate = taskQuery.value(3).toString();
    QString updatedDate = taskQuery.value(4).toString();
    //
    FlowParam* param = new FlowParam(id, name, comment, createdDate, updatedDate, false);
    flowList_.push_back(param);
  }
  return flowList_;
}

FlowParam* DatabaseManager::getFlowParamById(const int id) {
  DDEBUG_V("getFlowParamById : %d", id);

	FlowParam* result = NULL;

  string strQuery = "SELECT ";
  strQuery += "name, comment, created_date, last_updated_date ";
  strQuery += "FROM T_FLOW ";
  strQuery += "WHERE flow_id = " + toStr(id);
  QSqlQuery flowQuery(db_);
  flowQuery.exec(strQuery.c_str());
  if (flowQuery.next()==false) return false;
  //
  QString name = flowQuery.value(0).toString();
  QString comment = flowQuery.value(1).toString();
  QString createdDate = flowQuery.value(2).toString();
  QString updatedDate = flowQuery.value(3).toString();
	//
	result = new FlowParam(id, name, comment, createdDate, updatedDate, false);
  result->setName(name);
  result->setComment(comment);
  result->setCreatedDate(createdDate);
  result->setLastUpdatedDate(updatedDate);
  //
  string strItemQuery = "SELECT ";
  strItemQuery += "task_inst_id ";
  strItemQuery += "FROM T_FLOW_ITEM ";
  strItemQuery += "WHERE flow_id = " + toStr(id) + " ";
  strItemQuery += "ORDER BY seq";
  QSqlQuery itemQuery(db_);
  itemQuery.exec(strItemQuery.c_str());
  vector<int> taskIdList;
  while (itemQuery.next()) {
    int id = itemQuery.value(0).toInt();
    taskIdList.push_back(id);
  }
  if(taskIdList.size()==0) return result;
  //
  for(int index=0; index<taskIdList.size();index++) {
    int taskId = taskIdList[index];
    string strTaskQuery = "SELECT ";
    strTaskQuery += "name, comment, flow_id, created_date, last_updated_date ";
    strTaskQuery += "FROM T_TASK_MODEL_INST ";
    strTaskQuery += "WHERE task_inst_id =" + toStr(taskId);
    QSqlQuery taskQuery(db_);
    taskQuery.exec(strTaskQuery.c_str());
    while (taskQuery.next()) {
      QString name = taskQuery.value(0).toString();
      QString comment = taskQuery.value(1).toString();
      int flow_id = taskQuery.value(2).toInt();
      QString createdDate = taskQuery.value(3).toString();
      QString updatedDate = taskQuery.value(4).toString();
      //
      TaskModelParam* param = new TaskModelParam(taskId, name, comment, flow_id, -1, createdDate, updatedDate);
      taskModelList_.push_back(param);
      getDetailParams(param);
      result->addTask(param);
    }
  }
  return result;
}

vector<TaskModelParam*> DatabaseManager::searchTaskModels(vector<string>& condList, bool isOr) {
  taskModelList_.clear();

  string strQuery = "SELECT ";
  strQuery += "task_inst_id, name, comment, flow_id, created_date, last_updated_date ";
  strQuery += "FROM T_TASK_MODEL_INST ";
  string strQueryCond = "WHERE ( ";
  for(unsigned int index=0; index<condList.size(); index++) {
    if(0<index) {
      if(isOr) strQueryCond += " Or ";
      else     strQueryCond += " And ";
    }
    strQueryCond += "((name || ' ' || comment) like \"%" + condList[index] + "%\")";
  }
  if(0<condList.size()) {
    strQueryCond += " ) ";
    strQuery += strQueryCond;
  }
  strQuery += " ORDER BY task_inst_id";
  QSqlQuery taskQuery(db_);
  taskQuery.exec(strQuery.c_str());

  while (taskQuery.next()) {
    int id = taskQuery.value(0).toInt();
    QString name = taskQuery.value(1).toString();
    QString comment = taskQuery.value(2).toString();
    int flow_id = taskQuery.value(3).toInt();
    QString createdDate = taskQuery.value(4).toString();
    QString updatedDate = taskQuery.value(5).toString();
    //
    TaskModelParam* param = new TaskModelParam(id, name, comment, flow_id, -1, createdDate, updatedDate);
    taskModelList_.push_back(param);
    getDetailParams(param);
  }
  return taskModelList_;
}

TaskModelParam* DatabaseManager::getTaskModelById(const int taskId) {
  TaskModelParam* result = NULL;

  string strQuery = "SELECT ";
  strQuery += "task_inst_id, name, comment, flow_id, created_date, last_updated_date ";
  strQuery += "FROM T_TASK_MODEL_INST ";
  strQuery += "WHERE task_inst_id = " + toStr(taskId);
  QSqlQuery taskQuery(db_);
  taskQuery.exec(strQuery.c_str());

  if (taskQuery.next()) {
    int id = taskQuery.value(0).toInt();
    QString name = taskQuery.value(1).toString();
    QString comment = taskQuery.value(2).toString();
    int flow_id = taskQuery.value(3).toInt();
    QString createdDate = taskQuery.value(4).toString();
    QString updatedDate = taskQuery.value(5).toString();
    //
    result = new TaskModelParam(id, name, comment, flow_id, -1, createdDate, updatedDate);
    taskModelList_.push_back(result);
    getDetailParams(result);
  }
  return result;
}

void DatabaseManager::getDetailParams(TaskModelParam* target) {
    vector<ModelParam*> modelList = getModelParams(target->getId());
    std::vector<ModelParam*>::iterator itModel = modelList.begin();
    while (itModel != modelList.end() ) {
      target->addModel(*itModel);
      ++itModel;
    }
    //
    vector<ParameterParam*> paramList = getParameterParams(target->getId());
    std::vector<ParameterParam*>::iterator itParam = paramList.begin();
    while (itParam != paramList.end() ) {
      target->addParameter(*itParam);
      ++itParam;
    }
    //
    vector<ElementStmParam*> stateList = getStateParams(target->getId());
    std::vector<ElementStmParam*>::iterator itState = stateList.begin();
    while (itState != stateList.end() ) {
      target->addStmElement(*itState);
      ++itState;
    }
    //
    vector<ConnectionStmParam*> transList = getTransParams(target->getId());
    std::vector<ConnectionStmParam*>::iterator itTrans = transList.begin();
    while (itTrans != transList.end() ) {
      target->addStmConnection(*itTrans);
      ++itTrans;
    }
    //
    vector<ImageDataParam*> imageList = getFigureParams(target->getId());
    std::vector<ImageDataParam*>::iterator itImage = imageList.begin();
    while (itImage != imageList.end() ) {
      target->addImage(*itImage);
      ++itImage;
    }
    //
    vector<FileDataParam*> fileList = getFileParams(target->getId());
    std::vector<FileDataParam*>::iterator itFile = fileList.begin();
    while (itFile != fileList.end() ) {
      target->addFile(*itFile);
      ++itFile;
    }
}


TaskModelParam* DatabaseManager::getTaskParamById(int id) {
  for(int index=0; index<taskModelList_.size(); index++) {
    TaskModelParam* param = taskModelList_[index];
    if(param->getId() == id) {
      return param;
    }
  }
  return 0;
}

TaskModelParam* DatabaseManager::getTaskModel(int index) {
  if(index<0 || taskModelList_.size()<=index) return 0;
  return taskModelList_[index];
}

int DatabaseManager::getModelMaxIndex() {
  string strMaxQuery = "SELECT max(model_id) FROM T_MODEL_INFO "; 
  QSqlQuery maxQuery(db_);
  maxQuery.exec(strMaxQuery.c_str());
  int maxId = -1;
  if (maxQuery.next()) {
    maxId = maxQuery.value(0).toInt();
    maxId++;
  }
  return maxId;
}

bool DatabaseManager::checkFlowItem(const int id, vector<QString>& flowList) {
  DDEBUG_V("checkFlowItem : %d", id);
  bool result = false;
  errorStr_ = "";

  string strQuery = "SELECT distinct(T_FLOW.name) "; 
  strQuery += "FROM T_FLOW LEFT OUTER JOIN T_FLOW_ITEM ";
  strQuery += "ON T_FLOW.flow_id=T_FLOW_ITEM.flow_id ";
  strQuery += "WHERE T_FLOW_ITEM.task_inst_id=" + toStr(id);

  QSqlQuery query(db_);
  if(query.exec(strQuery.c_str())==false) {
    errorStr_ = "error:" + query.lastError().databaseText();
  }
  while (query.next()) {
    QString flowName = query.value(0).toString();
    flowList.push_back(flowName);
    result = true;
  }
  return result;
}
//////////
bool DatabaseManager::saveFlowModel(FlowParam* source) {
  //if(saveTaskModels(source->getTaskList())==false) return false;
  if(saveFlowData(source)==false) return false;
  return true;
}

bool DatabaseManager::saveTaskParameter(TaskModelParam* source) {
  vector<ParameterParam*> paramList = source->getParameterList();
  vector<ParameterParam*>::iterator itParam = paramList.begin();
  while (itParam != paramList.end() ) {
    if( saveTaskParameterData(source->getId(), *itParam)==false) {
      db_.rollback();
      return false;
    }
    ++itParam;
  }

  return true;
}

bool DatabaseManager::saveStateParameter(ElementStmParam* source) {
  DDEBUG("saveStateParameter");
  vector<ArgumentParam*> argList = source->getArgList();
  vector<ArgumentParam*>::iterator itArg = argList.begin();
  while (itArg != argList.end() ) {
    if( saveArgumentData(source->getId(), *itArg)==false) {
      db_.rollback();
      return false;
    }
    ++itArg;
  }
  //
  vector<ElementStmActionParam*> actionList = source->getActionList();
  vector<ElementStmActionParam*>::iterator itAction = actionList.begin();
  while (itAction != actionList.end() ) {
    if( saveElementStmActionData(source->getId(), *itAction)==false) {
      db_.rollback();
      return false;
    }
    ++itAction;
  }
  return true;
}

bool DatabaseManager::saveTaskModelsForNewRegist(TaskModelParam* source) {
  DDEBUG("saveTaskModelsForNewRegist");
  errorStr_ = "";
  db_.transaction();

  if(saveTaskInstanceData(source, false)==false) {
    db_.rollback();
    return false;
  }
  //
  if( saveDetailData(source)==false) {
    db_.rollback();
    return false;
  }
  //
  vector<ParameterParam*> paramList = source->getParameterList();
  vector<ParameterParam*>::iterator itParam = paramList.begin();
  while (itParam != paramList.end() ) {
    if( saveTaskParameterData(source->getId(), *itParam)==false) {
      db_.rollback();
      return false;
    }
    ++itParam;
  }
  //
  vector<ElementStmParam*> stateList = source->getStmElementList();
  vector<ElementStmParam*>::iterator itState = stateList.begin();
  while (itState != stateList.end() ) {
    (*itState)->setOrgId((*itState)->getId());
    if( saveElementStmData(source->getId(), *itState)==false) {
      db_.rollback();
      return false;
    }
    ++itState;
  }
  //
  vector<ConnectionStmParam*> transList = source->getStmConnectionList();
  vector<ConnectionStmParam*>::iterator itTrans = transList.begin();
  while (itTrans != transList.end() ) {
    vector<ElementStmParam*>::iterator sourceElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
    if(sourceElem== stateList.end()) {
      ++itTrans;
      DDEBUG("NOT Found SourceId");
      continue;
    }
    (*itTrans)->setSourceId((*sourceElem)->getId());

    vector<ElementStmParam*>::iterator targetElem = find_if( stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
    if(targetElem== stateList.end()) {
      ++itTrans;
      DDEBUG("NOT Found TargetId");
      continue;
    }
    (*itTrans)->setTargetId((*targetElem)->getId());
    DDEBUG_V("New Trans sourceId : %d, TargetId: %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());

    if( saveTransactionStmData(source->getId(), *itTrans)==false) {
      db_.rollback();
      return false;
    }
    ++itTrans;
  }
  //
  db_.commit();
  return true;
}

bool DatabaseManager::saveTaskModelsForLoad(vector<TaskModelParam*>& source) {
  DDEBUG("saveTaskModelsForLoad");
  errorStr_ = "";

  db_.transaction();
  try {
    vector<TaskModelParam*>::iterator itTask = source.begin();
    while (itTask != source.end() ) {
      if(saveTaskInstanceData(*itTask, true)==false) {
          db_.rollback();
          return false;
      }
      int taskInstId = (*itTask)->getId();
      DDEBUG_V("taskInstId : %d", taskInstId);
      //
      vector<ModelParam*> modelList = (*itTask)->getModelList();
      vector<ModelParam*>::iterator itModel = modelList.begin();
      while (itModel != modelList.end() ) {
        if( saveModelData(taskInstId, *itModel)==false) {
          db_.rollback();
          return false;
        }
        ++itModel;
      }
      //
      vector<ParameterParam*> paramList = (*itTask)->getParameterList();
      vector<ParameterParam*>::iterator itParam = paramList.begin();
      while (itParam != paramList.end() ) {
        if( saveTaskParameterData(taskInstId, *itParam)==false) {
          db_.rollback();
          return false;
        }
        ++itParam;
      }
      //
      vector<FileDataParam*> fileList = (*itTask)->getFileList();
      vector<FileDataParam*>::iterator itFile = fileList.begin();
      while (itFile != fileList.end() ) {
        if( saveFileData(taskInstId, *itFile)==false) {
          db_.rollback();
          return false;
        }
        ++itFile;
      }
      //
      vector<ImageDataParam*> imageList = (*itTask)->getImageList();
      vector<ImageDataParam*>::iterator itImage = imageList.begin();
      while (itImage != imageList.end() ) {
        if( saveImageData(taskInstId, *itImage)==false) {
          db_.rollback();
          return false;
        }
        ++itImage;
      }
      //
      vector<ElementStmParam*> stateList = (*itTask)->getStmElementList();
      vector<ElementStmParam*>::iterator itState = stateList.begin();
      while (itState != stateList.end() ) {
        if( saveElementStmData(taskInstId, *itState)==false) {
          db_.rollback();
          return false;
        }
        ++itState;
      }
      //
      vector<ConnectionStmParam*> transList = (*itTask)->getStmConnectionList();
      vector<ConnectionStmParam*>::iterator itTrans = transList.begin();
      while (itTrans != transList.end() ) {
        DDEBUG_V("Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
        if((*itTrans)->getSourceId()==(*itTrans)->getTargetId()) continue;
        vector<ElementStmParam*>::iterator sourceElem = find_if( stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
        if(sourceElem== stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found SourceId");
          continue;
        }
        (*itTrans)->setSourceId((*sourceElem)->getId());

        vector<ElementStmParam*>::iterator targetElem = find_if( stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
        if(targetElem== stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found TargetId");
          continue;
        }
        (*itTrans)->setTargetId((*targetElem)->getId());
        DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());

        if( saveTransactionStmData(taskInstId, *itTrans)==false) {
          db_.rollback();
          return false;
        }
        ++itTrans;
      }
      ++itTask;
    }
  } catch (...) {
    db_.rollback();
    return false;
  }
  db_.commit();
  return true;
}

bool DatabaseManager::saveTaskModels(vector<TaskModelParam*> source) {
  DDEBUG("saveTaskModels");
  errorStr_ = "";

  vector<TaskModelParam*>::iterator itTask = source.begin();
  db_.transaction();
  while (itTask != source.end() ) {
    if(saveTaskInstanceData(*itTask, true)==false) {
        db_.rollback();
        return false;
    }

    if( saveDetailData(*itTask)==false) {
      db_.rollback();
      return false;
    }
    //
    vector<ParameterParam*> paramList = (*itTask)->getParameterList();
    vector<ParameterParam*>::iterator itParam = paramList.begin();
    while (itParam != paramList.end() ) {
      if( saveTaskParameterData((*itTask)->getId(), *itParam)==false) {
        db_.rollback();
        return false;
      }
      ++itParam;
    }
    //
    vector<ElementStmParam*> stateList = (*itTask)->getStmElementList();
    vector<ElementStmParam*>::iterator itState = stateList.begin();
    while (itState != stateList.end() ) {
      if( saveElementStmData((*itTask)->getId(), *itState)==false) {
        db_.rollback();
        return false;
      }
      ++itState;
    }
    //
    vector<ConnectionStmParam*> transList = (*itTask)->getStmConnectionList();
    vector<ConnectionStmParam*>::iterator itTrans = transList.begin();
    while (itTrans != transList.end() ) {
      if((*itTrans)->getSourceId()<0) {
        vector<ElementStmParam*>::iterator sourceElem = find_if( stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
        if(sourceElem== stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found SourceId");
          continue;
        }
        (*itTrans)->setSourceId((*sourceElem)->getId());
      }

      if((*itTrans)->getTargetId()<0) {
        vector<ElementStmParam*>::iterator targetElem = find_if( stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
        if(targetElem== stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found TargetId");
          continue;
        }
        (*itTrans)->setTargetId((*targetElem)->getId());
        DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
      }
      if( saveTransactionStmData((*itTask)->getId(), *itTrans)==false) {
        db_.rollback();
        return false;
      }
      ++itTrans;
    }
    //
    itState = stateList.begin();
    while (itState != stateList.end() ) {
      (*itState)->updateId();
      ++itState;
    }
    ++itTask;
  }
  db_.commit();
  //
  //
  return true;
}

bool DatabaseManager::deleteTaskModel(TaskModelParam* target) {
  int id = target->getId();
  errorStr_ = "";
  DDEBUG_V("deleteTaskModel : %d", id);

  db_.transaction();
  {
    string strQuery = "DELETE FROM T_TASK_MODEL_INST "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_TASK_MODEL_INST) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    for(int index=0; index<target->getModelList().size(); index++) {
      ModelParam* model = target->getModelList()[index];

      string strQuery = "DELETE FROM T_MODEL_DETAIL "; 
      strQuery += "WHERE model_id = ? ";
      QSqlQuery query(QString::fromStdString(strQuery));
      query.addBindValue(model->getId());
      if(!query.exec()) {
        errorStr_ = "DELETE(T_MODEL_DETAIL) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
        db_.rollback();
        return false;
      }
    }
  }
  {
    string strQuery = "DELETE FROM T_MODEL_INFO "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_MODEL_INFO) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    string strQuery = "DELETE FROM T_TASK_INST_PARAMETER "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    string strQuery = "DELETE FROM T_FIGURE "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_FIGURE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    string strQuery = "DELETE FROM T_FILE "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_FILE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    vector<ElementStmParam*> stateList = target->getStmElementList();
    for(int index=0; index<stateList.size(); index++) {
      ElementStmParam* state = stateList[index];
      string strQuery = "DELETE FROM T_STATE_ACTION "; 
      strQuery += "WHERE state_id = ? ";
      QSqlQuery query(QString::fromStdString(strQuery));
      query.addBindValue(state->getId());
      if(!query.exec()) {
        errorStr_ = "DELETE(T_STATE_ACTION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
        db_.rollback();
        return false;
      }
      //
      string strQueryArg = "DELETE FROM T_ARGUMENT "; 
      strQueryArg += "WHERE state_id = ? ";
      QSqlQuery queryArg(QString::fromStdString(strQueryArg));
      queryArg.addBindValue(state->getId());
      if(!queryArg.exec()) {
        errorStr_ = "DELETE(T_ARGUMENT) error:" + queryArg.lastError().databaseText() + "-" + QString::fromStdString(strQueryArg);
        db_.rollback();
        return false;
      }

    }
  }
  {
    string strQuery = "DELETE FROM T_STATE "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  {
    string strQuery = "DELETE FROM T_TRANSITION "; 
    strQuery += "WHERE task_inst_id = ? ";
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(id);
    if(!query.exec()) {
      errorStr_ = "DELETE(T_TRANSITION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      db_.rollback();
      return false;
    }
  }
  db_.commit();
  return true;
}

/////
bool DatabaseManager::deleteFlowModel(int id) {
  DDEBUG("deleteFlowModel");

  string strQuery = "DELETE FROM T_FLOW "; 
  strQuery += "WHERE flow_id = ? ";

  QSqlQuery query(QString::fromStdString(strQuery));
  query.addBindValue(id);

  if(!query.exec()) {
    errorStr_ = "DELETE(T_FLOW) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
    return false;
  }
  //
  string strItemQuery = "DELETE FROM T_FLOW_ITEM "; 
  strItemQuery += "WHERE flow_id = ? ";
  QSqlQuery itemQuery(QString::fromStdString(strItemQuery));
  itemQuery.addBindValue(id);
  if(!itemQuery.exec()) {
    errorStr_ = "DELETE(T_FLOW_ITEM) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
    return false;
  }
  return true;
}
/////
bool DatabaseManager::saveDetailData(TaskModelParam* source) {
  vector<ModelParam*> modelList = source->getModelList();
  vector<ModelParam*>::iterator itModel = modelList.begin();
  while (itModel != modelList.end() ) {
    if( saveModelData(source->getId(), *itModel)==false) {
      db_.rollback();
      return false;
    }
    ++itModel;
  }
  //
  vector<ImageDataParam*> imageList = source->getImageList();
  vector<ImageDataParam*>::iterator itImage = imageList.begin();
  while (itImage != imageList.end() ) {
    if( saveImageData(source->getId(), *itImage)==false) {
      db_.rollback();
      return false;
    }
    ++itImage;
  }
  //
  vector<FileDataParam*> fileList = source->getFileList();
  vector<FileDataParam*>::iterator itFile = fileList.begin();
  while (itFile != fileList.end() ) {
    if( saveFileData(source->getId(), *itFile)==false) {
      db_.rollback();
      return false;
    }
    ++itFile;
  }
  return true;
}
//////////
QByteArray DatabaseManager::image2DB(QString& name, const QImage& source) {
    QByteArray result;
    QBuffer buffer (&result);
    buffer.open(QBuffer::WriteOnly);
    if(name.toUpper().endsWith("PNG")) {
      source.save(&buffer, "PNG");
    } else if(name.toUpper().endsWith("JPG")) {
      source.save(&buffer, "JPG");
    }
    return result;
  }

}
