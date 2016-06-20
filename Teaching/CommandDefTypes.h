#ifndef TEACHING_COMMAND_DEF_TYPES_H_INCLUDED
#define TEACHING_COMMAND_DEF_TYPES_H_INCLUDED

#include <string>
#include <vector>
#include <QString>

namespace teaching {

class ArgumentDefParam {
public:
  ArgumentDefParam(std::string name, std::string type, int length)
    : name_(name), type_(type), length_(length) {};

  inline std::string getName() const { return this->name_; }
  inline std::string getType() const { return this->type_; }
  inline int getLength() const { return this->length_; }

private:
  std::string name_;
  std::string type_;
  int length_;
};

class CommandDefParam {
public:
  CommandDefParam(int id, QString name, QString dispName, QString retType)
    : id_(id), name_(name), dispName_(dispName), retType_(retType) {};

  virtual ~CommandDefParam() {
    std::vector<ArgumentDefParam*>::iterator itArg = argumentList_.begin();
    while (itArg != argumentList_.end() ) {
      delete *itArg;
      ++itArg;
    }
    argumentList_.clear();
  };

  inline int getId() const { return this->id_; }
  inline QString getName() const { return this->name_; }
  inline QString getDispName() const { return this->dispName_; }
  inline QString getRetType() const { return this->retType_; }
  inline std::vector<ArgumentDefParam*> getArgList() const { return this->argumentList_; }

  inline void addArgument(ArgumentDefParam* target){ this->argumentList_.push_back(target); }

private:
  int id_;
  QString name_;
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
    return elem->getName()==name_;
  }
};
}
#endif
