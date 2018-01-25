#ifndef TEACHING_FLOW_SEARCH_DIALOG_H_INCLUDED
#define TEACHING_FLOW_SEARCH_DIALOG_H_INCLUDED

#include "TeachingTypes.h"
#include "QtUtil.h"

namespace teaching {

using namespace std;

class FlowSearchDialog : public QDialog {
  Q_OBJECT
public:
  FlowSearchDialog(bool canEdit, QWidget* parent = 0);

  void showGrid(const vector<FlowParamPtr>& flowList);

private Q_SLOTS:
  void searchClicked();
  void deleteClicked();
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit* leCond;
  QTableWidget* lstFlow;
};

}
#endif
