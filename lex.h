// Copyright 2016 Connor Taffe
#ifndef LEX_H_
#define LEX_H_

#include <stddef.h>

#include "token.h"

typedef struct Lexer {
  char *input;
  TokenList *head;
  TokenList *tail;
} Lexer;

typedef size_t (*StateFunc)(Lexer *);
typedef struct States {
  StateFunc func;
  // Array of possible next states
  struct States *states;
  size_t size;
} States;

void lex(Lexer *l, States s);
void lex_emit(Lexer *l, Token t);
void lex_pprint(Lexer *l);

enum {
  // reserve 4 bits for enumerated values
  kLexError = (sizeof(size_t) * 8) - 16,
  kLexTerminal
};

size_t lex_possible_eof(Lexer *l);
size_t lex_keyword(Lexer *l);
size_t lex_identifier(Lexer *l);
size_t lex_assignment(Lexer *l);
size_t lex_expression(Lexer *l);
size_t lex_terminal(Lexer *l);

#endif  // LEX_H_
