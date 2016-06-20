#include "DataBaseManager.h"
#include <QVariant>
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

/////T_FLOW/////
bool DatabaseManager::saveFlowData(FlowParam* source) {
  DDEBUG("updateFlowData");
  db_.transaction();
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(flow_id) FROM T_FLOW "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_FLOW "; 
    strQuery += "(flow_id, name) ";
    strQuery += "VALUES ( ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(QString::fromStdString(toStr(maxId)));
    query.addBindValue(source->getName());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_FLOW) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_FLOW "; 
    strQuery += "SET name = ? ";
    strQuery += "WHERE flow_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getName());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_FLOW) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_FLOW "; 
    strQuery += "WHERE flow_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_FLOW) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  //
  string strQuery = "DELETE FROM T_FLOW_ITEM "; 
  strQuery += "WHERE flow_id = ? ";
  QSqlQuery query(QString::fromStdString(strQuery));
  query.addBindValue(source->getId());
  if(!query.exec()) {
    errorStr_ = "DELETE(T_FLOW_ITEM) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
    return false;
  }
  //
  vector<TaskModelParam*> taskList = source->getTaskList();
  vector<TaskModelParam*>::iterator itTask = taskList.begin();
  while (itTask != taskList.end() ) {
    if( saveFlowItemData(source->getId(), *itTask)==false) {
      db_.rollback();
      return false;
    }
    ++itTask;
  }
  db_.commit();
  return true;
}
/////T_FLOW_ITEM/////
bool DatabaseManager::saveFlowItemData(int flowId, TaskModelParam* source) {
  DDEBUG("saveFlowItemData");
  if(source->getMode()==DB_MODE_DELETE || source->getMode()==DB_MODE_IGNORE) return true;

  string strMaxQuery = "SELECT max(flow_item_id) FROM T_FLOW_ITEM "; 
  QSqlQuery maxQuery(db_);
  maxQuery.exec(strMaxQuery.c_str());
  int maxId = -1;
  if (maxQuery.next()) {
    maxId = maxQuery.value(0).toInt();
    maxId++;
  }
  //
  string strQuery = "INSERT INTO T_FLOW_ITEM "; 
  strQuery += "(flow_item_id, flow_id, seq, task_inst_id) ";
  strQuery += "VALUES ( ?, ?, ?, ? )";

  QSqlQuery query(QString::fromStdString(strQuery));
  query.addBindValue(QString::fromStdString(toStr(maxId)));
  query.addBindValue(flowId);
  query.addBindValue(source->getSeq());
  query.addBindValue(source->getId());
  if(!query.exec()) {
    errorStr_ = "INSERT(T_FLOW_ITEM) error:" + query.lastError().databaseText();
    return false;
  }
  source->setNormal();
  return true;
}
/////T_TASK_MODEL_INST/////
bool DatabaseManager::saveTaskInstanceData(TaskModelParam* source) {
  DDEBUG("updateTaskInstanceData");
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(task_inst_id) FROM T_TASK_MODEL_INST "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_TASK_MODEL_INST "; 
    strQuery += "(task_inst_id, name, task_id, comment, flow_id) ";
    strQuery += "VALUES ( ?, ?, ?, ?, -1 )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(QString::fromStdString(toStr(maxId)));
    query.addBindValue(source->getName());
    query.addBindValue(source->getTaskId());
    query.addBindValue(source->getComment());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_TASK_MODEL_INST) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_TASK_MODEL_INST "; 
    strQuery += "SET name = ?, comment = ? ";
    strQuery += "WHERE task_inst_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getName());
    query.addBindValue(source->getComment());
    query.addBindValue(source->getId());
    if(!query.exec()) {
      errorStr_ = "UPDATE(T_TASK_MODEL_INST) error:"  + query.lastError().databaseText() + "-" ; QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}
/////T_STATE/////
vector<ElementStmParam*> DatabaseManager::getStateParams(int instId) {
  vector<ElementStmParam*> result;

  string strStmId = toStr(instId);
  string strStmQuery = "SELECT "; 
  strStmQuery += "state_id, task_inst_id, type, cmd_name, pos_x, pos_y ";
  strStmQuery += "FROM T_STATE ";
  strStmQuery += "WHERE task_inst_id = " + strStmId + " ORDER BY state_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int state_id = stmQuery.value(0).toInt();
    int type = stmQuery.value(2).toInt();
    QString cmd_name = stmQuery.value(3).toString();
    double posX = stmQuery.value(4).toDouble();
    double posY = stmQuery.value(5).toDouble();
    //
    ElementStmParam* param = new ElementStmParam(state_id, type, cmd_name, posX, posY);
    param->setOrgId(state_id);
    result.push_back(param);
    //
    vector<ElementStmActionParam*> actionList = getStmActionList(state_id);
    std::vector<ElementStmActionParam*>::iterator itAction = actionList.begin();
    while (itAction != actionList.end() ) {
      param->addModelAction(*itAction);
      ++itAction;
    }
    //
    vector<ArgumentParam*> argList = getArgumentParams(state_id);
    std::vector<ArgumentParam*>::iterator itArg = argList.begin();
    while (itArg != argList.end() ) {
      param->addArgument(*itArg);
      ++itArg;
    }
    //
    CommandDefParam* def = TaskExecutor::instance()->getCommandDef(cmd_name.toStdString());
    param->setCommadDefParam(def);

  }
  return result;
}

bool DatabaseManager::saveElementStmData(int parentId, ElementStmParam* source) {
  if(source->getMode()==DB_MODE_INSERT) {
    DDEBUG("saveElementStmData : INSERT");
    string strMaxQuery = "SELECT max(state_id) FROM T_STATE "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_STATE "; 
    strQuery += "(state_id, task_inst_id, type, cmd_name, pos_x, pos_y) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(parentId);
    query.addBindValue(source->getType());
    query.addBindValue(source->getCmdName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_STATE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    DDEBUG("saveElementStmData : UPDATE");
    string strQuery = "UPDATE T_STATE "; 
    strQuery += "SET type = ?, cmd_name = ?, pos_x = ?, pos_y = ? ";
    strQuery += "WHERE state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getCmdName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_STATE) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    DDEBUG("saveElementStmData : DELETE");
    string strQuery = "DELETE FROM T_STATE "; 
    strQuery += "WHERE state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
    //
    vector<ElementStmActionParam*> actionList = source->getActionList();
    vector<ElementStmActionParam*>::iterator itAction = actionList.begin();
    while (itAction != actionList.end() ) {
      (*itAction)->setDelete();
      ++itAction;
    }
    //
    vector<ArgumentParam*> argList = source->getArgList();
    vector<ArgumentParam*>::iterator itArg = argList.begin();
    while (itArg != argList.end() ) {
      (*itArg)->setDelete();
      ++itArg;
    }
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
  //
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
  return true;
}

/////T_STATE_ACTION/////
vector<ElementStmActionParam*> DatabaseManager::getStmActionList(int stateId) {
  vector<ElementStmActionParam*> result;

  string strStmId = toStr(stateId);
  string strStmQuery = "SELECT "; 
  strStmQuery += "state_action_id, seq, action, model, target ";
  strStmQuery += "FROM T_STATE_ACTION ";
  strStmQuery += "WHERE state_id = " + strStmId + " ORDER BY seq";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int id = stmQuery.value(0).toInt();
    int seq = stmQuery.value(1).toInt();
    QString action = stmQuery.value(2).toString();
    QString model = stmQuery.value(3).toString();
    QString target = stmQuery.value(4).toString();
    //
    ElementStmActionParam* param = new ElementStmActionParam(id, stateId, seq, action, model, target, false);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveElementStmActionData(int stateId, ElementStmActionParam* source) {
  DDEBUG_V("updateElementStmActionData : %d", source->getMode());
  if(source->getMode()==DB_MODE_INSERT) {
    DDEBUG("saveElementStmActionData : INSERT");
    string strMaxQuery = "SELECT max(state_action_id) FROM T_STATE_ACTION "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_STATE_ACTION "; 
    strQuery += "(state_action_id, state_id, seq, action, model, target) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());
    query.addBindValue(stateId);
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getAction());
    query.addBindValue(source->getModel());
    query.addBindValue(source->getTarget());
    source->setStateId(stateId);
    if(!query.exec()) {
      errorStr_ = "INSERT(T_STATE_ACTION) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_STATE_ACTION "; 
    strQuery += "SET seq = ?, action = ?, model = ?, target = ? ";
    strQuery += "WHERE state_action_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getAction());
    query.addBindValue(source->getModel());
    query.addBindValue(source->getTarget());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_STATE_ACTION) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_STATE_ACTION "; 
    strQuery += "WHERE state_action_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_STATE_ACTION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}

/////T_ARGUMENT/////
vector<ArgumentParam*> DatabaseManager::getArgumentParams(int stateId) {
  vector<ArgumentParam*> result;

  string strStmId = toStr(stateId);
  string strStmQuery = "SELECT "; 
  strStmQuery += "arg_id, seq, name, value ";
  strStmQuery += "FROM T_ARGUMENT ";
  strStmQuery += "WHERE state_id = " + strStmId + " ORDER BY seq";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int id = stmQuery.value(0).toInt();
    int seq = stmQuery.value(1).toInt();
    QString name = stmQuery.value(2).toString();
    QString value = stmQuery.value(3).toString();
    //
    ArgumentParam* param = new ArgumentParam(id, stateId, seq, name, value);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveArgumentData(int stateId, ArgumentParam* source) {
  if(source->getMode()==DB_MODE_INSERT) {
    DDEBUG("saveArgumentData : INSERT");
    string strMaxQuery = "SELECT max(arg_id) FROM T_ARGUMENT "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_ARGUMENT "; 
    strQuery += "(arg_id, state_id, seq, name, value) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());
    query.addBindValue(stateId);
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getName());
    query.addBindValue(source->getValueDesc());
    source->setStateId(stateId);
    if(!query.exec()) {
      errorStr_ = "INSERT(T_ARGUMENT) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_ARGUMENT "; 
    strQuery += "SET value = ? ";
    strQuery += "WHERE arg_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getValueDesc());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_ARGUMENT) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_ARGUMENT "; 
    strQuery += "WHERE arg_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_ARGUMENT) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}
/////T_TRANSITION/////
vector<ConnectionStmParam*> DatabaseManager::getTransParams(int instId) {
  vector<ConnectionStmParam*> result;

  string strStmId = toStr(instId);
  string strStmQuery = "SELECT "; 
  strStmQuery += "trans_id, task_inst_id, source_id, target_id, condition ";
  strStmQuery += "FROM T_TRANSITION ";
  strStmQuery += "WHERE task_inst_id = " + strStmId + " ORDER BY trans_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int trans_id = stmQuery.value(0).toInt();
    int source_id = stmQuery.value(2).toInt();
    int target_id = stmQuery.value(3).toInt();
    QString condition = stmQuery.value(4).toString();
    //
    ConnectionStmParam* param = new ConnectionStmParam(trans_id, source_id, target_id, condition);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveTransactionStmData(int parentId, ConnectionStmParam* source) {
  bool isNew = false;

  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(trans_id) FROM T_TRANSITION "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_TRANSITION "; 
    strQuery += "(trans_id, task_inst_id, source_id, target_id, condition) ";
    strQuery += "VALUES ( :trans_id, :task_inst_id, :source_id, :target_id, :condition )";

    QSqlQuery queryTra(QString::fromStdString(strQuery));
    queryTra.bindValue(":trans_id", maxId);
    queryTra.bindValue(":task_inst_id", parentId);
    queryTra.bindValue(":source_id", source->getSourceId());
    queryTra.bindValue(":target_id", source->getTargetId());
    queryTra.bindValue(":condition", source->getCondition());
    if(!queryTra.exec()) {
      errorStr_ = "INSERT(T_TRANSITION) error:" + queryTra.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_TRANSITION "; 
    strQuery += "SET condition = ? ";
    strQuery += "WHERE trans_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getCondition());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_TRANSITION) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_TRANSITION "; 
    strQuery += "WHERE trans_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_TRANSITION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}
/////T_TASK_PARAMETER, T_TASK_INST_PARAMETER/////
vector<ParameterParam*> DatabaseManager::getParameterParams(int taskId, int instId) {
  DDEBUG_V("getParameterParams %d, %d", taskId, instId);
  vector<ParameterParam*> result;

  string strInstId = toStr(instId);
  string strInstQuery = "SELECT "; 
  strInstQuery += "parameter_id, task_inst_id, value, T_TASK_INST_PARAMETER.task_param_id, elem_num, ";
  strInstQuery += "name, rname, unit, type, model_name, elem_types ";
  strInstQuery += "FROM T_TASK_INST_PARAMETER ";
  strInstQuery += "LEFT OUTER JOIN T_TASK_PARAMETER ";
  strInstQuery += "ON T_TASK_INST_PARAMETER.task_param_id = T_TASK_PARAMETER.task_param_id ";
  strInstQuery += "WHERE task_inst_id = " + strInstId + " ORDER BY parameter_id";
  QSqlQuery instQuery(db_);
  instQuery.exec(strInstQuery.c_str());
  while (instQuery.next()) {
    int id = instQuery.value(0).toInt();
    QString value = instQuery.value(2).toString();
    int task_param_id = instQuery.value(3).toInt();
    int elemNum = instQuery.value(4).toInt();
    QString name = instQuery.value(5).toString();
    QString rname = instQuery.value(6).toString();
    QString unit = instQuery.value(7).toString();
    int type = instQuery.value(8).toInt();
    QString modelName = instQuery.value(9).toString();
    QString elemTypes = instQuery.value(10).toString();

    ParameterParam* param = new ParameterParam(id, type, modelName, elemNum, elemTypes, task_param_id, name, rname, unit);
    param->setDBValues(value);
    result.push_back(param);
  }
  //
  if(0<result.size()) return result;
  string strId = toStr(taskId);
  string strQuery = "SELECT "; 
  strQuery += "task_param_id, elem_num, name, rname, unit, type, model_name, elem_types ";
  strQuery += "FROM T_TASK_PARAMETER ";
  strQuery += "WHERE task_id = " + strId + " ORDER BY task_param_id";
  QSqlQuery query(db_);
  query.exec(strQuery.c_str());
  while (query.next()) {
    int task_param_id = query.value(0).toInt();
    int elemNum = query.value(1).toInt();
    QString name = query.value(2).toString();
    QString rname = query.value(3).toString();
    QString unit = query.value(4).toString();
    int type = instQuery.value(5).toInt();
    QString modelName = instQuery.value(6).toString();
    QString elemTypes = instQuery.value(7).toString();
    ParameterParam* param = new ParameterParam(0, type, modelName, elemNum, elemTypes, task_param_id, name, rname, unit);
    param->setNew();
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveTaskParameterData(int taskId, ParameterParam* source) {
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(task_param_id) FROM T_TASK_PARAMETER "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setTaskParamId(maxId);
    //
    string strQuery = "INSERT INTO T_TASK_PARAMETER "; 
    strQuery += "(task_param_id, task_id, elem_num, name, rname, unit, type, model_name, elem_types) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(taskId);
    query.addBindValue(source->getElemNum());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getUnit());
    query.addBindValue(source->getType());
    query.addBindValue(source->getModelName());
    query.addBindValue(source->getElemTypes());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_TASK_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_TASK_PARAMETER "; 
    strQuery += "SET type = ?, model_name = ?, elem_num = ?, elem_types = ?, name = ?, ";
    strQuery += "rname = ?, unit = ? ";
    strQuery += "WHERE task_param_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getModelName());
    query.addBindValue(source->getElemNum());
    query.addBindValue(source->getElemTypes());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getUnit());
    query.addBindValue(source->getId());
    if(!query.exec()) {
      errorStr_ = "UPDATE(T_TASK_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_TASK_PARAMETER "; 
    strQuery += "WHERE task_param_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_TASK_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    //
    string strQueryInst = "DELETE FROM T_TASK_INST_PARAMETER "; 
    strQueryInst += "WHERE task_param_id = ? ";

    QSqlQuery queryInst(QString::fromStdString(strQueryInst));
    queryInst.addBindValue(source->getId());

    if(!queryInst.exec()) {
      errorStr_ = "DELETE(T_TASK_INST_PARAMETER) error:" + queryInst.lastError().databaseText() + "-" + QString::fromStdString(strQueryInst);
      return false;
    }
    source->setNormal();
  }
  return true;
}

bool DatabaseManager::saveTaskInstParameterData(int parentId, ParameterParam* source) {
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(parameter_id) FROM T_TASK_INST_PARAMETER "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_TASK_INST_PARAMETER "; 
    strQuery += "(parameter_id, task_inst_id, task_param_id, value) VALUES ( ?, ?, ?, ?)";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(parentId);
    query.addBindValue(source->getTaskParamId());
    query.addBindValue(source->getDBValues());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    DDEBUG_V("UPDATE Data : %s, Id : %d", source->getDBValues().toUtf8().constData(), source->getId());
    string strQuery = "UPDATE T_TASK_INST_PARAMETER "; 
    strQuery += "SET value = ? ";
    strQuery += "WHERE parameter_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getDBValues());
    query.addBindValue(source->getId());
    if(!query.exec()) {
      errorStr_ = "UPDATE(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}

/////T_MODEL_INFO/////
vector<ModelParam*> DatabaseManager::getModelParams(int id) {
  vector<ModelParam*> result;

  string strId = toStr(id);
  string strQuery = "SELECT "; 
  strQuery += "model_id, task_inst_id, model_type, name, rname, file_name, model_data, ";
  strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z ";
  strQuery += "FROM T_MODEL_INFO WHERE task_inst_id = " + strId + " ORDER BY model_id";

  QSqlQuery query(db_);
  query.exec(strQuery.c_str());
  while (query.next()) {
    int model_id = query.value(0).toInt();
    int task_id = query.value(1).toInt();
    int model_type = query.value(2).toInt();
    QString name = query.value(3).toString();
    QString rname = query.value(4).toString();
    QString fileName = query.value(5).toString();
    double posX = query.value(7).toDouble();
    double posY = query.value(8).toDouble();
    double posZ = query.value(9).toDouble();
    double rotX = query.value(10).toDouble();
    double rotY = query.value(11).toDouble();
    double rotZ = query.value(12).toDouble();
    //
    ModelParam* param = new ModelParam(model_id, model_type, name, rname, fileName, posX, posY, posZ, rotX, rotY, rotZ, false);
    param->setData(query.value(6).toByteArray());
    result.push_back(param);
    //
    string strSubQuery = "SELECT "; 
    strSubQuery += "model_detail_id, file_name, model_data ";
    strSubQuery += "FROM T_MODEL_DETAIL WHERE model_id = " + toStr(model_id) + " ORDER BY model_detail_id";
    QSqlQuery subQuery(db_);
    subQuery.exec(strSubQuery.c_str());
    while (subQuery.next()) {
      int detail_id = subQuery.value(0).toInt();
      QString detailName = subQuery.value(1).toString();
      ModelDetailParam* detailParam = new ModelDetailParam(detail_id, detailName);
      detailParam->setData(subQuery.value(2).toByteArray());
      param->addModelDetail(detailParam);
    }
  }
  return result;
}

bool DatabaseManager::saveModelData(int parentId, ModelParam* source) {
  DDEBUG_V("updateModelData : %d", parentId);
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(model_id) FROM T_MODEL_INFO "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_MODEL_INFO "; 
    strQuery += "(model_id, task_inst_id, model_type, name, rname, file_name, model_data, ";
    strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ";
    strQuery += "?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(parentId);
    query.addBindValue(source->getType());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getFileName());
    query.addBindValue(source->getData());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getPosZ());
    query.addBindValue(source->getRotRx());
    query.addBindValue(source->getRotRy());
    query.addBindValue(source->getRotRz());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_MODEL_INFO) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_MODEL_INFO "; 
    strQuery += "SET model_type = ?, name = ?, rname = ?, file_name = ?, model_data = ?, ";
    strQuery += "pos_x = ?, pos_y = ?, pos_z = ?, rot_x = ?, rot_y = ?, rot_z = ? ";
    strQuery += "WHERE model_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getFileName());
    query.addBindValue(source->getData());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getPosZ());
    query.addBindValue(source->getRotRx());
    query.addBindValue(source->getRotRy());
    query.addBindValue(source->getRotRz());
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "UPDATE(T_MODEL_INFO) error:" + query.lastError().databaseText() + "-"  + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_MODEL_INFO "; 
    strQuery += "WHERE model_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_MODEL_INFO) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  //
  vector<ModelDetailParam*> detailList = source->getModelDetailList();
  for(int index=0; index<detailList.size(); index++) {
    ModelDetailParam* detail = detailList[index];
    if( saveModelDetailData(source->getId(), detail)==false ) {
      return false;
    }
  }
  //
  return true;
}

/////T_MODEL_DETAIL/////
bool DatabaseManager::saveModelDetailData(int modelId, ModelDetailParam* source) {
  DDEBUG_V("saveModelDetailData : %d, %d", modelId, source->getMode());
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(model_detail_id) FROM T_MODEL_DETAIL "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO T_MODEL_DETAIL "; 
    strQuery += "(model_detail_id, model_id, file_name, model_data) ";
    strQuery += "VALUES ( ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(modelId);
    query.addBindValue(source->getFileName());
    query.addBindValue(source->getData());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_MODEL_DETAIL) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_MODEL_DETAIL "; 
    strQuery += "WHERE model_detail_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if(!query.exec()) {
      errorStr_ = "DELETE(T_MODEL_DETAIL) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}

/////T_FIGURE/////
vector<ImageDataParam*> DatabaseManager::getFigureParams(int id) {
  vector<ImageDataParam*> result;

  string strQuery = "SELECT "; 
  strQuery += "figure_id, task_inst_id, name, data ";
  strQuery += "FROM T_FIGURE WHERE task_inst_id = " + toStr(id) + " " + "ORDER BY figure_id";

  QSqlQuery query(db_);
  if(query.exec(strQuery.c_str())==false) {
    DDEBUG_V("error: %s", query.lastError().databaseText().toUtf8().constData());
  }
  while (query.next()) {
    int id = query.value(0).toInt();
    //int task_id = query.value(1).toInt();
    QString name = query.value(2).toString();
    //
    ImageDataParam* param = new ImageDataParam(id, name);
    param->setRawData(query.value(3).toByteArray());
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveImageData(int parentId, ImageDataParam* source) {

  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(figure_id) FROM T_FIGURE "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_FIGURE "; 
    strQuery += "(figure_id, task_inst_id, name, data) VALUES ( ?, ?, ?, ?)";

    QString strName = source->getName();

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(parentId);
    query.addBindValue(strName);
    query.addBindValue(image2DB(strName, source->getData()));
    if(!query.exec()) {
      errorStr_ = "INSERT(T_FIGURE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_FIGURE WHERE figure_id = " + toStr(source->getId());
    QSqlQuery query(QString::fromStdString(strQuery));
    if(!query.exec()) {
      errorStr_ = "DELETE(T_FIGURE) error:" + query.lastError().databaseText() +  "=" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}

/////T_FILE/////
vector<FileDataParam*> DatabaseManager::getFileParams(int id) {
  vector<FileDataParam*> result;

  string strQuery = "SELECT "; 
  strQuery += "file_id, task_inst_id, name, file_data ";
  strQuery += "FROM T_FILE WHERE task_inst_id = " + toStr(id) + " " + "ORDER BY file_id";

  QSqlQuery query(db_);
  if(query.exec(strQuery.c_str())==false) {
    DDEBUG_V("error: %s", query.lastError().databaseText().toUtf8().constData());
  }
  while (query.next()) {
    int id = query.value(0).toInt();
    //int task_id = query.value(1).toInt();
    QString name = query.value(2).toString();
    //
    FileDataParam* param = new FileDataParam(id, name);
    param->setData(query.value(3).toByteArray());
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFileData(int parentId, FileDataParam* source) {
  DDEBUG("updateFileData");
  if(source->getMode()==DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(file_id) FROM T_FILE "; 
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_FILE "; 
    strQuery += "(file_id, task_inst_id, name, file_data) VALUES ( ?, ?, ?, ?)";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(maxId);
    query.addBindValue(parentId);
    query.addBindValue(source->getName());
    query.addBindValue(source->getData());
    if(!query.exec()) {
      errorStr_ = "INSERT(T_FILE) error:"  + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if(source->getMode()==DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_FILE WHERE file_id = " + toStr(source->getId());
    QSqlQuery query(QString::fromStdString(strQuery));
    if(!query.exec()) {
      errorStr_ = "DELETE(T_FILE) error:" + query.lastError().databaseText()  + "=" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();
  }
  return true;
}

}
