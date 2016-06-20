#ifndef TEACHING_TEACHING_UTIL_H_INCLUDED
#define TEACHING_TEACHING_UTIL_H_INCLUDED

#include <vector>
#include "TeachingTypes.h"
#include "CommandDefTypes.h"
#include <QString>

namespace teaching {

static const std::string PARAMETER_FILE = "Setting.prm";
static const double DBL_DELTA = 0.0000000000001;

inline bool dbl_eq(double d1, double d2) {
  if (-DBL_DELTA < (d1 - d2) && (d1 - d2) < DBL_DELTA)
    return true;
  else
    return false;
}

template <typename T> std::string toStr(const T& t) {
    std::ostringstream os; os<<t; return os.str();
}

class TeachingUtil {
public:
  static bool loadTaskDef(QString& strFName, std::vector<TaskModelParam*>& taskInstList);
  static bool outputTaskDef(QString& strFName, TaskModelParam* targetTask);
  static bool loadModelDetail(QString& strFName, ModelParam* targetModel);
  static void loadTaskDetailData(TaskModelParam* target);

private:
  static int getModelType(QString& source);
  static QImage db2Image(const QString& name, const QByteArray& source);

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
  SettingManager() {};

public:
  static SettingManager& getInstance(void) {
    static SettingManager singleton;
    return singleton;
  }

  inline std::string getDatabase() const { return this->dataBase_; }
  inline void setDatabase(std::string value) { this->dataBase_ = value; }
  inline std::string getRobotModelName() const { return this->robotModelName_; }
  inline void setRobotModelName(std::string value) { this->robotModelName_ = value; }

  void clearExtList();
  void setTargetApp(std::string strExt, std::string strApp);
  std::vector<std::string> getExtList();
  std::string getTargetApp(std::string strExt);

  bool loadSetting();
  bool saveSetting();

private:
  std::string dataBase_;
  std::string robotModelName_;

  std::map<std::string, std::string> appMap_;

};

}
#endif
