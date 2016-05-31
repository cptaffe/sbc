// Copyright 2016 Connor Taffe

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lex.h"
#include "./token.h"

// returns true if there is no more input to lex
bool lex_eof(Lexer *l) { return !*l->input; }

// moves to the next character
void lex_next(Lexer *l) { l->input++; }

// returns the current input character
char lex_current(Lexer *l) { return *l->input; }

void lex_emit(Lexer *l, Token t) {
  TokenList *tl = (TokenList *)calloc(1, sizeof(TokenList));
  tl->token = t;
  if (l->head == NULL) {
    l->head = tl;
    l->tail = tl;
  } else {
    l->tail->next = tl;
    l->tail = tl;
  }
}

static bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// scan through whitespace
static void scan_whitespace(Lexer *l) {
  char c;
  for (; (c = lex_current(l)), !lex_eof(l); lex_next(l)) {
    if (!is_whitespace(c)) {
      break;
    }
  }
}

size_t lex_keyword(Lexer *l) {
  enum { kKwExprFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if (c >= 'a' && c <= 'z') {
      // keywords are always lowercase alphabetics
    } else if (c == ' ') {
      // proper end of a keyword
      char *s[] = {"var", "func"};
      Keyword k[] = {kKeywordVar, kKeywordFunc};
      for (size_t j = 0; j < sizeof(k) / sizeof(Keyword); j++) {
        if (memcmp(&l->input[-i], s[j], i) == 0) {
          // found keyword
          scan_whitespace(l);
          lex_emit(l, (Token){
                          .type = kTokenTypeKeyword, .keyword = k[j],
                      });
          return kKwExprFunc;
        }
      }
      // not a known keyword
      lex_emit(l, (Token){.type = kTokenTypeError, .error = "unknown keyword"});
      return kLexError;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)malloc(s);
      snprintf(e, s, "unexpected character '%c' in keyword", c);
      lex_emit(l, (Token){
                      .type = kTokenTypeError, .error = e,
                  });
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in keyword"});
  return kLexError;
}

size_t lex_identifier(Lexer *l) {
  enum { kPostIdentFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (i > 0 && ((c >= '0' && c <= '9') || c == '\'')) || c == '_') {
      // any alphabetic or underscore, numerals after the
      // first character
    } else if (i > 0) {
      // proper end of an identifier
      char *id = (char *)calloc(i + 1, sizeof(char));
      memcpy(id, &l->input[-i], i);
      scan_whitespace(l);
      lex_emit(l, (Token){
                      .type = kTokenTypeIdent, .token = id,
                  });
      return kPostIdentFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected identifier", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in identifier"});
  return kLexError;
}

size_t lex_hex_number(Lexer *l) {
  enum { kPostNumeralFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if ((c >= '0' && c <= '9') || (c >= 'a' || c <= 'f') || c == '_') {
      // hexadecimal digits, fuck uppercase letters
    } else if (i > 0) {
      return kPostNumeralFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected hex digit", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in hex number"});
  return kLexError;
}

size_t lex_octal_number(Lexer *l) {
  enum { kPostNumeralFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if ((c >= '0' && c <= '7') || c == '_') {
      // octal digits
    } else if (i > 0) {
      return kPostNumeralFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected octal digit", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in ocatal number"});
  return kLexError;
}

size_t lex_decimal_number(Lexer *l) {
  enum { kPostNumeralFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if ((c >= '0' && c <= '9') || c == '_') {
      // decimal digits
    } else if (i > 0) {
      return kPostNumeralFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected decimal digit", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in decimal number"});
  return kLexError;
}

size_t lex_binary_number(Lexer *l) {
  enum { kPostNumeralFunc };
  char c;
  for (size_t i = 0; (c = lex_current(l)), !lex_eof(l); i++, lex_next(l)) {
    if ((c >= '0' && c <= '1') || c == '_') {
      // decimal digits
    } else if (i > 0) {
      return kPostNumeralFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected binary digit", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in binary number"});
  return kLexError;
}

size_t lex_prefix_number(Lexer *l) {
  enum { kHexFunc, kOctalFunc, kBinaryFunc };
  char c = lex_current(l);
  if (!lex_eof(l)) {
    lex_next(l);
    if (c == 'x') {
      // hexadecimal number
      return kHexFunc;
    } else if (c == 'c') {
      // octal number
      return kOctalFunc;
    } else if (c == 'b') {
      // binary number
      return kBinaryFunc;
    } else {
      // unexpected character
      lex_emit(l, (Token){.type = kTokenTypeError,
                          .error = "unknown prefix in number"});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in number prefix"});
  return kLexError;
}

// Lex a number:
// 0c1, 0c01, 0x4f, 0b101_1011, 123, 0.34e6
size_t lex_number(Lexer *l) {
  enum { kDecimal, kPrefix };
  // read something
  char c = lex_current(l);
  if (!lex_eof(l)) {
    lex_next(l);
    if (c >= '1' && c <= '9') {
      // decimal number
      return kDecimal;
    } else if (c == '0') {
      // a prefix
      return kPrefix;
    } else {
      // unexpected character
      lex_emit(l, (Token){.type = kTokenTypeError,
                          .error = "unexpected character in number"});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(
      l, (Token){.type = kTokenTypeError, .error = "unexpected eof in number"});
  return kLexError;
}

size_t lex_assignment(Lexer *l) {
  enum { kExprFunc };
  char c = lex_current(l);
  if (!lex_eof(l)) {
    lex_next(l);
    if (c == '=') {
      scan_whitespace(l);
      return kExprFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c' in assignment", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in identifier"});
  return kLexError;
}

size_t lex_expression(Lexer *l) {
  enum { kIdentFunc, kCharFunc, kNumFunc, kTermFunc };
  char c = lex_current(l);
  if (!lex_eof(l)) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
      // identifier
      return kIdentFunc;
    } else if (c == '\'') {
      // character literal
      return kCharFunc;
    } else if (c >= '0' && c <= '9') {
      // numeric literal
      return kNumFunc;
    } else if (c == ';') {
      // statement termination
      return kTermFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c' in expression", c);
      lex_emit(l, (Token){.type = kTokenTypeError, .error = e});
      return kLexError;
    }
  }
  // expected eof
  return kLexTerminal;
}

size_t lex_terminal(Lexer *l) {
  enum { kPostExpr };
  char c = lex_current(l);
  if (!lex_eof(l)) {
    lex_next(l);
    if (c == ';') {
      // statement termination
      lex_emit(l, (Token){.type = kTokenTypeExprTerm});
      scan_whitespace(l);
      return kPostExpr;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c' in terminal", c);
      lex_emit(l, (Token){
                      .type = kTokenTypeError, .error = e,
                  });
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l, (Token){.type = kTokenTypeError,
                      .error = "unexpected eof in identifier"});
  return kLexError;
}

size_t lex_possible_eof(Lexer *l) {
  enum { kNotEofFunc };
  return (*l->input) ? kNotEofFunc : kLexTerminal;
}

// possible state map
// lex_keyword -> lex_identifier -> lex_assign ->
void lex(Lexer *l, States states) {
  size_t f;
  for (;;) {
    f = states.func(l);
    printf("%s, %ld\n", states.name, f);
    if (f == kLexError || f == kLexTerminal) {
      // Terminate
      break;
    }
    if (f >= states.size) {
      // out of bounds error
      lex_emit(l, (Token){.type = kTokenTypeError,
                          .error = "State function index out of bounds"});
      break;
    }
    states = states.states[f];
  }
}

// pprint maximum language parallelism
static void lang_pprint(Lexer *lexer) {
  for (TokenList *l = lexer->head; l != NULL; l = l->next) {
    Token t = l->token;
    switch (t.type) {
      case kTokenTypeChar:
        printf("'%s' ", t.token);
        break;
      case kTokenTypeError:
        printf("error: '%s'\n", t.error);
        break;
      case kTokenTypeFloat:
      case kTokenTypeIdent:
      case kTokenTypeInteger:
      case kTokenTypeString:
        printf("%s ", t.token);
        break;
      case kTokenTypeKeyword:
        switch (t.keyword) {
          case kKeywordFunc:
            printf("func ");
            break;
          case kKeywordVar:
            printf("var ");
            break;
        }
        break;
      case kTokenTypeExprTerm:
        printf(";\n");
        break;
    }
    free(l);
  }
}

// print maximum token readability
static void token_pprint(Lexer *lexer) {
  for (TokenList *l = lexer->head; l != NULL; l = l->next) {
    Token t = l->token;
    switch (t.type) {
      case kTokenTypeChar:
        printf("char: '%s'\n", t.token);
        break;
      case kTokenTypeError:
        printf("error: '%s'\n", t.error);
        break;
      case kTokenTypeFloat:
        printf("float: %s\n", t.token);
        break;
      case kTokenTypeIdent:
        printf("id: '%s'\n", t.token);
        break;
      case kTokenTypeInteger:
        printf("integer: '%s'\n", t.token);
        break;
      case kTokenTypeKeyword:
        switch (t.keyword) {
          case kKeywordFunc:
            printf("keyword: 'func'\n");
            break;
          case kKeywordVar:
            printf("keyword: 'var'\n");
            break;
        }
        break;
      case kTokenTypeString:
        printf("string: '%s'\n", t.token);
        break;
      case kTokenTypeExprTerm:
        printf("expression terminator ';'\n");
        break;
    }
    free(l);
  }
}

void lex_pprint(Lexer *lexer, bool debug) {
  if (debug) {
    token_pprint(lexer);
  } else {
    lang_pprint(lexer);
  }
}
