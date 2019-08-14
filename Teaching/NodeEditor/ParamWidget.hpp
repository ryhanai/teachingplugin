#pragma once

#include "../QtUtil.h"
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
    xEdit->setText("0.000000");
    yEdit->setText("0.000000");
    zEdit->setText("0.000000");
    rxEdit->setText("0.000000");
    ryEdit->setText("0.000000");
    rzEdit->setText("0.000000");

    QStringList valList = value.split(",");
    if (0 < valList.size()) xEdit->setText(QString::number(valList[0].toDouble(), 'f', 6));
    if (1 < valList.size()) yEdit->setText(QString::number(valList[1].toDouble(), 'f', 6));
    if (2 < valList.size()) zEdit->setText(QString::number(valList[2].toDouble(), 'f', 6));
    if (3 < valList.size()) rxEdit->setText(QString::number(valList[3].toDouble(), 'f', 6));
    if (4 < valList.size()) ryEdit->setText(QString::number(valList[4].toDouble(), 'f', 6));
    if (5 < valList.size()) rzEdit->setText(QString::number(valList[5].toDouble(), 'f', 6));
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

  inline QString getNameInfo() {
    return nameEdit->text();
  }
  inline void setNameInfo(QString name) {
    nameEdit->setText(name);
  }

private Q_SLOTS:
  void modelSelectionChanged(int index);

private:
  QLineEdit* nameEdit;
  QComboBox * cmbModelName;
  QGraphicsView* imageView;
  QGraphicsScene* scene;
  int flowModelParamId_;

  vector<ModelMasterParamPtr> modelMasterList_;
};
