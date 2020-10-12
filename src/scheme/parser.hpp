#pragma once
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
namespace scheme {

template<char... c> using Char = tao::pegtl::one<c...>;

template<char l, char r, class R>
using Group = tao::pegtl::seq<
  Char<l>, R, Char<r>
>;

struct UnsignedNumber : tao::pegtl::plus<tao::pegtl::digit> {};

struct Number : tao::pegtl::seq<
  tao::pegtl::star<
    Char<'+', '-'>
  >,
  UnsignedNumber
>{};

struct String : Group<
  '"', '"', 
  tao::pegtl::star<tao::pegtl::not_one<'"'>>
>
{};

struct Literal : tao::pegtl::sor< Number, String > {};

template<class R> using Parentheses = Group<'(', ')', R>;

struct Symbol : public tao::pegtl::sor<
  tao::pegtl::identifier,
  Char<'+', '-', '*', '/', '<', '>'>,
  TAO_PEGTL_STRING("&&"), TAO_PEGTL_STRING("||"),
  TAO_PEGTL_STRING("=="),
  TAO_PEGTL_STRING("<="), TAO_PEGTL_STRING(">=")
> {};
struct SExpr;
struct SExpr : public Parentheses<
  tao::pegtl::seq<
    tao::pegtl::pad<
      tao::pegtl::sor<
        Symbol,
        SExpr
      >,
      tao::pegtl::space
    >,
    tao::pegtl::list<
      tao::pegtl::sor<
        Symbol,
        Literal,
        SExpr
      >,
      tao::pegtl::space
    >
  >
> {};

struct File : public tao::pegtl::until<
  tao::pegtl::eof,
  tao::pegtl::pad<SExpr, tao::pegtl::eol>
> {};
template<class Rule>
using ASTSelector = tao::pegtl::parse_tree::selector<
  Rule,
  tao::pegtl::parse_tree::store_content::on<
    SExpr, Symbol, String, Number
  >
>;
}