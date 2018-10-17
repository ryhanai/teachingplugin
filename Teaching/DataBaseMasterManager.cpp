#include "DataBaseManager.h"

#include <qsqlerror.h>
#include "TeachingUtil.h"

#include "LoggerUtil.h"

using namespace std;
using namespace boost;
using namespace cnoid;

namespace teaching {

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
	return result;
}

ModelMasterParamPtr DatabaseManager::getModelMaster(int master_id) {
	ModelMasterParamPtr result;

	string strQuery = "SELECT ";
	strQuery += "model_id, name, file_name, model_data, hash, image_file_name, image_data ";
	strQuery += "FROM M_MODEL ";
	strQuery += "WHERE model_id = " + toStr(master_id);
	QSqlQuery query(db_);
	query.exec(strQuery.c_str());
	if (query.next()) {
		int model_id = query.value(0).toInt();
		QString name = query.value(1).toString();
		QString fileName = query.value(2).toString();

		result = std::make_shared<ModelMasterParam>(model_id, name, fileName);
		result->setData(query.value(3).toByteArray());
    result->setHash(query.value(4).toString());
    result->setImageFileName(query.value(5).toByteArray());
    result->setRawData(query.value(6).toByteArray());
    result->loadData();
    result->setNormal();
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
				result->addModelDetail(detailParam);
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
				result->addModelParameter(modelParam);
			}
		}
	}
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
			strQuery += "VALUES ( ?, ?, ?, ?, ?, ?, ? )";

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
      //
      string strQuerydetail = "DELETE FROM M_MODEL_DETAIL WHERE model_id = ? ";
			QSqlQuery queryDetail(QString::fromStdString(strQuerydetail));
			queryDetail.addBindValue(source->getId());
			if (!queryDetail.exec()) {
				errorStr_ = "DELETE(M_MODEL_DETAIL) error:" + queryDetail.lastError().databaseText() + "-" + QString::fromStdString(strQuerydetail);
				return false;
			}
      //
      string strQueryParam = "DELETE FROM M_MODEL_PARAMETER WHERE model_id = ? ";
			QSqlQuery queryParam(QString::fromStdString(strQueryParam));
			queryParam.addBindValue(source->getId());
			if (!queryParam.exec()) {
				errorStr_ = "DELETE(M_MODEL_PARAMETER) error:" + queryParam.lastError().databaseText() + "-" + QString::fromStdString(strQueryParam);
				return false;
			}
      //
			source->setIgnore();
		}
		//
		vector<ModelDetailParamPtr> detailList = source->getModelDetailList();
		DDEBUG_V("detailList: %d", detailList.size());
		if (0 < detailList.size()) {
			for (ModelDetailParamPtr detail : source->getModelDetailList()) {
				if (saveModelDetailData(source->getId(), detail) == false) {
					return false;
				}
			}
		}
		//
		vector<ModelParameterParamPtr> paramList = source->getModelParameterList();
		DDEBUG_V("paramList: %d", paramList.size());
		if (0 < paramList.size()) {
			for (ModelParameterParamPtr detail : paramList) {
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

//
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
