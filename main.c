// Copyright 2016 Connor Taffe

#include "lex.h"

int main() {
  Lexer lexer = (Lexer){.input = "var _h2 = jj; var a = haza;"};

  // Lexing state map
  States s1, s2[1], s3[1], s4[1], s5[4], s6[1], s7[1];
  s7[0] = (States){.func = lex_possible_eof,
                   .states = &s1,
                   .size = sizeof(s1) / sizeof(States)};
  s6[0] = (States){
      .func = lex_terminal, .states = s7, .size = sizeof(s7) / sizeof(States)};
  s5[0] = (States){.func = lex_identifier,
                   .states = s6,
                   .size = sizeof(s6) / sizeof(States)};
  s4[0] = (States){.func = lex_expression,
                   .states = s5,
                   .size = sizeof(s5) / sizeof(States)};
  s3[0] = (States){.func = lex_assignment,
                   .states = s4,
                   .size = sizeof(s4) / sizeof(States)};
  s2[0] = (States){.func = lex_identifier,
                   .states = s3,
                   .size = sizeof(s3) / sizeof(States)};
  s1 = (States){
      .func = lex_keyword, .states = s2, .size = sizeof(s2) / sizeof(States)};
  s5[3] = s1;

  lex(&lexer, s1);
  lex_pprint(&lexer);
}
