#ifndef TEACHING_FLOW_VIEW_H_INCLUDED
#define TEACHING_FLOW_VIEW_H_INCLUDED

#include <cnoid/View>
#include "QtUtil.h"
#include "TeachingTypes.h"

#include "StateMachineView.h"

#include "NodeEditor/FlowEditor.hpp"
#include "NodeEditor/DataModelRegistry.hpp"

using namespace cnoid;
using namespace QtNodes;

namespace teaching {

class TaskInstanceView;

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
  QLineEdit * leCond;
  QTableWidget* lstFlow;
};

class NodeDispDialog : public QDialog {
  Q_OBJECT
public:
  NodeDispDialog(FlowParamPtr param, QWidget* parent = 0);

  private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTableWidget * lstNode_;
  QTableWidget * lstModel_;
  QTableWidget * lstParam_;

  void showNodeList();
  void showModelList();
  void showParamList();

  FlowParamPtr targetParam_;
};

class PortDispDialog : public QDialog {
  Q_OBJECT
public:
  PortDispDialog(TaskModelParamPtr param, QWidget* parent = 0);

  private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QTableWidget * lstModel_;
  QTableWidget * lstParam_;

  void showModelList();
  void showParamList();

  TaskModelParamPtr targetParam_;
};

class TaskInfoDialog : public QDialog {
  Q_OBJECT
public:
  TaskInfoDialog(ElementStmParamPtr param, QWidget* parent = 0);

  private Q_SLOTS:
  void oKClicked();
  void cancelClicked();

private:
  QLineEdit * txtName;

  ElementStmParamPtr targetParam_;
};

class FlowViewImpl : public QWidget {
  Q_OBJECT
public:
  FlowViewImpl(QWidget* parent = 0);
  ~FlowViewImpl();

  void flowSelectionChanged(TaskModelParamPtr target);
  void setButtonEnableMode(bool isEnable);
  void clearView();
  void dispView(FlowParamPtr& target);
  void createStateMachine(FlowParamPtr& target);

  inline bool updateTargetFlowParam(QString& errMessage) { return grhStateMachine->updateTargetFlowParam(errMessage); };
  inline void paramInfoUpdated(ElementStmParamPtr targetState) { grhStateMachine->paramInfoUpdated(targetState); };
  inline void modelParamUpdated(int flowModelId, ModelMasterParamPtr masterParam) { grhStateMachine->modelParamUpdated(flowModelId, masterParam); };
  inline bool checkOutConnection(int nodeId, int portIndex) { return grhStateMachine->checkOutConnection(nodeId, portIndex); }
  void setEditMode(bool canEdit);
  void cancelAllModel();
  void setExecState(bool isActive);


  bool renameNode(QString currentName, QString newName);
  void getNodeByName(QString name);
  bool connectNodes(QString from, QString fromPort, QString to, QString toPort);
  bool createNode(QString modelName, QPoint pos);
  void clearFlowScene();
  bool connectModelToTask(Node* fromNode, QString fromPort, Node* toNode, QString toPort);


  public Q_SLOTS:
  void editClicked();

  private Q_SLOTS :
  void searchClicked();
  void newFlowClicked();
  void registFlowClicked();
  void deleteTaskClicked();
  void runFlowClicked();
  void runTaskClicked();
  void abortClicked();
  void initPosClicked();
  void exportFlowClicked();
  void importFlowClicked();
  void hideClicked();
  void dispClicked();
  void paramDispClicked();
  void modelToggled();

private:
  QLineEdit * leName;
  QLineEdit* leComment;

  QPushButton* btnSearch;
  QPushButton* btnNewFlow;
  QPushButton* btnRegistFlow;

  QPushButton* btnHide;
  QPushButton* btnDisp;
  QPushButton* btnParamDisp;
  QPushButton* btnModelDisp;

  QPushButton* btnDeleteTask;
  QPushButton* btnRunFlow;
  QPushButton* btnRunTask;
  QPushButton* btnInitPos;
  QPushButton* btnEdit;
  QPushButton* btnExport;
  QPushButton* btnImport;
  QPushButton* btnAbort;

  FlowEditor* grhStateMachine;

  void changeEnables(bool value);

  void setStyle();
  std::shared_ptr<DataModelRegistry> registerDataModels();
};

class FlowView : public cnoid::View {
public:
  FlowView();
  ~FlowView();

  inline bool updateTargetFlowParam(QString& errMessage) { return viewImpl->updateTargetFlowParam(errMessage); };
  inline void paramInfoUpdated(ElementStmParamPtr targetState) { viewImpl->paramInfoUpdated(targetState); };
  inline void modelParamUpdated(int flowModelId, ModelMasterParamPtr masterParam) { viewImpl->modelParamUpdated(flowModelId, masterParam); };
  inline bool checkOutConnection(int nodeId, int portIndex) { return viewImpl->checkOutConnection(nodeId, portIndex); }
  inline void cancelAllModel() { viewImpl->cancelAllModel(); };

private:
  FlowViewImpl * viewImpl;
};

}
#endif
