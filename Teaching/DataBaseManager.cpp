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
	DDEBUG_V("DatabaseManager::searchTaskModels %s", strQuery.c_str());
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
	DDEBUG_V("DatabaseManager::getTaskModelById %d", taskId);

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
  vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
  while (itTrans != transList.end()) {
    if ((*itTrans)->getMode() == DB_MODE_INSERT) {
      vector<ElementStmParamPtr>::iterator sourceElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
      if (sourceElem == stateList.end()) {
        ++itTrans;
        DDEBUG("NOT Found SourceId");
        continue;
      }
      (*itTrans)->setSourceId((*sourceElem)->getId());

      vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
      if (targetElem == stateList.end()) {
        ++itTrans;
        DDEBUG("NOT Found TargetId");
        continue;
      }
      (*itTrans)->setTargetId((*targetElem)->getId());
      DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
    }

    if (saveFlowTransactionStmData(source->getId(), *itTrans) == false) {
      db_.rollback();
      return false;
    }
    ++itTrans;
  }
  //
  itState = stateList.begin();
  while (itState != stateList.end()) {
    (*itState)->updateId();
    ++itState;
  }
  //
  db_.commit();
  return true;
}

bool DatabaseManager::saveTaskModelsForLoad(vector<TaskModelParamPtr>& source) {
	DDEBUG("saveTaskModelsForLoad");
	errorStr_ = "";

	db_.transaction();
	try {
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
			//
			vector<ElementStmParamPtr> stateList = (*itTask)->getStmElementList();
			vector<ConnectionStmParamPtr> transList = (*itTask)->getStmConnectionList();
			vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
			while (itTrans != transList.end()) {
				DDEBUG_V("Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
				if ((*itTrans)->getSourceId() == (*itTrans)->getTargetId()) continue;
				vector<ElementStmParamPtr>::iterator sourceElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
				if (sourceElem == stateList.end()) {
					++itTrans;
					DDEBUG("NOT Found SourceId");
					continue;
				}
				(*itTrans)->setSourceId((*sourceElem)->getId());

				vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
				if (targetElem == stateList.end()) {
					++itTrans;
					DDEBUG("NOT Found TargetId");
					continue;
				}
				(*itTrans)->setTargetId((*targetElem)->getId());
				DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());

				if (saveTransitionStmData((*itTask)->getId(), *itTrans) == false) {
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
			//
			int taskId = (*itTask)->getId();
			DDEBUG("T_TASK_MODEL_INST Saved");

			vector<ImageDataParamPtr>::iterator itImage = (*itTask)->getImageList().begin();
			while (itImage != (*itTask)->getImageList().end()) {
				if (saveImageData(taskId, *itImage) == false) {
					db_.rollback();
					return false;
				}
				++itImage;
			}
			DDEBUG("T_FIGURE Saved");
			//
			vector<FileDataParamPtr>::iterator itFile = (*itTask)->getFileList().begin();
			while (itFile != (*itTask)->getFileList().end()) {
				if (saveFileData(taskId, *itFile) == false) {
					db_.rollback();
					return false;
				}
				++itFile;
			}
			DDEBUG("T_FILE Saved");
			//
			for (int index = 0; index < (*itTask)->getParameterList().size(); index++) {
				ParameterParamPtr param = (*itTask)->getParameterList()[index];
				if (saveTaskParameterData(taskId, param) == false) {
					db_.rollback();
					return false;
				}
			}
			DDEBUG("T_TASK_INST_PARAMETER Saved");
			//
			DDEBUG_V("T_MODEL_INFO Saved %d", (*itTask)->getModelList().size());
			for (int index = 0; index < (*itTask)->getModelList().size(); index++) {
				DDEBUG_V("index %d", index);
				ModelParamPtr param = (*itTask)->getModelList()[index];
				bool isExist = false;
				for (int index = 0; index < modelMasterList.size(); index++) {
					ModelMasterParamPtr master = modelMasterList[index];
					DDEBUG_V("Master  Org:%d, Id:%d", master->getOrgId(), master->getId());
					if (param->getMasterId() == master->getOrgId()) {
						param->setMasterId(master->getId());
						isExist = true;
						break;
					}
				}
				if (isExist == false) {
					DDEBUG_V("Master NOT Exist %d", index);
					db_.rollback();
					return false;
				}
				if (saveModelData(taskId, param) == false) {
					db_.rollback();
					return false;
				}
			}
			DDEBUG("T_MODEL_INFO Saved");
			//
			vector<ElementStmParamPtr> stateList = (*itTask)->getStmElementList();
			vector<ElementStmParamPtr>::iterator itState = stateList.begin();
			while (itState != stateList.end()) {
				if (saveElementStmData(taskId, *itState) == false) {
					db_.rollback();
					return false;
				}
				++itState;
			}
			DDEBUG("T_STATE Saved");
			//
			vector<ConnectionStmParamPtr> transList = (*itTask)->getStmConnectionList();
			vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
      while (itTrans != transList.end()) {
        DDEBUG_V("Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
        if ((*itTrans)->getSourceId() == (*itTrans)->getTargetId()) continue;
        vector<ElementStmParamPtr>::iterator sourceElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
        if (sourceElem == stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found SourceId");
          continue;
        }
        (*itTrans)->setSourceId((*sourceElem)->getId());

        vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
        if (targetElem == stateList.end()) {
          ++itTrans;
          DDEBUG("NOT Found TargetId");
          continue;
        }
        (*itTrans)->setTargetId((*targetElem)->getId());
        DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());

        if (saveTransitionStmData((*itTask)->getId(), *itTrans) == false) {
          db_.rollback();
          return false;
        }
        ++itTrans;
      }
			DDEBUG("T_TRANSITION Saved");
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

  vector<ElementStmParamPtr> stateList = source->getStmElementList();
  vector<ConnectionStmParamPtr> transList = source->getStmConnectionList();
  vector<ConnectionStmParamPtr>::iterator itTrans = transList.begin();
  while (itTrans != transList.end()) {
    //V‹K’Ç‰Á‚µ‚½ó‘Ô—p
    if ((*itTrans)->getSourceId() < 0 || (*itTrans)->getMode() == DB_MODE_INSERT) {
      vector<ElementStmParamPtr>::iterator sourceElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getSourceId()));
      if (sourceElem == stateList.end()) {
        ++itTrans;
        DDEBUG("NOT Found SourceId");
        continue;
      }
      (*itTrans)->setSourceId((*sourceElem)->getId());
    }

    if ((*itTrans)->getTargetId() < 0 || (*itTrans)->getMode() == DB_MODE_INSERT) {
      vector<ElementStmParamPtr>::iterator targetElem = find_if(stateList.begin(), stateList.end(), ElementStmParamComparator((*itTrans)->getTargetId()));
      if (targetElem == stateList.end()) {
        ++itTrans;
        DDEBUG("NOT Found TargetId");
        continue;
      }
      (*itTrans)->setTargetId((*targetElem)->getId());
      DDEBUG_V("New Trans sourceId : %d, TargetId : %d", (*itTrans)->getSourceId(), (*itTrans)->getTargetId());
    }
    if (saveTransitionStmData(source->getId(), *itTrans) == false) {
      db_.rollback();
      return false;
    }
    ++itTrans;
  }
  //
  vector<ElementStmParamPtr>::iterator itState = stateList.begin();
  while (itState != stateList.end()) {
    (*itState)->updateId();
    ++itState;
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
	if (deleteDataById("T_VIA_POINT", "task_inst_id", task_inst_id) == false) return false;
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
	if (deleteDataById("T_FLOW_VIA_POINT", "flow_id", id) == false) return false;

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
QByteArray DatabaseManager::image2DB(QString& name, const QImage& source) {
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
