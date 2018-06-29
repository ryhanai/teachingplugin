#include "FlowSearchDialog.h"

#include "TeachingEventHandler.h"

#include "gettext.h"
#include "LoggerUtil.h"

namespace teaching {

FlowSearchDialog::FlowSearchDialog(bool canEdit, QWidget* parent)
  : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint) {
  QFrame* condFrame = new QFrame;
  QLabel* lblCond = new QLabel(_("Condition:"));
  leCond = new QLineEdit;
  QPushButton* btnSearch = new QPushButton(_("Search"));
  btnSearch->setIcon(QIcon(":/Teaching/icons/Search.png"));
  btnSearch->setToolTip(_("Search Flow"));

  QHBoxLayout* topLayout = new QHBoxLayout;
  condFrame->setLayout(topLayout);
  topLayout->addWidget(lblCond);
  topLayout->addWidget(leCond);
  topLayout->addWidget(btnSearch);
  //
  lstFlow = new QTableWidget(0, 4);
  lstFlow->setSelectionBehavior(QAbstractItemView::SelectRows);
  lstFlow->setSelectionMode(QAbstractItemView::SingleSelection);
  lstFlow->setEditTriggers(QAbstractItemView::NoEditTriggers);
  lstFlow->verticalHeader()->setVisible(false);
  lstFlow->setColumnWidth(0, 200);
  lstFlow->setColumnWidth(1, 400);
  lstFlow->setColumnWidth(2, 150);
  lstFlow->setColumnWidth(3, 150);
  lstFlow->setRowCount(0);
  lstFlow->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");
  //
  QFrame* frmButtons = new QFrame;
  QPushButton* btnSelect = new QPushButton(_("Open"));
  btnSelect->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  btnSelect->setToolTip(_("Open selected Flow"));

  QPushButton* btnDelete = new QPushButton(_("Delete"));
  btnDelete->setIcon(QIcon(":/Teaching/icons/Delete.png"));
  btnDelete->setToolTip(_("Delete selected Flow"));

  QPushButton* btnCancel = new QPushButton(_("Cancel"));
  btnCancel->setIcon(QIcon(":/Teaching/icons/Cancel.png"));
  btnCancel->setToolTip(_("Cancel Flow select"));

  QHBoxLayout* buttonLayout = new QHBoxLayout(frmButtons);
  buttonLayout->setContentsMargins(2, 2, 2, 2);
  buttonLayout->addWidget(btnSelect);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnCancel);
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(condFrame);
  mainLayout->addWidget(lstFlow);
  mainLayout->addWidget(frmButtons);
  setLayout(mainLayout);
  //
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
  connect(leCond, SIGNAL(editingFinished()), this, SLOT(searchClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnSelect, SIGNAL(clicked()), this, SLOT(oKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(lstFlow, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(oKClicked()));
  //
  setWindowTitle(_("Flow List"));
  setMinimumHeight(sizeHint().height());
  setMinimumWidth(600);
  //
  btnDelete->setEnabled(canEdit);
  //
	TeachingEventHandler::instance()->fsd_Loaded(this);
}

void FlowSearchDialog::showGrid(const vector<FlowParamPtr>& flowList) {
	lstFlow->clear();
	lstFlow->setRowCount(0);
	lstFlow->setHorizontalHeaderLabels(QStringList() << "Name" << "Comment" << "Created" << "Last Updated");

	for (FlowParamPtr param : flowList) {
		int row = lstFlow->rowCount();
		lstFlow->insertRow(row);

		QTableWidgetItem* itemName = new QTableWidgetItem;
		lstFlow->setItem(row, 0, itemName);
		itemName->setText(param->getName());
		itemName->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemComment = new QTableWidgetItem;
		lstFlow->setItem(row, 1, itemComment);
		itemComment->setText(param->getComment());
		itemComment->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemCreated = new QTableWidgetItem;
		lstFlow->setItem(row, 2, itemCreated);
		itemCreated->setText(param->getCreatedDate());
		itemCreated->setData(Qt::UserRole, param->getId());

		QTableWidgetItem* itemLastUpdated = new QTableWidgetItem;
		lstFlow->setItem(row, 3, itemLastUpdated);
		itemLastUpdated->setText(param->getLastUpdatedDate());
		itemLastUpdated->setData(Qt::UserRole, param->getId());
	}
}

void FlowSearchDialog::searchClicked() {
  DDEBUG("FlowSearchDialog::searchClicked()");
	TeachingEventHandler::instance()->fsd_SeachClicked(leCond->text());
}

void FlowSearchDialog::deleteClicked() {
  DDEBUG("FlowSearchDialog::deleteClicked()");

  QTableWidgetItem* item = lstFlow->currentItem();
  if (item) {
    int targetId = item->data(Qt::UserRole).toInt();
    if (TeachingEventHandler::instance()->fsd_DeleteClicked(targetId)) {
      QMessageBox::information(this, _("Database"), _("Database updated"));
    } else {
      QMessageBox::warning(this, _("Database Error"), TeachingDataHolder::instance()->getErrorStr());
    }

  } else {
    QMessageBox::warning(this, _("Flow List"), _("Select Delete Flow"));
    return;
  }
}

void FlowSearchDialog::oKClicked() {
  DDEBUG("FlowSearchDialog::oKClicked()");
  QTableWidgetItem* item = lstFlow->currentItem();
  if (item) {
    int targetId = item->data(Qt::UserRole).toInt();
		TeachingEventHandler::instance()->fsd_OKClicked(targetId);
  } else {
    QMessageBox::warning(this, _("Flow List"), _("Select Target Flow"));
    return;
  }
	this->setResult(QDialog::Accepted);
}

void FlowSearchDialog::cancelClicked() {
  DDEBUG("FlowSearchDialog::cancelClicked()");
	this->setResult(QDialog::Rejected);
	close();
}

}
