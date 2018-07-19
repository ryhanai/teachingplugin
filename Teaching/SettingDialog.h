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
  QCheckBox* chkReal;

  QComboBox* cmbLogLevel;
  QLineEdit* leLogDir;

  bool isDBUpdated_;
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
