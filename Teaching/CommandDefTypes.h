#ifndef TEACHING_COMMAND_DEF_TYPES_H_INCLUDED
#define TEACHING_COMMAND_DEF_TYPES_H_INCLUDED

#include <string>
#include <vector>
#include <QString>

namespace teaching {

class ArgumentDefParam {
public:
  ArgumentDefParam(std::string name, std::string type, int length)
    : name_(name), type_(type), length_(length), direction_(0) {};
  ArgumentDefParam(std::string name, std::string type, int length, int direction)
    : name_(name), type_(type), length_(length), direction_(direction) {};

  inline std::string getName() const { return this->name_; }
  inline std::string getType() const { return this->type_; }
  inline int getLength() const { return this->length_; }
  inline int getDirection() const { return this->direction_; }

private:
  std::string name_;
  std::string type_;
  int length_;
  int direction_;
};

class CommandDefParam {
public:
  CommandDefParam(QString name, QString dispName, QString retType)
    : cmdName_(name), dispName_(dispName), retType_(retType) {};

  virtual ~CommandDefParam() {
    std::vector<ArgumentDefParam*>::iterator itArg = argumentList_.begin();
    while (itArg != argumentList_.end()) {
      delete *itArg;
      ++itArg;
    }
    argumentList_.clear();
  };

  inline QString getCmdName() const { return this->cmdName_; }
  inline QString getDispName() const { return this->dispName_; }
  inline QString getRetType() const { return this->retType_; }
  inline std::vector<ArgumentDefParam*> getArgList() const { return this->argumentList_; }

  inline void addArgument(ArgumentDefParam* target){ this->argumentList_.push_back(target); }

private:
  QString cmdName_;
  QString dispName_;
  QString retType_;

  std::vector<ArgumentDefParam*> argumentList_;
};

struct CommandDefParamComparator {
  QString name_;

  CommandDefParamComparator(QString value) {
    name_ = value;
  }
  bool operator()(const CommandDefParam* elem) const {
    return elem->getCmdName() == name_;
  }
};

}
#endif
