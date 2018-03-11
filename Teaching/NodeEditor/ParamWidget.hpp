#pragma once

#include "QtUtil.h"
#include "../TeachingDataHolder.h"

using namespace teaching;

class ParamWidget : public QWidget {
    Q_OBJECT
public:
    explicit ParamWidget(QWidget* parent = 0);

    inline QString getName() const { return nameEdit->text(); }

    inline QString getValue() const { return valueEdit->text(); }
    inline void setValue(QString value) { valueEdit->setText(value); }

    void setParamInfo(QString name, QString value);

private:
  QLineEdit* nameEdit;
  QLineEdit* valueEdit;
};

class ModelWidget : public QWidget {
  Q_OBJECT
public:
  explicit ModelWidget(QWidget* parent = 0);
  inline void setFlowModelParamId(int value) { this->flowModelParamId_ = value; }

  void showModelInfo();

  inline int getMasterId() const {
    int index = cmbModelName->currentIndex();
    return this->cmbModelName->itemData(index).toInt();
  }

  inline int getMasterParamId() const {
    int index = cmbModelParamName->currentIndex();
    return this->cmbModelParamName->itemData(index).toInt();
  }

  void setMasterInfo(int masterId, int masterParamId);

private Q_SLOTS:
  void modelSelectionChanged(int index);

private:
  QComboBox * cmbModelName;
  QComboBox* cmbModelParamName;
  QGraphicsView* imageView;
  QGraphicsScene* scene;
  int flowModelParamId_;

  vector<ModelMasterParamPtr> modelMasterList_;
};
