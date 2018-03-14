//
// Linux
// g++ -o testParser testParser.cpp -std=c++11
//

#include "Parser.h"

namespace teaching {

  class tostr_visitor : public boost::static_visitor<std::string>
  {
  public:
    std::string operator()(ValueNodeSp const& node) const
    {
      return boost::lexical_cast<std::string>(node->v_);
    }
    std::string operator()(Vector3dConstNodeSp const& node) const
    {
      return (boost::format("[%1%,%2%,%3%]")
              % boost::apply_visitor(*this, node->x_)
              % boost::apply_visitor(*this, node->y_)
              % boost::apply_visitor(*this, node->z_)).str();
    }
    std::string operator()(Vector6dConstNodeSp const& node) const
    {
      return (boost::format("[%1%,%2%,%3%,%4%,%5%,%6%]")
              % boost::apply_visitor(*this, node->x_)
              % boost::apply_visitor(*this, node->y_)
              % boost::apply_visitor(*this, node->z_)
              % boost::apply_visitor(*this, node->Rx_)
              % boost::apply_visitor(*this, node->Ry_)
              % boost::apply_visitor(*this, node->Rz_)).str();
    }
    std::string operator()(BinOpNodeSp const& node) const
    {
      return (boost::format("(%1%%2%%3%)")
              % boost::apply_visitor(*this, node->lhs_)
              % node->op_
              % boost::apply_visitor(*this, node->rhs_)).str();
    }
    std::string operator()(VariableNodeSp const& node) const
    {
      return boost::lexical_cast<std::string>(node->nm_);
    }
    std::string operator()(FunCallNodeSp const& node) const
    {
      return (boost::format("%1%(%2%)")
              % boost::apply_visitor(*this, node->fun_)
              % boost::apply_visitor(*this, node->args_)).str();

    }
  };
}

namespace qi = boost::spirit::qi;

int main()
{
  std::string str;
  teaching::Node result;
  teaching::makeTree<std::string::iterator> parser;
  std::vector<teaching::Node> nodeList;
  std::string prompt = "> ";

  std::cout << prompt;
  while (getline(std::cin, str)) {
    if (qi::phrase_parse(str.begin(), str.end(), parser, qi::space, result)) {
      std::cout << boost::apply_visitor(teaching::tostr_visitor(), result) << std::endl;
    } else {
      std::cout << parser.getErrorMessage() << std::endl;
      parser.clearErrorMessage();
    }

    std::cout << prompt;
  }

  return 0;
}
