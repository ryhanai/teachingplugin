#ifndef TEACHING_PARSER_H_INCLUDED
#define TEACHING_PARSER_H_INCLUDED

#include <boost/config/warning_disable.hpp>

#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/spirit/include/support_utree.hpp>


namespace teaching {

  class ValueNode;
  class Vector3dConstNode;
  class Vector6dConstNode;
  class BinOpNode;
  class VariableNode;
  class FunCallNode;

  typedef boost::shared_ptr<ValueNode> ValueNodeSp;
  typedef boost::shared_ptr<Vector3dConstNode> Vector3dConstNodeSp;
  typedef boost::shared_ptr<Vector6dConstNode> Vector6dConstNodeSp;
  typedef boost::shared_ptr<BinOpNode> BinOpNodeSp;
  typedef boost::shared_ptr<VariableNode> VariableNodeSp;
  typedef boost::shared_ptr<FunCallNode> FunCallNodeSp;
  typedef boost::variant<ValueNodeSp, Vector3dConstNodeSp, Vector6dConstNodeSp, BinOpNodeSp, VariableNodeSp, FunCallNodeSp> Node;

  class ValueNode {
  public:
    static Node create(double v) { return ValueNodeSp(new ValueNode(v)); }
  private:
  ValueNode(double v) :v_(v) {}
    double v_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

  class Vector3dConstNode {
  public:
    static Node create(Node const& x, Node const& y, Node const& z) { return Vector3dConstNodeSp(new Vector3dConstNode(x, y, z)); }
  private:
  Vector3dConstNode(Node const&x, Node const& y, Node const& z) :x_(x), y_(y), z_(z) {}
    Node x_;
    Node y_;
    Node z_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

  class Vector6dConstNode {
  public:
    static Node create(Node const& x, Node const& y, Node const& z, Node const& rx, Node const& ry, Node const& rz) { return Vector6dConstNodeSp(new Vector6dConstNode(x, y, z, rx, ry, rz)); }
  private:
  Vector6dConstNode(Node const& x, Node const& y, Node const& z, Node const& rx, Node const& ry, Node const& rz) :x_(x), y_(y), z_(z), Rx_(rx), Ry_(ry), Rz_(rz) {}
    Node x_;
    Node y_;
    Node z_;
    Node Rx_;
    Node Ry_;
    Node Rz_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

  class BinOpNode {
  public:
    static Node create(std::string const& op, Node const& lhs, Node const& rhs) { return BinOpNodeSp(new BinOpNode(op, lhs, rhs)); }
  private:
  BinOpNode(std::string const& op, Node const& lhs, Node const& rhs) :op_(op), lhs_(lhs), rhs_(rhs) {}
    std::string op_;
    Node lhs_;
    Node rhs_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

  class VariableNode {
  public:
    static Node create(std::string const& nm) { return VariableNodeSp(new VariableNode(nm)); }
  private:
  VariableNode(std::string const& nm) :nm_(nm) {}
    std::string nm_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

  class FunCallNode {
  public:
    static Node create(Node const& fun, Node const& args) { return FunCallNodeSp(new FunCallNode(fun, args)); }
  private:
  FunCallNode(Node const& fun, Node const& args) :fun_(fun), args_(args) {}
    Node fun_;
    Node args_;
    friend class evaluator;
    friend class Calculator;
    friend class tostr_visitor;
  };

// parser
  namespace qi = boost::spirit::qi;
  namespace phx = boost::phoenix;

  template<typename Iterator>
    struct makeTree : qi::grammar<Iterator, Node(), qi::space_type> {
    qi::rule<Iterator, Node(), qi::space_type> topexpr, expr,
      logical_and_expr, equality_expr, relational_expr, additive_expr,
      term, factor, variable, literal, args, vector3d_const, vector6d_const;
  makeTree() : makeTree::base_type(topexpr) {
      topexpr = expr > qi::eoi;
#if _MSC_VER >= 1900
      const char* str_plus = "+";
      const char* str_minus = "-";
      const char* str_astarisc = "*";
      const char* str_slash = "/";
      const char* str_comma = ",";
      const char* str_lor = "||";
      const char* str_land = "&&";
      const char* str_eq = "==";
      const char* str_neq = "!=";
      const char* str_le = "<=";
      const char* str_ge = ">=";
      const char* str_lt = "<";
      const char* str_gt = ">";

      expr = logical_and_expr[qi::_val = qi::_1]
        > *("||" > logical_and_expr[qi::_val = phx::bind(&BinOpNode::create, str_lor, qi::_val, qi::_1)]);
      logical_and_expr = equality_expr[qi::_val = qi::_1]
        > *("&&" > equality_expr[qi::_val = phx::bind(&BinOpNode::create, str_land, qi::_val, qi::_1)]);
      equality_expr = relational_expr[qi::_val = qi::_1]
        > *(("==" > relational_expr[qi::_val = phx::bind(&BinOpNode::create, str_eq, qi::_val, qi::_1)])
            | ("!=" > relational_expr[qi::_val = phx::bind(&BinOpNode::create, str_neq, qi::_val, qi::_1)]));
      relational_expr = additive_expr[qi::_val = qi::_1]
        > *(("<=" > additive_expr[qi::_val = phx::bind(&BinOpNode::create, str_le, qi::_val, qi::_1)])
            | (">=" > additive_expr[qi::_val = phx::bind(&BinOpNode::create, str_ge, qi::_val, qi::_1)])
            | ('<' > additive_expr[qi::_val = phx::bind(&BinOpNode::create, str_lt, qi::_val, qi::_1)])
            | ('>' > additive_expr[qi::_val = phx::bind(&BinOpNode::create, str_gt, qi::_val, qi::_1)]));
      additive_expr = term[qi::_val = qi::_1]
        > *(('+' > term[qi::_val = phx::bind(&BinOpNode::create, str_plus, qi::_val, qi::_1)])
            | ('-' > term[qi::_val = phx::bind(&BinOpNode::create, str_minus, qi::_val, qi::_1)]));

      expr = term[qi::_val = qi::_1]
        > *(('+' > term[qi::_val = phx::bind(&BinOpNode::create, str_plus, qi::_val, qi::_1)])
            | ('-' > term[qi::_val = phx::bind(&BinOpNode::create, str_minus, qi::_val, qi::_1)]));
      term = factor[qi::_val = qi::_1]
        > *(('*' > factor[qi::_val = phx::bind(&BinOpNode::create, str_astarisc, qi::_val, qi::_1)])
            | ('/' > factor[qi::_val = phx::bind(&BinOpNode::create, str_slash, qi::_val, qi::_1)]));
      factor = literal[qi::_val = qi::_1]
        | vector3d_const[qi::_val = qi::_1]
        | vector6d_const[qi::_val = qi::_1]
        | ('(' > expr > ')')[qi::_val = qi::_1]
        | (variable >> '(' > args > ')')[qi::_val = phx::bind(&FunCallNode::create, qi::_1, qi::_2)]
        | variable[qi::_val = qi::_1];
      args = expr[qi::_val = qi::_1]
        > *(',' > expr[qi::_val = phx::bind(&BinOpNode::create, str_comma, qi::_val, qi::_1)]);
#else
      // <expr> corresponds to <logical-or-expr> in C
      expr = logical_and_expr[qi::_val = qi::_1]
        > *("||" > logical_and_expr[qi::_val = phx::bind(&BinOpNode::create, "||", qi::_val, qi::_1)]);
      logical_and_expr = equality_expr[qi::_val = qi::_1]
        > *("&&" > equality_expr[qi::_val = phx::bind(&BinOpNode::create, "&&", qi::_val, qi::_1)]);
      equality_expr = relational_expr[qi::_val = qi::_1]
        > *(("==" > relational_expr[qi::_val = phx::bind(&BinOpNode::create, "==", qi::_val, qi::_1)])
            | ("!=" > relational_expr[qi::_val = phx::bind(&BinOpNode::create, "!=", qi::_val, qi::_1)]));
      relational_expr = additive_expr[qi::_val = qi::_1]
        > *(("<=" > additive_expr[qi::_val = phx::bind(&BinOpNode::create, "<=", qi::_val, qi::_1)])
            | (">=" > additive_expr[qi::_val = phx::bind(&BinOpNode::create, ">=", qi::_val, qi::_1)])
            | ('<' > additive_expr[qi::_val = phx::bind(&BinOpNode::create, "<", qi::_val, qi::_1)])
            | ('>' > additive_expr[qi::_val = phx::bind(&BinOpNode::create, ">", qi::_val, qi::_1)]));
      additive_expr = term[qi::_val = qi::_1]
        > *(('+' > term[qi::_val = phx::bind(&BinOpNode::create, "+", qi::_val, qi::_1)])
            | ('-' > term[qi::_val = phx::bind(&BinOpNode::create, "-", qi::_val, qi::_1)]));
      term = factor[qi::_val = qi::_1]
        > *(('*' > factor[qi::_val = phx::bind(&BinOpNode::create, "*", qi::_val, qi::_1)])
            | ('/' > factor[qi::_val = phx::bind(&BinOpNode::create, "/", qi::_val, qi::_1)]));
      factor = literal[qi::_val = qi::_1]
        | vector3d_const[qi::_val = qi::_1]
        | vector6d_const[qi::_val = qi::_1]
        | ('(' > expr > ')')[qi::_val = qi::_1]
        | (variable >> '(' > args > ')')[qi::_val = phx::bind(&FunCallNode::create, qi::_1, qi::_2)]
        | variable[qi::_val = qi::_1];
      args = expr[qi::_val = qi::_1]
        > *(',' > expr[qi::_val = phx::bind(&BinOpNode::create, ",", qi::_val, qi::_1)]);
#endif
      vector3d_const = ('[' > expr > ',' > expr > ',' > expr > ']')
        [qi::_val = phx::bind(&Vector3dConstNode::create, qi::_1, qi::_2, qi::_3)];
      vector6d_const = ('[' > expr > ',' > expr > ',' > expr > ',' > expr > ',' > expr > ',' > expr > ']')
        [qi::_val = phx::bind(&Vector6dConstNode::create, qi::_1, qi::_2, qi::_3, qi::_4, qi::_5, qi::_6)];
      variable = qi::as_string[qi::lexeme[(qi::alpha | qi::char_('_') | qi::char_('.')) > *(qi::alnum | qi::char_('_') | qi::char_('.'))]]
        [qi::_val = phx::bind(&VariableNode::create, qi::_1)];
      literal = qi::double_[qi::_val = phx::bind(&ValueNode::create, qi::_1)];
      qi::on_error<qi::fail>(
        topexpr,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        expr,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        term,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        factor,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        args,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        vector3d_const,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        vector6d_const,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        variable,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
      qi::on_error<qi::fail>(
        literal,
        std::cerr << phx::val("Expecting : ") << qi::_4 << phx::val(" here: \"")
        << phx::construct<std::string>(qi::_3, qi::_2) << phx::val("\"")
        << phx::val(" in \"") << phx::construct<std::string>(qi::_1, qi::_2) << phx::val("\"")
        << std::endl
        );
    }
  };

}
#endif
