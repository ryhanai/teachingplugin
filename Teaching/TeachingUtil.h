#ifndef TEACHING_TEACHING_UTIL_H_INCLUDED
#define TEACHING_TEACHING_UTIL_H_INCLUDED

#include <vector>
#include "TeachingTypes.h"
#include "CommandDefTypes.h"
#include <QString>
#include <QTableWidget>
#include "ArgumentEstimator.h"

#include <boost/uuid/sha1.hpp>
#include <boost/array.hpp>

namespace teaching {

static const std::string PARAMETER_FILE = "Setting.prm";

template <typename T> std::string toStr(const T& t) {
  std::ostringstream os; os << t; return os.str();
}

class TeachingUtil {
public:
  static bool importTask(QString& strFName, std::vector<TaskModelParamPtr>& taskInstList, vector<ModelMasterParamPtr>& modelMasterList, QString& errMessage);
  static bool exportTask(QString& strFName, TaskModelParamPtr targetTask);
  static bool loadModelDetail(QString& strFName, ModelMasterParamPtr targetModel);
  static void loadTaskDetailData(TaskModelParamPtr target);

  static bool exportFlow(QString& strFName, FlowParamPtr targetFlow);
  static bool importFlow(QString& strFName, std::vector<FlowParamPtr>& flowModelList, vector<ModelMasterParamPtr>& modelMasterList, QString& errMessage);

  static QString getSha1Hash(const void *data, const std::size_t byte_count);

private:
  static int getModelType(QString& source);
  static bool importTaskModel(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage);
  static bool importTaskParameter(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage);
  static bool importTaskState(Mapping* taskMap, TaskModelParamPtr taskParam, QString taskNameErr, QString& errMessage);
  static bool importTaskFile(Mapping* taskMap, TaskModelParamPtr taskParam, QString& path, QString taskNameErr, QString& errMessage);
  static bool importTaskImage(Mapping* taskMap, TaskModelParamPtr taskParam, QString& path, QString taskNameErr, QString& errMessage);
  static bool importTaskMaster(Mapping* taskMap, vector<ModelMasterParamPtr>& modelMasterList, QString& path, QString taskNameErr, QString& errMessage);

};
/////
class UIUtil {
public:
  static QTableWidget* makeTableWidget(int colNo, bool isExpanding);
  static QTableWidgetItem* makeTableItem(QTableWidget* table, int rowNo, int colNo, const QString& text);
  static QTableWidgetItem* makeTableItemWithData(QTableWidget* table, int rowNo, int colNo, const QString& text, int data);
  static QString getTypeName(int source);
  static QString getParamTypeName(int source);
};
/////
class SettingManager {
private:
  SettingManager() : isReal_(false) {};

public:
  static SettingManager& getInstance(void) {
    static SettingManager singleton;
    return singleton;
  }

  inline std::string getDatabase() const { return this->dataBase_; }
  inline void setDatabase(std::string value) { this->dataBase_ = value; }
  inline std::string getRobotModelName() const { return this->robotModelName_; }
  inline void setRobotModelName(std::string value) { this->robotModelName_ = value; }
  inline int getLogLevel() const { return this->logLevel_; }
  inline void setLogLevel(int value) { this->logLevel_ = value; }
  inline std::string getLogDir() const { return this->logDir_; }
  inline void setLogDir(std::string value) { this->logDir_ = value; }
  inline std::string getController() const { return this->controller_; }
  inline void setController(std::string value) { this->controller_ = value; }

  void clearExtList();
  void setTargetApp(std::string strExt, std::string strApp);
  std::vector<std::string> getExtList();
  std::string getTargetApp(std::string strExt);

  bool loadSetting();
  bool saveSetting();

private:
  std::string dataBase_;
  std::string robotModelName_;
  int logLevel_;
  std::string logDir_;
  std::string controller_;

  std::map<std::string, std::string> appMap_;

  bool isReal_;

};

/////
class EstimatorFactory {
private:
  EstimatorFactory() {};

public:
  static EstimatorFactory& getInstance(void) {
    static EstimatorFactory singleton;
    return singleton;
  }

  ArgumentEstimator* createArgEstimator(TaskModelParamPtr targetParam);
  ArgumentEstimator* createArgEstimator(FlowParamPtr targetParam);
  void deleteArgEstimator(ArgumentEstimator* handler);
};

struct ModelParamComparator {
  int id_;
  ModelParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ModelParamPtr elem) const {
    return (elem->getId() == id_
      && (elem->getMode() != DB_MODE_DELETE && elem->getMode() != DB_MODE_IGNORE));
  }
};

struct ModelParamComparatorByRName {
  QString rname_;
  ModelParamComparatorByRName(QString value) {
    rname_ = value;
  }
  bool operator()(const ModelParamPtr elem) const {
    return elem->getRName() == rname_;
  }
};

struct ModelMasterComparator {
  int id_;
  ModelMasterComparator(int value) {
    id_ = value;
  }
  bool operator()(const ModelMasterParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct ModelMasterParamComparator {
	int id_;
  ModelMasterParamComparator(int value) {
    id_ = value;
	}
	bool operator()(const ModelParameterParamPtr elem) const {
		return elem->getId() == id_;
	}
};

struct ModelMasterParamComparatorByName {
  QString name_;
  ModelMasterParamComparatorByName(QString value) {
    name_ = value;
  }
  bool operator()(const ModelParameterParamPtr elem) const {
    return elem->getName() == name_;
  }
};

struct ElementStmParamComparator {
  int id_;
  ElementStmParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ElementStmParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct FlowModelParamComparator {
  int id_;
  FlowModelParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const FlowModelParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct FlowParameterParamComparator {
  int id_;
  FlowParameterParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const FlowParameterParamPtr elem) const {
    return elem->getId() == id_;
  }
};

struct FlowParameterParamByNameComparator {
  QString name_;
  FlowParameterParamByNameComparator(QString value) {
    name_ = value;
  }
  bool operator()(const FlowParameterParamPtr elem) const {
    return elem->getName() == name_;
  }
};

struct ParameterParamComparator {
  int id_;
  ParameterParamComparator(int value) {
    id_ = value;
  }
  bool operator()(const ParameterParamPtr elem) const {
    return elem->getId() == id_;
  }
};

}
#endif
