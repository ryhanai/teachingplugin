#ifndef TEACHING_CHOREONOID_UTIL_H_INCLUDED
#define TEACHING_CHOREONOID_UTIL_H_INCLUDED

#include <cnoid/BodyItem>
#include <cnoid/RootItem>
#include "TeachingTypes.h"

namespace teaching {

class ChoreonoidUtil {
public:
  static bool updateModelItemPosition(const cnoid::BodyItemPtr& target, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz);
  static void showAllModelItem();

  static void selectTreeItem(ModelParamPtr target);
	static void selectTreeItem(ModelMasterParamPtr target);
	static void deselectTreeItem();
  static void updateScene();

  static cnoid::BodyItem* searchParentModel(const std::string targetName);

};

}
#endif
