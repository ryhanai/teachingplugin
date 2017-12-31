#include "ParamWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

ParamWidget::ParamWidget(QWidget* parent) :
    QWidget(parent)
{
  //setStyleSheet( "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }" );
  //QLabel* label = new QLabel(this);
  //label->setText("screw pos");
  QLineEdit* nameEdit = new QLineEdit(this);
  QLineEdit* valueEdit = new QLineEdit(this);
  nameEdit->setText("screw pos");
  valueEdit->setText("0.0");
  QHBoxLayout* layout = new QHBoxLayout();
  //layout->addWidget(label);
  layout->addWidget(nameEdit);
  layout->addWidget(valueEdit);
  setLayout(layout);
}
