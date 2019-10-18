#ifndef TEACHING_SETTING_DIALOG_H_INCLUDED
#define TEACHING_SETTING_DIALOG_H_INCLUDED

#include "QtUtil.h"

namespace teaching {

struct AppExtParam {
  QString ext_;
  QString appPath_;
};

class SettingDialog : public QDialog {
  Q_OBJECT
public:
  SettingDialog(QWidget* parent = 0);

  inline bool IsDBUpdated() const { return this->isDBUpdated_; }
  inline bool IsCtrlUpdated() const { return this->isCtrlUpdated_; }

  private Q_SLOTS:
  void refDBClicked();
  void appSelectionChanged();
  void refAppClicked();
  void refLogClicked();
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit* leDatabase;
  QLineEdit* leRobotModel;

  QTableWidget* lstApp;
  QLineEdit* leExt;
  QLineEdit* leApp;

  QComboBox* cmbController;

  QComboBox* cmbLogLevel;
  QLineEdit* leLogDir;

  bool isDBUpdated_;
  bool isCtrlUpdated_;
  int currentRowIndex_;
  std::vector<AppExtParam> appList_;

  void showAppList();
};

class SearchList : public QTableWidget {
public:
  SearchList(int rows, int cols, QWidget* parent = 0);
protected:
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
private:
  QPoint startPos;
};

}
#endif
