#include "ParamWidget.hpp"
#include "TeachingEventHandler.h"
#include "TeachingUtil.h"

ParamWidget::ParamWidget(QWidget* parent) : QWidget(parent) {
  //setStyleSheet( "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }" );
  nameEdit = new QLineEdit(this);
  valueEdit = new QLineEdit(this);

  nameEdit->setText("screw pos");
  valueEdit->setText("0.0");

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(nameEdit);
  layout->addWidget(valueEdit);
  setLayout(layout);
}

FrameParamWidget::FrameParamWidget(QWidget* parent) : QWidget(parent) {
  //setStyleSheet( "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }" );
  nameEdit = new QLineEdit(this);
  xEdit = new QLineEdit(this);
  yEdit = new QLineEdit(this);
  zEdit = new QLineEdit(this);
  rxEdit = new QLineEdit(this);
  ryEdit = new QLineEdit(this);
  rzEdit = new QLineEdit(this);

  nameEdit->setText("screw pos");
  xEdit->setText("0.0");
  yEdit->setText("0.0");
  zEdit->setText("0.0");
  rxEdit->setText("0.0");
  ryEdit->setText("0.0");
  rzEdit->setText("0.0");

  QGridLayout* layout = new QGridLayout();
  layout->addWidget(nameEdit, 0, 0, 1, 3);
  layout->addWidget(xEdit, 1, 0, 1, 1);
  layout->addWidget(yEdit, 1, 1, 1, 1);
  layout->addWidget(zEdit, 1, 2, 1, 1);
  layout->addWidget(rxEdit, 2, 0, 1, 1);
  layout->addWidget(ryEdit, 2, 1, 1, 1);
  layout->addWidget(rzEdit, 2, 2, 1, 1);
  setLayout(layout);
}
//////////
ModelWidget::ModelWidget(QWidget* parent) : QWidget(parent), flowModelParamId_(NULL_ID){
  imageView = new QGraphicsView();
  scene = new QGraphicsScene();
  imageView->setScene(scene);

  cmbModelName = new QComboBox;
  cmbModelName->addItem("XXXXXXXXXXXXXXX");
  //
  QGridLayout* layout = new QGridLayout();
  layout->addWidget(imageView, 0, 0, 2, 1);
  layout->addWidget(cmbModelName, 0, 1, 1, 1);
  setLayout(layout);

  connect(cmbModelName, SIGNAL(currentIndexChanged(int)), this, SLOT(modelSelectionChanged(int)));
}

void ModelWidget::showModelInfo() {
  cmbModelName->clear();
  modelMasterList_ = TeachingDataHolder::instance()->getModelMasterList();
  for (int index = 0; index < modelMasterList_.size(); index++) {
    ModelMasterParamPtr param = modelMasterList_[index];
    cmbModelName->addItem(param->getName(), param->getId());
  }
}

void ModelWidget::modelSelectionChanged(int index) {
  scene->clear();

  int modelId = cmbModelName->itemData(index).toInt();
  vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList_.begin(), modelMasterList_.end(), ModelMasterComparator(modelId));
  if (masterParamItr == modelMasterList_.end()) return;

  QPixmap pixmap = QPixmap::fromImage((*masterParamItr)->getImage());
  if (!pixmap.isNull()) {
    pixmap = pixmap.scaled(imageView->width() - 5, imageView->height() - 5, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  scene->addPixmap(pixmap);
  //
  if (0 < flowModelParamId_) {
    TeachingEventHandler::instance()->flv_ModelParamChanged(flowModelParamId_, *masterParamItr);
  }
}

void ModelWidget::setMasterInfo(int masterId) {
  int masterIndex = 0;
  for (int index = 0; index < cmbModelName->count(); index++) {
    if (cmbModelName->itemData(index) == masterId) {
      masterIndex = index;
      break;
    }
  }
  cmbModelName->setCurrentIndex(masterIndex);
}
