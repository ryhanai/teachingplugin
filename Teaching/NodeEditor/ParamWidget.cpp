#include "ParamWidget.hpp"

ParamWidget::ParamWidget(QWidget* parent) : QWidget(parent) {
  //setStyleSheet( "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }" );
  nameEdit = new QLineEdit(this);
  valueEdit = new QLineEdit(this);

  nameEdit->setText("screw pos");
  valueEdit->setText("0.0");

  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(nameEdit);
  layout->addWidget(valueEdit);
  setLayout(layout);
}

void ParamWidget::setParamInfo(QString name, QString value) {
  nameEdit->setText(name);
  valueEdit->setText(value);
}

//////////
ModelWidget::ModelWidget(QWidget* parent) : QWidget(parent) {
  imageView = new QGraphicsView();
  scene = new QGraphicsScene();
  imageView->setScene(scene);

  cmbModelName = new QComboBox;
  cmbModelName->addItem("XXXXXXXXXXXXXXX");

  cmbModelParamName = new QComboBox;
  cmbModelParamName->addItem("XXXXXXXXXXXXXXX");
  //
  QGridLayout* layout = new QGridLayout();
  layout->addWidget(imageView, 0, 0, 2, 1);
  layout->addWidget(cmbModelName, 0, 1, 1, 1);
  layout->addWidget(cmbModelParamName, 1, 1, 1, 1);
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
  /////
  cmbModelParamName->clear();
  cmbModelParamName->addItem("origin", -1);
  ModelMasterParamPtr baseParam = modelMasterList_[0];
  for (int index = 0; index < baseParam->getActiveParamList().size(); index++) {
    ModelParameterParamPtr param = baseParam->getActiveParamList()[index];
    cmbModelParamName->addItem(param->getName(), param->getId());
  }
}

void ModelWidget::modelSelectionChanged(int index) {
  scene->clear();
  cmbModelParamName->clear();
  cmbModelParamName->addItem("origin", -1);

  int modelId = cmbModelName->itemData(index).toInt();
  vector<ModelMasterParamPtr>::iterator masterParamItr = find_if(modelMasterList_.begin(), modelMasterList_.end(), ModelMasterComparator(modelId));
  if (masterParamItr == modelMasterList_.end()) return;

  QPixmap pixmap = QPixmap::fromImage((*masterParamItr)->getImage());
  if (!pixmap.isNull()) {
    pixmap = pixmap.scaled(imageView->width() - 5, imageView->height() - 5, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  scene->addPixmap(pixmap);

  for (int index = 0; index < (*masterParamItr)->getActiveParamList().size(); index++) {
    ModelParameterParamPtr param = (*masterParamItr)->getActiveParamList()[index];
    cmbModelParamName->addItem(param->getName(), param->getId());
  }
}

void ModelWidget::setMasterInfo(int masterId, int masterParamId) {
  int masterIndex = 0;
  for (int index = 0; index < cmbModelName->count(); index++) {
    if (cmbModelName->itemData(index) == masterId) {
      masterIndex = index;
      break;
    }
  }
  cmbModelName->setCurrentIndex(masterIndex);
  //
  int masterParamIndex = 0;
  if (0 < masterParamId) {
    for (int index = 0; index < cmbModelParamName->count(); index++) {
      if (cmbModelParamName->itemData(index) == masterParamId) {
        masterParamIndex = index;
        break;
      }
    }
  }
  cmbModelParamName->setCurrentIndex(masterParamIndex);
}
