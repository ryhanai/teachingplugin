#ifndef TEACHING_CHOREONOID_UTIL_H_INCLUDED
#define TEACHING_CHOREONOID_UTIL_H_INCLUDED

#include <cnoid/BodyItem>
#include <cnoid/RootItem>
#include "TeachingTypes.h"

namespace teaching {

class ChoreonoidUtil {
public:
  static bool readModelItem(ModelParam* target, QString& fileName);
  static bool makeModelItem(ModelParam* target);

  static bool loadTaskModelItem(TaskModelParam* target);
  static bool unLoadTaskModelItem(TaskModelParam* target);

  static bool loadModelItem(ModelParam* target);
  static bool unLoadModelItem(ModelParam* target);
  static bool updateModelItemPosition(const cnoid::BodyItemPtr& target, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz);
  static void showAllModelItem();

  static void selectTreeItem(ModelParam* target);
  static void updateScene();

private:
  static cnoid::BodyItem* searchParentModel(const std::string targetName);

};

}
#endif
