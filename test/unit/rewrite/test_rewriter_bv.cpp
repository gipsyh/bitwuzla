/***
 * Bitwuzla: Satisfiability Modulo Theories (SMT) solver.
 *
 * Copyright (C) 2022 by the authors listed in the AUTHORS file at
 * https://github.com/bitwuzla/bitwuzla/blob/main/AUTHORS
 *
 * This file is part of Bitwuzla under the MIT license. See COPYING for more
 * information at https://github.com/bitwuzla/bitwuzla/blob/main/COPYING
 */

#include <gtest/gtest.h>

#include <cmath>

#include "bv/bitvector.h"
#include "node/node_manager.h"
#include "rewrite/rewriter.h"
#include "solver/fp/floating_point.h"
#include "test/unit/rewrite/test_rewriter.h"

namespace bzla::test {

using namespace bzla::node;

class TestRewriterBv : public TestRewriter
{
 protected:
  void test_elim_rule_bv(node::Kind kind,
                         const std::vector<uint64_t> indices = {})
  {
    for (uint64_t i = 0; i < d_bv_sizes.size(); ++i)
    {
      std::vector<uint64_t> idxs = {};
      if (!indices.empty()) idxs.push_back(indices[i]);
      test_elim_rule(kind, d_nm.mk_bv_type(d_bv_sizes[i]), idxs);
    }
  }
  template <RewriteRuleKind K>
  void test_eval_elim_rule_bv(const Node& node, const Node& expected)
  {
    test_rewrite(node, expected);
    test_rewrite(d_rewriter.eval(node), d_rewriter.rewrite(node));
  }

  std::vector<uint64_t> d_bv_sizes = {1, 2, 3, 4, 8};
};

/* bvadd -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_add_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_ADD, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_ADD, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
}

TEST_F(TestRewriterBv, bv_add_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_CONST;
  Node a_val                     = d_nm.mk_value(BitVector::from_ui(4, 5));
  Node b_val                     = d_nm.mk_value(BitVector::from_ui(4, 1));
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), b_val}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {b_val, d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), a_val}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_add_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_add_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ADD, {d_bv1_a, d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_add_not)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_NOT;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_NOT, {d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_NOT, {d_bv4_a}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv1_a, d_nm.mk_node(Kind::BV_NOT, {d_bv1_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_NOT, {d_bv1_a}), d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_NOT, {d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_add_neg)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_NEG;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_NEG, {d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv1_a, d_nm.mk_node(Kind::BV_NEG, {d_bv1_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_NEG, {d_bv1_a}), d_bv1_a}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ADD,
                   {d_bv4_a,
                    RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                        d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}))
                        .first}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_add_urem)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_UREM;
  //// applies
  // (bvadd a (bvneg (bvmul (bvudiv a b) b)))
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_MUL,
               {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_MUL,
               {d_bv4_b, d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_MUL,
               {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_MUL,
               {d_bv4_b, d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})})}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
           d_rewriter,
           d_nm.mk_node(
               Kind::BV_NEG,
               {d_nm.mk_node(Kind::BV_MUL,
                             {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}),
                              d_bv4_b})}))
           .first}));
  // (bvadd a (bvmul (bvneg (bvudiv a b)) b)))
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {d_nm.mk_node(Kind::BV_NEG,
                         {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})}),
            d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_MUL,
                    {d_bv4_b,
                     d_nm.mk_node(Kind::BV_NEG,
                                  {d_nm.mk_node(Kind::BV_UDIV,
                                                {d_bv4_a, d_bv4_b})})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(
           Kind::BV_MUL,
           {d_nm.mk_node(Kind::BV_NEG,
                         {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})}),
            d_bv4_b}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(
           Kind::BV_MUL,
           {d_bv4_b,
            d_nm.mk_node(Kind::BV_NEG,
                         {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})})}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                d_rewriter,
                d_nm.mk_node(Kind::BV_NEG,
                             {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})}))
                .first,
            d_bv4_b})}));
  // (bvadd a (bvmul (bvudiv a b)) (bvneg b))))
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_MUL,
                    {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}),
                     d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_MUL,
                    {d_nm.mk_node(Kind::BV_NEG, {d_bv4_b}),
                     d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::BV_MUL,
                    {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}),
                     d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::BV_MUL,
                    {d_nm.mk_node(Kind::BV_NEG, {d_bv4_b}),
                     d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})}),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_MUL,
                    {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}),
                     RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                         d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_b}))
                         .first})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_ADD,
               {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_b,
       d_nm.mk_node(
           Kind::BV_NEG,
           {d_nm.mk_node(
               Kind::BV_MUL,
               {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {d_nm.mk_node(Kind::BV_NEG,
                         {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})}),
            d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a,
       d_nm.mk_node(
           Kind::BV_MUL,
           {

               d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}), d_bv4_b})}));
}

// reversed by NORM_BV_NOT_OR_SHL
// TEST_F(TestRewriterBv, bv_add_shl)
//{
//  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_SHL;
//  //// applies
//  test_rule<kind>(d_nm.mk_node(
//      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_SHL, {d_bv4_b,
//      d_bv4_a})}));
//  test_rule<kind>(d_nm.mk_node(
//      Kind::BV_ADD, {d_nm.mk_node(Kind::BV_SHL, {d_bv4_b, d_bv4_a}),
//      d_bv4_a}));
//  //// does not apply
//  test_rule_does_not_apply<kind>(d_nm.mk_node(
//      Kind::BV_ADD, {d_bv4_a, d_nm.mk_node(Kind::BV_SHL, {d_bv4_a,
//      d_bv4_b})}));
//}

TEST_F(TestRewriterBv, bv_add_neg_mul)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_NEG_MUL;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_bv4_a, d_nm.mk_node(Kind::BV_MUL, {d_bv4_b, d_bv4_a})})),
       d_bv4_one}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_bv4_a, d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_a})})),
       d_bv4_one}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_nm.mk_node(Kind::BV_MUL, {d_bv4_b, d_bv4_a}), d_bv4_a})),
       d_bv4_one}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b}), d_bv4_a})),
       d_bv4_one}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_nm.mk_node(Kind::BV_MUL, {d_nm.invert_node(d_bv4_a), d_bv4_b}),
            d_nm.invert_node(d_bv4_a)})),
       d_bv4_one}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_ADD,
           {d_bv4_a,
            d_nm.mk_node(Kind::BV_MUL, {d_bv4_b, d_nm.invert_node(d_bv4_a)})})),
       d_bv4_one}));
}

TEST_F(TestRewriterBv, bv_add_ite1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_ITE1;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ADD,
                   {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ADD,
                   {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {d_c, d_bv4_c, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_c, d_bv4_b}))}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_zero})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d}))}));
}

TEST_F(TestRewriterBv, bv_add_ite2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_ITE2;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_zero, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_zero})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_zero, d_bv4_zero})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_zero, d_bv4_a}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_zero}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_zero, d_bv4_zero}), d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_one, d_bv4_a})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})}));
}

/* bvand -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_and_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_AND, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_AND, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
  ////// special const 1
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv1_one, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv1_a, d_bv1_one}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_AND, {d_bv4_one, d_bv4_a}));
  }
  ////// special const ones
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv4_ones, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_ones}));
  }
}

TEST_F(TestRewriterBv, bv_and_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_CONST;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_zero, d_nm.mk_node(Kind::BV_AND, {d_bv4_one, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_zero, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_one})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_one, d_bv4_a}), d_bv4_zero}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_one}), d_bv4_zero}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_zero, d_nm.mk_node(Kind::BV_OR, {d_bv4_one, d_bv4_a})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_zero, d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
}

TEST_F(TestRewriterBv, bv_and_idem1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_IDEM1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_AND, {d_bv1_a, d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_and_idem2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_IDEM2;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
}

TEST_F(TestRewriterBv, bv_and_idem3)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_IDEM3;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_bv4_a, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c}), d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.invert_node(d_bv4_a),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
}

TEST_F(TestRewriterBv, bv_and_contra)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_CONTRA1;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_nm.invert_node(d_bv4_a)}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_a}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_bv1_a, d_nm.invert_node(d_bv1_a)}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv1_a), d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_and_contra2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_CONTRA2;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_c})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_nm.invert_node(d_bv4_a)})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_c})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_nm.invert_node(d_bv4_a)}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_nm.invert_node(d_bv4_a)})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_nm.invert_node(d_bv4_a)}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
}

TEST_F(TestRewriterBv, bv_and_contra3)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_CONTRA3;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_nm.invert_node(d_bv4_a)})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.invert_node(d_bv4_a),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.invert_node(d_bv4_a),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_bv4_a, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_and_subsum1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_SUBSUM1;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_c)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_c)})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_c}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_a)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_a)})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_OR, {d_bv4_c, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_c, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_c)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_c)})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_c})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_c}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_a)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_a)})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_OR, {d_bv4_c, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_c, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_d)}))}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND,
                    {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_c)})}));
}

TEST_F(TestRewriterBv, bv_and_subsum2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_SUBSUM2;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_a), d_nm.invert_node(d_bv4_b)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_bv4_a, d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_b), d_nm.invert_node(d_bv4_a)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_nm.mk_node(Kind::BV_OR, {d_bv4_b, d_bv4_a}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_bv4_a, d_nm.mk_node(Kind::BV_OR, {d_bv4_b, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_nm.mk_node(Kind::BV_OR, {d_bv4_b, d_bv4_a}), d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_b), d_nm.invert_node(d_bv4_c)}))}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(
           Kind::BV_AND,
           {d_nm.invert_node(d_bv4_c), d_nm.invert_node(d_bv4_d)}))}));
}

TEST_F(TestRewriterBv, bv_and_not_and1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_NOT_AND1;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a}),
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_a})),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_c, d_bv4_d}))}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.invert_node(d_nm.mk_node(
                        Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_c}))}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_c})}));
}

TEST_F(TestRewriterBv, bv_and_not_and2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_NOT_AND2;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_b,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),
       d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),
       d_bv4_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_bv4_a,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_bv4_c}))}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND, {d_bv4_a, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_and_concat)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_CONCAT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_zero})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_zero}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_ones}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_ones})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_ones}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_ones, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_ones, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_ones})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_zero})),
       d_nm.invert_node(
           d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a}))}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_zero, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_AND,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_zero}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_one, d_bv4_a})}));
}

TEST_F(TestRewriterBv, bv_and_resol1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_RESOL1;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),

       d_nm.invert_node(
           d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_nm.invert_node(d_bv4_b)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {

          d_nm.invert_node(
              d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_nm.invert_node(d_bv4_b)})),
          d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),

       d_nm.invert_node(
           d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_b), d_bv4_a}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {

          d_nm.invert_node(
              d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_b), d_bv4_a})),
          d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),

       d_nm.invert_node(
           d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_nm.invert_node(d_bv4_a)}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {

          d_nm.invert_node(
              d_nm.mk_node(Kind::BV_AND, {d_bv4_b, d_nm.invert_node(d_bv4_a)})),
          d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),

       d_nm.invert_node(
           d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {

          d_nm.invert_node(
              d_nm.mk_node(Kind::BV_AND, {d_nm.invert_node(d_bv4_a), d_bv4_b})),
          d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_nm.invert_node(d_bv4_b)})}));
}

/* bvashr ------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_ashr_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ASHR_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_ASHR, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_ASHR, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_ASHR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
}

/* bvconcat ----------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_concat_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_CONCAT_CONST;
  Node a_val                     = d_nm.mk_value(BitVector::from_ui(4, 5));
  Node b_val                     = d_nm.mk_value(BitVector::from_ui(4, 1));
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, a_val}), b_val}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {a_val, d_nm.mk_node(Kind::BV_CONCAT, {b_val, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {a_val, d_bv4_a}), b_val}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {a_val, d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, b_val})}));
}

TEST_F(TestRewriterBv, bv_concat_extract)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_CONCAT_EXTRACT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3}),
                    d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 0})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 2}),
                    d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {1, 1})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 1}),
                    d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {0, 0})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 0})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 2})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {1, 1})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 1})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {0, 0})})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3}),
                    d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {1, 0})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3}),
                    d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_b}, {2, 0})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 0})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 3})}),
       d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 0})}));
}

TEST_F(TestRewriterBv, bv_concat_and)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_CONCAT_AND;
  Node bv4_add = d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b});
  //// applies
  // match:  (bvconcat (bvand a b) c)
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}), d_bv4_c}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_zero}), d_bv4_zero}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}), bv4_add}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b}), bv4_add}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bv4_add}), bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b}), d_bv4_c}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b}), bv4_add}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b}), bv4_add}));
  // match:  (bvconcat (bvnot (bvand a b)) c)
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),
       d_bv4_c}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_zero})),
       d_bv4_zero}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})),
       bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b})),
       bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bv4_add})),
       bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b})),
       d_bv4_c}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b})),
       bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b})),
       bv4_add}));
  // match:  (bvconcat a (bvand b c))
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_bv4_c, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_bv4_zero, d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_zero})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {bv4_add, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {bv4_add, d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {bv4_add, d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bv4_add})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {bv4_add, d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_bv4_c, d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add, d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, bv4_add}), bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add, d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, bv4_add})}));
  // match:  (bvconcat a (bvnot (bvand b c)))
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_bv4_c,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_CONCAT,
                               {d_bv4_zero,
                                d_nm.invert_node(d_nm.mk_node(
                                    Kind::BV_AND, {d_bv4_zero, d_bv4_zero}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bv4_add}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {bv4_add, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_bv4_c,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, d_bv4_b}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, bv4_add})),
       bv4_add}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_CONCAT,
      {bv4_add,
       d_nm.invert_node(d_nm.mk_node(Kind::BV_AND, {d_bv4_zero, bv4_add}))}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b}), d_bv4_c}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_CONCAT,
                   {d_nm.mk_node(Kind::BV_AND, {bv4_add, bv4_add}), d_bv4_c}));
}

/* bvextract ---------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_extract_full)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_FULL;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 1}));
}

TEST_F(TestRewriterBv, bv_extract_extract)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_EXTRACT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 1})},
                   {1, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 1})})},
      {1, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {3, 1}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT, {d_nm.mk_node(Kind::BV_NOT, {d_bv4_a})}, {3, 1}));
}

TEST_F(TestRewriterBv, bv_extract_concat_full_lhs)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_CONCAT_FULL_LHS;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 4}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 4}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {3, 0}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {3, 0}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {4, 0}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {6, 4}));
}

TEST_F(TestRewriterBv, bv_extract_concat_full_rhs)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_CONCAT_FULL_RHS;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {3, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {3, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 4}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 4}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {4, 0}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {6, 4}));
}

TEST_F(TestRewriterBv, bv_extract_concat_lhs_rhs)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_CONCAT_LHS_RHS;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {3, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {3, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {1, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {1, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 4}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 4}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 6}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 6}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
}

TEST_F(TestRewriterBv, bv_extract_concat)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_CONCAT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {4, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {4, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 4}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 4}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})},
                   {7, 6}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})})},
      {7, 6}));
}

TEST_F(TestRewriterBv, bv_extract_and)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_AND;
  Node bvand = d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b});
  Node bvextract                 = d_nm.mk_node(
      Kind::BV_EXTRACT, {d_nm.mk_const(d_nm.mk_bv_type(8))}, {3, 0});
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})},
                               {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bvand})},
                               {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {bvand, d_bv4_b})},
                               {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_one})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_one, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {d_bv4_one, bvand})},
                               {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {bvand, d_bv4_one})},
                               {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {bvextract, bvextract})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bvextract})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_AND, {bvextract, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {bvextract, bvand})},
                               {2, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_AND, {bvand, bvextract})},
                               {2, 0}));

  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvand, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_one})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_one, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_one, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvand, d_bv4_one})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvextract, bvextract})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {d_bv4_a, bvextract})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvextract, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvextract, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_AND, {bvand, bvextract})})},
      {2, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b})})},
      {2, 0}));
}

TEST_F(TestRewriterBv, bv_extract_ite)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_ITE;
  Node cond                      = d_nm.mk_const(d_bool_type);
  Node bvand = d_nm.mk_node(Kind::BV_AND, {d_bv4_a, d_bv4_b});
  Node bvextract                 = d_nm.mk_node(
      Kind::BV_EXTRACT, {d_nm.mk_const(d_nm.mk_bv_type(8))}, {3, 0});
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, bvand})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvand, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_one})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_one, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_one, bvand})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvand, d_bv4_one})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvextract, bvextract})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, bvextract})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvextract, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvextract, bvand})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::ITE, {cond, bvand, bvextract})},
                   {2, 0}));

  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvand, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_one})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_one, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_one, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvand, d_bv4_one})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvextract, bvextract})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, bvextract})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvextract, d_bv4_b})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvextract, bvand})})},
      {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, bvand, bvextract})})},
      {2, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_OR, {d_bv4_a, d_bv4_b})})},
      {2, 0}));
}

TEST_F(TestRewriterBv, bv_extract_add_mul)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_ADD_MUL;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})},
                               {1, 0}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_EXTRACT,
                               {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})},
                               {1, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})})},
      {1, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})})},
      {1, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})},
                   {2, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})})},
      {3, 0}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})})},
      {3, 0}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})},
                   {1, 1}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})},
                   {1, 1}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b})})},
      {1, 1}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_ADD, {d_bv4_a, d_bv4_b})})},
      {1, 1}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT,
                   {d_nm.mk_node(Kind::BV_SUB, {d_bv4_a, d_bv4_b})},
                   {1, 0}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_EXTRACT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b})})},
      {1, 0}));
}

/* bvmul -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_mul_special_const)
{
  ////// special const 0
  {
    constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_ZERO;
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_MUL, {d_nm.mk_value(BitVector(4, "1010")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_MUL, {d_bv4_a, d_nm.mk_value(BitVector(4, "1010"))}));
  }
  ////// special const 1
  {
    constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_ONE;
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_one, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_one, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_a, d_bv1_one}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_one}));
  }
  ////// special const ones
  {
    constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_ONES;
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_ones, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_ones}));
  }
  ////// special const pow2
  {
    constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_POW2;
    //// applies
    auto pow2 = d_nm.mk_value(BitVector(4, "0100"));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {pow2, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, pow2}));
  }
  ////// special const negative pow2
  {
    constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_POW2;
    //// applies
    auto pow2 = d_nm.mk_value(BitVector(4, "0100").ibvneg());
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {pow2, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, pow2}));
  }
}

TEST_F(TestRewriterBv, bv_mul_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_mul_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_CONST;
  Node a_val                     = d_nm.mk_value(BitVector::from_ui(4, 5));
  Node b_val                     = d_nm.mk_value(BitVector::from_ui(4, 1));
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_MUL, {a_val, d_bv4_a}), b_val}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {b_val, d_nm.mk_node(Kind::BV_MUL, {a_val, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_MUL, {a_val, d_bv4_a}), a_val}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_MUL, {a_val, d_bv4_a}), d_bv4_a}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_UDIV, {a_val, d_bv4_a}), d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_mul_const_add)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_CONST_ADD;
  Node a_val                     = d_nm.mk_value(BitVector::from_ui(4, 5));
  Node b_val                     = d_nm.mk_value(BitVector::from_ui(4, 1));
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), b_val}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {b_val, d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), a_val}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_ADD, {a_val, d_bv4_a}), d_bv4_a}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_UDIV, {a_val, d_bv4_a}), d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_mul_ite)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_ITE;
  Node cond                      = d_nm.mk_const(d_bool_type);
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {cond, d_bv4_zero, d_bv4_a})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_zero})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {cond, d_bv4_zero, d_bv4_zero})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_nm.mk_node(Kind::ITE, {cond, d_bv4_zero, d_bv4_a}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_zero}), d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_nm.mk_node(Kind::ITE, {cond, d_bv4_zero, d_bv4_zero}), d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {cond, d_bv4_one, d_bv4_a})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_mul_neg)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_NEG;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL,
                               {d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}),
                                d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_bv4_a, d_nm.mk_node(Kind::BV_NEG, {d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}), d_bv4_b}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_MUL,
                   {RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                        d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}))
                        .first,
                    RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                        d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_b}))
                        .first}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_MUL,
                   {d_bv4_a,
                    RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                        d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_b}))
                        .first}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_MUL,
                   {RewriteRule<RewriteRuleKind::BV_NEG_ELIM>::apply(
                        d_rewriter, d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}))
                        .first,
                    d_bv4_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_bv4_a, d_nm.mk_node(Kind::BV_NOT, {d_bv4_b})}));
}

TEST_F(TestRewriterBv, bv_mul_ones)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_ONES;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_ones, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_a, d_bv4_ones}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_one, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv1_a, d_bv1_one}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_MUL, {d_bv4_ones, d_bv4_ones}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_MUL, {d_bv4_one, d_bv4_a}));
}

/* bvnot -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_not_bv_not)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NOT_BV_NOT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_NOT, {d_nm.mk_node(Kind::BV_NOT, {d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_NOT, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_not_bv_neg)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NOT_BV_NEG;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_NOT, {d_nm.mk_node(Kind::BV_NEG, {d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_NOT, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_not_bv_concat)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NOT_BV_CONCAT;
  //// applies
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_NOT, {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_zero})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_NOT, {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_ones, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_NOT, {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b})}));
}

/* bvshl -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_shl_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHL_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_SHL, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SHL, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SHL, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_SHL, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_SHL, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
}

TEST_F(TestRewriterBv, bv_shl_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHL_CONST;
  Type bv64_type                 = d_nm.mk_bv_type(64);
  Node a64                       = d_nm.mk_const(bv64_type);
  Type bv65_type                 = d_nm.mk_bv_type(65);
  Node a65                       = d_nm.mk_const(bv65_type);
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHL,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "1111"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHL,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0100"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHL,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHL,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHL, {a64, d_nm.mk_value(BitVector::from_ui(64, UINT64_MAX))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHL, {a64, d_nm.mk_value(BitVector::from_ui(64, 244))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHL, {a64, d_nm.mk_value(BitVector::from_ui(64, 24))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHL, {a65, d_nm.mk_value(BitVector::from_ui(65, 24))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHL, {d_bv4_a, d_bv4_b}));
}

/* bvshr -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_shr_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHR_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_SHR, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_SHR, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_SHR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
}

TEST_F(TestRewriterBv, bv_shr_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHR_CONST;
  Type bv64_type                 = d_nm.mk_bv_type(64);
  Node a64                       = d_nm.mk_const(bv64_type);
  Type bv65_type                 = d_nm.mk_bv_type(65);
  Node a65                       = d_nm.mk_const(bv65_type);
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "1111"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0100"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHR, {a64, d_nm.mk_value(BitVector::from_ui(64, UINT64_MAX))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHR, {a64, d_nm.mk_value(BitVector::from_ui(64, 244))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHR, {a64, d_nm.mk_value(BitVector::from_ui(64, 24))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SHR, {a65, d_nm.mk_value(BitVector::from_ui(65, 24))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_shr_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHR_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SHR, {d_bv1_a, d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_shr_not)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHR_NOT;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_SHR, {d_nm.invert_node(d_bv4_a), d_bv4_a}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_nm.invert_node(d_bv4_a)}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SHR, {d_bv4_a, d_bv4_a}));
}

/* bvslt -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_slt_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_SPECIAL_CONST;
  Node a2                        = d_nm.mk_const(d_nm.mk_bv_type(2));
  Node bv2_one                   = d_nm.mk_value(BitVector::mk_one(2));
  ////// special const 0
  {
    Node dis = d_nm.mk_node(Kind::NOT,
                            {d_nm.mk_node(Kind::EQUAL, {d_bv1_a, d_bv1_zero})});
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_zero, d_bv1_a}));
    // rhs 0
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, d_bv1_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, d_bv1_a}));
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, d_bv4_zero}));
  }
  ////// special const 1
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_one, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {bv2_one, a2}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, d_bv1_one}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {a2, bv2_one}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT, {d_bv4_one, d_bv4_a}));
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, d_bv4_one}));
  }
  ////// special const ones
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_ones, d_bv1_a}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT, {d_bv4_ones, d_bv4_a}));
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_SLT,
                     {d_nm.mk_value(BitVector::mk_ones(2)),
                      d_nm.mk_const(d_nm.mk_bv_type(2))}));
  }
  ////// special const min_signed
  {
    Node min1_s = d_nm.mk_value(BitVector::mk_min_signed(1));
    Node min4_s = d_nm.mk_value(BitVector::mk_min_signed(4));
    //// applies
    // lhs min_signed
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {min1_s, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {min4_s, d_bv4_a}));
    // rhs min_signed
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, min1_s}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, min4_s}));
  }
  ////// special const max_signed
  {
    Node max1_s = d_nm.mk_value(BitVector::mk_max_signed(1));
    Node max4_s = d_nm.mk_value(BitVector::mk_max_signed(4));
    //// applies
    // lhs max_signed
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {max1_s, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {max4_s, d_bv4_a}));
    // rhs max_signed
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, max1_s}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, max4_s}));
  }
}

TEST_F(TestRewriterBv, bv_slt_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_slt_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_SLT, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SLT, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_slt_concat)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_CONCAT;
  Node c4                        = d_nm.mk_const(d_bv4_type);
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {c4, d_bv4_a})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_CONCAT, {c4, d_bv4_a})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_const(d_nm.mk_bv_type(8))}));
}

TEST_F(TestRewriterBv, bv_slt_ite)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_ITE;
  Node cond                      = d_nm.mk_const(d_bool_type);
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SLT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_SLT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SLT,
      {d_nm.mk_node(Kind::ITE, {d_nm.mk_const(d_bool_type), d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SLT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_d})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SLT,
      {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SLT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})}));
}

/* bvudiv-------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_udiv_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_UDIV, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_UDIV, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
  ////// special const 1
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv1_a, d_bv1_one}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_one}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_UDIV, {d_bv1_one, d_bv1_a}));
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_UDIV, {d_bv4_one, d_bv4_a}));
  }
  ////// special const pow2
  {
    //// applies
    auto pow2 = d_nm.mk_value(BitVector(4, "0100"));
    test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, pow2}));
  }
}

TEST_F(TestRewriterBv, bv_udiv_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_udiv_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv1_a, d_bv1_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_udiv_pow2)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_POW2;
  for (uint64_t i = 0; i < (1u << 4); ++i)
  {
    Node c = d_nm.mk_value(BitVector::from_ui(4, i));
    if (i > 0
        && std::log2(i)
               == (static_cast<double>(static_cast<uint64_t>(std::log2(i)))))
    {
      ////// applies
      test_rule<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, c}));
    }
    else
    {
      ////// does not apply
      test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_UDIV, {d_bv4_a, c}));
    }
  }
}

TEST_F(TestRewriterBv, bv_udiv_ite)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_ITE;
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_UDIV,
                   {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_UDIV,
                   {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {d_c, d_bv4_c, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_UDIV,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d}))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_UDIV,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_c, d_bv4_b}))}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UDIV,
      {d_bv4_a, d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_zero})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UDIV,
      {d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b})),
       d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UDIV,
      {d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_b}),
       d_nm.invert_node(d_nm.mk_node(Kind::ITE, {d_c, d_bv4_a, d_bv4_d}))}));
}

/* bvult -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_ult_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_zero, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_a, d_bv1_zero}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_ULT, {d_bv1_a, d_bv1_a}));
  }
  ////// special const 1
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_one, d_bv1_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_a, d_bv1_one}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_one}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_ULT, {d_bv4_one, d_bv4_a}));
  }
  ////// special const ones
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_ones, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_ones}));
  }
}

TEST_F(TestRewriterBv, bv_ult_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_ult_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_ULT, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ULT, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_ult_concat)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_CONCAT;
  Node c4                        = d_nm.mk_const(d_bv4_type);
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_b, d_bv4_a}),
                    d_nm.mk_node(Kind::BV_CONCAT, {c4, d_bv4_a})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, c4})}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::BV_CONCAT, {c4, d_bv4_a})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::BV_CONCAT, {d_bv4_a, d_bv4_b}),
                    d_nm.mk_const(d_nm.mk_bv_type(8))}));
}

TEST_F(TestRewriterBv, bv_ult_ite)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_ITE;
  Node cond                      = d_nm.mk_const(d_bool_type);
  //// applies
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})}));
  test_rule<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ULT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})})}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ULT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})})}));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ULT,
      {d_nm.mk_node(Kind::ITE, {d_nm.mk_const(d_bool_type), d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})}));
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ULT,
                   {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
                    d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_d})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ULT,
      {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b}),
       d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_d})})}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ULT,
      {d_nm.mk_node(Kind::BV_NOT,
                    {d_nm.mk_node(Kind::ITE, {cond, d_bv4_a, d_bv4_b})}),
       d_nm.mk_node(Kind::ITE, {cond, d_bv4_c, d_bv4_b})}));
}

/* bvurem ------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_urem_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UREM_SPECIAL_CONST;
  ////// special const 0
  {
    //// applies
    test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv4_zero, d_bv4_a}));
    test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, d_bv4_zero}));
    //// does not apply
    test_rule_does_not_apply<kind>(
        d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_UREM, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
    test_rule_does_not_apply<kind>(d_nm.mk_node(
        Kind::BV_UREM, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
  }
  ////// special const pow2
  {
    //// applies
    auto pow2 = d_nm.mk_value(BitVector(4, "0100"));
    test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, pow2}));
  }
}

TEST_F(TestRewriterBv, bv_urem_bv1)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UREM_BV1;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv1_a, d_bv1_b}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_urem_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UREM_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_UREM, {d_bv4_a, d_bv4_b}));
}

/* bvxor -------------------------------------------------------------------- */

TEST_F(TestRewriterBv, bv_xor_same)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_XOR_SAME;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv1_a, d_bv1_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv4_a, d_bv4_a}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_XOR, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_xor_special_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_XOR_SPECIAL_CONST;
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv4_zero, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv4_a, d_bv4_zero}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv4_ones, d_bv4_a}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_XOR, {d_bv4_a, d_bv4_ones}));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_XOR, {d_bv4_a, d_bv4_a}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_XOR, {d_nm.mk_value(BitVector(4, "1110")), d_bv4_a}));
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_XOR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

/* --- Evaluation (Constant Folding) Rules----------------------------------- */

TEST_F(TestRewriterBv, bv_add_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ADD_EVAL;
  //// applies
  Node bvadd0 = d_nm.mk_node(Kind::BV_ADD,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvadd0, d_nm.mk_value(BitVector(4, "0111")));
  Node bvadd1 =
      d_nm.mk_node(Kind::BV_ADD, {d_nm.mk_value(BitVector(4, "1001")), bvadd0});
  test_rewrite(bvadd1, d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(
      d_nm.mk_node(Kind::BV_ADD, {bvadd1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "1110")));
  test_rewrite(d_nm.mk_node(Kind::BV_ADD, {bvadd1, bvadd1}),
               d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ADD, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_and_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_AND_EVAL;
  //// applies
  Node bvand0 = d_nm.mk_node(Kind::BV_AND,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvand0, d_nm.mk_value(BitVector(4, "1000")));
  Node bvand1 =
      d_nm.mk_node(Kind::BV_AND, {d_nm.mk_value(BitVector(4, "1001")), bvand0});
  test_rewrite(bvand1, d_nm.mk_value(BitVector(4, "1000")));
  test_rewrite(
      d_nm.mk_node(Kind::BV_AND, {bvand1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "1000")));
  test_rewrite(d_nm.mk_node(Kind::BV_AND, {bvand1, bvand1}),
               d_nm.mk_value(BitVector(4, "1000")));
  //// does not apply
  Node bvand_x0 = d_nm.mk_node(Kind::BV_AND,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))});
  test_rule_does_not_apply<kind>(bvand_x0);
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_AND, {bvand_x0, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_ashr_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ASHR_EVAL;
  //// applies
  Node bvashr0 = d_nm.mk_node(Kind::BV_ASHR,
                              {d_nm.mk_value(BitVector(4, "1101")),
                               d_nm.mk_value(BitVector(4, "0001"))});
  test_rewrite(bvashr0, d_nm.mk_value(BitVector(4, "1110")));
  Node bvashr1 = d_nm.mk_node(Kind::BV_ASHR,
                              {d_nm.mk_value(BitVector(4, "0111")), bvashr0});
  test_rewrite(bvashr1, d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(d_nm.mk_node(Kind::BV_ASHR,
                            {bvashr1, d_nm.mk_value(BitVector(4, "0010"))}),
               d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(d_nm.mk_node(Kind::BV_ASHR, {bvashr1, bvashr1}),
               d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ASHR, {d_bv4_a, d_nm.mk_value(BitVector(4, "0010"))}));
}

TEST_F(TestRewriterBv, bv_ashr_const)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ASHR_CONST;
  Type bv64_type                 = d_nm.mk_bv_type(64);
  Node a64                       = d_nm.mk_const(bv64_type);
  Type bv65_type                 = d_nm.mk_bv_type(65);
  Node a65                       = d_nm.mk_const(bv65_type);
  //// applies
  test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "1111"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0100"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR,
                               {d_bv4_a, d_nm.mk_value(BitVector(4, "0011"))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ASHR, {a64, d_nm.mk_value(BitVector::from_ui(64, UINT64_MAX))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ASHR, {a64, d_nm.mk_value(BitVector::from_ui(64, 244))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ASHR, {a64, d_nm.mk_value(BitVector::from_ui(64, 24))}));
  test_rule<kind>(d_nm.mk_node(
      Kind::BV_ASHR, {a65, d_nm.mk_value(BitVector::from_ui(65, 24))}));
  test_rule<kind>(d_nm.mk_node(Kind::BV_ASHR, {d_bv4_a, d_bv4_b}));
}

TEST_F(TestRewriterBv, bv_comp_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_COMP_EVAL;
  //// applies
  Node bvcomp0 = d_nm.mk_node(Kind::BV_COMP,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvcomp0, d_nm.mk_value(BitVector(1, "0")));
  Node bvcomp1 = d_nm.mk_node(Kind::BV_COMP,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1001"))});
  test_rewrite(bvcomp1, d_nm.mk_value(BitVector(1, "1")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_COMP, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_concat_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_CONCAT_EVAL;
  //// applies
  Node bvconcat0 = d_nm.mk_node(Kind::BV_CONCAT,
                                {d_nm.mk_value(BitVector(4, "1001")),
                                 d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvconcat0, d_nm.mk_value(BitVector(8, "10011110")));
  Node bvconcat1 = d_nm.mk_node(
      Kind::BV_CONCAT, {d_nm.mk_value(BitVector(4, "1001")), bvconcat0});
  test_rewrite(bvconcat1, d_nm.mk_value(BitVector(12, "100110011110")));
  test_rewrite(d_nm.mk_node(Kind::BV_CONCAT,
                            {bvconcat1, d_nm.mk_value(BitVector(4, "1110"))}),
               d_nm.mk_value(BitVector(16, "1001100111101110")));
  test_rewrite(d_nm.mk_node(Kind::BV_CONCAT, {bvconcat1, bvconcat1}),
               d_nm.mk_value(BitVector(24, "100110011110100110011110")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_CONCAT, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_dec_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_DEC_EVAL;
  //// applies
  Node bvdec0 =
      d_nm.mk_node(Kind::BV_DEC, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_DEC_ELIM>(
      bvdec0, d_nm.mk_value(BitVector(4, "1000")));
  Node bvdec1 = d_nm.mk_node(Kind::BV_DEC, {bvdec0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_DEC_ELIM>(
      bvdec1, d_nm.mk_value(BitVector(4, "0111")));
  Node bvdec2 = d_nm.mk_node(Kind::BV_DEC, {bvdec1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_DEC_ELIM>(
      d_nm.mk_node(Kind::BV_DEC, {bvdec2}),
      d_nm.mk_value(BitVector(4, "0101")));
  Node bvdec3 = d_nm.mk_node(Kind::BV_DEC, {bvdec2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_DEC_ELIM>(
      d_nm.mk_node(Kind::BV_DEC, {bvdec3}),
      d_rewriter.rewrite(d_nm.mk_value(BitVector(4, "0100"))));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_DEC, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_extract_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_EXTRACT_EVAL;
  //// applies
  Node bvextract0 = d_nm.mk_node(
      Kind::BV_EXTRACT, {d_nm.mk_value(BitVector(4, "1001"))}, {1, 0});
  test_rewrite(bvextract0, d_nm.mk_value(BitVector(2, "01")));
  test_rewrite(d_nm.mk_node(Kind::BV_EXTRACT, {bvextract0}, {1, 1}),
               d_nm.mk_value(BitVector(1, "0")));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_EXTRACT, {d_bv4_a}, {2, 0}));
}

TEST_F(TestRewriterBv, bv_inc_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_INC_EVAL;
  //// applies
  Node bvinc0 =
      d_nm.mk_node(Kind::BV_INC, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_INC_ELIM>(
      bvinc0, d_nm.mk_value(BitVector(4, "1010")));
  Node bvinc1 = d_nm.mk_node(Kind::BV_INC, {bvinc0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_INC_ELIM>(
      bvinc1, d_nm.mk_value(BitVector(4, "1011")));
  Node bvinc2 = d_nm.mk_node(Kind::BV_INC, {bvinc1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_INC_ELIM>(
      d_nm.mk_node(Kind::BV_INC, {bvinc2}),
      d_nm.mk_value(BitVector(4, "1101")));
  Node bvinc3 = d_nm.mk_node(Kind::BV_INC, {bvinc2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_INC_ELIM>(
      d_nm.mk_node(Kind::BV_INC, {bvinc3}),
      d_nm.mk_value(BitVector(4, "1110")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_INC, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_mul_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_MUL_EVAL;
  //// applies
  Node bvmul0 = d_nm.mk_node(Kind::BV_MUL,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvmul0, d_nm.mk_value(BitVector(4, "1110")));
  Node bvmul1 =
      d_nm.mk_node(Kind::BV_MUL, {d_nm.mk_value(BitVector(4, "1001")), bvmul0});
  test_rewrite(bvmul1, d_nm.mk_value(BitVector(4, "1110")));
  test_rewrite(
      d_nm.mk_node(Kind::BV_MUL, {bvmul1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "0100")));
  test_rewrite(d_nm.mk_node(Kind::BV_MUL, {bvmul1, bvmul1}),
               d_nm.mk_value(BitVector(4, "0100")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_MUL, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_smulo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SMULO_EVAL;
  //// applies
  Node bvsmulo0 = d_nm.mk_node(Kind::BV_SMULO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMULO_ELIM>(bvsmulo0, d_true);
  Node bvsmulo1 = d_nm.mk_node(Kind::BV_SMULO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMULO_ELIM>(bvsmulo1, d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMULO_ELIM>(
      d_nm.mk_node(Kind::BV_SMULO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMULO_ELIM>(
      d_nm.mk_node(Kind::BV_SMULO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SMULO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_umulo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UMULO_EVAL;
  //// applies
  Node bvumulo0 = d_nm.mk_node(Kind::BV_UMULO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMULO_ELIM>(bvumulo0, d_true);
  Node bvumulo1 = d_nm.mk_node(Kind::BV_UMULO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UMULO_ELIM>(bvumulo1, d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UMULO_ELIM>(
      d_nm.mk_node(Kind::BV_UMULO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UMULO_ELIM>(
      d_nm.mk_node(Kind::BV_UMULO,
                   {d_nm.mk_value(BitVector(4, "0110")),
                    d_nm.mk_value(BitVector(4, "0001"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UMULO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_nand_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NAND_EVAL;
  //// applies
  Node bvnand0 = d_nm.mk_node(Kind::BV_NAND,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NAND_ELIM>(
      bvnand0, d_nm.mk_value(BitVector(4, "0111")));
  Node bvnand1 = d_nm.mk_node(Kind::BV_NAND,
                              {d_nm.mk_value(BitVector(4, "1001")), bvnand0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NAND_ELIM>(
      bvnand1, d_nm.mk_value(BitVector(4, "1110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NAND_ELIM>(
      d_nm.mk_node(Kind::BV_NAND,
                   {bvnand1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "0001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NAND_ELIM>(
      d_nm.mk_node(Kind::BV_NAND, {bvnand1, bvnand1}),
      d_nm.mk_value(BitVector(4, "0001")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_NAND, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_neg_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NEG_EVAL;
  //// applies
  Node bvneg0 =
      d_nm.mk_node(Kind::BV_NEG, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEG_ELIM>(
      bvneg0, d_nm.mk_value(BitVector(4, "0111")));
  Node bvneg1 = d_nm.mk_node(Kind::BV_NEG, {bvneg0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEG_ELIM>(
      bvneg1, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEG_ELIM>(
      d_nm.mk_node(Kind::BV_NEG, {bvneg1}),
      d_nm.mk_value(BitVector(4, "0111")));
  //// does neg apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_NEG, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_nego_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NEGO_EVAL;
  //// applies
  Node bvnego0 =
      d_nm.mk_node(Kind::BV_NEGO, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEGO_ELIM>(bvnego0, d_false);
  Node bvnego1 =
      d_nm.mk_node(Kind::BV_NEGO, {d_nm.mk_value(BitVector(4, "0111"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEGO_ELIM>(bvnego1, d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NEGO_ELIM>(
      d_nm.mk_node(Kind::BV_NEGO, {d_nm.mk_value(BitVector(4, "1000"))}),
      d_true);
  //// does neg apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_NEGO, {d_bv1_a}));
}

TEST_F(TestRewriterBv, bv_nor_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NOR_EVAL;
  //// applies
  Node bvnor0 = d_nm.mk_node(Kind::BV_NOR,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NOR_ELIM>(
      bvnor0, d_nm.mk_value(BitVector(4, "0000")));
  Node bvnor1 =
      d_nm.mk_node(Kind::BV_NOR, {d_nm.mk_value(BitVector(4, "1001")), bvnor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NOR_ELIM>(
      bvnor1, d_nm.mk_value(BitVector(4, "0110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NOR_ELIM>(
      d_nm.mk_node(Kind::BV_NOR, {bvnor1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "0001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_NOR_ELIM>(
      d_nm.mk_node(Kind::BV_NOR, {bvnor1, bvnor1}),
      d_nm.mk_value(BitVector(4, "1001")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_NOR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_not_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_NOT_EVAL;
  //// applies
  Node bvnot0 =
      d_nm.mk_node(Kind::BV_NOT, {d_nm.mk_value(BitVector(4, "1001"))});
  test_rewrite(bvnot0, d_nm.mk_value(BitVector(4, "0110")));
  Node bvnot1 = d_nm.mk_node(Kind::BV_NOT, {bvnot0});
  test_rewrite(bvnot1, d_nm.mk_value(BitVector(4, "1001")));
  test_rewrite(d_nm.mk_node(Kind::BV_NOT, {bvnot1}),
               d_nm.mk_value(BitVector(4, "0110")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_NOT, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_or_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_OR_EVAL;
  //// applies
  Node bvor0 = d_nm.mk_node(Kind::BV_OR,
                            {d_nm.mk_value(BitVector(4, "1001")),
                             d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_OR_ELIM>(
      bvor0, d_nm.mk_value(BitVector(4, "1111")));
  Node bvor1 =
      d_nm.mk_node(Kind::BV_OR, {d_nm.mk_value(BitVector(4, "1001")), bvor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_OR_ELIM>(
      bvor1, d_nm.mk_value(BitVector(4, "1111")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_OR_ELIM>(
      d_nm.mk_node(Kind::BV_OR, {bvor1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "1111")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_OR_ELIM>(
      d_nm.mk_node(Kind::BV_OR, {bvor1, bvor1}),
      d_nm.mk_value(BitVector(4, "1111")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_OR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_redand_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_REDAND_EVAL;
  //// applies
  Node bvredand0 =
      d_nm.mk_node(Kind::BV_REDAND, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDAND_ELIM>(
      bvredand0, d_nm.mk_value(BitVector(1, "0")));
  Node bvredand1 = d_nm.mk_node(Kind::BV_REDAND, {bvredand0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDAND_ELIM>(
      bvredand1, d_nm.mk_value(BitVector(1, "0")));
  Node bvredand2 =
      d_nm.mk_node(Kind::BV_REDAND, {d_nm.mk_value(BitVector(4, "1111"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDAND_ELIM>(
      bvredand2, d_nm.mk_value(BitVector(1, "1")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDAND_ELIM>(
      d_nm.mk_node(Kind::BV_REDAND, {bvredand2}),
      d_nm.mk_value(BitVector(1, "1")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_REDAND, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_redor_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_REDOR_EVAL;
  //// applies
  Node bvredor0 =
      d_nm.mk_node(Kind::BV_REDOR, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDOR_ELIM>(
      bvredor0, d_nm.mk_value(BitVector(1, "1")));
  Node bvredor1 = d_nm.mk_node(Kind::BV_REDOR, {bvredor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDOR_ELIM>(
      bvredor1, d_nm.mk_value(BitVector(1, "1")));
  Node bvredor2 =
      d_nm.mk_node(Kind::BV_REDOR, {d_nm.mk_value(BitVector(4, "0000"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDOR_ELIM>(
      bvredor2, d_nm.mk_value(BitVector(1, "0")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDOR_ELIM>(
      d_nm.mk_node(Kind::BV_REDOR, {bvredor2}),
      d_nm.mk_value(BitVector(1, "0")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_REDOR, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_redxor_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_REDXOR_EVAL;
  //// applies
  Node bvredoxor0 =
      d_nm.mk_node(Kind::BV_REDXOR, {d_nm.mk_value(BitVector(4, "1001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      bvredoxor0, d_nm.mk_value(BitVector(1, "0")));
  Node bvredoxor1 = d_nm.mk_node(Kind::BV_REDXOR, {bvredoxor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      bvredoxor1, d_nm.mk_value(BitVector(1, "0")));
  Node bvredoxor2 =
      d_nm.mk_node(Kind::BV_REDXOR, {d_nm.mk_value(BitVector(4, "1101"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      bvredoxor2, d_nm.mk_value(BitVector(1, "1")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      d_nm.mk_node(Kind::BV_REDXOR, {bvredoxor2}),
      d_nm.mk_value(BitVector(1, "1")));
  Node bvredoxor3 =
      d_nm.mk_node(Kind::BV_REDXOR, {d_nm.mk_value(BitVector(4, "0000"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      bvredoxor3, d_nm.mk_value(BitVector(1, "0")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REDXOR_ELIM>(
      d_nm.mk_node(Kind::BV_REDXOR, {bvredoxor3}),
      d_nm.mk_value(BitVector(1, "0")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_REDXOR, {d_bv4_a}));
}

TEST_F(TestRewriterBv, bv_repeat_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_REPEAT_EVAL;
  //// applies
  Node bvrepeat0 =
      d_nm.mk_node(Kind::BV_REPEAT, {d_nm.mk_value(BitVector(4, "1001"))}, {1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REPEAT_ELIM>(
      bvrepeat0, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REPEAT_ELIM>(
      d_nm.mk_node(Kind::BV_REPEAT, {bvrepeat0}, {1}),
      d_nm.mk_value(BitVector(4, "1001")));
  Node bvrepeat1 =
      d_nm.mk_node(Kind::BV_REPEAT, {d_nm.mk_value(BitVector(4, "1001"))}, {2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REPEAT_ELIM>(
      bvrepeat1, d_nm.mk_value(BitVector(8, "10011001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_REPEAT_ELIM>(
      d_nm.mk_node(Kind::BV_REPEAT, {bvrepeat0}, {4}),
      d_rewriter.rewrite(d_nm.mk_node(Kind::BV_REPEAT, {bvrepeat1}, {2})));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_REPEAT, {d_bv4_a}, {2}));
}

TEST_F(TestRewriterBv, bv_sext_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SIGN_EXTEND_EVAL;
  //// applies
  Node bvsext0 = d_nm.mk_node(
      Kind::BV_SIGN_EXTEND, {d_nm.mk_value(BitVector(4, "1001"))}, {1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SIGN_EXTEND_ELIM>(
      bvsext0, d_nm.mk_value(BitVector(5, "11001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SIGN_EXTEND_ELIM>(
      d_nm.mk_node(Kind::BV_SIGN_EXTEND, {bvsext0}, {1}),
      d_nm.mk_value(BitVector(6, "111001")));
  Node bvsext1 = d_nm.mk_node(
      Kind::BV_SIGN_EXTEND, {d_nm.mk_value(BitVector(4, "0101"))}, {2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SIGN_EXTEND_ELIM>(
      bvsext1, d_nm.mk_value(BitVector(6, "000101")));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_SIGN_EXTEND, {d_bv4_a}, {2}));
}

TEST_F(TestRewriterBv, bv_zext_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ZERO_EXTEND_EVAL;
  //// applies
  Node bvzext0 = d_nm.mk_node(
      Kind::BV_ZERO_EXTEND, {d_nm.mk_value(BitVector(4, "1001"))}, {1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ZERO_EXTEND_ELIM>(
      bvzext0, d_nm.mk_value(BitVector(5, "01001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ZERO_EXTEND_ELIM>(
      d_nm.mk_node(Kind::BV_ZERO_EXTEND, {bvzext0}, {1}),
      d_nm.mk_value(BitVector(6, "001001")));
  Node bvzext1 = d_nm.mk_node(
      Kind::BV_ZERO_EXTEND, {d_nm.mk_value(BitVector(4, "0101"))}, {2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ZERO_EXTEND_ELIM>(
      bvzext1, d_nm.mk_value(BitVector(6, "000101")));
  //// does not apply
  test_rule_does_not_apply<kind>(
      d_nm.mk_node(Kind::BV_ZERO_EXTEND, {d_bv4_a}, {2}));
}

TEST_F(TestRewriterBv, bv_roli_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ROLI_EVAL;
  //// applies
  Node bvroli0 =
      d_nm.mk_node(Kind::BV_ROLI, {d_nm.mk_value(BitVector(4, "1001"))}, {0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROLI_ELIM>(
      bvroli0, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROLI_ELIM>(
      d_nm.mk_node(Kind::BV_ROLI, {d_nm.mk_value(BitVector(4, "1001"))}, {4}),
      d_nm.mk_value(BitVector(4, "1001")));
  Node bvroli1 = d_nm.mk_node(Kind::BV_ROLI, {bvroli0}, {1});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROLI_ELIM>(
      bvroli1, d_nm.mk_value(BitVector(4, "0011")));
  Node bvroli2 =
      d_nm.mk_node(Kind::BV_ROLI, {d_nm.mk_value(BitVector(4, "1001"))}, {2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROLI_ELIM>(
      bvroli2, d_nm.mk_value(BitVector(4, "0110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROLI_ELIM>(
      d_nm.mk_node(Kind::BV_ROLI, {bvroli1}, {2}),
      d_rewriter.rewrite(d_nm.mk_node(Kind::BV_ROLI, {bvroli0}, {7})));

  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_ROLI, {d_bv4_a}, {2}));
}

TEST_F(TestRewriterBv, bv_rol_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ROL_EVAL;
  //// applies
  Node bvrol0 = d_nm.mk_node(Kind::BV_ROL,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "0000"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROL_ELIM>(
      bvrol0, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROL_ELIM>(
      d_nm.mk_node(Kind::BV_ROL,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "0100"))}),
      d_nm.mk_value(BitVector(4, "1001")));
  Node bvrol1 =
      d_nm.mk_node(Kind::BV_ROL, {bvrol0, d_nm.mk_value(BitVector(4, "0001"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROL_ELIM>(
      bvrol1, d_nm.mk_value(BitVector(4, "0011")));
  Node bvrol2 = d_nm.mk_node(Kind::BV_ROL,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "0010"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROL_ELIM>(
      bvrol2, d_nm.mk_value(BitVector(4, "0110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROL_ELIM>(
      d_nm.mk_node(Kind::BV_ROL, {bvrol1, d_nm.mk_value(BitVector(4, "0010"))}),
      d_rewriter.rewrite(d_nm.mk_node(
          Kind::BV_ROL, {bvrol0, d_nm.mk_value(BitVector(4, "0111"))})));

  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ROL, {d_bv4_a, d_nm.mk_value(BitVector(4, "0111"))}));
}

TEST_F(TestRewriterBv, bv_rori_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_RORI_EVAL;

  //// applies
  Node bvrori0 =
      d_nm.mk_node(Kind::BV_RORI, {d_nm.mk_value(BitVector(4, "1001"))}, {0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      bvrori0, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      d_nm.mk_node(Kind::BV_RORI, {d_nm.mk_value(BitVector(4, "1001"))}, {4}),
      d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      d_nm.mk_node(Kind::BV_RORI, {bvrori0}, {4}),
      d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      d_nm.mk_node(Kind::BV_RORI, {bvrori0}, {1}),
      d_nm.mk_value(BitVector(4, "1100")));
  Node bvrori1 =
      d_nm.mk_node(Kind::BV_RORI, {d_nm.mk_value(BitVector(4, "1001"))}, {2});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      bvrori1, d_nm.mk_value(BitVector(4, "0110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_RORI_ELIM>(
      d_nm.mk_node(Kind::BV_RORI, {bvrori0}, {1}),
      d_rewriter.rewrite(d_nm.mk_node(Kind::BV_RORI, {bvrori1}, {7})));

  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(Kind::BV_RORI, {d_bv4_a}, {2}));
}

TEST_F(TestRewriterBv, bv_ror_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ROR_EVAL;

  //// applies
  Node bvror0 = d_nm.mk_node(Kind::BV_ROR,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "0000"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      bvror0, d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      d_nm.mk_node(Kind::BV_ROR,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "0100"))}),
      d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      d_nm.mk_node(Kind::BV_ROR, {bvror0, d_nm.mk_value(BitVector(4, "0100"))}),
      d_nm.mk_value(BitVector(4, "1001")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      d_nm.mk_node(Kind::BV_ROR, {bvror0, d_nm.mk_value(BitVector(4, "0001"))}),
      d_nm.mk_value(BitVector(4, "1100")));
  Node bvror1 = d_nm.mk_node(Kind::BV_ROR,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "0010"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      bvror1, d_nm.mk_value(BitVector(4, "0110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ROR_ELIM>(
      d_nm.mk_node(Kind::BV_ROR, {bvror0, d_nm.mk_value(BitVector(4, "0001"))}),
      d_rewriter.rewrite(d_nm.mk_node(
          Kind::BV_ROR, {bvror1, d_nm.mk_value(BitVector(4, "0111"))})));

  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ROR, {d_bv4_a, d_nm.mk_value(BitVector(4, "0111"))}));
}

TEST_F(TestRewriterBv, bv_saddo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SADDO_EVAL;
  //// applies
  Node bvsaddo0 = d_nm.mk_node(Kind::BV_SADDO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SADDO_ELIM>(bvsaddo0, d_true);
  Node bvsaddo1 = d_nm.mk_node(Kind::BV_SADDO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "0111"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SADDO_ELIM>(bvsaddo1, d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SADDO_ELIM>(
      d_nm.mk_node(Kind::BV_SADDO,
                   {d_nm.mk_value(BitVector(4, "0000")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SADDO_ELIM>(
      d_nm.mk_node(Kind::BV_SADDO,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SADDO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_uaddo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UADDO_EVAL;
  //// applies
  Node bvuaddo0 = d_nm.mk_node(Kind::BV_UADDO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UADDO_ELIM>(bvuaddo0, d_true);
  Node bvuaddo1 = d_nm.mk_node(Kind::BV_UADDO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "0111"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UADDO_ELIM>(bvuaddo1, d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UADDO_ELIM>(
      d_nm.mk_node(Kind::BV_UADDO,
                   {d_nm.mk_value(BitVector(4, "0000")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UADDO_ELIM>(
      d_nm.mk_node(Kind::BV_UADDO,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UADDO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_usubo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_USUBO_EVAL;
  //// applies
  Node bvusubo0 = d_nm.mk_node(Kind::BV_USUBO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_USUBO_ELIM>(bvusubo0, d_true);
  Node bvusubo1 = d_nm.mk_node(Kind::BV_USUBO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "0111"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_USUBO_ELIM>(bvusubo1, d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_USUBO_ELIM>(
      d_nm.mk_node(Kind::BV_USUBO,
                   {d_nm.mk_value(BitVector(4, "0000")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_USUBO_ELIM>(
      d_nm.mk_node(Kind::BV_USUBO,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_USUBO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_shl_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHL_EVAL;
  //// applies
  Node bvshl0 = d_nm.mk_node(Kind::BV_SHL,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "0001"))});
  test_rewrite(bvshl0, d_nm.mk_value(BitVector(4, "0010")));
  Node bvshl1 =
      d_nm.mk_node(Kind::BV_SHL, {d_nm.mk_value(BitVector(4, "1111")), bvshl0});
  test_rewrite(bvshl1, d_nm.mk_value(BitVector(4, "1100")));
  test_rewrite(
      d_nm.mk_node(Kind::BV_SHL, {bvshl1, d_nm.mk_value(BitVector(4, "0010"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(d_nm.mk_node(Kind::BV_SHL, {bvshl1, bvshl1}),
               d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SHL, {d_bv4_a, d_nm.mk_value(BitVector(4, "0010"))}));
}

TEST_F(TestRewriterBv, bv_shr_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SHR_EVAL;
  //// applies
  Node bvshr0 = d_nm.mk_node(Kind::BV_SHR,
                             {d_nm.mk_value(BitVector(4, "1101")),
                              d_nm.mk_value(BitVector(4, "0011"))});
  test_rewrite(bvshr0, d_nm.mk_value(BitVector(4, "0001")));
  Node bvshr1 =
      d_nm.mk_node(Kind::BV_SHR, {d_nm.mk_value(BitVector(4, "1111")), bvshr0});
  test_rewrite(bvshr1, d_nm.mk_value(BitVector(4, "0111")));
  test_rewrite(
      d_nm.mk_node(Kind::BV_SHR, {bvshr1, d_nm.mk_value(BitVector(4, "0010"))}),
      d_nm.mk_value(BitVector(4, "0001")));
  test_rewrite(d_nm.mk_node(Kind::BV_SHR, {bvshr1, bvshr1}),
               d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SHR, {d_bv4_a, d_nm.mk_value(BitVector(4, "0010"))}));
}

TEST_F(TestRewriterBv, bv_sle_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLE_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SLE_ELIM>(
      d_nm.mk_node(Kind::BV_SLE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SLE_ELIM>(
      d_nm.mk_node(Kind::BV_SLE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SLE_ELIM>(
      d_nm.mk_node(Kind::BV_SLE,
                   {d_nm.mk_value(BitVector(4, "0001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SLE, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_slt_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SLT_EVAL;
  //// applies
  test_rewrite(d_nm.mk_node(Kind::BV_SLT,
                            {d_nm.mk_value(BitVector(4, "1001")),
                             d_nm.mk_value(BitVector(4, "1110"))}),
               d_true);
  test_rewrite(d_nm.mk_node(Kind::BV_SLT,
                            {d_nm.mk_value(BitVector(4, "0001")),
                             d_nm.mk_value(BitVector(4, "1110"))}),
               d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SLT, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_sge_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SGE_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SGE_ELIM>(
      d_nm.mk_node(Kind::BV_SGE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SGE_ELIM>(
      d_nm.mk_node(Kind::BV_SGE,
                   {d_nm.mk_value(BitVector(4, "0001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SGE_ELIM>(
      d_nm.mk_node(Kind::BV_SGE,
                   {d_nm.mk_value(BitVector(4, "0001")),
                    d_nm.mk_value(BitVector(4, "0001"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SGE, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_sgt_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SGT_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SGT_ELIM>(
      d_nm.mk_node(Kind::BV_SGT,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SGT_ELIM>(
      d_nm.mk_node(Kind::BV_SGT,
                   {d_nm.mk_value(BitVector(4, "0001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SGT, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_udiv_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UDIV_EVAL;
  //// applies
  Node bvudiv0 = d_nm.mk_node(Kind::BV_UDIV,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvudiv0, d_nm.mk_value(BitVector(4, "0000")));
  Node bvudiv1 = d_nm.mk_node(Kind::BV_UDIV,
                              {d_nm.mk_value(BitVector(4, "1001")), bvudiv0});
  test_rewrite(bvudiv1, d_nm.mk_value(BitVector(4, "1111")));
  test_rewrite(d_nm.mk_node(Kind::BV_UDIV,
                            {bvudiv1, d_nm.mk_value(BitVector(4, "0110"))}),
               d_nm.mk_value(BitVector(4, "0010")));
  test_rewrite(d_nm.mk_node(Kind::BV_UDIV, {bvudiv1, bvudiv1}),
               d_nm.mk_value(BitVector(4, "0001")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UDIV, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_sdiv_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SDIV_EVAL;
  //// applies
  Node bvsdiv0 = d_nm.mk_node(Kind::BV_SDIV,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIV_ELIM>(
      bvsdiv0, d_nm.mk_value(BitVector(4, "0011")));
  Node bvsdiv1 = d_nm.mk_node(Kind::BV_SDIV,
                              {d_nm.mk_value(BitVector(4, "1001")), bvsdiv0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIV_ELIM>(
      bvsdiv1, d_nm.mk_value(BitVector(4, "1110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIV_ELIM>(
      d_nm.mk_node(Kind::BV_SDIV,
                   {bvsdiv1, d_nm.mk_value(BitVector(4, "0110"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIV_ELIM>(
      d_nm.mk_node(Kind::BV_SDIV, {bvsdiv1, bvsdiv1}),
      d_nm.mk_value(BitVector(4, "0001")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SDIV, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_sdivo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SDIVO_EVAL;
  //// applies
  Node bvsdivo0 = d_nm.mk_node(Kind::BV_SDIVO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIVO_ELIM>(bvsdivo0, d_false);
  Node bvsdivo1 = d_nm.mk_node(Kind::BV_SDIVO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "0011"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIVO_ELIM>(bvsdivo1, d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIVO_ELIM>(
      d_nm.mk_node(Kind::BV_SDIVO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "0110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SDIVO_ELIM>(
      d_nm.mk_node(Kind::BV_SDIVO,
                   {d_nm.mk_value(BitVector(4, "1000")),
                    d_nm.mk_value(BitVector(4, "1111"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SDIVO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_smod_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SMOD_EVAL;
  //// applies
  Node bvsmod0 = d_nm.mk_node(Kind::BV_SMOD,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMOD_ELIM>(
      bvsmod0, d_nm.mk_value(BitVector(4, "1111")));
  Node bvsmod1 = d_nm.mk_node(Kind::BV_SMOD,
                              {d_nm.mk_value(BitVector(4, "1001")), bvsmod0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMOD_ELIM>(
      bvsmod1, d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMOD_ELIM>(
      d_nm.mk_node(Kind::BV_SMOD,
                   {bvsmod1, d_nm.mk_value(BitVector(4, "0110"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SMOD_ELIM>(
      d_nm.mk_node(Kind::BV_SMOD, {bvsmod1, bvsmod1}),
      d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SMOD, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_srem_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SREM_EVAL;
  //// applies
  Node bvsrem0 = d_nm.mk_node(Kind::BV_SREM,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SREM_ELIM>(
      bvsrem0, d_nm.mk_value(BitVector(4, "1111")));
  Node bvsrem1 = d_nm.mk_node(Kind::BV_SREM,
                              {d_nm.mk_value(BitVector(4, "1001")), bvsrem0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SREM_ELIM>(
      bvsrem1, d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SREM_ELIM>(
      d_nm.mk_node(Kind::BV_SREM,
                   {bvsrem1, d_nm.mk_value(BitVector(4, "0110"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SREM_ELIM>(
      d_nm.mk_node(Kind::BV_SREM, {bvsrem1, bvsrem1}),
      d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SREM, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_sub_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SUB_EVAL;
  //// applies
  Node bvsub0 = d_nm.mk_node(Kind::BV_SUB,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SUB_ELIM>(
      bvsub0, d_nm.mk_value(BitVector(4, "1011")));
  Node bvsub1 =
      d_nm.mk_node(Kind::BV_SUB, {d_nm.mk_value(BitVector(4, "1001")), bvsub0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SUB_ELIM>(
      bvsub1, d_nm.mk_value(BitVector(4, "1110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SUB_ELIM>(
      d_nm.mk_node(Kind::BV_SUB, {bvsub1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SUB_ELIM>(
      d_nm.mk_node(Kind::BV_SUB, {bvsub1, bvsub1}),
      d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SUB, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_ssubo_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_SSUBO_EVAL;
  //// applies
  Node bvssubo0 = d_nm.mk_node(Kind::BV_SSUBO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SSUBO_ELIM>(bvssubo0, d_false);
  Node bvssubo1 = d_nm.mk_node(Kind::BV_SSUBO,
                               {d_nm.mk_value(BitVector(4, "1001")),
                                d_nm.mk_value(BitVector(4, "1011"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SSUBO_ELIM>(bvssubo1, d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SSUBO_ELIM>(
      d_nm.mk_node(Kind::BV_SSUBO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_SSUBO_ELIM>(
      d_nm.mk_node(Kind::BV_SSUBO,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_SSUBO, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_ule_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULE_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ULE_ELIM>(
      d_nm.mk_node(Kind::BV_ULE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ULE_ELIM>(
      d_nm.mk_node(Kind::BV_ULE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_ULE_ELIM>(
      d_nm.mk_node(Kind::BV_ULE,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "0001"))}),
      d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ULE, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_ult_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_ULT_EVAL;
  //// applies
  test_rewrite(d_nm.mk_node(Kind::BV_ULT,
                            {d_nm.mk_value(BitVector(4, "1001")),
                             d_nm.mk_value(BitVector(4, "1110"))}),
               d_true);
  test_rewrite(d_nm.mk_node(Kind::BV_ULT,
                            {d_nm.mk_value(BitVector(4, "1110")),
                             d_nm.mk_value(BitVector(4, "0001"))}),
               d_false);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_ULT, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_uge_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UGE_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UGE_ELIM>(
      d_nm.mk_node(Kind::BV_UGE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UGE_ELIM>(
      d_nm.mk_node(Kind::BV_UGE,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1001"))}),
      d_true);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UGE_ELIM>(
      d_nm.mk_node(Kind::BV_UGE,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "0001"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UGE, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_ugt_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UGT_EVAL;
  //// applies
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UGT_ELIM>(
      d_nm.mk_node(Kind::BV_UGT,
                   {d_nm.mk_value(BitVector(4, "1001")),
                    d_nm.mk_value(BitVector(4, "1110"))}),
      d_false);
  test_eval_elim_rule_bv<RewriteRuleKind::BV_UGT_ELIM>(
      d_nm.mk_node(Kind::BV_UGT,
                   {d_nm.mk_value(BitVector(4, "1110")),
                    d_nm.mk_value(BitVector(4, "0001"))}),
      d_true);
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UGT, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_urem_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_UREM_EVAL;
  //// applies
  Node bvurem0 = d_nm.mk_node(Kind::BV_UREM,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_rewrite(bvurem0, d_nm.mk_value(BitVector(4, "1001")));
  Node bvurem1 = d_nm.mk_node(Kind::BV_UREM,
                              {d_nm.mk_value(BitVector(4, "1001")), bvurem0});
  test_rewrite(bvurem1, d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(d_nm.mk_node(Kind::BV_UREM,
                            {bvurem1, d_nm.mk_value(BitVector(4, "0110"))}),
               d_nm.mk_value(BitVector(4, "0000")));
  test_rewrite(d_nm.mk_node(Kind::BV_UREM, {bvurem1, bvurem1}),
               d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_UREM, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_xor_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_XOR_EVAL;
  //// applies
  Node bvxor0 = d_nm.mk_node(Kind::BV_XOR,
                             {d_nm.mk_value(BitVector(4, "1001")),
                              d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XOR_ELIM>(
      bvxor0, d_nm.mk_value(BitVector(4, "0111")));
  Node bvxor1 =
      d_nm.mk_node(Kind::BV_XOR, {d_nm.mk_value(BitVector(4, "1001")), bvxor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XOR_ELIM>(
      bvxor1, d_nm.mk_value(BitVector(4, "1110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XOR_ELIM>(
      d_nm.mk_node(Kind::BV_XOR, {bvxor1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "0000")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XOR_ELIM>(
      d_nm.mk_node(Kind::BV_XOR, {bvxor1, bvxor1}),
      d_nm.mk_value(BitVector(4, "0000")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_XOR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

TEST_F(TestRewriterBv, bv_xnor_eval)
{
  constexpr RewriteRuleKind kind = RewriteRuleKind::BV_XNOR_EVAL;
  //// applies
  Node bvxnor0 = d_nm.mk_node(Kind::BV_XNOR,
                              {d_nm.mk_value(BitVector(4, "1001")),
                               d_nm.mk_value(BitVector(4, "1110"))});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XNOR_ELIM>(
      bvxnor0, d_nm.mk_value(BitVector(4, "1000")));
  Node bvxnor1 = d_nm.mk_node(Kind::BV_XNOR,
                              {d_nm.mk_value(BitVector(4, "1001")), bvxnor0});
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XNOR_ELIM>(
      bvxnor1, d_nm.mk_value(BitVector(4, "1110")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XNOR_ELIM>(
      d_nm.mk_node(Kind::BV_XNOR,
                   {bvxnor1, d_nm.mk_value(BitVector(4, "1110"))}),
      d_nm.mk_value(BitVector(4, "1111")));
  test_eval_elim_rule_bv<RewriteRuleKind::BV_XNOR_ELIM>(
      d_nm.mk_node(Kind::BV_XNOR, {bvxnor1, bvxnor1}),
      d_nm.mk_value(BitVector(4, "1111")));
  //// does not apply
  test_rule_does_not_apply<kind>(d_nm.mk_node(
      Kind::BV_XNOR, {d_bv4_a, d_nm.mk_value(BitVector(4, "1110"))}));
}

/* --- Elimination Rules ---------------------------------------------------- */

// not supported by parser
#if 0
TEST_F(TestRewriterBv, bv_dec_elim) { test_elim_rule_bv(Kind::BV_DEC); }

TEST_F(TestRewriterBv, bv_inc_elim) { test_elim_rule_bv(Kind::BV_INC); }
#endif

TEST_F(TestRewriterBv, bv_nand_elim) { test_elim_rule_bv(Kind::BV_NAND); }

TEST_F(TestRewriterBv, bv_neg_elim) { test_elim_rule_bv(Kind::BV_NEG); }

TEST_F(TestRewriterBv, bv_nor_elim) { test_elim_rule_bv(Kind::BV_NOR); }

TEST_F(TestRewriterBv, bv_or_elim) { test_elim_rule_bv(Kind::BV_OR); }

TEST_F(TestRewriterBv, bv_redand_elim) { test_elim_rule_bv(Kind::BV_REDAND); }

TEST_F(TestRewriterBv, bv_redor_elim) { test_elim_rule_bv(Kind::BV_REDOR); }

// not supported by Bitwuzla main
// TEST_F(TestRewriterBv, bv_redxor_elim) { test_elim_rule_bv(Kind::BV_REDXOR);
// }

TEST_F(TestRewriterBv, bv_roli_elim)
{
  test_elim_rule_bv(Kind::BV_ROLI, {0, 1, 2, 3, 12});
}

TEST_F(TestRewriterBv, bv_rori_elim)
{
  test_elim_rule_bv(Kind::BV_RORI, {0, 1, 2, 3, 12});
}

TEST_F(TestRewriterBv, bv_repeat_elim)
{
  test_elim_rule_bv(Kind::BV_REPEAT, {1, 1, 2, 3, 4});
}

TEST_F(TestRewriterBv, bv_saddo_elim) { test_elim_rule_bv(Kind::BV_SADDO); }

TEST_F(TestRewriterBv, bv_sdiv_elim) { test_elim_rule_bv(Kind::BV_SDIV); }

TEST_F(TestRewriterBv, bv_sdivo_elim) { test_elim_rule_bv(Kind::BV_SDIVO); }

TEST_F(TestRewriterBv, bv_sge_elim) { test_elim_rule_bv(Kind::BV_SGE); }

TEST_F(TestRewriterBv, bv_sgt_elim) { test_elim_rule_bv(Kind::BV_SGT); }

TEST_F(TestRewriterBv, bv_sign_extend_elim)
{
  test_elim_rule_bv(Kind::BV_SIGN_EXTEND, {0, 1, 2, 3, 4});
}

TEST_F(TestRewriterBv, bv_sle_elim) { test_elim_rule_bv(Kind::BV_SLE); }

TEST_F(TestRewriterBv, bv_smod_elim) { test_elim_rule_bv(Kind::BV_SMOD); }

TEST_F(TestRewriterBv, bv_smulo_elim) { test_elim_rule_bv(Kind::BV_SMULO); }

TEST_F(TestRewriterBv, bv_srem_elim) { test_elim_rule_bv(Kind::BV_SREM); }

TEST_F(TestRewriterBv, bv_ssubo_elim) { test_elim_rule_bv(Kind::BV_SSUBO); }

TEST_F(TestRewriterBv, bv_sub_elim) { test_elim_rule_bv(Kind::BV_SUB); }

TEST_F(TestRewriterBv, bv_uaddo_elim) { test_elim_rule_bv(Kind::BV_UADDO); }

TEST_F(TestRewriterBv, bv_uge_elim) { test_elim_rule_bv(Kind::BV_UGE); }

TEST_F(TestRewriterBv, bv_ugt_elim) { test_elim_rule_bv(Kind::BV_UGT); }

TEST_F(TestRewriterBv, bv_ule_elim) { test_elim_rule_bv(Kind::BV_ULE); }

TEST_F(TestRewriterBv, bv_umulo_elim) { test_elim_rule_bv(Kind::BV_UMULO); }

TEST_F(TestRewriterBv, bv_usubo_elim) { test_elim_rule_bv(Kind::BV_USUBO); }

TEST_F(TestRewriterBv, bv_xnor_elim) { test_elim_rule_bv(Kind::BV_XNOR); }

TEST_F(TestRewriterBv, bv_zero_extend_elim)
{
  test_elim_rule_bv(Kind::BV_ZERO_EXTEND, {0, 1, 2, 3, 4});
}

/* -------------------------------------------------------------------------- */
}  // namespace bzla::test
