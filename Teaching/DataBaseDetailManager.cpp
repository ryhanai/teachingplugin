#include "DataBaseManager.h"
#include <QVariant>
#include <qsqlerror.h>
#include "TeachingUtil.h"
#include "TeachingDataHolder.h"
#include "TaskExecutor.h"
#include <cnoid/UTF8>

#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

/////T_FLOW/////
bool DatabaseManager::saveFlowData(FlowParamPtr source) {
  DDEBUG("saveFlowData");
  if (source->getMode() == DB_MODE_INSERT) {
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
    strQuery += "(flow_id, name, comment, created_date, last_updated_date) ";
    strQuery += "VALUES ( ?, ?, ?, datetime('now', 'localtime'), datetime('now', 'localtime') )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(QString::fromStdString(toStr(maxId)));
    query.addBindValue(source->getName());
    query.addBindValue(source->getComment());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_FLOW) error:" + query.lastError().databaseText();
      return false;
    }
    //
    string strQueryUpd = "SELECT ";
    strQueryUpd += "created_date, last_updated_date ";
    strQueryUpd += "FROM T_FLOW ";
    strQueryUpd += "WHERE flow_id = " + toStr(source->getId());
    QSqlQuery taskQuery(db_);
    taskQuery.exec(strQueryUpd.c_str());
    if (taskQuery.next()) {
      QString createdDate = taskQuery.value(0).toString();
      QString updatedDate = taskQuery.value(1).toString();
      source->setCreatedDate(createdDate);
      source->setLastUpdatedDate(updatedDate);
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_FLOW ";
    strQuery += "SET name = ?, comment = ?, last_updated_date = datetime('now', 'localtime')  ";
    strQuery += "WHERE flow_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getName());
    query.addBindValue(source->getComment());
    query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_FLOW) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    string strQueryUpd = "SELECT ";
    strQueryUpd += "last_updated_date ";
    strQueryUpd += "FROM T_FLOW ";
    strQueryUpd += "WHERE flow_id = " + toStr(source->getId());
    QSqlQuery taskQuery(db_);
    taskQuery.exec(strQueryUpd.c_str());
    if (taskQuery.next()) {
      QString updatedDate = taskQuery.value(0).toString();
      source->setLastUpdatedDate(updatedDate);
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_FLOW ";
    strQuery += "WHERE flow_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_FLOW) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}

/////T_FLOW_STATE/////
vector<ElementStmParamPtr> DatabaseManager::getFlowStateParams(int flowId) {
  //DDEBUG_V("DatabaseManager::getFlowStateParams : %d", flowId);

  vector<ElementStmParamPtr> result;

  string strStmId = toStr(flowId);
  string strStmQuery = "SELECT ";
  strStmQuery += "state_id, flow_id, type, task_inst_id, pos_x, pos_y, condition ";
  strStmQuery += "FROM T_FLOW_STATE ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY state_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int state_id = stmQuery.value(0).toInt();
    int type = stmQuery.value(2).toInt();
    int task_inst_id = stmQuery.value(3).toInt();
    double posX = stmQuery.value(4).toDouble();
    double posY = stmQuery.value(5).toDouble();
    QString condition = stmQuery.value(6).toString();
    //
		ElementStmParamPtr param = std::make_shared<ElementStmParam>(state_id, type, "", "", posX, posY, condition);
    if (0 <= task_inst_id) {
			TaskModelParamPtr taskParam = getTaskModelById(task_inst_id);
      if (taskParam == 0) {
        throw std::runtime_error("Task Instance Model NOT EXISTS.");
      }
      param->setCmdDspName(taskParam->getName());
      param->setTaskParam(taskParam);
      taskParam->setStateParam(param);
    }
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFlowStmData(int parentId, ElementStmParamPtr source) {
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG("saveFlowStmData : INSERT");

    string strQuery = "INSERT INTO T_FLOW_STATE ";
    strQuery += "(state_id, flow_id, type, task_inst_id, pos_x, pos_y, condition) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getId());
    query.addBindValue(parentId);
    query.addBindValue(source->getType());
    if (source->getTaskParam()) {
      query.addBindValue(source->getTaskParam()->getId());
    } else {
      query.addBindValue(-1);
    }
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getCondition());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_FLOW_STATE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    DDEBUG("saveFlowStmData : UPDATE");
    string strQuery = "UPDATE T_FLOW_STATE ";
    strQuery += "SET type = ?, task_inst_id = ?, pos_x = ?, pos_y = ?, condition = ? ";
    strQuery += "WHERE flow_id = ? AND state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    if (source->getTaskParam()) {
      query.addBindValue(source->getTaskParam()->getId());
    } else {
      query.addBindValue(-1);
    }
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getCondition());
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_FLOW_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    DDEBUG("saveFlowStmData : DELETE");
    string strQuery = "DELETE FROM T_FLOW_STATE ";
    strQuery += "WHERE flow_id = ? AND state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_FLOW_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}
/////T_FLOW_TRANSITION/////
vector<ConnectionStmParamPtr> DatabaseManager::getFlowTransParams(int flowId) {
  DDEBUG_V("DatabaseManager::getFlowTransParams : %d", flowId);
  vector<ConnectionStmParamPtr> result;

  string strStmId = toStr(flowId);
  string strStmQuery = "SELECT ";
  strStmQuery += "trans_id, flow_id, type, source_id, source_index, target_id, target_index ";
  strStmQuery += "FROM T_FLOW_TRANSITION ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY trans_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int trans_id = stmQuery.value(0).toInt();
    int type = stmQuery.value(2).toInt();
    int source_id = stmQuery.value(3).toInt();
    int source_index = stmQuery.value(4).toInt();
    int target_id = stmQuery.value(5).toInt();
    int target_index = stmQuery.value(6).toInt();
    //
		ConnectionStmParamPtr param = std::make_shared<ConnectionStmParam>(trans_id, type, source_id, source_index, target_id, target_index);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFlowTransactionStmData(int parentId, vector<ConnectionStmParamPtr>& source) {
	DDEBUG_V("DatabaseManager::saveFlowTransactionStmData : %d", parentId);

	string strQuery = "DELETE FROM T_FLOW_TRANSITION ";
	strQuery += "WHERE flow_id = ?";

	QSqlQuery query(QString::fromStdString(strQuery));
	query.addBindValue(parentId);
	if (!query.exec()) {
		errorStr_ = "DELETE(T_FLOW_TRANSITION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
		return false;
	}
	//
	for (int index = 0; index < source.size(); index++) {
		ConnectionStmParamPtr param = source[index];

		string strQuery = "INSERT INTO T_FLOW_TRANSITION ";
		strQuery += "(flow_id, trans_id, type, source_id, source_index, target_id, target_index) ";
		strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ? )";

		QSqlQuery queryTra(QString::fromStdString(strQuery));
		queryTra.addBindValue(parentId);
		queryTra.addBindValue(index + 1);
    queryTra.addBindValue(param->getType());
    queryTra.addBindValue(param->getSourceId());
    queryTra.addBindValue(param->getSourceIndex());
    queryTra.addBindValue(param->getTargetId());
		queryTra.addBindValue(param->getTargetIndex());

		if (!queryTra.exec()) {
		  errorStr_ = "INSERT(T_FLOW_TRANSITION) error:" + queryTra.lastError().databaseText();
		  return false;
		}
		param->setNormal();
	}
  //
  return true;
}
/////T_FLOW_MODEL_PARAM/////
vector<FlowModelParamPtr> DatabaseManager::getFlowModelParams(int flowId) {
  DDEBUG_V("DatabaseManager::getFlowModelParams : %d", flowId);
  vector<FlowModelParamPtr> result;

  string strStmId = toStr(flowId);
  string strStmQuery = "SELECT ";
  strStmQuery += "flow_id, model_id, master_id, master_param_id, pos_x, pos_y ";
  strStmQuery += "FROM T_FLOW_MODEL_PARAM ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY model_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int model_id = stmQuery.value(1).toInt();
    int master_id = stmQuery.value(2).toInt();
    int master_param_id = stmQuery.value(3).toInt();
    double pos_x = stmQuery.value(4).toDouble();
    double pos_y = stmQuery.value(5).toDouble();
    //
    FlowModelParamPtr param = std::make_shared<FlowModelParam>(model_id, master_id, master_param_id);
    param->setPosX(pos_x);
    param->setPosY(pos_y);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFlowModelParam(int parentId, vector<FlowModelParamPtr>& source) {
  DDEBUG_V("DatabaseManager::saveFlowModelParam : %d", parentId);

  string strQuery = "DELETE FROM T_FLOW_MODEL_PARAM ";
  strQuery += "WHERE flow_id = ?";
  QSqlQuery query(QString::fromStdString(strQuery));
  query.addBindValue(parentId);
  if (!query.exec()) {
    errorStr_ = "DELETE(T_FLOW_MODEL_PARAM) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
    return false;
  }
  //
  for (int index = 0; index < source.size(); index++) {
    FlowModelParamPtr param = source[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    string strQuery = "INSERT INTO T_FLOW_MODEL_PARAM ";
    strQuery += "(flow_id, model_id, master_id, master_param_id, pos_x, pos_y) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery queryTra(QString::fromStdString(strQuery));
    queryTra.addBindValue(parentId);
    queryTra.addBindValue(index + 1);
    queryTra.addBindValue(param->getMasterId());
    queryTra.addBindValue(param->getMasterParamId());
    queryTra.addBindValue(param->getPosX());
    queryTra.addBindValue(param->getPosY());

    if (!queryTra.exec()) {
      errorStr_ = "INSERT(T_FLOW_MODEL_PARAM) error:" + queryTra.lastError().databaseText();
      return false;
    }
    param->setNormal();
  }
  //
  return true;
}

/////T_FLOW_PARAMETER/////
vector<FlowParameterParamPtr> DatabaseManager::getFlowParamerer(int flowId) {
  DDEBUG_V("DatabaseManager::getFlowParamerer : %d", flowId);
  vector<FlowParameterParamPtr> result;

  string strStmId = toStr(flowId);
  string strStmQuery = "SELECT ";
  strStmQuery += "flow_id, param_id, name, value, pos_x, pos_y ";
  strStmQuery += "FROM T_FLOW_PARAMETER ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY param_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int param_id = stmQuery.value(1).toInt();
    QString name = stmQuery.value(2).toString();
    QString vallue = stmQuery.value(3).toString();
    double pos_x = stmQuery.value(4).toDouble();
    double pos_y = stmQuery.value(5).toDouble();
    //
    FlowParameterParamPtr param = std::make_shared<FlowParameterParam>(param_id, name, vallue);
    param->setPosX(pos_x);
    param->setPosY(pos_y);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFlowParameter(int parentId, vector<FlowParameterParamPtr>& source) {
  DDEBUG_V("DatabaseManager::FlowParameterParamPtr : %d", parentId);

  string strQuery = "DELETE FROM T_FLOW_PARAMETER ";
  strQuery += "WHERE flow_id = ?";
  QSqlQuery query(QString::fromStdString(strQuery));
  query.addBindValue(parentId);
  if (!query.exec()) {
    errorStr_ = "DELETE(T_FLOW_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
    return false;
  }
  //
  for (int index = 0; index < source.size(); index++) {
    FlowParameterParamPtr param = source[index];
    if (param->getMode() == DB_MODE_DELETE || param->getMode() == DB_MODE_IGNORE) continue;

    string strQuery = "INSERT INTO T_FLOW_PARAMETER ";
    strQuery += "(flow_id, param_id, name, value, pos_x, pos_y) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery queryTra(QString::fromStdString(strQuery));
    queryTra.addBindValue(parentId);
    queryTra.addBindValue(index + 1);
    queryTra.addBindValue(param->getName());
    queryTra.addBindValue(param->getValue());
    queryTra.addBindValue(param->getPosX());
    queryTra.addBindValue(param->getPosY());

    if (!queryTra.exec()) {
      errorStr_ = "INSERT(T_FLOW_PARAMETER) error:" + queryTra.lastError().databaseText();
      return false;
    }
    param->setNormal();
  }
  //
  return true;
}
/////T_TASK_MODEL_INST/////
bool DatabaseManager::saveTaskInstanceData(TaskModelParamPtr source, bool updateDate) {
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG("INSERT TaskInstanceData");
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
    strQuery += "(task_inst_id, name, comment, exec_env, flow_id, created_date, last_updated_date) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, datetime('now', 'localtime'), datetime('now', 'localtime') )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(QString::fromStdString(toStr(maxId)));
    query.addBindValue(source->getName());
    query.addBindValue(source->getComment());
    query.addBindValue(source->getExecEnv());
    query.addBindValue(source->getFlowId());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_TASK_MODEL_INST) error:" + query.lastError().databaseText();
      return false;
    }
    //
    if (updateDate) {
      string strQueryUpd = "SELECT ";
      strQueryUpd += "created_date, last_updated_date ";
      strQueryUpd += "FROM T_TASK_MODEL_INST ";
      strQueryUpd += "WHERE task_inst_id = " + toStr(source->getId());
      QSqlQuery taskQuery(db_);
      taskQuery.exec(strQueryUpd.c_str());
      if (taskQuery.next()) {
        QString createdDate = taskQuery.value(0).toString();
        QString updatedDate = taskQuery.value(1).toString();
        source->setCreatedDate(createdDate);
        source->setLastUpdatedDate(updatedDate);
      }
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    DDEBUG("UPDATE TaskInstanceData");
    string strQuery = "UPDATE T_TASK_MODEL_INST ";
    strQuery += "SET name = ?, comment = ?, exec_env = ?, last_updated_date = datetime('now', 'localtime') ";
    strQuery += "WHERE task_inst_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getName());
    query.addBindValue(source->getComment());
    query.addBindValue(source->getExecEnv());
    query.addBindValue(source->getId());
    if (!query.exec()) {
      errorStr_ = "UPDATE(T_TASK_MODEL_INST) error:" + query.lastError().databaseText() + "-"; QString::fromStdString(strQuery);
      return false;
    }
    //
    string strQueryUpd = "SELECT ";
    strQueryUpd += "last_updated_date ";
    strQueryUpd += "FROM T_TASK_MODEL_INST ";
    strQueryUpd += "WHERE task_inst_id = " + toStr(source->getId());
    QSqlQuery taskQuery(db_);
    taskQuery.exec(strQueryUpd.c_str());
    if (taskQuery.next()) {
      QString updatedDate = taskQuery.value(0).toString();
      source->setLastUpdatedDate(updatedDate);
    }
    source->setNormal();
  }
  return true;
}
/////T_STATE/////
vector<ElementStmParamPtr> DatabaseManager::getStateParams(int instId) {
  vector<ElementStmParamPtr> result;

  string strStmId = toStr(instId);
  string strStmQuery = "SELECT ";
  strStmQuery += "state_id, task_inst_id, type, cmd_name, pos_x, pos_y, condition, disp_name ";
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
    QString condition = stmQuery.value(6).toString();
    QString disp_name = stmQuery.value(7).toString();
    //
		ElementStmParamPtr param = std::make_shared<ElementStmParam>(state_id, type, cmd_name, disp_name, posX, posY, condition);
    result.push_back(param);
    //
    vector<ElementStmActionParamPtr> actionList = getStmActionList(instId, state_id);
    std::vector<ElementStmActionParamPtr>::iterator itAction = actionList.begin();
    while (itAction != actionList.end()) {
      param->addModelAction(*itAction);
      ++itAction;
    }
    //
    vector<ArgumentParamPtr> argList = getArgumentParams(instId, state_id);
    std::vector<ArgumentParamPtr>::iterator itArg = argList.begin();
    while (itArg != argList.end()) {
      param->addArgument(*itArg);
      ++itArg;
    }
    //
    CommandDefParam* def = TaskExecutor::instance()->getCommandDef(cmd_name.toStdString());
    param->setCommadDefParam(def);
    if (disp_name == NULL || disp_name.size() == 0) {
      if (def) {
        param->setCmdDspName(def->getDispName());
      }
    }
  }
  return result;
}

bool DatabaseManager::saveStateParameter(int taskId, ElementStmParamPtr source) {
	DDEBUG("saveStateParameter");
	vector<ArgumentParamPtr> argList = source->getArgList();
	vector<ArgumentParamPtr>::iterator itArg = argList.begin();
	while (itArg != argList.end()) {
		if (saveArgumentData(taskId, source->getId(), *itArg) == false) {
			db_.rollback();
			return false;
		}
		++itArg;
	}
	//
	vector<ElementStmActionParamPtr> actionList = source->getActionList();
	vector<ElementStmActionParamPtr>::iterator itAction = actionList.begin();
	while (itAction != actionList.end()) {
		if (saveElementStmActionData(taskId, source->getId(), *itAction) == false) {
			db_.rollback();
			return false;
		}
		++itAction;
	}
	return true;
}

bool DatabaseManager::saveElementStmData(int parentId, ElementStmParamPtr source) {
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG("saveElementStmData : INSERT");
    string strQuery = "INSERT INTO T_STATE ";
    strQuery += "(task_inst_id, state_id, type, cmd_name, pos_x, pos_y, condition, disp_name) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(source->getId());
    query.addBindValue(source->getType());
    query.addBindValue(source->getCmdName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getCondition());
    query.addBindValue(source->getCmdDspName());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_STATE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    DDEBUG("saveElementStmData : UPDATE");
    string strQuery = "UPDATE T_STATE ";
    strQuery += "SET type = ?, cmd_name = ?, pos_x = ?, pos_y = ?, condition = ?, disp_name = ? ";
    strQuery += "WHERE task_inst_id = ? AND state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getCmdName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getCondition());
    query.addBindValue(source->getCmdDspName());
    query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    DDEBUG("saveElementStmData : DELETE");
    string strQuery = "DELETE FROM T_STATE ";
    strQuery += "WHERE task_inst_id = ? AND state_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_STATE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
    //
    vector<ElementStmActionParamPtr> actionList = source->getActionList();
    vector<ElementStmActionParamPtr>::iterator itAction = actionList.begin();
    while (itAction != actionList.end()) {
      (*itAction)->setDelete();
      ++itAction;
    }
    //
    vector<ArgumentParamPtr> argList = source->getArgList();
    vector<ArgumentParamPtr>::iterator itArg = argList.begin();
    while (itArg != argList.end()) {
      (*itArg)->setDelete();
      ++itArg;
    }
  }
  //
  vector<ElementStmActionParamPtr> actionList = source->getActionList();
  vector<ElementStmActionParamPtr>::iterator itAction = actionList.begin();
  while (itAction != actionList.end()) {
    if (saveElementStmActionData(parentId, source->getId(), *itAction) == false) {
      db_.rollback();
      return false;
    }
    ++itAction;
  }
  //
  vector<ArgumentParamPtr> argList = source->getArgList();
  vector<ArgumentParamPtr>::iterator itArg = argList.begin();
  while (itArg != argList.end()) {
    if (saveArgumentData(parentId, source->getId(), *itArg) == false) {
      db_.rollback();
      return false;
    }
    ++itArg;
  }
  //
  return true;
}

/////T_STATE_ACTION/////
vector<ElementStmActionParamPtr> DatabaseManager::getStmActionList(int taskId, int stateId) {
  vector<ElementStmActionParamPtr> result;

  string strStmId = toStr(stateId);
	string strTaskId = toStr(taskId);

  string strStmQuery = "SELECT ";
  strStmQuery += "state_action_id, seq, action, model, target ";
  strStmQuery += "FROM T_STATE_ACTION ";
  strStmQuery += "WHERE task_inst_id = " + strTaskId + " AND state_id = " + strStmId + " ORDER BY seq";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int id = stmQuery.value(0).toInt();
    int seq = stmQuery.value(1).toInt();
    QString action = stmQuery.value(2).toString();
    QString model = stmQuery.value(3).toString();
    QString target = stmQuery.value(4).toString();
    //
		ElementStmActionParamPtr param = std::make_shared<ElementStmActionParam>(id, stateId, seq, action, model, target, false);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveElementStmActionData(int taskId, int stateId, ElementStmActionParamPtr source) {
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG("saveElementStmActionData : INSERT");
    string strMaxQuery = "SELECT max(state_action_id) FROM T_STATE_ACTION WHERE task_inst_id = " + toStr(taskId) + " AND state_id = " + toStr(stateId);
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
    strQuery += "(task_inst_id, state_id, state_action_id, seq, action, model, target) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getAction());
    query.addBindValue(source->getModel());
    query.addBindValue(source->getTarget());
    source->setStateId(stateId);
    if (!query.exec()) {
      errorStr_ = "INSERT(T_STATE_ACTION) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_STATE_ACTION ";
    strQuery += "SET seq = ?, action = ?, model = ?, target = ? ";
    strQuery += "WHERE task_inst_id = ? AND state_id = ? AND state_action_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getAction());
    query.addBindValue(source->getModel());
    query.addBindValue(source->getTarget());
    query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_STATE_ACTION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_STATE_ACTION ";
		strQuery += "WHERE task_inst_id = ? AND state_id = ? AND state_action_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_STATE_ACTION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}

/////T_ARGUMENT/////
vector<ArgumentParamPtr> DatabaseManager::getArgumentParams(int taskId, int stateId) {
  vector<ArgumentParamPtr> result;

	string strTaskId = toStr(taskId);
	string strStmId = toStr(stateId);

  string strStmQuery = "SELECT ";
  strStmQuery += "arg_id, seq, name, value ";
  strStmQuery += "FROM T_ARGUMENT ";
  strStmQuery += "WHERE task_inst_id = " + strTaskId + " AND state_id = " + strStmId + " ORDER BY seq";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int id = stmQuery.value(0).toInt();
    int seq = stmQuery.value(1).toInt();
    QString name = stmQuery.value(2).toString();
    QString value = stmQuery.value(3).toString();
    //
		ArgumentParamPtr param = std::make_shared<ArgumentParam>(id, stateId, seq, name, value);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveArgumentData(int taskId, int stateId, ArgumentParamPtr source) {
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG("saveArgumentData : INSERT");
    string strMaxQuery = "SELECT max(arg_id) FROM T_ARGUMENT WHERE task_inst_id = " + toStr(taskId) + " AND state_id = " + toStr(stateId);
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
    strQuery += "(task_inst_id, state_id, arg_id, seq, name, value) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());
    query.addBindValue(source->getSeq());
    query.addBindValue(source->getName());
    query.addBindValue(source->getValueDesc());
    source->setStateId(stateId);
    if (!query.exec()) {
      errorStr_ = "INSERT(T_ARGUMENT) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_ARGUMENT ";
    strQuery += "SET value = ? ";
    strQuery += "WHERE task_inst_id = ? AND state_id = ? AND arg_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getValueDesc());
    query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_ARGUMENT) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_ARGUMENT ";
		strQuery += "WHERE task_inst_id = ? AND state_id = ? AND arg_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(taskId);
		query.addBindValue(stateId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_ARGUMENT) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}
/////T_TRANSITION/////
vector<ConnectionStmParamPtr> DatabaseManager::getTransParams(int instId) {
	//DDEBUG_V("DatabaseManager::getTransParams");
  vector<ConnectionStmParamPtr> result;

  string strStmId = toStr(instId);
  string strStmQuery = "SELECT ";
  strStmQuery += "trans_id, task_inst_id, source_id, source_index, target_id, target_index ";
  strStmQuery += "FROM T_TRANSITION ";
  strStmQuery += "WHERE task_inst_id = " + strStmId + " ORDER BY trans_id";
	QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int trans_id = stmQuery.value(0).toInt();
    int source_id = stmQuery.value(2).toInt();
    int source_index = stmQuery.value(3).toInt();
    int target_id = stmQuery.value(4).toInt();
		int target_index = stmQuery.value(5).toInt();
    //
		ConnectionStmParamPtr param = std::make_shared<ConnectionStmParam>(trans_id, 0, source_id, source_index, target_id, target_index);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveTransitionStmData(int parentId, const vector<ConnectionStmParamPtr>& source) {
	DDEBUG("DatabaseManager::saveTransitionStmData");

	string strQuery = "DELETE FROM T_TRANSITION WHERE task_inst_id = ?";
	QSqlQuery query(QString::fromStdString(strQuery));
	query.addBindValue(parentId);
	if (!query.exec()) {
	  errorStr_ = "DELETE(T_TRANSITION) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
	  return false;
	}
	//
	for (int index = 0; index < source.size(); index++) {
		ConnectionStmParamPtr param = source[index];

		string strQuery = "INSERT INTO T_TRANSITION ";
		strQuery += "(task_inst_id, trans_id, source_id, source_index, target_id, target_index) ";
		strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

		QSqlQuery queryTra(QString::fromStdString(strQuery));
		queryTra.addBindValue(parentId);
		queryTra.addBindValue(index+1);
		queryTra.addBindValue(param->getSourceId());
    queryTra.addBindValue(param->getSourceIndex());
    queryTra.addBindValue(param->getTargetId());
		queryTra.addBindValue(param->getTargetIndex());

		if (!queryTra.exec()) {
		  errorStr_ = "INSERT(T_TRANSITION) error:" + queryTra.lastError().databaseText();
		  return false;
		}
		param->setNormal();
	}
  return true;
}
/////T_TASK_INST_PARAMETER/////
vector<ParameterParamPtr> DatabaseManager::getParameterParams(int instId) {
	//DDEBUG_V("DatabaseManager::getParameterParams: %d", instId);
	vector<ParameterParamPtr> result;

  string strInstId = toStr(instId);
  string strInstQuery = "SELECT ";
  strInstQuery += "task_inst_id, task_param_id, type, elem_num, ";
  strInstQuery += "name, rname, unit, value, hide, model_id, model_param_id ";
  strInstQuery += "FROM T_TASK_INST_PARAMETER ";
  strInstQuery += "WHERE task_inst_id = " + strInstId + " ORDER BY task_param_id";
  QSqlQuery instQuery(db_);
  instQuery.exec(strInstQuery.c_str());
  while (instQuery.next()) {
    int task_inst_id = instQuery.value(0).toInt();
		int id = instQuery.value(1).toInt();
    int type = instQuery.value(2).toInt();
    int elemNum = instQuery.value(3).toInt();
    QString name = instQuery.value(4).toString();
    QString rname = instQuery.value(5).toString();
    QString unit = instQuery.value(6).toString();
    QString value = instQuery.value(7).toString();
		int hide = instQuery.value(8).toInt();
    int model_id = instQuery.value(9).toInt();
    int model_param_id = instQuery.value(10).toInt();

		ParameterParamPtr param = std::make_shared<ParameterParam>(id, type, elemNum, task_inst_id, name, rname, unit, model_id, model_param_id, hide);
    param->setDBValues(value);
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveTaskParameter(TaskModelParamPtr source) {
	vector<ParameterParamPtr> paramList = source->getParameterList();
	vector<ParameterParamPtr>::iterator itParam = paramList.begin();
	while (itParam != paramList.end()) {
		if (saveTaskParameterData(source->getId(), *itParam) == false) {
			db_.rollback();
			return false;
		}
		++itParam;
	}

	return true;
}

bool DatabaseManager::saveTaskParameterData(int taskId, ParameterParamPtr source) {
  //DDEBUG_V("saveTaskParameterData id=%d, taskInstId=%d, mode=%d", source->getId(), taskId, source->getMode())

  if (source->getMode() == DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(task_param_id) FROM T_TASK_INST_PARAMETER WHERE task_inst_id = " + toStr(taskId);
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    source->setParentId(taskId);
    //
    string strQuery = "INSERT INTO T_TASK_INST_PARAMETER ";
    strQuery += "(task_inst_id, task_param_id, type, elem_num, name, rname, unit, value, model_id, model_param_id, hide) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(taskId);
		query.addBindValue(maxId);
    query.addBindValue(source->getType());
    query.addBindValue(source->getElemNum());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getUnit());
    query.addBindValue(source->getDBValues());
    query.addBindValue(source->getModelId());
    query.addBindValue(source->getModelParamId());
    query.addBindValue(source->getHide());
		if (!query.exec()) {
      errorStr_ = "INSERT(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_TASK_INST_PARAMETER ";
    strQuery += "SET type = ?, elem_num = ?, name = ?, ";
    strQuery += "rname = ?, unit = ?, value = ?, model_id = ?, model_param_id = ?, hide = ? ";
    strQuery += "WHERE task_inst_id = ? AND task_param_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getElemNum());
    query.addBindValue(source->getName());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getUnit());
    query.addBindValue(source->getDBValues());
		query.addBindValue(source->getModelId());
    query.addBindValue(source->getModelParamId());
    query.addBindValue(source->getHide());
    query.addBindValue(source->getParentId());
		query.addBindValue(source->getId());
		if (!query.exec()) {
      errorStr_ = "UPDATE(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_TASK_INST_PARAMETER ";
    strQuery += "WHERE task_inst_id = ? AND task_param_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getParentId());
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_TASK_INST_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    //
    source->setIgnore();
  }
  return true;
}

/////T_MODEL_INFO/////
vector<ModelParamPtr> DatabaseManager::getModelParams(int id) {
  vector<ModelParamPtr> result;

  string strId = toStr(id);
  string strQuery = "SELECT ";
  strQuery += "model_id, model_master_id, model_type, rname, ";
  strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z ";
  strQuery += "FROM T_MODEL_INFO WHERE task_inst_id = " + strId + " ORDER BY model_id";

  QSqlQuery query(db_);
  query.exec(strQuery.c_str());
  while (query.next()) {
    int model_id = query.value(0).toInt();
		int master_id = query.value(1).toInt();
		int model_type = query.value(2).toInt();
    QString rname = query.value(3).toString();
    double posX = query.value(4).toDouble();
    double posY = query.value(5).toDouble();
    double posZ = query.value(6).toDouble();
    double rotX = query.value(7).toDouble();
    double rotY = query.value(8).toDouble();
    double rotZ = query.value(9).toDouble();
    //
		ModelParamPtr param = std::make_shared<ModelParam>(model_id, master_id, model_type, rname, posX, posY, posZ, rotX, rotY, rotZ, false);
		ModelMasterParamPtr master = TeachingDataHolder::instance()->getModelMasterById(master_id);
		if (!master) {
			DDEBUG_V("Master NOT Exists: %d", master_id);
		}
		param->setModelMaster(master);

    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveModelData(int parentId, ModelParamPtr source) {
  DDEBUG_V("updateModelData : %d, Mode=%d", parentId, source->getMode());
  if (source->getMode() == DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(model_id) FROM T_MODEL_INFO WHERE task_inst_id = " + toStr(parentId);
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
    strQuery += "(task_inst_id, model_id, model_master_id, model_type, rname, ";
    strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ";
    strQuery += "?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(maxId);
		query.addBindValue(source->getMasterId());
		query.addBindValue(source->getType());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getPosZ());
    query.addBindValue(source->getRotRx());
    query.addBindValue(source->getRotRy());
    query.addBindValue(source->getRotRz());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_MODEL_INFO) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_MODEL_INFO ";
    strQuery += "SET model_master_id = ?, model_type = ?, rname = ?, ";
    strQuery += "pos_x = ?, pos_y = ?, pos_z = ?, rot_x = ?, rot_y = ?, rot_z = ? ";
    strQuery += "WHERE task_inst_id = ? AND model_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(source->getMasterId());
		query.addBindValue(source->getType());
    query.addBindValue(source->getRName());
    query.addBindValue(source->getPosX());
    query.addBindValue(source->getPosY());
    query.addBindValue(source->getPosZ());
    query.addBindValue(source->getRotRx());
    query.addBindValue(source->getRotRy());
    query.addBindValue(source->getRotRz());
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "UPDATE(T_MODEL_INFO) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    DDEBUG_V("deleteModelData : %d", source->getId());
    string strQuery = "DELETE FROM T_MODEL_INFO ";
		strQuery += "WHERE task_inst_id = ? AND model_id = ? ";
		
    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_MODEL_INFO) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  //
  return true;
}

/////M_MODEL/////
vector<ModelMasterParamPtr> DatabaseManager::getModelMasterList() {
	vector<ModelMasterParamPtr> result;

	string strQuery = "SELECT ";
	strQuery += "model_id, name, file_name, model_data, hash, image_file_name, image_data ";
	strQuery += "FROM M_MODEL ORDER BY model_id";

	QSqlQuery query(db_);
	query.exec(strQuery.c_str());
	while (query.next()) {
		int model_id = query.value(0).toInt();
		QString name = query.value(1).toString();
		QString fileName = query.value(2).toString();

		ModelMasterParamPtr param = std::make_shared<ModelMasterParam>(model_id, name, fileName);
		param->setData(query.value(3).toByteArray());
    param->setHash(query.value(4).toString());
    param->setImageFileName(query.value(5).toByteArray());
    param->setRawData(query.value(6).toByteArray());
    param->loadData();
    param->setNormal();
    result.push_back(param);
		//
		{
			string strSubQuery = "SELECT ";
			strSubQuery += "model_detail_id, file_name, model_data ";
			strSubQuery += "FROM M_MODEL_DETAIL WHERE model_id = " + toStr(model_id) + " ORDER BY model_detail_id";
			QSqlQuery subQuery(db_);
			subQuery.exec(strSubQuery.c_str());
			while (subQuery.next()) {
				int detail_id = subQuery.value(0).toInt();
				QString detailName = subQuery.value(1).toString();
				ModelDetailParamPtr detailParam = std::make_shared<ModelDetailParam>(detail_id, detailName);
				detailParam->setData(subQuery.value(2).toByteArray());
				param->addModelDetail(detailParam);
			}
		}
		//
		{
			string strSubQuery = "SELECT ";
			strSubQuery += "model_param_id, name, value ";
			strSubQuery += "FROM M_MODEL_PARAMETER WHERE model_id = " + toStr(model_id) + " ORDER BY model_param_id";
			QSqlQuery subQuery(db_);
			subQuery.exec(strSubQuery.c_str());
			while (subQuery.next()) {
				int param_id = subQuery.value(0).toInt();
				QString parmName = subQuery.value(1).toString();
				QString parmDesc = subQuery.value(2).toString();
				ModelParameterParamPtr modelParam = std::make_shared<ModelParameterParam>(model_id, param_id, parmName, parmDesc);
				param->addModelParameter(modelParam);
			}
		}
	}

	DDEBUG_V("getModelMasterList : %d", result.size());
	return result;
}

int DatabaseManager::checkModelMaster(QString target) {
  DDEBUG_V("DatabaseManager::checkModelMaster : %s", target.toStdString().c_str());
  int result = -1;
  string strQuery = "SELECT ";
  strQuery += "model_id ";
  strQuery += "FROM M_MODEL ";
  strQuery += "WHERE hash = '" + target.toStdString() + "'";

  QSqlQuery query(db_);
  query.exec(strQuery.c_str());
  if (query.next()) {
    result = query.value(0).toInt();
  }
  return result;
}

bool DatabaseManager::saveModelMasterList(vector<ModelMasterParamPtr> target) {
	for (int index = 0; index < target.size(); index++) {
		ModelMasterParamPtr source = target[index];
		source->setOrgId(source->getId());
		DDEBUG_V("saveModelMasterList : %d, Mode=%d", source->getId(), source->getMode());

		if (source->getMode() == DB_MODE_INSERT) {
      DDEBUG_V("saveModelMasterList INSERT %d", index);
			string strMaxQuery = "SELECT max(model_id) FROM M_MODEL";
			QSqlQuery maxQuery(db_);
			maxQuery.exec(strMaxQuery.c_str());
			int maxId = -1;
			if (maxQuery.next()) {
				maxId = maxQuery.value(0).toInt();
				maxId++;
			}
			source->setId(maxId);
			//
			string strQuery = "INSERT INTO M_MODEL ";
			strQuery += "(model_id, name, file_name, model_data, hash, image_file_name, image_data) ";
			strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

			QSqlQuery query(QString::fromStdString(strQuery));
			query.addBindValue(maxId);
			query.addBindValue(source->getName());
			query.addBindValue(source->getFileName());
			query.addBindValue(source->getData());
      query.addBindValue(source->getHash());
      query.addBindValue(source->getImageFileName());
      query.addBindValue(image2DB(source->getImageFileName(), source->getImage()));
      if (!query.exec()) {
				errorStr_ = "INSERT(M_MODEL) error:" + query.lastError().databaseText();
				DDEBUG_V("INSERT Err : %s", errorStr_.toStdString().c_str());
				return false;
			}
			source->setNormal();

		}	else if (source->getMode() == DB_MODE_UPDATE) {
      DDEBUG_V("saveModelMasterList UPDATE %d", index);
      string strQuery = "UPDATE M_MODEL ";
			strQuery += "SET name = ?, file_name = ?, model_data = ?, hash = ?, image_file_name = ?, image_data = ? ";
			strQuery += "WHERE model_id = ? ";

			QSqlQuery query(QString::fromStdString(strQuery));
			query.addBindValue(source->getName());
			query.addBindValue(source->getFileName());
			query.addBindValue(source->getData());
      query.addBindValue(source->getHash());
      query.addBindValue(source->getImageFileName());
      query.addBindValue(image2DB(source->getImageFileName(), source->getImage()));
      query.addBindValue(source->getId());

			if (!query.exec()) {
				errorStr_ = "UPDATE(M_MODEL) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
				return false;
			}
			source->setNormal();

		}	else if (source->getMode() == DB_MODE_DELETE) {
      DDEBUG_V("saveModelMasterList DELETE %d", index);
      string strQuery = "DELETE FROM M_MODEL WHERE model_id = ? ";
			QSqlQuery query(QString::fromStdString(strQuery));
			query.addBindValue(source->getId());

			if (!query.exec()) {
				errorStr_ = "DELETE(M_MODEL) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
				return false;
			}
			source->setIgnore();
		}
		//
		vector<ModelDetailParamPtr> detailList = source->getModelDetailList();
		DDEBUG_V("detailList: %d", detailList.size());
		if (0 < detailList.size()) {
			for (int idxDet = 0; idxDet < detailList.size(); idxDet++) {
				ModelDetailParamPtr detail = detailList[idxDet];
				if (saveModelDetailData(source->getId(), detail) == false) {
					return false;
				}
			}
		}
		//
		vector<ModelParameterParamPtr> paramList = source->getModelParameterList();
		if (0 < paramList.size()) {
			for (int idxDet = 0; idxDet < paramList.size(); idxDet++) {
				ModelParameterParamPtr detail = paramList[idxDet];
				if (saveModelParameter(source->getId(), detail) == false) {
					return false;
				}
			}
		}
	}
	//
	return true;
}
/////M_MODEL_DETAIL/////
bool DatabaseManager::saveModelDetailData(int modelId, ModelDetailParamPtr source) {
  DDEBUG_V("saveModelDetailData : %d, %d", modelId, source->getMode());
  if (source->getMode() == DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(model_detail_id) FROM M_MODEL_DETAIL WHERE model_id = " + toStr(modelId);
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    source->setId(maxId);
    //
    string strQuery = "INSERT INTO M_MODEL_DETAIL ";
    strQuery += "(model_id, model_detail_id, file_name, model_data) ";
    strQuery += "VALUES ( ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(modelId);
		query.addBindValue(maxId);
		query.addBindValue(source->getFileName());
    query.addBindValue(source->getData());
    if (!query.exec()) {
      errorStr_ = "INSERT(M_MODEL_DETAIL) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM M_MODEL_DETAIL ";
    strQuery += "WHERE model_id = ? AND model_detail_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(modelId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(M_MODEL_DETAIL) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}

/////M_MODEL_PARAMETER/////
bool DatabaseManager::saveModelParameter(int modelId, ModelParameterParamPtr source) {
	DDEBUG_V("saveModelParameter : %d, %d", modelId, source->getMode());
	if (source->getMode() == DB_MODE_INSERT) {
		string strMaxQuery = "SELECT max(model_param_id) FROM M_MODEL_PARAMETER WHERE model_id = " + toStr(modelId);
		QSqlQuery maxQuery(db_);
		maxQuery.exec(strMaxQuery.c_str());
		int maxId = -1;
		if (maxQuery.next()) {
			maxId = maxQuery.value(0).toInt();
			maxId++;
		}
		source->setId(maxId);
		//
		string strQuery = "INSERT INTO M_MODEL_PARAMETER ";
		strQuery += "(model_id, model_param_id, name, value) ";
		strQuery += "VALUES ( ?, ?, ?, ? )";

		QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(modelId);
		query.addBindValue(maxId);
		query.addBindValue(source->getName());
		query.addBindValue(source->getValueDesc());
		if (!query.exec()) {
			errorStr_ = "INSERT(M_MODEL_PARAMETER) error:" + query.lastError().databaseText();
			return false;
		}
		source->setNormal();

	} else if (source->getMode() == DB_MODE_UPDATE) {
		string strQuery = "UPDATE M_MODEL_PARAMETER ";
		strQuery += "SET name = ?, value = ? ";
		strQuery += "WHERE model_id = ? AND model_param_id = ?";

		QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(source->getName());
		query.addBindValue(source->getValueDesc());
		query.addBindValue(source->getMasterId());
		query.addBindValue(source->getId());

		if (!query.exec()) {
			errorStr_ = "UPDATE(M_MODEL_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
			return false;
		}
		source->setNormal();

	} else if (source->getMode() == DB_MODE_DELETE) {
		string strQuery = "DELETE FROM M_MODEL_PARAMETER ";
		strQuery += "WHERE model_id = ? AND model_param_id = ? ";

		QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(modelId);
		query.addBindValue(source->getId());

		if (!query.exec()) {
			errorStr_ = "DELETE(M_MODEL_PARAMETER) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
			return false;
		}
		source->setIgnore();
	}
	return true;
}

/////T_FIGURE/////
vector<ImageDataParamPtr> DatabaseManager::getFigureParams(int id) {
  vector<ImageDataParamPtr> result;

  string strQuery = "SELECT ";
  strQuery += "figure_id, seq, name, data ";
  strQuery += "FROM T_FIGURE WHERE task_inst_id = " + toStr(id) + " " + "ORDER BY seq";

  QSqlQuery query(db_);
  if (query.exec(strQuery.c_str()) == false) {
    DDEBUG_V("error: %s", query.lastError().databaseText().toUtf8().constData());
  }
  while (query.next()) {
    int id = query.value(0).toInt();
		int seq = query.value(1).toInt();
		QString name = query.value(2).toString();
    //
		ImageDataParamPtr param = std::make_shared<ImageDataParam>(id, seq, name);
    param->setRawData(query.value(3).toByteArray());
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveImageData(int parentId, ImageDataParamPtr source) {

  if (source->getMode() == DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(figure_id) FROM T_FIGURE WHERE task_inst_id = " + toStr(parentId);
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_FIGURE ";
    strQuery += "(task_inst_id, figure_id, seq, name, data) VALUES ( ?, ?, ?, ?, ? )";

    QString strName = source->getName();

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(maxId);
		query.addBindValue(source->getSeq());
		query.addBindValue(strName);
    query.addBindValue(image2DB(strName, source->getData()));
    if (!query.exec()) {
      errorStr_ = "INSERT(T_FIGURE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

	}
	else if (source->getMode() == DB_MODE_UPDATE) {
		string strQuery = "UPDATE T_FIGURE ";
		strQuery += "SET seq = ? ";
		strQuery += "WHERE task_inst_id = ? AND figure_id = ?";

		QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(source->getSeq());
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

		if (!query.exec()) {
			errorStr_ = "UPDATE(T_FIGURE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
			return false;
		}
		source->setNormal();

	} else if (source->getMode() == DB_MODE_DELETE) {
		DDEBUG_V("DatabaseManager::saveImageData DELETE : %d, %d", parentId, source->getId());

    string strQuery = "DELETE FROM T_FIGURE WHERE task_inst_id = ? AND figure_id = ?";
    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

    if (!query.exec()) {
      errorStr_ = "DELETE(T_FIGURE) error:" + query.lastError().databaseText() + "=" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}

/////T_FILE/////
vector<FileDataParamPtr> DatabaseManager::getFileParams(int id) {
	vector<FileDataParamPtr> result;

  string strQuery = "SELECT ";
  strQuery += "file_id, seq, name, file_data ";
  strQuery += "FROM T_FILE WHERE task_inst_id = " + toStr(id) + " " + "ORDER BY seq";

  QSqlQuery query(db_);
  if (query.exec(strQuery.c_str()) == false) {
    DDEBUG_V("error: %s", query.lastError().databaseText().toUtf8().constData());
  }
  while (query.next()) {
    int id = query.value(0).toInt();
		int seq = query.value(1).toInt();
		QString name = query.value(2).toString();
		//
		FileDataParamPtr param = std::make_shared<FileDataParam>(id, seq, name);
    param->setData(query.value(3).toByteArray());
    result.push_back(param);
  }
  return result;
}

bool DatabaseManager::saveFileData(int parentId, FileDataParamPtr source) {
  if (source->getMode() == DB_MODE_INSERT) {
    string strMaxQuery = "SELECT max(file_id) FROM T_FILE WHERE task_inst_id = " + toStr(parentId);
    QSqlQuery maxQuery(db_);
    maxQuery.exec(strMaxQuery.c_str());
    int maxId = -1;
    if (maxQuery.next()) {
      maxId = maxQuery.value(0).toInt();
      maxId++;
    }
    //
    string strQuery = "INSERT INTO T_FILE ";
    strQuery += "(task_inst_id, file_id, seq, name, file_data) VALUES ( ?, ?, ?, ?, ?)";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(maxId);
		query.addBindValue(source->getSeq());
		query.addBindValue(source->getName());
    query.addBindValue(source->getData());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_FILE) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

	}	else if (source->getMode() == DB_MODE_UPDATE) {
		string strQuery = "UPDATE T_FILE ";
		strQuery += "SET seq = ? ";
		strQuery += "WHERE task_inst_id = ? AND file_id = ?";

		QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(source->getSeq());
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

		if (!query.exec()) {
			errorStr_ = "UPDATE(T_FILE) error:" + query.lastError().databaseText() + "-" + QString::fromStdString(strQuery);
			return false;
		}
		source->setNormal();

	} else if (source->getMode() == DB_MODE_DELETE) {
    string strQuery = "DELETE FROM T_FILE WHERE task_inst_id = ? AND file_id = ?";
    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(parentId);
		query.addBindValue(source->getId());

		if (!query.exec()) {
      errorStr_ = "DELETE(T_FILE) error:" + query.lastError().databaseText() + "=" + QString::fromStdString(strQuery);
      return false;
    }
    source->setIgnore();
  }
  return true;
}

void DatabaseManager::reNewModelMaster(ModelMasterParamPtr target) {
  //対象データの存在チェック
  {
    string strQuery = "SELECT name FROM M_MODEL WHERE model_id = '" + toStr(target->getId()) + "'";
    QSqlQuery query(db_);
    if (query.exec(strQuery.c_str()) == false) return;
    if (!query.next()) return;
  }
  //同一ハッシュデータの取得
  {
    vector<int> idList;
    string strQuery = "SELECT model_id FROM M_MODEL WHERE hash = '" + target->getHash().toStdString() + "'";
    QSqlQuery query(db_);
    if (query.exec(strQuery.c_str()) == false) return;
    while (query.next()) {
      int id = query.value(0).toInt();
      idList.push_back(id);
    }
    if (idList.size() <= 1) return;

    int baseId = idList[0];
    for (int index = 1; index < idList.size(); index++) {
      int targetId = idList[index];
      db_.rollback();
      {
        string strQuery = "UPDATE T_MODEL_INFO ";
        strQuery += "SET model_master_id = ? ";
        strQuery += "WHERE model_master_id = ?";
        QSqlQuery query(QString::fromStdString(strQuery));
        query.addBindValue(baseId);
        query.addBindValue(targetId);
        query.exec();
      }
      {
        string strQuery = "DELETE FROM M_MODEL_PARAMETER WHERE model_id = ?";
        QSqlQuery query(QString::fromStdString(strQuery));
        query.addBindValue(targetId);
        query.exec();
      }
      {
        string strQuery = "DELETE FROM M_MODEL_DETAIL WHERE model_id = ?";
        QSqlQuery query(QString::fromStdString(strQuery));
        query.addBindValue(targetId);
        query.exec();
      }
      {
        string strQuery = "DELETE FROM M_MODEL WHERE model_id = ?";
        QSqlQuery query(QString::fromStdString(strQuery));
        query.addBindValue(targetId);
        query.exec();
      }
      db_.commit();
    }
  }
}

}
