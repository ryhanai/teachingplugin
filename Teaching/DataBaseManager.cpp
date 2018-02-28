#include "DataBaseManager.h"
#include <QVariant>
#include <QBuffer>
#include <qsqlerror.h>
#include <cnoid/UTF8>

#include "TeachingUtil.h"

#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

DatabaseManager::DatabaseManager() {
}

bool DatabaseManager::connectDB() {
  DDEBUG("DatabaseManager::connectDB");
	db_ = QSqlDatabase::addDatabase("QSQLITE");
	db_.setDatabaseName(QString::fromStdString(SettingManager::getInstance().getDatabase()));
	if (db_.open() == false) {
		return false;
  }
	return true;
}

bool DatabaseManager::reConnectDB() {
  closeDB();
  //
  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(QString::fromStdString(SettingManager::getInstance().getDatabase()));
  if (db_.open() == false) {
    return false;
  }
  return true;
}

void DatabaseManager::closeDB() {
  DDEBUG("DatabaseManager::closeDB");
  if (db_.isOpen()) {
    db_.close();
  }
}
/////
vector<FlowParamPtr> DatabaseManager::searchFlowList(vector<string>& condList, bool isOr) {
	vector<FlowParamPtr> result;
	
  string strQuery = "SELECT ";
  strQuery += "flow_id, name, comment, created_date, last_updated_date ";
  strQuery += "FROM T_FLOW ";
  string strQueryCond = "WHERE ( ";
  for (unsigned int index = 0; index < condList.size(); index++) {
    if (0 < index) {
      if (isOr) strQueryCond += " Or ";
      else     strQueryCond += " And ";
    }
    strQueryCond += "((name || ' ' || comment) like \"%" + condList[index] + "%\")";
  }
  if (0 < condList.size()) {
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
		FlowParamPtr param = std::make_shared<FlowParam>(id, name, comment, createdDate, updatedDate);
		result.push_back(param);
  }
  return result;
}

FlowParamPtr DatabaseManager::getFlowParamById(const int id) {
  DDEBUG_V("getFlowParamById : %d", id);

  string strQuery = "SELECT ";
  strQuery += "name, comment, created_date, last_updated_date ";
  strQuery += "FROM T_FLOW ";
  strQuery += "WHERE flow_id = " + toStr(id);
  QSqlQuery flowQuery(db_);
  flowQuery.exec(strQuery.c_str());
  if (flowQuery.next() == false) return false;
  //
  QString name = flowQuery.value(0).toString();
  QString comment = flowQuery.value(1).toString();
  QString createdDate = flowQuery.value(2).toString();
  QString updatedDate = flowQuery.value(3).toString();
  //
	FlowParamPtr result = std::make_shared<FlowParam>(id, name, comment, createdDate, updatedDate);
  result->setName(name);
  result->setComment(comment);
  result->setCreatedDate(createdDate);
  result->setLastUpdatedDate(updatedDate);
  //
  try {
    vector<ElementStmParamPtr> stateList = getFlowStateParams(id);
    std::vector<ElementStmParamPtr>::iterator itState = stateList.begin();
    while (itState != stateList.end()) {
      result->addStmElement(*itState);
      ++itState;
    }
    vector<ConnectionStmParamPtr> transList = getFlowTransParams(id);
    std::vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
    while (itTrans != transList.end()) {
      result->addStmConnection(*itTrans);
      ++itTrans;
    }
    vector<FlowModelParamPtr> modelList = getFlowModelParams(id);
    std::vector<FlowModelParamPtr>::iterator itModel = modelList.begin();
    while (itModel != modelList.end()) {
      result->addModel(*itModel);
      ++itModel;
    }
    vector<FlowParameterParamPtr> paramlList = getFlowParamerer(id);
    std::vector<FlowParameterParamPtr>::iterator itParam = paramlList.begin();
    while (itParam != paramlList.end()) {
      result->addFlowParam(*itParam);
      ++itParam;
    }
  } catch (const std::exception& ex) {
    return NULL;
  }

  return result;
}

vector<TaskModelParamPtr> DatabaseManager::getAllTask() {
	taskModelList_.clear();

	string strQuery = "SELECT ";
	strQuery += "task_inst_id, name, comment, flow_id, created_date, last_updated_date, exec_env ";
	strQuery += "FROM T_TASK_MODEL_INST ";
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
		QString execEnv = taskQuery.value(6).toString();
		//
		TaskModelParamPtr param = std::make_shared<TaskModelParam>(id, name, comment, execEnv, flow_id, createdDate, updatedDate);
		taskModelList_.push_back(param);
		getDetailParams(param);
	}
	return taskModelList_;
}

vector<TaskModelParamPtr> DatabaseManager::searchTaskModels(vector<string>& condList, bool isOr) {
	DDEBUG("DatabaseManager::searchTaskModels");

	taskModelList_.clear();

  string strQuery = "SELECT ";
  strQuery += "task_inst_id, name, comment, flow_id, created_date, last_updated_date, exec_env ";
  strQuery += "FROM T_TASK_MODEL_INST ";
  strQuery += "WHERE flow_id < 0 ";
  string strQueryCond = " AND ( ";
	for (unsigned int index = 0; index < condList.size(); index++) {
    if (0 < index) {
      if (isOr) strQueryCond += " Or ";
      else     strQueryCond += " And ";
    }
    strQueryCond += "((name || ' ' || comment) like \"%" + condList[index] + "%\")";
  }
  if (0 < condList.size()) {
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
    QString execEnv = taskQuery.value(6).toString();
    //
		TaskModelParamPtr param = std::make_shared<TaskModelParam>(id, name, comment, execEnv, flow_id, createdDate, updatedDate);
    taskModelList_.push_back(param);
    getDetailParams(param);
  }
	return taskModelList_;
}

TaskModelParamPtr DatabaseManager::getTaskModelById(const int taskId) {
	//DDEBUG_V("DatabaseManager::getTaskModelById %d", taskId);

	TaskModelParamPtr result = NULL;

  string strQuery = "SELECT ";
  strQuery += "task_inst_id, name, comment, flow_id, created_date, last_updated_date, exec_env ";
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
    QString execEnv = taskQuery.value(6).toString();
    //
    result.reset(new TaskModelParam(id, name, comment, execEnv, flow_id, createdDate, updatedDate));
    taskModelList_.push_back(result);
    getDetailParams(result);
  }
  return result;
}

void DatabaseManager::getDetailParams(TaskModelParamPtr target) {
	//DDEBUG("DatabaseManager::getDetailParams");

  vector<ModelParamPtr> modelList = getModelParams(target->getId());
  std::vector<ModelParamPtr>::iterator itModel = modelList.begin();
  while (itModel != modelList.end()) {
    target->addModel(*itModel);
    ++itModel;
  }
  //
  vector<ParameterParamPtr> paramList = getParameterParams(target->getId());
  std::vector<ParameterParamPtr>::iterator itParam = paramList.begin();
  while (itParam != paramList.end()) {
    target->addParameter(*itParam);
    ++itParam;
  }
  //
  vector<ElementStmParamPtr> stateList = getStateParams(target->getId());
  std::vector<ElementStmParamPtr>::iterator itState = stateList.begin();
  while (itState != stateList.end()) {
    target->addStmElement(*itState);
    ++itState;
  }
  //
  vector<ConnectionStmParamPtr> transList = getTransParams(target->getId());
  std::vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
  while (itTrans != transList.end()) {
    target->addStmConnection(*itTrans);
    ++itTrans;
  }
  //
  vector<ImageDataParamPtr> imageList = getFigureParams(target->getId());
  std::vector<ImageDataParamPtr>::iterator itImage = imageList.begin();
  while (itImage != imageList.end()) {
    target->addImage(*itImage);
    ++itImage;
  }
  //
  vector<FileDataParamPtr> fileList = getFileParams(target->getId());
  std::vector<FileDataParamPtr>::iterator itFile = fileList.begin();
  while (itFile != fileList.end()) {
    target->addFile(*itFile);
    ++itFile;
  }
}

TaskModelParamPtr DatabaseManager::getTaskModel(int index) {
  if (index < 0 || taskModelList_.size() <= index) return 0;
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
//////////
bool DatabaseManager::saveFlowModel(FlowParamPtr source) {
	DDEBUG("DatabaseManager::saveFlowModel");
	db_.transaction();
  if (saveFlowData(source) == false) {
    db_.rollback();
    return false;
  }
  //
  vector<ElementStmParamPtr> stateList = source->getStmElementList();
  vector<ElementStmParamPtr>::iterator itState = stateList.begin();
  while (itState != stateList.end()) {
    DDEBUG_V("saveFlowModel type=%d", (*itState)->getType());
    if ((*itState)->getType() == ELEMENT_COMMAND) {
			TaskModelParamPtr param = (*itState)->getTaskParam();
      param->setFlowId(source->getId());
      if (saveTaskModel(param) == false) {
        db_.rollback();
        return false;
      }
      (*itState)->setTaskParam(param);
    }
    if (saveFlowStmData(source->getId(), *itState) == false) {
      db_.rollback();
      return false;
    }
    ++itState;
  }
  //
  vector<ConnectionStmParamPtr> transList = source->getStmConnectionList();
	if (saveFlowTransactionStmData(source->getId(), transList) == false) {
		db_.rollback();
		return false;
	}
  vector<FlowModelParamPtr> modelParamList = source->getModelList();
  if (saveFlowModelParam(source->getId(), modelParamList) == false) {
    db_.rollback();
    return false;
  }
  vector<FlowParameterParamPtr> flowParamList = source->getFlowParamList();
  if (saveFlowParameter(source->getId(), flowParamList) == false) {
    db_.rollback();
    return false;
  }
  //
  db_.commit();
  return true;
}

bool DatabaseManager::saveImportedTaskModel(vector<TaskModelParamPtr>& source, vector<ModelMasterParamPtr>& modelMasterList) {
  DDEBUG("DatabaseManager::saveImportedTaskModel");
  errorStr_ = "";

  db_.transaction();
  try {
		if (saveModelMasterList(modelMasterList) == false) {
			db_.rollback();
			return false;
		}
		DDEBUG("M_MODEL Saved");
		/////
    vector<TaskModelParamPtr>::iterator itTask = source.begin();
    while (itTask != source.end()) {
      if (saveTaskInstanceData(*itTask, true) == false) {
        db_.rollback();
        return false;
      }
			if (saveDetailData(*itTask) == false) {
				db_.rollback();
				return false;
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

bool DatabaseManager::saveTaskModel(TaskModelParamPtr source) {
  DDEBUG("saveTaskModel");
  errorStr_ = "";

  db_.transaction();
  if (saveTaskInstanceData(source, true) == false) {
    db_.rollback();
    return false;
  }

  if (saveDetailData(source) == false) {
    db_.rollback();
    return false;
  }
  db_.commit();
  //
  return true;
}
/////
bool DatabaseManager::saveDetailData(TaskModelParamPtr source) {
  vector<ModelParamPtr> modelList = source->getModelList();
  vector<ModelParamPtr>::iterator itModel = modelList.begin();
  while (itModel != modelList.end()) {
    if (saveModelData(source->getId(), *itModel) == false) {
      db_.rollback();
      return false;
    }
    ++itModel;
  }
  //
  vector<ImageDataParamPtr> imageList = source->getImageList();
  vector<ImageDataParamPtr>::iterator itImage = imageList.begin();
  while (itImage != imageList.end()) {
    if (saveImageData(source->getId(), *itImage) == false) {
      db_.rollback();
      return false;
    }
    ++itImage;
  }
  //
  vector<FileDataParamPtr> fileList = source->getFileList();
  vector<FileDataParamPtr>::iterator itFile = fileList.begin();
  while (itFile != fileList.end()) {
    if (saveFileData(source->getId(), *itFile) == false) {
      db_.rollback();
      return false;
    }
    ++itFile;
  }
  //
  vector<ParameterParamPtr> paramList = source->getParameterList();
  vector<ParameterParamPtr>::iterator itParam = paramList.begin();
  while (itParam != paramList.end()) {
    if (saveTaskParameterData(source->getId(), *itParam) == false) {
      db_.rollback();
      return false;
    }
    ++itParam;
  }
  //
  vector<ElementStmParamPtr> stateList = source->getStmElementList();
  vector<ElementStmParamPtr>::iterator itState = stateList.begin();
  while (itState != stateList.end()) {
    if (saveElementStmData(source->getId(), *itState) == false) {
      db_.rollback();
      return false;
    }
    ++itState;
  }
	//
	if (saveTransitionStmData(source->getId(), source->getStmConnectionList()) == false) {
		db_.rollback();
		return false;
	}

  return true;
}

bool DatabaseManager::deleteTaskModel(int task_inst_id) {
	errorStr_ = "";
	DDEBUG_V("deleteTaskModel : %d", task_inst_id);
	db_.transaction();

	if (deleteDataById("T_TASK_MODEL_INST", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_FIGURE", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_FILE", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_TASK_INST_PARAMETER", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_STATE", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_STATE_ACTION", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_ARGUMENT", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_TRANSITION", "task_inst_id", task_inst_id) == false) return false;
	if (deleteDataById("T_MODEL_INFO", "task_inst_id", task_inst_id) == false) return false;

	db_.commit();
	return true;
}

bool DatabaseManager::deleteFlowModel(int id) {
	DDEBUG("deleteFlowModel");
	db_.transaction();

	if (deleteDataById("T_FLOW", "flow_id", id) == false) return false;
	if (deleteDataById("T_FLOW_STATE", "flow_id", id) == false) return false;
	if (deleteDataById("T_FLOW_TRANSITION", "flow_id", id) == false) return false;

	db_.commit();
	return true;
}

bool DatabaseManager::deleteDataById(QString tableName, QString strKey, int id) {
	QString strQuery = "DELETE FROM " + tableName + " WHERE " + strKey + " = ? ";
	QSqlQuery query(strQuery);
	query.addBindValue(id);
	if (!query.exec()) {
		errorStr_ = "DELETE(" + tableName + ") error:" + query.lastError().databaseText() + "-" + strQuery;
		db_.rollback();
		return false;
	}
	return true;
}
//////////
QByteArray DatabaseManager::image2DB(const QString& name, const QImage& source) {
  QByteArray result;
  QBuffer buffer(&result);
  buffer.open(QBuffer::WriteOnly);
  if (name.toUpper().endsWith("PNG")) {
    source.save(&buffer, "PNG");
  } else if (name.toUpper().endsWith("JPG") || name.toUpper().endsWith("JPEG")) {
    source.save(&buffer, "JPG");
  }
  return result;
}

}
