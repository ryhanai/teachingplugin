#ifndef TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED
#define TEACHING_TEACHING_TASKEXECUTOR_H_INCLUDED

#include "TeachingTypes.h"
#include "ControllerBase.h"

namespace teaching {

class AttachedModel {
public:
  AttachedModel() {};
  AttachedModel(AttachedModel& source)
    : object(source.object), target(source.target) {
  };
  cnoid::BodyItemPtr object;
  int target;
};

class TaskExecutor {
public:
  static TaskExecutor* instance();
  ~TaskExecutor();

  std::vector<CommandDefParam*> getCommandDefList();
  CommandDefParam* getCommandDef(const std::string& commandName);
  CommandDefParam* getCommandDef(const int commandId);

  void setRootName(std::string value);
  bool executeCommand(const std::string& commandName, std::vector<CompositeParamType>& params, bool simulation = true);
  bool attachModelItem(cnoid::BodyItemPtr object, int target);
  bool detachModelItem(cnoid::BodyItemPtr object, int target);
  bool detachAllModelItem();

private:
  TaskExecutor();
  std::vector<AttachedModel*> modelList;
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
