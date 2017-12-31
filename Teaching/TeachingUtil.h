#ifndef TEACHING_TEACHING_UTIL_H_INCLUDED
#define TEACHING_TEACHING_UTIL_H_INCLUDED

#include <vector>
#include "TeachingTypes.h"
#include "CommandDefTypes.h"
#include <QString>
#include <QTableWidget>
#include "ArgumentEstimator.h"

namespace teaching {

static const std::string PARAMETER_FILE = "Setting.prm";
static const double DBL_DELTA = 0.0000001;

inline bool dbl_eq(double d1, double d2) {
  if (-DBL_DELTA < (d1 - d2) && (d1 - d2) < DBL_DELTA)
    return true;
  else
    return false;
}

template <typename T> std::string toStr(const T& t) {
  std::ostringstream os; os << t; return os.str();
}

class TeachingUtil {
public:
  static bool importTask(QString& strFName, std::vector<TaskModelParamPtr>& taskInstList, vector<ModelMasterParamPtr>& modelMasterList);
  static bool exportTask(QString& strFName, TaskModelParamPtr targetTask);
  static bool loadModelDetail(QString& strFName, ModelMasterParamPtr targetModel);
  static void loadTaskDetailData(TaskModelParamPtr target);

  static bool exportFlow(QString& strFName, FlowParamPtr targetFlow);
  static bool importFlow(QString& strFName, std::vector<FlowParamPtr>& flowModelList, vector<ModelMasterParamPtr>& modelMasterList);

private:
  static int getModelType(QString& source);

};
/////
class UIUtil {
public:
  static QTableWidget* makeTableWidget(int colNo, bool isExpanding);
  static QTableWidgetItem* makeTableItem(QTableWidget* table, int rowNo, int colNo, const QString& text);
  static QTableWidgetItem* makeTableItemWithData(QTableWidget* table, int rowNo, int colNo, const QString& text, int data);
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

  inline bool getIsReal() const { return this->isReal_; }
  inline void setIsReal(bool value) { this->isReal_ = value; }

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
  void deleteArgEstimator(ArgumentEstimator* handler);
};

}
#endif
