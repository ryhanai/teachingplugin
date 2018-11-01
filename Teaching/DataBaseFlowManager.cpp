#include "DataBaseManager.h"

#include <qsqlerror.h>
#include "TeachingUtil.h"
#include "TeachingDataHolder.h"
#include "TaskExecutor.h"

#include "LoggerUtil.h"

using namespace std;
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
    DDEBUG_V("saveFlowStmData : INSERT FlowId=%d, StateId=%d", parentId, source->getId());

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

    TaskModelParamPtr task = source->getTaskParam();
    if (task) {
      deleteTaskModel(task->getId());
    }
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
  strStmQuery += "flow_id, model_id, master_id, pos_x, pos_y, name ";
  strStmQuery += "FROM T_FLOW_MODEL_PARAM ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY model_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int model_id = stmQuery.value(1).toInt();
    int master_id = stmQuery.value(2).toInt();
    double pos_x = stmQuery.value(3).toDouble();
    double pos_y = stmQuery.value(4).toDouble();
    QString name = stmQuery.value(5).toString();
    //
    FlowModelParamPtr param = std::make_shared<FlowModelParam>(model_id, master_id, name);
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
    strQuery += "(flow_id, model_id, master_id, pos_x, pos_y, name) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ? )";

    QSqlQuery queryTra(QString::fromStdString(strQuery));
    queryTra.addBindValue(parentId);
    queryTra.addBindValue(index + 1);
    queryTra.addBindValue(param->getMasterId());
    queryTra.addBindValue(param->getPosX());
    queryTra.addBindValue(param->getPosY());
    queryTra.addBindValue(param->getName());

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
  strStmQuery += "flow_id, param_id, param_type, name, value, pos_x, pos_y ";
  strStmQuery += "FROM T_FLOW_PARAMETER ";
  strStmQuery += "WHERE flow_id = " + strStmId + " ORDER BY param_id";
  QSqlQuery stmQuery(db_);
  stmQuery.exec(strStmQuery.c_str());
  while (stmQuery.next()) {
    int param_id = stmQuery.value(1).toInt();
    int type = stmQuery.value(2).toInt();
    QString name = stmQuery.value(3).toString();
    QString vallue = stmQuery.value(4).toString();
    double pos_x = stmQuery.value(5).toDouble();
    double pos_y = stmQuery.value(6).toDouble();
    //
    FlowParameterParamPtr param = std::make_shared<FlowParameterParam>(param_id, type, name, vallue);
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
    strQuery += "(flow_id, param_id, param_type, name, value, pos_x, pos_y) ";
    strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ? )";

    QSqlQuery queryTra(QString::fromStdString(strQuery));
    queryTra.addBindValue(parentId);
    queryTra.addBindValue(param->getId());
    queryTra.addBindValue(param->getType());
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

}
