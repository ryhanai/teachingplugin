#ifndef TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED
#define TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED

#include "TeachingTypes.h"
#include "ControllerBase.h"

namespace teaching {

class AttachedModel {
public:
  AttachedModel() {};
  AttachedModel(AttachedModel& source)
    : parent(source.parent), child(source.child), target(source.target) {
  };
  cnoid::BodyItemPtr parent;
  cnoid::BodyItemPtr child;
  int target;
  AttachedItemsPtr item;
};
typedef std::shared_ptr<AttachedModel> AttachedModelPtr;

class TaskExecutor {
public:
  static TaskExecutor* instance();
  ~TaskExecutor();

  std::vector<CommandDefParam*> getCommandDefList();
  CommandDefParam* getCommandDef(const std::string& commandName);

  void setRootName(std::string value);
  bool executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params);
  bool attachModelItem(cnoid::BodyItemPtr parent, cnoid::BodyItemPtr child, int target);
  bool detachModelItem(cnoid::BodyItemPtr parent, cnoid::BodyItemPtr child, int target);
  bool detachAllModelItem();

private:
  TaskExecutor();
  std::vector<AttachedModelPtr> modelList;
  ControllerBase* handler_;
};

struct ParameterParamComparatorByRName {
  QString rname_;
  ParameterParamComparatorByRName(QString value) {
    rname_ = value;
  }
  bool operator()(const ParameterParamPtr elem) const {
    return elem->getRName() == rname_;
  }
};

}
#endif
