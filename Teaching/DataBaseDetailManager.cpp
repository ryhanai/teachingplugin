#include "DataBaseManager.h"

#include <qsqlerror.h>
#include "TeachingUtil.h"
#include "TeachingDataHolder.h"
#include "TaskExecutor.h"

#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

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
  DDEBUG_V("saveElementStmData : taskId=%d, stateId=%d", parentId, source->getId());
  if (source->getMode() == DB_MODE_INSERT) {
    DDEBUG_V("saveElementStmData : INSERT taskId=%d, stateId=%d", parentId, source->getId());
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
		ElementStmActionParamPtr param = std::make_shared<ElementStmActionParam>(id, seq, action, model, target, false);
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
		ArgumentParamPtr param = std::make_shared<ArgumentParam>(id, seq, name, value);
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
  strInstQuery += "task_inst_id, task_param_id, type, param_type, ";
  strInstQuery += "name, rname, unit, value, hide, model_id, model_param_id ";
  strInstQuery += "FROM T_TASK_INST_PARAMETER ";
  strInstQuery += "WHERE task_inst_id = " + strInstId + " ORDER BY task_param_id";
  QSqlQuery instQuery(db_);
  instQuery.exec(strInstQuery.c_str());
  while (instQuery.next()) {
    int task_inst_id = instQuery.value(0).toInt();
		int id = instQuery.value(1).toInt();
    int type = instQuery.value(2).toInt();
    int paramType = instQuery.value(3).toInt();
    QString name = instQuery.value(4).toString();
    QString rname = instQuery.value(5).toString();
    QString unit = instQuery.value(6).toString();
    QString value = instQuery.value(7).toString();
		int hide = instQuery.value(8).toInt();
    int model_id = instQuery.value(9).toInt();
    int model_param_id = instQuery.value(10).toInt();

		ParameterParamPtr param = std::make_shared<ParameterParam>(id, type, paramType, task_inst_id, name, rname, unit, model_id, model_param_id, hide);
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
    strQuery += "(task_inst_id, task_param_id, type, param_type, name, rname, unit, value, model_id, model_param_id, hide) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery query(QString::fromStdString(strQuery));
		query.addBindValue(taskId);
		query.addBindValue(maxId);
    query.addBindValue(source->getType());
    query.addBindValue(source->getParamType());
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
    strQuery += "SET type = ?, param_type = ?, name = ?, ";
    strQuery += "rname = ?, unit = ?, value = ?, model_id = ?, model_param_id = ?, hide = ? ";
    strQuery += "WHERE task_inst_id = ? AND task_param_id = ? ";

    QSqlQuery query(QString::fromStdString(strQuery));
    query.addBindValue(source->getType());
    query.addBindValue(source->getParamType());
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
  strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, hide ";
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
    int hide = query.value(10).toInt();
    //
		ModelParamPtr param = std::make_shared<ModelParam>(model_id, master_id, model_type, rname, posX, posY, posZ, rotX, rotY, rotZ, hide, false);
    ModelMasterParamPtr master = 0;
    if(master_id<0) {
      master = TeachingDataHolder::instance()->getFPMaster();
    } else {
      master = TeachingDataHolder::instance()->getModelMasterById(master_id);
      if (!master) {
        DDEBUG_V("Master NOT Exists: %d", master_id);
      }
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
    strQuery += "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, hide) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ";
    strQuery += "?, ?, ?, ?, ?, ? )";

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
    query.addBindValue(source->getHide());
    if (!query.exec()) {
      errorStr_ = "INSERT(T_MODEL_INFO) error:" + query.lastError().databaseText();
      return false;
    }
    source->setNormal();

  } else if (source->getMode() == DB_MODE_UPDATE) {
    string strQuery = "UPDATE T_MODEL_INFO ";
    strQuery += "SET model_master_id = ?, model_type = ?, rname = ?, ";
    strQuery += "pos_x = ?, pos_y = ?, pos_z = ?, rot_x = ?, rot_y = ?, rot_z = ? , hide = ? ";
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
    query.addBindValue(source->getHide());
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
//////////////////
bool DatabaseManager::checkFlowState(int taskId) {
  string strQuery = "SELECT flow_id ";
  strQuery += "FROM T_FLOW_STATE WHERE task_inst_id = " + toStr(taskId);
  QSqlQuery query(db_);
  if (query.exec(strQuery.c_str()) == false) {
    DDEBUG_V("error: %s", query.lastError().databaseText().toUtf8().constData());
  }
  if (query.next()) {
    return true;
  }
  return false;
}

}
