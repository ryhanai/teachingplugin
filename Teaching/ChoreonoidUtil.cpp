#include "ChoreonoidUtil.h"
#include <cnoid/RootItem>
#include <cnoid/EigenUtil>
#include <cnoid/InverseKinematics>
#include <cnoid/ItemTreeView>
#include <cnoid/SceneView>
#include <cnoid/KinematicsBar>
#include <cnoid/PenetrationBlocker>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include "TeachingUtil.h"
#include <cnoid/MessageView>  /* modified by qtconv.rb 0th rule*/  

#include "LoggerUtil.h"

using namespace std;
using namespace cnoid;

namespace teaching {

bool ChoreonoidUtil::readModelItem(ModelMasterParamPtr target, QString& fileName) {
  //loadBodyItem
  BodyItemPtr item = new BodyItem();
  if (item->load(fileName.toStdString()) == false) return false;
  target->setModelItem(item);
  return true;
}

bool ChoreonoidUtil::makeModelItem(ModelMasterParamPtr target) {
	if (target->getModelItem() != 0) return true;
	/////
  QString fileName = target->getFileName();
  if (fileName.length() == 0) return false;
  //
  QStringList saved;
  QString strPath = QFileInfo(".").absolutePath();
  QDir dir = QDir(strPath + QString("/work"));
  if (dir.exists() == false) dir.mkdir(strPath + QString("/work"));
  QString strModel = strPath + QString("/work/") + fileName;
  QFile file(strModel);
  file.open(QIODevice::WriteOnly);
  file.write(target->getData());
  file.close();
  saved.append(strModel);
  //
  vector<ModelDetailParamPtr> detailList = target->getModelDetailList();
  for (int index = 0; index < detailList.size(); index++) {
		ModelDetailParamPtr detail = detailList[index];
    QString strDetail = strPath + QString("/work/") + detail->getFileName();
    QFile fileDetail(strDetail);
    fileDetail.open(QIODevice::WriteOnly);
    fileDetail.write(detail->getData());
    fileDetail.close();
    saved.append(strDetail);
  }
  //
  //onSigOptionsParsed
  BodyItemPtr item = new BodyItem();
  if (item->load(strModel.toStdString())) {
    if (item->name().empty()) {
      item->setName(item->body()->modelName());
    }
    target->setModelItem(item);
  }
  //
  for (int index = 0; index < saved.size(); index++) {
    QFile::remove(saved[index]);
  }
  dir.rmdir(strPath + QString("/work"));
	//
	target->setModelItem(item);

  return true;
}

bool ChoreonoidUtil::loadTaskModelItem(TaskModelParamPtr target) {
  vector<ModelParamPtr> modelList = target->getModelList();
  for (int index = 0; index < modelList.size(); index++) {
		ModelParamPtr model = modelList[index];
    if (model->getMode() == DB_MODE_DELETE || model->getMode() == DB_MODE_IGNORE) continue;
    if (loadModelItem(model) == false) return false;
  }
  ItemTreeView::mainInstance()->update();
  target->setModelLoaded(true);
  return true;
}

bool ChoreonoidUtil::unLoadTaskModelItem(TaskModelParamPtr target) {
	if (target->IsModelLoaded() == false) return true;

  vector<ModelParamPtr> modelList = target->getModelList();
  for (int index = 0; index < modelList.size(); index++) {
		ModelParamPtr model = modelList[index];
    if (model->getMode() == DB_MODE_DELETE || model->getMode() == DB_MODE_IGNORE) continue;
    if (unLoadModelItem(model) == false) return false;
  }
  target->setModelLoaded(false);
  return true;
}

cnoid::BodyItem* ChoreonoidUtil::searchParentModel(const std::string targetName) {
  ItemList<BodyItem> bodyItems;
  bodyItems.extractChildItems(RootItem::instance());
  for (size_t i = 0; i < bodyItems.size(); i++) {
    BodyItem* item = bodyItems.get(i);
    if (item->name() == targetName) {
      return item;
    }
  }

  return NULL;
}

bool ChoreonoidUtil::loadModelItem(ModelParamPtr target) {
	DDEBUG("ChoreonoidUtil::loadModelItem");
	if (target == NULL)  return false;
  if (target->getModelMaster()->getModelItem()) {
    string robotModel = SettingManager::getInstance().getRobotModelName();
    BodyItemPtr item = target->getModelMaster()->getModelItem();
    ChoreonoidUtil::updateModelItemPosition(item,
      target->getPosX(), target->getPosY(), target->getPosZ(),
      target->getRotRx(), target->getRotRy(), target->getRotRz());
    if (target->getType() == MODEL_EE) {
      BodyItem* parentModel = searchParentModel(robotModel);
      if (parentModel) {
        parentModel->addChildItem(item);
      }
    } else {
      RootItem::mainInstance()->addChildItem(item);
    }
  }
  return true;
}

bool ChoreonoidUtil::unLoadModelItem(ModelParamPtr target) {
	DDEBUG("ChoreonoidUtil::unLoadModelItem");
	if (target->getModelMaster()->getModelItem()) {
    target->getModelMaster()->getModelItem()->detachFromParentItem();
  }
  return true;
}


bool ChoreonoidUtil::loadModelMasterItem(ModelMasterParamPtr target) {
	if (target == NULL)  return false;
	if (target->getModelItem() == 0) {
		if (makeModelItem(target) == 0) {
			target->setModelItem(0);
		}
	}
	if (target->getModelItem()) {
		string robotModel = SettingManager::getInstance().getRobotModelName();
		BodyItemPtr item = target->getModelItem();
		RootItem::mainInstance()->addChildItem(item);
	}
	return true;
}

bool ChoreonoidUtil::unLoadModelMasterItem(ModelMasterParamPtr target) {
	if (target->getModelItem()) {
		target->getModelItem()->detachFromParentItem();
	}
	return true;
}

bool ChoreonoidUtil::updateModelItemPosition(const BodyItemPtr& target, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz) {
  //BodyLinkViewImpl::doInverseKinematics
  Vector3 pos, rpy;
  pos[0] = posX; pos[1] = posY; pos[2] = posZ;
  rpy[0] = rotRx * PI / 180.0;
  rpy[1] = rotRy * PI / 180.0;
  rpy[2] = rotRz * PI / 180.0;

  Link* currentLink = target->body()->rootLink();
  Matrix3 R = currentLink->calcRfromAttitude(rotFromRpy(rpy));

  InverseKinematicsPtr ik = target->getCurrentIK(currentLink);
  if (ik) {
    target->beginKinematicStateEdit();

    if (KinematicsBar::instance()->isPenetrationBlockMode()) {
      PenetrationBlockerPtr blocker = target->createPenetrationBlocker(currentLink, true);
      if (blocker) {
        Vector3 p;
        p.x() = posX; p.y() = posY; p.z() = posZ;
        Position T;
        T.translation() = p;
        T.linear() = R;
        if (blocker->adjust(T, Vector3(p - currentLink->p()))) {
          p = T.translation();
          R = T.linear();
        }
      }
    }

    if (ik->calcInverseKinematics(pos, R)) {
      target->notifyKinematicStateChange(true);
      target->acceptKinematicStateEdit();
    }
  }
  return true;
}

void ChoreonoidUtil::showAllModelItem() {
	DDEBUG("ChoreonoidUtil::showAllModelItem");
	ItemTreeView::mainInstance()->update();
  ItemTreeView::mainInstance()->selectAllItems();
  cnoid::ItemList<cnoid::Item> itemList = ItemTreeView::mainInstance()->selectedItems();
  for (int index = 0; index < itemList.size(); index++) {
    ItemTreeView::mainInstance()->checkItem(itemList[index].get(), true);
  }
  ItemTreeView::mainInstance()->clearSelection();
}

void ChoreonoidUtil::selectTreeItem(ModelParamPtr target) {
	DDEBUG("ChoreonoidUtil::selectTreeItem");
	ItemTreeView::mainInstance()->clearSelection();
  if (target->getModelMaster()->getModelItem()) {
    ItemTreeView::mainInstance()->selectItem(target->getModelMaster()->getModelItem());
  }
}

void ChoreonoidUtil::selectTreeItem(ModelMasterParamPtr target) {
	ItemTreeView::mainInstance()->clearSelection();
	if (target->getModelItem()) {
		ItemTreeView::mainInstance()->selectItem(target->getModelItem());
	}
}

void ChoreonoidUtil::deselectTreeItem() {
  ItemTreeView::mainInstance()->clearSelection();
}

void ChoreonoidUtil::updateScene() {
  QCoreApplication::sendPostedEvents();
}

}
