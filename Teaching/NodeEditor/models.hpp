#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <QtGui/QDoubleValidator>

#include "ParamWidget.hpp"

#include "NodeData.hpp"
#include "NodeDataModel.hpp"

#include <memory>

#include <iostream>
//class TaskData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class ControlData : public NodeData
{
public:

  ControlData() 
  {}

  NodeDataType type() const override
  {
    return NodeDataType {"cntl", "ctrl"};
  }
};

class ParamData : public NodeData
{
public:

  ParamData() : _value(0.0)
  {}

  ParamData(double const value) : _value(value)
  {}

  NodeDataType type() const override
  { 
    return NodeDataType {"data", ""};
  }

private:

  double _value;
};

class ModelParamData : public NodeData {
public:

  ModelParamData() : _value(0.0) {}

  ModelParamData(double const value) : _value(value) {}

  NodeDataType type() const override {
    return NodeDataType{ "modeldata", "" };
  }

private:

  double _value;
};
//------------------------------------------------------------------------------

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class TaskDataModel : public NodeDataModel {
  //Q_OBJECT

public:
	TaskDataModel() {
		canMany_ = false;
	}
  virtual ~TaskDataModel() {}

public:

  QString caption() const override {
    return _taskName;
  }

  QString portCaption(PortType portType, PortIndex portIndex) const override {
    if (portType == PortType::In && portIndex >= 1) {
			return portNames.at(portIndex - 1).name_;
		}

    return QString("");
  }

  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  QString name() const override {
    return QString("Task");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<TaskDataModel>();
  }

public:
  QJsonObject save() const override {
    QJsonObject modelJson;

    modelJson["name"] = name();

    return modelJson;
  }

public:

  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
			return portNames.size() + 1;
    } else { // PortType::Out
      return 1;
    }
  }

  void onTextEdited(QString const &string) {
    Q_UNUSED(string);
  
    // bool ok = false;
    // double number = _lineEdit->text().toDouble(&ok);
  
    // if (ok)
    // {
    //   _number = std::make_shared<DecimalData>(number);
  
    //   emit dataUpdated(0);
    // }
    // else
    // {
    //   emit dataInvalidated(0);
    // }
  }

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
		if (portIndex == 0) {
  		return ControlData().type();
		} else {
      if (portNames[portIndex - 1].type_ == 1) {
        return ModelParamData().type();
      } else {
        return ParamData().type();
      }
		}
  }

  std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ControlData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override {
    //
  }

  QWidget * embeddedWidget() override { return nullptr; }

  // bool
  // resizable() const override { return true; }

public:
  void setTaskName(QString const &taskName) override {
    _taskName = taskName;
    // _lineEdit->setText(_taskName);
    std::cout << "task name changed: " << _taskName.toStdString() << std::endl;
  }

  QString taskName() const override {
    return _taskName;
  }

private:

  // std::shared_ptr<TaskData> _task;
  QString _taskName = "unknown task";
  // QLineEdit * _lineEdit;
};

class ParamDataModel : public NodeDataModel {
public:
  ParamDataModel() : _paramEdit(new ParamWidget()) {
		canMany_ = true;
  }

  virtual ~ParamDataModel() {}

  inline QString getName() const { return _paramEdit->getName(); }
  inline QString getValue() const { return _paramEdit->getValue(); }
  inline void setParamInfo(QString name, QString value) {
    _paramEdit->setParamInfo(name, value);
  }

public:

  QString caption() const override {
    return QString("Flow Param");
  }

  QString name() const override {
    return QString("Flow Param");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<ParamDataModel>();
  }

public:

  QJsonObject save() const override {
    QJsonObject modelJson;

    modelJson["name"] = name();

    return modelJson;
  }

public:

  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 0;
    } else {
      return 1;
    }
  }

  void onTextEdited(QString const &string) {
    Q_UNUSED(string);
  
    // bool ok = false;
    // double value = _lineEdit->text().toDouble(&ok);
  
    // if (ok)
    // {
    //   _value = std::make_shared<ParamData>(value);
  
    //   emit dataUpdated(0);
    // }
    // else
    // {
    //   emit dataInvalidated(0);
    // }
  }

  NodeDataType dataType(PortType, PortIndex) const override {
    return ParamData().type();
  }

  std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ParamData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override {
    //
  }

  QWidget * embeddedWidget() override { return _paramEdit; }
  // QWidget *
  // embeddedWidget() override { return _lineEdit; }

protected:
  std::shared_ptr<ParamData> _value;
  ParamWidget * _paramEdit;
};

class IntParamDataModel : public ParamDataModel {
public:
  IntParamDataModel() : ParamDataModel(){
  };

  QString caption() const override {
    return QString("Flow Param (Integer)");
  }

  QString name() const override {
    return QString("Flow Param (Integer)");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<IntParamDataModel>();
  }
};

class DoubleParamDataModel : public ParamDataModel {
public:
  DoubleParamDataModel() : ParamDataModel(){
  };

  QString caption() const override {
    return QString("Flow Param (Double)");
  }

  QString name() const override {
    return QString("Flow Param (Double)");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<DoubleParamDataModel>();
  }
};

class StringParamDataModel : public ParamDataModel {
public:
  StringParamDataModel() : ParamDataModel() {
  };

  QString caption() const override {
    return QString("Flow Param (String)");
  }

  QString name() const override {
    return QString("Flow Param (String)");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<StringParamDataModel>();
  }
};

class FrameParamDataModel : public ParamDataModel {
public:
  FrameParamDataModel() : _frameParamEdit(new FrameParamWidget()), ParamDataModel() {
  };

  inline QString getName() const { return _frameParamEdit->getName(); }
  inline QString getValue() const { return _frameParamEdit->getValue(); }
  inline void setParamInfo(QString name, QString value) {
    _frameParamEdit->setParamInfo(name, value);
  }

  QString caption() const override {
    return QString("Flow Param (Frame)");
  }

  QString name() const override {
    return QString("Flow Param (Frame)");
  }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<FrameParamDataModel>();
  }

  QWidget * embeddedWidget() override { return _frameParamEdit; }

private:
  FrameParamWidget * _frameParamEdit;

};

class InitialDataModel : public NodeDataModel {
//Q_OBJECT

public:
	InitialDataModel() {
		canMany_ = false;
	}
  virtual ~InitialDataModel() {}

public:

  QString caption() const override { return QStringLiteral("Initial"); }
  bool captionVisible() const override { return true; }
  QString name() const override { return QStringLiteral("Initial"); }

  QString portCaption(PortType portType, PortIndex portIndex) const override { return QString(""); }
  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  std::unique_ptr<NodeDataModel> clone() const override { 
    return std::make_unique<InitialDataModel>(); 
  }

public:

  QJsonObject save() const override {
    QJsonObject modelJson;
    modelJson["name"] = name();
    return modelJson;
  }

public:

  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 0;
    } else {
      return 1;
    }
  }

  NodeDataType dataType(PortType, PortIndex) const override {
    return ControlData().type();
  }

  std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ControlData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override { }

  QWidget * embeddedWidget() override { return nullptr; }

//private slots:

};

class FinalDataModel : public NodeDataModel {
//Q_OBJECT

public:
	FinalDataModel() {
		canMany_ = false;
	}
  virtual ~FinalDataModel() {}

public:
  QString caption() const override { return QStringLiteral("Final"); }
  bool captionVisible() const override { return true; }
  QString name() const override { return QStringLiteral("Final"); }

  QString portCaption(PortType portType, PortIndex portIndex) const override { return QString(""); }
  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  std::unique_ptr<NodeDataModel> clone() const override { 
    return std::make_unique<FinalDataModel>(); 
  }

public:
	QJsonObject save() const override {
    QJsonObject modelJson;
    modelJson["name"] = name();
    return modelJson;
  }

public:

  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 1;
    } else {
      return 0;
    }
  }

  NodeDataType dataType(PortType, PortIndex) const override {
    return ControlData().type();
  }

  std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ControlData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override { }

  QWidget * embeddedWidget() override { return nullptr; }

//private slots:

};

class DecisionDataModel : public NodeDataModel {
//Q_OBJECT

public:
	DecisionDataModel() {
		canMany_ = false;
	}
  virtual ~DecisionDataModel() {}

public:
  QString caption() const override { return QStringLiteral("Decision"); }
  bool captionVisible() const override { return true; }
  QString name() const override { return QStringLiteral("Decision"); }

  QString portCaption(PortType portType, PortIndex portIndex) const override { 
    if (portType == PortType::In) {
      return QString("");
    } else {
      if (portIndex == 0) {
        return QString("true");
      } else {
        return QString("false");        
      }
    }
  }

  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  std::unique_ptr<NodeDataModel> clone() const override { 
    return std::make_unique<DecisionDataModel>(); 
  }

public:

  QJsonObject save() const override {
    QJsonObject modelJson;
    modelJson["name"] = name();
    return modelJson;
  }

public:
  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 1;
    } else {
      return 2;
    }
  }

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
    if (portType == PortType::In) {
      return ControlData().type();
    } else {
      if (portIndex == 0) {
        return ControlData().type();
      } else {
        return ControlData().type();
      }
    }
  }

	std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ControlData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override { }

  QWidget * embeddedWidget() override { return nullptr; }

//private slots:

};


class MergeDataModel : public NodeDataModel {
//Q_OBJECT

public:
	MergeDataModel() {
		canMany_ = false;
	}
  virtual ~MergeDataModel() {}

public:
  QString caption() const override { return QStringLiteral("Merge"); }
  bool captionVisible() const override { return true; }
  QString name() const override { return QStringLiteral("Merge"); }

  QString portCaption(PortType portType, PortIndex portIndex) const override { return QString(""); }
  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  std::unique_ptr<NodeDataModel> clone() const override { 
    return std::make_unique<MergeDataModel>(); 
  }

public:

  QJsonObject save() const override {
    QJsonObject modelJson;
    modelJson["name"] = name();
    return modelJson;
  }

public:
  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 2;
    } else {
      return 1;
    }
  }

  NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
    if (portType == PortType::In) {
      return ControlData().type();
    } else {
      return ControlData().type();
    }
  }

	std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ControlData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override { }

  QWidget * embeddedWidget() override { return nullptr; }

//private slots:

};


class TransformDataModel : public NodeDataModel {
//  Q_OBJECT

public:
  
  TransformDataModel() : _modelEdit(new ModelWidget()) {
    canMany_ = true;
  }

  virtual ~TransformDataModel() {}

  inline void setFlowModelParamId(int value) { _modelEdit->setFlowModelParamId(value); }

public:

  void initialize() {
    _modelEdit->showModelInfo();
  }

  inline int getMasterId() const { return _modelEdit->getMasterId(); }
  inline void setMasterInfo(int masterId) {
    _modelEdit->setMasterInfo(masterId);
  }

  inline QString getName() const { return _modelEdit->getNameInfo(); }
  inline void setName(QString name) {
    _modelEdit->setNameInfo(name);
  }

  QString caption() const override {
    return QString("");
  }

  QString name() const override {
    return QString("Model Param");
  }

  QString portCaption(PortType portType, PortIndex portIndex) const override {
    if (portType == PortType::Out) {
      return portNames.at(portIndex).name_;
    }
    return QString("");
  }
  bool portCaptionVisible(PortType portType, PortIndex portIndex) const override { return true; }

  std::unique_ptr<NodeDataModel> clone() const override {
    return std::make_unique<TransformDataModel>();
  }

public:

  QJsonObject save() const override {
    QJsonObject modelJson;

    modelJson["name"] = name();

    return modelJson;
  }

public:

  unsigned int nPorts(PortType portType) const override {
    if (portType == PortType::In) {
      return 0;
    } else {
      return portNames.size();
    }
  }
  
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
    if (portNames[portIndex].type_ == 1) {
      return ModelParamData().type();
    } else {
      return ParamData().type();
    }

  }

  std::shared_ptr<NodeData> outData(PortIndex) override {
    return std::make_shared<ModelParamData>();
  }

  void setInData(std::shared_ptr<NodeData>, int) override {
    //
  }

  QWidget * embeddedWidget() override { return _modelEdit; }

private:
  ModelWidget* _modelEdit;

};
