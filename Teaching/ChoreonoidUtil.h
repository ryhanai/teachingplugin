#ifndef TEACHING_CHOREONOID_UTIL_H_INCLUDED
#define TEACHING_CHOREONOID_UTIL_H_INCLUDED

#include <cnoid/BodyItem>
#include <cnoid/RootItem>
#include "TeachingTypes.h"

namespace teaching {

class ChoreonoidUtil {
public:
  static bool readModelItem(ModelMasterParamPtr target, QString& fileName);
  static bool makeModelItem(ModelMasterParamPtr target);

  static bool loadTaskModelItem(TaskModelParamPtr target);
  static bool unLoadTaskModelItem(TaskModelParamPtr target);

  static bool loadModelItem(ModelParamPtr target);
  static bool unLoadModelItem(ModelParamPtr target);

	static bool loadModelMasterItem(ModelMasterParamPtr target);
	static bool unLoadModelMasterItem(ModelMasterParamPtr target);

  static void replaceMaster(ModelParamPtr source, ModelMasterParamPtr target);

  static bool updateModelItemPosition(const cnoid::BodyItemPtr& target, double posX, double posY, double posZ, double rotRx, double rotRy, double rotRz);
  static void showAllModelItem();

  static void selectTreeItem(ModelParamPtr target);
	static void selectTreeItem(ModelMasterParamPtr target);
	static void deselectTreeItem();
  static void updateScene();

private:
  static cnoid::BodyItem* searchParentModel(const std::string targetName);

};

}
#endif
