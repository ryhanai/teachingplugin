#ifndef TEACHING_FLOW_SEARCH_DIALOG_H_INCLUDED
#define TEACHING_FLOW_SEARCH_DIALOG_H_INCLUDED

#include "QtUtil.h"
#include "DataBaseManager.h"

using namespace cnoid;

namespace teaching {

class FlowSearchDialog : public QDialog {
  Q_OBJECT
public:
  FlowSearchDialog(QWidget* parent = 0);

  inline bool IsOK() const { return this->isOk_; }
  inline int getSelectedIndex() const { return this->selected_; }

private Q_SLOTS:
  void searchClicked();
  void deleteClicked();
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit* leCond;
  QTableWidget* lstFlow;

  std::vector<FlowParam*> flowList_;
  int selected_;
  bool isOk_;

  void showGrid();
};

}
#endif
