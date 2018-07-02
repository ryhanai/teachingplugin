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

    void setParamInfo(QString name, QString value) {
      nameEdit->setText(name);
      setValue(value);
    };

private:
  QLineEdit* nameEdit;
  QLineEdit* valueEdit;
};

class FrameParamWidget : public QWidget {
  Q_OBJECT
public:
  explicit FrameParamWidget(QWidget* parent = 0);

  inline QString getName() const { return nameEdit->text(); }

  inline QString getValue() const {
    return xEdit->text() + "," + yEdit->text() + "," + zEdit->text()
            + "," + rxEdit->text() + "," + ryEdit->text() + "," + rzEdit->text();
  }
  inline void setValue(QString value) {
    QStringList valList = value.split(",");
    if (0 < valList.size()) xEdit->setText(valList[0]);
    if (1 < valList.size()) yEdit->setText(valList[1]);
    if (2 < valList.size()) zEdit->setText(valList[2]);
    if (3 < valList.size()) rxEdit->setText(valList[3]);
    if (4 < valList.size()) ryEdit->setText(valList[4]);
    if (5 < valList.size()) rzEdit->setText(valList[5]);
  }

  void setParamInfo(QString name, QString value) {
    nameEdit->setText(name);
    setValue(value);
  };

private:
  QLineEdit* nameEdit;
  QLineEdit* xEdit;
  QLineEdit* yEdit;
  QLineEdit* zEdit;
  QLineEdit* rxEdit;
  QLineEdit* ryEdit;
  QLineEdit* rzEdit;
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

  void setMasterInfo(int masterId);

private Q_SLOTS:
  void modelSelectionChanged(int index);

private:
  QComboBox * cmbModelName;
  QGraphicsView* imageView;
  QGraphicsScene* scene;
  int flowModelParamId_;

  vector<ModelMasterParamPtr> modelMasterList_;
};
