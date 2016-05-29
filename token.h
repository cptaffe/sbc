// Copryright 2016 Connor Taffe

#ifndef TOKEN_H_
#define TOKEN_H_

#include <stdint.h>

typedef enum {
  kKeywordVar,
  kKeywordFunc,
} Keyword;

typedef struct {
  enum {
    kTokenTypeError,
    kTokenTypeKeyword,
    kTokenTypeIdent,
    kTokenTypeString,
    kTokenTypeInteger,
    kTokenTypeFloat,
    kTokenTypeChar,
    kTokenTypeExprTerm,  // ;
  } type;
  union {
    char *error, *string, *ident;
    Keyword keyword;
    uint64_t integer;
    double floatng;
    char character;
  };
} Token;

typedef struct TokenList {
  Token token;
  struct TokenList *next;
} TokenList;

#endif  // TOKEN_H_
