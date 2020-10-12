#include "../src/scheme/parser.hpp"
#include <gtest/gtest.h>
template<class R>
struct Action {
  template<class T>
  static void apply(const T& in) {
  }
};
TEST(llvmtest, schemesexpr){
  using RuleSExpr = tao::pegtl::must<scheme::SExpr>;
  using RuleSymbol = tao::pegtl::must<scheme::Symbol>;
  tao::pegtl::memory_input<> in0("def", "");
  tao::pegtl::memory_input<> in1("(def a b c)", "");
  tao::pegtl::memory_input<> in2("(def a \"b\" (x y z))", "");
  tao::pegtl::parse<RuleSymbol, Action>(in0);
  tao::pegtl::parse<RuleSExpr, Action>(in1);
  tao::pegtl::parse<RuleSExpr, Action>(in2);
}