#include "StateMachineView.h"
#include "TeachingUtil.h"
#include "Calculator.h"
#include "DataBaseManager.h"
#include "TaskExecutor.h"
#include "ArgumentDialog.h"

#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

EditorView::EditorView(QWidget* parent)
   : QGraphicsView(parent), targetTask_(0), modeCnt_(false), newStateNum(0),
     targetNode_(0), targetConnection_(0) {
  //scene_ = new QGraphicsScene(0, 0, this->width(), this->height());
  scene_ = new QGraphicsScene(0, 0, 1000, 1000);
  setScene(scene_);
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  setAcceptDrops(true);
}

void EditorView::dragEnterEvent(QDragEnterEvent* event) {
  if(event->mimeData()->hasFormat("application/StateMachineItem")){
    event->acceptProposedAction();
  }
}

void EditorView::dragMoveEvent(QDragMoveEvent *event) {
  if(event->mimeData()->hasFormat("application/StateMachineItem")){
    event->acceptProposedAction();
  }
}

void EditorView::dropEvent(QDropEvent* event) {
  const QMimeData *mime = event->mimeData();
  QString strTarget = event->mimeData()->text();
  QVariant varData = event->mimeData()->property("CommandId");
  int id = varData.toInt();

  ElementNode* node = new ElementNode(strTarget);
  scene_->addItem(node);
  QPointF position = mapToScene(event->pos());
  node->setPos(position.x(), position.y());
  elementList_.push_back(node);
  newStateNum--;
  ElementStmParam* newParam = new ElementStmParam(newStateNum, node->getElementType(), strTarget, position.x(), position.y());
  newParam->setOrgId(newStateNum);
  //newParam->setRealElem(node);
  newParam->setNew();
  node->setElemParam(newParam);
  targetTask_->addStmElement(newParam);
  node->setData(Qt::UserRole, TYPE_ELEMENT);

  targetNode_ = 0;
  if(targetConnection_) {
    targetConnection_->setPen(QPen(Qt::black, LINE_WIDTH));
    targetConnection_ = 0;
  }
}

void EditorView::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton){
    QPointF pos = mapToScene(event->pos());
    QGraphicsItem* gItem = scene_->itemAt(pos.x(), pos.y());
    if(targetNode_) {
      targetNode_->updateSelect(false);
    }
    if(targetConnection_) {
      targetConnection_->setPen(QPen(Qt::black, LINE_WIDTH));
      targetConnection_ = 0;
    }
    targetNode_ = 0;

    if(gItem) {
      QGraphicsItem* parentItem = gItem->parentItem();
      int type = parentItem->data(Qt::UserRole).toInt();
      if( type==TYPE_ELEMENT) {
        targetNode_ = (ElementNode*)gItem->parentItem();
        targetNode_->updateSelect(true);
        if( modeCnt_) {
          ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
          item->setSource(targetNode_);
          item->setPen(QPen(Qt::gray));
          item->setZValue(-10);
          targetNode_->addToGroup(item);
          targetNode_->addConnection(item);
        }

      } else if(type==TYPE_CONNECTION) {
        ConnectionNode* targetConn = (ConnectionNode*)gItem->parentItem();
        targetConn->setPen(QPen(Qt::red, LINE_WIDTH));
        targetConnection_ = targetConn;
      }
    }
  }
  QWidget::mousePressEvent(event);
}

void EditorView::mouseMoveEvent(QMouseEvent* event) {
  if(targetNode_) {
    if( modeCnt_) {
      QPointF position = mapToScene(event->pos());
      ConnectionNode* item = targetNode_->getCurrentConnection();
      item->setLine(targetNode_->pos().x(), targetNode_->pos().y(), position.x()+POS_DELTA, position.y()+POS_DELTA);

    } else {
      QPointF position = mapToScene(event->pos());
      targetNode_->updatePosition(position.x(), position.y());
      targetNode_->getElemParam()->setPosX(position.x());
      targetNode_->getElemParam()->setPosY(position.y());
      targetNode_->getElemParam()->setUpdate();
    }
  }
  QWidget::mouseMoveEvent(event);
}

void EditorView::mouseReleaseEvent(QMouseEvent* event) {
  if(modeCnt_ && targetNode_) {
    QPointF pos = mapToScene(event->pos());
    QGraphicsItem* gItem = scene_->itemAt(pos.x(), pos.y());
    if(gItem) {
      ConnectionNode* item = targetNode_->getCurrentConnection();
      ElementNode* currNode = (ElementNode*)gItem->parentItem();
      item->setTarget(currNode);
      item->setPen(QPen(Qt::black, LINE_WIDTH));
      item->setData(Qt::UserRole, TYPE_CONNECTION);
      item->reDrawConnection();
      currNode->addConnection(item);
      connectionList_.push_back(item);
      ConnectionStmParam* newConn = new ConnectionStmParam(-1, item->getSource()->getElemParam()->getId(), item->getTarget()->getElemParam()->getId(), "");
      newConn->setNew();
      item->setConnParam(newConn);
      newConn->setSourceId(item->getSource()->getElemParam()->getId());
      newConn->setTargetId(item->getTarget()->getElemParam()->getId());
      DDEBUG_V("mouseReleaseEvent:Sourcd=%d, Target=%d", newConn->getSourceId(), newConn->getTargetId());
      targetTask_->addStmConnection(newConn);
      targetNode_->removeFromGroup(item);
      //
      ConnectionStmParam* newParam = new ConnectionStmParam(-1, -1, -1 ,"");
      newParam->setNew();
      targetTask_->getStmConnectionList().push_back(newParam);

      if(item->getSource()->getElementType() == ELEMENT_DECISION ) {
        item->setText("false");
      }

    } else {
      ConnectionNode* item = targetNode_->getCurrentConnection();
      item->setPen(QPen(Qt::white));
      targetNode_->removeCurrentConnection();
    }
  }
  QWidget::mouseReleaseEvent(event);
}

void EditorView::wheelEvent(QWheelEvent* event ) {
  double dSteps = (double)event->delta() / 120.0;
  double scaleVal = 1.0;
  scaleVal -= (dSteps / 20.0);
  DDEBUG_V("wheelEvent View  %f : %f ", dSteps, scaleVal);
  scale(scaleVal, scaleVal);
}

void EditorView::keyPressEvent(QKeyEvent* event) {
  DDEBUG("keyPressEvent View ");

  if (event->key() == Qt::Key_Delete) {
    deleteCurrent();
  }
  QWidget::keyPressEvent(event);
}

void EditorView::deleteCurrent() {
  if(targetConnection_) {
    deleteConnection(targetConnection_);
    targetConnection_ = 0;
  }
  if(targetNode_) {
    deleteElement(targetNode_);
    targetNode_ = 0;
  }
}

void EditorView::deleteConnection(ConnectionNode* target) {
  target->getSource()->removeTargetConnection(target);
  target->getTarget()->removeTargetConnection(target);
  target->getConnParam()->setDelete();
  vector<ConnectionNode*>::iterator itRemove = std::remove(this->connectionList_.begin(), this->connectionList_.end(), target);
  if(itRemove != this->connectionList_.end()) {
    this->connectionList_.erase(itRemove, this->connectionList_.end());
    target->setVisible(false);
    //delete target;
  }
}

void EditorView::deleteElement(ElementNode* target) {
  vector<ConnectionNode*> conns = target->getConnectionList();
  vector<ConnectionNode*>::iterator itConn = conns.begin();
  while (itConn != conns.end() ) {
    deleteConnection((*itConn));
    ++itConn;
  }
  //
  target->getElemParam()->setDelete();
  vector<ElementNode*>::iterator itRemove = std::remove(this->elementList_.begin(), this->elementList_.end(), target);
  if(itRemove != this->elementList_.end()) {
    this->elementList_.erase(itRemove, this->elementList_.end());
    target->setVisible(false);
  }
}

void EditorView::removeAll() {
  scene_->clear();
}

void EditorView::createStateMachine(TaskModelParam* param) {
  DDEBUG_V("createStateMachine : %d", param->getStmConnectionList().size());
  targetNode_ = 0;
  targetConnection_ = 0;
  scene_->clear();
  //
  vector<ElementStmParam*> elemList = param->getStmElementList();
  for(int index=0; index<elemList.size(); index++) {
    ElementStmParam* target = elemList[index];
    if(target->getMode()==DB_MODE_DELETE || target->getMode()==DB_MODE_IGNORE) continue;

    QString strName = "";
    CommandDefParam* def = target->getCommadDefParam();
    if(def) {
      strName = def->getDispName();
    }
    ElementNode* node = new ElementNode(target->getType(), strName);
    scene_->addItem(node);
    node->setPos(target->getPosX(), target->getPosY());
    elementList_.push_back(node);
    target->setRealElem(node);
    node->setElemParam(target);
    node->setData(Qt::UserRole, TYPE_ELEMENT);
  }
  //
  vector<ConnectionStmParam*> connList = param->getStmConnectionList();
  for(int index=0; index<connList.size(); index++) {
    ConnectionStmParam* target = connList[index];
    if(target->getMode()==DB_MODE_DELETE || target->getMode()==DB_MODE_IGNORE) continue;

    vector<ElementStmParam*>::iterator sourceElem = find_if( elemList.begin(), elemList.end(), ElementStmParamComparator(target->getSourceId()));
    if(sourceElem== elemList.end()) continue;
    vector<ElementStmParam*>::iterator targetElem = find_if( elemList.begin(), elemList.end(), ElementStmParamComparator(target->getTargetId()));
    if(targetElem== elemList.end()) continue;
    //
    ConnectionNode* item = new ConnectionNode(0, 0, 0, 0);
    ElementNode* sourceNode = (*sourceElem)->getRealElem();
    ElementNode* targetNode = (*targetElem)->getRealElem();
    item->setSource(sourceNode);
    item->setTarget(targetNode);
    item->setPen(QPen(Qt::black, LINE_WIDTH));
    item->setData(Qt::UserRole, TYPE_CONNECTION);
    item->setZValue(-10);
    sourceNode->addToGroup(item);
    sourceNode->addConnection(item);
    targetNode->addConnection(item);
    item->reDrawConnection();
    connectionList_.push_back(item);
    item->setConnParam(target);
    sourceNode->removeFromGroup(item);
    if(item->getSource()->getElementType() == ELEMENT_DECISION ) {
      item->setText(target->getCondition());
    }
  }
}
/////
ItemList::ItemList(QWidget* parent) : QListWidget(parent) {
}
void ItemList::mousePressEvent(QMouseEvent* event) {
  if( event->button() == Qt::LeftButton) {
    startPos = event->pos();
  }
  QListWidget::mousePressEvent(event);
}

void ItemList::mouseMoveEvent(QMouseEvent* event) {
  if( event->buttons() == Qt::LeftButton) {
    int distance = (event->pos() - startPos).manhattanLength();
    if( QApplication::startDragDistance() <= distance ) {
      QModelIndexList indexes = selectionModel()->selection().indexes();
      QModelIndex selected = indexes.at(0);
      if( 0<=selected.row() ) {
        QByteArray itemData;
        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/StateMachineItem", itemData);
        mimeData->setText(item(selected.row())->text());
        mimeData->setProperty("CommandId", item(selected.row())->data(Qt::UserRole));
        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
      }
    }
  }
  QListWidget::mouseMoveEvent(event);
}
/////
StateMachineViewImpl::StateMachineViewImpl(QWidget* parent) : QWidget(parent), targetTask_(0) {
  lblTarget = new QLabel;
  lblTarget->setText("");

  btnDelete = new QPushButton(tr("Delete"));
  btnDelete->setEnabled(false);

  //btnEdit = new QPushButton("Edit");
  btnEdit = new QPushButton();
  btnEdit->setIcon(QIcon(":/Teaching/icons/Options.png"));
  btnEdit->setToolTip(tr("Edit"));
  btnEdit->setEnabled(false);

  btnRun = new QPushButton(tr("Run Command"));
  btnRun->setEnabled(false);

  QHBoxLayout* topLayout = new QHBoxLayout;
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->addWidget(lblTarget);
  topLayout->addStretch();
  topLayout->addWidget(btnRun);
  topLayout->addWidget(btnEdit);
  topLayout->addWidget(btnDelete);
  QFrame* frmTop = new QFrame;
  frmTop->setLayout(topLayout);

  btnTrans = new QPushButton();
  btnTrans->setText("Transition");
  btnTrans->setCheckable(true);
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawLine(0, 15, 30, 15);
  painter->drawLine(29, 15, 20, 5);
  painter->drawLine(29, 15, 20, 25);
  btnTrans->setIconSize(QSize(30,30));
  btnTrans->setIcon(QIcon(*pix));
  btnTrans->setStyleSheet("text-align:left;");
  btnTrans->setEnabled(false);

  lstItem = new ItemList();
  lstItem->setStyleSheet("background-color: rgb( 212, 206, 199 )};");
  lstItem->setEnabled(false);

  grhStateMachine = new EditorView(this);
  grhStateMachine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  grhStateMachine->setStyleSheet("background-color: white;");
  grhStateMachine->setAcceptDrops(true);
  grhStateMachine->setEnabled(false);
  setAcceptDrops(true);

  QLabel* lblGuard = new QLabel("Guard:");
  rdTrue = new QRadioButton("True");
  rdFalse = new QRadioButton("False");
  btnSet = new QPushButton("Set");
  rdTrue->setEnabled(false);
  rdFalse->setEnabled(false);
  btnSet->setEnabled(false);

  QVBoxLayout* itemLayout = new QVBoxLayout;
  itemLayout->setContentsMargins(0, 0, 0, 0);
  itemLayout->addWidget(btnTrans);
  itemLayout->addWidget(lstItem);
  QFrame* frmItem = new QFrame;
  frmItem->setLayout(itemLayout);

  QHBoxLayout* guardLayout = new QHBoxLayout;
  guardLayout->addStretch();
  guardLayout->addWidget(lblGuard);
  guardLayout->addWidget(rdTrue);
  guardLayout->addWidget(rdFalse);
  guardLayout->addWidget(btnSet);
  frmGuard = new QFrame;
  frmGuard->setLayout(guardLayout);

  QVBoxLayout* editorLayout = new QVBoxLayout;
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->addWidget(grhStateMachine);
  editorLayout->addWidget(frmGuard);
  QFrame* editorFrame = new QFrame;
  editorFrame->setLayout(editorLayout);

  QSplitter* splBase = new QSplitter(Qt::Horizontal);
  splBase->addWidget(frmItem);
  splBase->addWidget(editorFrame);
  splBase->setStretchFactor(0, 0);
  splBase->setStretchFactor(1, 1);
  splBase->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  splBase->show();
  //
  createInitialNodeTarget();
  createFinalNodeTarget();
  createDecisionNodeTarget();
  //createForkNodeTarget();
  //
  SettingManager::getInstance().loadSetting();
  commandList_ = TaskExecutor::instance()->getCommandDefList();
  vector<CommandDefParam*>::iterator itCmd = commandList_.begin();
  while (itCmd != commandList_.end() ) {
    createCommandNodeTarget((*itCmd)->getId(), (*itCmd)->getDispName());
    ++itCmd;
  }
  //
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frmTop);
  mainLayout->addWidget(splBase);
  setLayout(mainLayout);
  //
  connect(btnTrans, SIGNAL(toggled(bool)), this, SLOT(modeChanged()));
  connect(btnSet, SIGNAL(clicked()), this, SLOT(setClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
  connect(btnRun, SIGNAL(clicked()), this, SLOT(runClicked()));
}

void StateMachineViewImpl::setTaskParam(TaskModelParam* param) {
  this->targetTask_ = param;
  lblTarget->setText(param->getName());
  //
  btnTrans->setEnabled(true);
  lstItem->setEnabled(true);
  grhStateMachine->setEnabled(true);
  rdTrue->setEnabled(true);
  rdFalse->setEnabled(true);
  btnSet->setEnabled(true);
  btnEdit->setEnabled(true);
  btnDelete->setEnabled(true);
  btnRun->setEnabled(true);
  //
  grhStateMachine->setTaskParam(param);
  grhStateMachine->createStateMachine(param);
}

void StateMachineViewImpl::clearTaskParam() {
  this->targetTask_ = 0;
  lblTarget->setText("");
  //
  btnTrans->setEnabled(false);
  lstItem->setEnabled(false);
  grhStateMachine->removeAll();
  grhStateMachine->setEnabled(false);
  rdTrue->setEnabled(false);
  rdFalse->setEnabled(false);
  btnSet->setEnabled(false);
  btnRun->setEnabled(false);
}

void StateMachineViewImpl::createInitialNodeTarget() {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  painter->drawEllipse(5,5,20,20);

  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem("Start", lstItem);
  item->setIcon(QIcon(*pix));
}

void StateMachineViewImpl::createFinalNodeTarget() {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
  painter->setPen(QPen(Qt::black));
  painter->drawEllipse(5,5,20,20);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  painter->drawEllipse(10,10,10,10);

  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem("Final", lstItem);
  item->setIcon(QIcon(*pix));
}

void StateMachineViewImpl::createDecisionNodeTarget() {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
  QPolygon polygon;
  polygon << QPoint(15.0, 5.0) << QPoint(0.0, 15.0)
            << QPoint(15.0, 25.0) << QPoint(30.0, 15.0) << QPoint(15.0, 5.0);
  painter->drawPolygon(polygon);
  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem("Decision/Merge", lstItem);
  item->setIcon(QIcon(*pix));
}

void StateMachineViewImpl::createForkNodeTarget() {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawLine(0, 15, 30, 15);
  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem("Fork/Join", lstItem);
  item->setIcon(QIcon(*pix));
}

void StateMachineViewImpl::createCommandNodeTarget(int id, QString name) {
  QPixmap *pix = new QPixmap(30,30);
  pix->fill( QColor(212, 206, 199) );
  QPainter* painter = new QPainter(pix);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(Qt::black, 3.0));
  painter->drawEllipse(5, 5, 20, 20);
  lstItem->setIconSize(QSize(30,30));
  QListWidgetItem* item = new QListWidgetItem(name, lstItem);
  item->setIcon(QIcon(*pix));
  item->setData(Qt::UserRole, id);
}

void StateMachineViewImpl::setClicked() {
  ConnectionNode* conn = grhStateMachine->getCurrentConnection();
  if(conn) {
    if( conn->getSource()->getElementType()==ELEMENT_DECISION ) {
      if( rdTrue->isChecked() ) {
        conn->setText("true");
        conn->getConnParam()->setCondition("true");
      } else {
        conn->setText("false");
        conn->getConnParam()->setCondition("false");
      }
      conn->getConnParam()->setUpdate();
    }
  }
}

void StateMachineViewImpl::modeChanged() {
  grhStateMachine->setCntMode(btnTrans->isChecked());
}

void StateMachineViewImpl::deleteClicked() {
  grhStateMachine->deleteCurrent();
}

void StateMachineViewImpl::editClicked() {
  ElementNode* target = grhStateMachine->getCurrentNode();
  if(target) {
    ElementStmParam* targetStm = target->getElemParam();
    if(targetStm->getType()!=ELEMENT_COMMAND) {
      QMessageBox::warning(this, tr("Command"), tr("Please select Command Element."));
      return;
    }
    if( targetStm->getArgList().size()==0 ) {
      QString strCmd = targetStm->getCmdName();
      for(int index=0; index<commandList_.size(); index++) {
        CommandDefParam* param = commandList_[index];
        if(param->getName()==strCmd) {
          vector<ArgumentDefParam*> argList = param->getArgList();
          for(int idxArg=0; idxArg<argList.size(); idxArg++) {
            ArgumentDefParam* arg = argList[idxArg];
            ArgumentParam* argParam = new ArgumentParam(-1, targetStm->getId(), idxArg+1, QString::fromStdString(arg->getName()), "");
            argParam->setNew();
            targetStm->addArgument(argParam);
          }
        }
      }
    }

    ArgumentDialog dialog(targetTask_, targetStm, this);
    dialog.exec();
  } else {
    QMessageBox::warning(this, tr("Command"), tr("Please select Target Command."));
    return;
  }
}

void StateMachineViewImpl::runClicked() {
  ElementNode* target = grhStateMachine->getCurrentNode();
  if(target) {
    ElementStmParam* targetStm = target->getElemParam();
    if(targetStm->getType()!=ELEMENT_COMMAND) {
      QMessageBox::warning(this, tr("Run Command"), tr("Please select Command Element."));
      return;
    }
    //
    Calculator* calculator = new Calculator();
    std::vector<CompositeParamType> parameterList;
    //à¯êîÇÃëgÇ›óßÇƒ
    for(int idxArg=0; idxArg<targetStm->getArgList().size();idxArg++) {
      ArgumentParam* arg = targetStm->getArgList()[idxArg];
      QString valueDesc = arg->getValueDesc();
      //
      if(targetStm->getCommadDefParam()==0) {
        delete calculator;
        QMessageBox::warning(this, tr("Run Command"), tr("Target Comannd is NOT EXIST."));
        return;
      }
      ArgumentDefParam* argDef = targetStm->getCommadDefParam()->getArgList()[idxArg];
      if(argDef->getType() == "double") {
        bool ret = calculator->calculate(valueDesc, targetTask_);
        if(calculator->getValMode()==VAL_SCALAR) {
          DDEBUG_V("name : %s, %f", arg->getName().toStdString().c_str(), calculator->getResultScalar());
          parameterList.push_back(calculator->getResultScalar());
        } else {
          DDEBUG_V("name : %s = %f, %f, %f", arg->getName().toStdString().c_str(), calculator->getResultVector()[0], calculator->getResultVector()[1], calculator->getResultVector()[2]);
          parameterList.push_back(calculator->getResultVector());
        }

      } else {
        vector<ParameterParam*> paramList = targetTask_->getParameterList();
        vector<ParameterParam*>::iterator targetParam = find_if( paramList.begin(), paramList.end(), ParameterParamComparatorByRName(valueDesc));
        QString strVal;
        if(targetParam != paramList.end()) {
          strVal = QString::fromStdString( (*targetParam)->getValues(0) );
        } else {
          strVal = valueDesc;
        }
        if(argDef->getType() == "int") {
          parameterList.push_back(strVal.toInt());
        } else {
          parameterList.push_back(strVal.toStdString());
        }
      }
    }
    TaskExecutor::instance()->setRootName(SettingManager::getInstance().getRobotModelName());
    bool cmdRet = TaskExecutor::instance()->executeCommand(targetStm->getCmdName().toStdString(), parameterList, false);
    if(cmdRet) {
      QMessageBox::information(this, tr("Run Command"), tr("Target Command is FINISHED."));
    } else {
      QMessageBox::information(this, tr("Run Command"), tr("Target Command is FAILED."));
    }
  }
}
/////
StateMachineView::StateMachineView(): viewImpl(0) {
    setName("State Machine");
    setDefaultLayoutArea(View::BOTTOM);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewImpl = new StateMachineViewImpl(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(viewImpl);
    setLayout(vbox);
}

StateMachineView::~StateMachineView() {
};

}
