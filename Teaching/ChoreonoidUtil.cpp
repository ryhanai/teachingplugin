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
  if(target->getModelItem()) {
    ItemTreeView::mainInstance()->selectItem(target->getModelItem());
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
