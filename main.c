// Copyright 2016 Connor Taffe

#include "lex.h"

int main() {
  Lexer lexer = (Lexer){.input = "var _h2 = jj; var a = haza;"};

  enum { kKeyword, kIdent, kAssign, kExpr, kPostExpr, kTerm, kEof, kSize };
  States s[kSize][kSize] = {
          // keyword -> ident
          [kKeyword] = {{.name = "kKeyword",
                         .func = lex_keyword,
                         .states = s[kIdent],
                         .size = sizeof(s[kIdent]) / sizeof(States)}},
          // id -> assign
          [kIdent] = {{.name = "kIdent",
                       .func = lex_identifier,
                       .states = s[kAssign],
                       .size = sizeof(s[kAssign]) / sizeof(States)}},
          // assign -> expr
          [kAssign] = {{.name = "kAssign",
                        .func = lex_assignment,
                        .states = s[kExpr],
                        .size = sizeof(s[kExpr]) / sizeof(States)}},
          // expr -> term
          [kExpr] = {{.name = "kExpr",
                      .func = lex_expression,
                      .states = s[kPostExpr],
                      .size = sizeof(s[kPostExpr]) / sizeof(States)}},
          // id -> expr, terminal -> eof
          [kPostExpr] = {[0] = {.name = "kPostExpr[0]",
                                .func = lex_identifier,
                                .states = s[kExpr],
                                .size = sizeof(s[kExpr]) / sizeof(States)},
                         [3] = {.name = "kPostExpr[3]",
                                .func = lex_terminal,
                                .states = s[kEof],
                                .size = sizeof(s[kEof]) / sizeof(States)}},
          // eof -> keyword
          [kEof] = {{.name = "kEof",
                     .func = lex_possible_eof,
                     .states = s[kKeyword],
                     .size = sizeof(s[kKeyword]) / sizeof(States)}}};

  lex(&lexer, s[0][0]);
  lex_pprint(&lexer, false);
}
