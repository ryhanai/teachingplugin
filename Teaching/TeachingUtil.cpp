#include <fstream>
#include <iostream>
#include "TeachingUtil.h"
//
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>

#include "PythonWrapper.h"
#include "Calculator.h"

#include "LoggerUtil.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

typedef boost::array<boost::uint8_t, 20> hash_data_t;

namespace teaching {

int TeachingUtil::getModelType(QString& source) {
  int result = 0;
  if (source == "Env") {
    result = 0;
  } else if (source == "E.E.") {
    result = 1;
  } else if (source == "Work") {
    result = 2;
  }
  return result;
}

QString TeachingUtil::getTypeName(const int type) {
  QString typeName;
  switch (type) {
    case PARAM_TYPE_INTEGER:
      typeName = "Integer"; break;
    case PARAM_TYPE_DOUBLE:
      typeName = "Double"; break;
    case PARAM_TYPE_FRAME:
      typeName = "Frame"; break;
  }
  return typeName;
}

QString TeachingUtil::getTypeRName(const int type) {
  QString typeName;
  switch (type) {
    case PARAM_TYPE_INTEGER:
      typeName = "Int"; break;
    case PARAM_TYPE_DOUBLE:
      typeName = "Dbl"; break;
    case PARAM_TYPE_FRAME:
      typeName = "Frm"; break;
  }
  return typeName;
}

QString TeachingUtil::getSha1Hash(const void *data, const std::size_t byte_count) {
  boost::uuids::detail::sha1 sha1;
  sha1.process_bytes(data, byte_count);
  unsigned int digest[5];
  sha1.get_digest(digest);
  const boost::uint8_t *p_digest = reinterpret_cast<const boost::uint8_t *>(digest);
  hash_data_t hash_data;
  for (int index = 0; index < 5; ++index) {
    hash_data[index * 4] = p_digest[index * 4 + 3];
    hash_data[index * 4 + 1] = p_digest[index * 4 + 2];
    hash_data[index * 4 + 2] = p_digest[index * 4 + 1];
    hash_data[index * 4 + 3] = p_digest[index * 4];
  }
  //
  hash_data_t::const_iterator itr = hash_data.begin();
  const hash_data_t::const_iterator end_itr = hash_data.end();
  QString result;
  for (; itr != end_itr; ++itr) {
    result = result + QString("%02X").arg(*itr);
  }

  return result;
}
/////
QTableWidget* UIUtil::makeTableWidget(int colNo, bool isExpanding) {
  QTableWidget* target = new QTableWidget(0, colNo);
  target->setSelectionBehavior(QAbstractItemView::SelectRows);
  target->setSelectionMode(QAbstractItemView::SingleSelection);
  target->setEditTriggers(QAbstractItemView::NoEditTriggers);
  target->verticalHeader()->setVisible(false);
  target->setRowCount(0);
  if (isExpanding) {
    target->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }
  return target;
}

QTableWidgetItem* UIUtil::makeTableItemWithData(QTableWidget* table, int rowNo, int colNo, const QString& text, int data) {
  QTableWidgetItem* target = makeTableItem(table, rowNo, colNo, text);
  target->setData(Qt::UserRole, data);
  return target;
}

QTableWidgetItem* UIUtil::makeTableItem(QTableWidget* table, int rowNo, int colNo, const QString& text) {
  QTableWidgetItem* target = new QTableWidgetItem;
  table->setItem(rowNo, colNo, target);
  target->setData(Qt::UserRole, 1);
  target->setText(text);
  return target;
}

QString UIUtil::getTypeName(int source) {
  QString result = "";

  switch (source) {
    case PARAM_KIND_NORMAL:
      result = "Normal";
      break;
    case PARAM_KIND_MODEL:
      result = "Model";
      break;
  }
  return result;
}
/////
bool SettingManager::loadSetting() {
  std::ifstream inputFs(PARAMETER_FILE.c_str());
  if (inputFs.fail()) return false;

  std::string eachLine;
  std::vector<std::string> contents;

  while (getline(inputFs, eachLine)) {
    contents.push_back(eachLine);
  }

  logLevel_ = LOG_NO;
  logDir_ = "Log";

  for (int index = 0; index < contents.size(); index++) {
    QString each = QString::fromStdString(contents[index]);
    int startPos = each.indexOf(":");
    if (startPos == std::string::npos) continue;

    QString value = each.mid(startPos + 1);
    value = value.trimmed();
    //
    if (each.startsWith("database")) {
      dataBase_ = value.toStdString();

    } else if (each.startsWith("robotModelName")) {
      robotModelName_ = value.toStdString();

    } else if (each.startsWith("logLevel")) {
      logLevel_ = value.toInt();

    } else if (each.startsWith("logDir")) {
      logDir_ = value.toStdString();

    } else if (each.startsWith("controller")) {
      controller_ = value.toStdString();

    } else if (each.startsWith("application")) {
      QStringList elems = value.split("|");
      if (elems.size() < 2) continue;
      appMap_[elems[0].toStdString()] = elems[1].toStdString();
    }
  }

  return true;
}

bool SettingManager::saveSetting() {
  std::ofstream outputFs(PARAMETER_FILE.c_str());

  outputFs << "database : " << dataBase_ << std::endl;
  outputFs << "robotModelName : " << robotModelName_ << std::endl;
  outputFs << "logLevel : " << logLevel_ << std::endl;
  outputFs << "logDir : " << logDir_ << std::endl;
  outputFs << "controller : " << controller_ << std::endl;
  std::map<std::string, std::string>::const_iterator itKey;
  for (itKey = appMap_.begin(); itKey != appMap_.end(); itKey++) {
    outputFs << "application : " << itKey->first << "|" << itKey->second << std::endl;
  }

  return true;
}

void SettingManager::clearExtList() {
  appMap_.clear();
}

void SettingManager::setTargetApp(std::string strExt, std::string strApp) {
  appMap_[strExt] = strApp;
}

std::string SettingManager::getTargetApp(std::string strExt) {
  map<std::string, std::string>::iterator itr = appMap_.find(strExt);
  std::string result = "";
  if (itr != appMap_.end()) {
    result = appMap_[strExt];
  }
  return result;
}

std::vector<std::string> SettingManager::getExtList() {
  std::vector<std::string> result;

  std::map<std::string, std::string>::const_iterator itKey;
  for (itKey = appMap_.begin(); itKey != appMap_.end(); itKey++) {
    result.push_back(itKey->first);
  }
  return result;
}
/////
ArgumentEstimator* EstimatorFactory::createArgEstimator(TaskModelParamPtr targetParam) {
  ArgumentEstimator* handler = new Calculator();
  //ArgumentEstimator* handler = new PythonWrapper();
  handler->initialize(targetParam);
  return handler;
}

ArgumentEstimator* EstimatorFactory::createArgEstimator(FlowParamPtr targetParam) {
  ArgumentEstimator* handler = new Calculator();
  //ArgumentEstimator* handler = new PythonWrapper();
  //handler->initialize(targetParam);
  return handler;
}

void EstimatorFactory::deleteArgEstimator(ArgumentEstimator* handler) {
  DDEBUG("EstimatorFactory::deleteArgEstimator");
  if(handler) {
    handler->finalize();
    delete handler;
  }
  handler = 0;
}
/////////
bool TeachingUtil::importMasterModel(Mapping* targetMap, vector<ModelMasterParamPtr>& modelMasterList, QString& path, QString& errMessage) {
  Listing* masterList = targetMap->findListing("model_master");
  if (masterList) {
    for (int idxMaster = 0; idxMaster < masterList->size(); idxMaster++) {
      Mapping* masterMap = masterList->at(idxMaster)->toMapping();

      QString name = "";
      QString fileName = "";
      QString imageFileName = "";

      try {
        name = QString::fromStdString(masterMap->get("name").toString());
      } catch (...) {
        errMessage = _("Failed to read the name of the modelMaster.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        fileName = QString::fromStdString(masterMap->get("file_name").toString());
      } catch (...) {
        errMessage = _("Failed to read the file_name of the modelMaster.");
        DDEBUG(errMessage.toStdString().c_str());
        return false;
      }
      try {
        imageFileName = QString::fromStdString(masterMap->get("image_file_name").toString());
      } catch (...) {
      }

      ModelMasterParamPtr masterParam = std::make_shared<ModelMasterParam>(NULL_ID, name, fileName);
      masterParam->setNew();
      if (0 < fileName.length()) {
        QString strFullModelFile = path + QString("/") + fileName;
        QFile file(strFullModelFile);
        if (file.exists() == false) {
          errMessage = "Target Master file NOT EXIST. " + strFullModelFile;
          DDEBUG(errMessage.toStdString().c_str());
          return false;
        }
        if (file.open(QIODevice::ReadOnly) == false) {
          errMessage = "Failed to open Master file. " + strFullModelFile;
          DDEBUG(errMessage.toStdString().c_str());
          return false;
        }
        masterParam->setData(file.readAll());
        //
        //参照モデルの読み込み
        if (TeachingUtil::loadModelDetail(strFullModelFile, masterParam) == false) {
          errMessage = "Failed to load Model Detail file. " + strFullModelFile;
          return false;
        }
      }
      //
      DDEBUG_V("imageFileName:%s", imageFileName.toStdString().c_str());
      if (0 < imageFileName.length()) {
        QString strImageFile = path + QString("/") + imageFileName;
        QFile file(strImageFile);
        if (file.exists() == false) {
          errMessage = "Target Master Image file NOT EXIST. " + strImageFile;
          DDEBUG(errMessage.toStdString().c_str());
          return false;
        }
        masterParam->setImageFileName(imageFileName);
        QImage image(strImageFile);
        masterParam->setImage(image);
      }
      modelMasterList.push_back(masterParam);
      //
      Listing* featureList = masterMap->findListing("features");
      if (featureList) {
        for (int idxFeat = 0; idxFeat < featureList->size(); idxFeat++) {
          Mapping* featMap = featureList->at(idxFeat)->toMapping();

          QString nameFet = "";
          QString valueFet = "";
          try {
            nameFet = QString::fromStdString(featMap->get("name").toString());
          } catch (...) {
            errMessage = _("Failed to read the name of the master feature. : ") + name;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          if (nameFet.isEmpty()) {
            errMessage = _("name of themaster feature is EMPTY. : ") + name;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          try {
            valueFet = QString::fromStdString(featMap->get("value").toString());
          } catch (...) {
            errMessage = _("Failed to read the value of the master feature. : ") + name;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }
          if (valueFet.isEmpty()) {
            errMessage = _("value of themaster feature is EMPTY. : ") + name;
            DDEBUG(errMessage.toStdString().c_str());
            return false;
          }

          ModelParameterParamPtr featureParam = std::make_shared<ModelParameterParam>(NULL_ID, NULL_ID, nameFet, valueFet);
          featureParam->setNew();
          masterParam->addModelParameter(featureParam);
        }
      }
    }
  }
  return true;
}

bool TeachingUtil::checkNameStr(QString target) {
  if (target.contains("|")) return false;
  return true;
}
}
