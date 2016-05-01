// Copyright 2016 Connor Taffe

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lex.h"
#include "./token.h"

const size_t kLexError = (size_t)-1;

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
  for (size_t i = 0; (c = *l->input); i++, l->input++) {
    if (!is_whitespace(c)) {
      break;
    }
  }
}

size_t lex_keyword(Lexer *l) {
  enum { kKwExprFunc };
  char c;
  for (size_t i = 0; (c = *l->input); i++, l->input++) {
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
      lex_emit(l, (Token){
                      .type = kTokenTypeError, .error = "unknown keyword",
                  });
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
  lex_emit(l, (Token){
                  .type = kTokenTypeError, .error = "unexpected eof in keyword",
              });
  return kLexError;
}

size_t lex_identifier(Lexer *l) {
  enum { kPostIdentFunc };
  char c;
  for (size_t i = 0; (c = *l->input); i++, l->input++) {
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
                      .type = kTokenTypeIdent, .ident = id,
                  });
      return kPostIdentFunc;
    } else {
      // erronious character
      const size_t s = 100;
      char *e = (char *)calloc(s, sizeof(char));
      snprintf(e, s, "unexpected character '%c', expected identifier", c);
      lex_emit(l, (Token){
                      .type = kTokenTypeError, .error = e,
                  });
      return kLexError;
    }
  }
  // unexpected eof
  lex_emit(l,
           (Token){
               .type = kTokenTypeError, .error = "unexpected eof in identifier",
           });
  return kLexError;
}

size_t lex_assignment(Lexer *l) {
  enum { kExprFunc };
  char c = *l->input;
  l->input++;
  if (c == '=') {
    scan_whitespace(l);
    return kExprFunc;
  } else {
    // erronious character
    const size_t s = 100;
    char *e = (char *)calloc(s, sizeof(char));
    snprintf(e, s, "unexpected character '%c' in assignment", c);
    lex_emit(l, (Token){
                    .type = kTokenTypeError, .error = e,
                });
    return kLexError;
  }
}

size_t lex_expression(Lexer *l) {
  enum { kIdentFunc, kCharFunc, kNumFunc, kTermFunc };
  char c = *l->input;
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
    lex_emit(l, (Token){
                    .type = kTokenTypeError, .error = e,
                });
    return kLexError;
  }
}

// possible state map
// lex_keyword -> lex_identifier -> lex_assign ->
void lex(Lexer *l, States states) {
  size_t f;
  do {
    f = states.func(l);
    printf("i = %zu\n", f);
    assert(f < states.size);
    states = states.states[f];
  } while (f != kLexError);
}

int main() {
  Lexer lexer = (Lexer){.input = "var _h2 = jj;"};

  // Lexing state map
  States s5[] = {(States){
      .func = lex_identifier,
  }};
  States s4[] = {(States){.func = lex_expression,
                          .states = s5,
                          .size = sizeof(s5) / sizeof(States)}};
  States s3[] = {(States){.func = lex_assignment,
                          .states = s4,
                          .size = sizeof(s4) / sizeof(States)}};
  States s2[] = {(States){.func = lex_identifier,
                          .states = s3,
                          .size = sizeof(s3) / sizeof(States)}};
  States s1 = (States){
      .func = lex_keyword, .states = s2, .size = sizeof(s2) / sizeof(States)};

  lex(&lexer, s1);
  for (TokenList *l = lexer.head; l != NULL; l = l->next) {
    Token t = l->token;
    switch (t.type) {
      case kTokenTypeChar:
        printf("char: '%c'\n", t.character);
        break;
      case kTokenTypeError:
        printf("error: '%s'\n", t.error);
        free(t.error);
        break;
      case kTokenTypeFloat:
        printf("float: %f\n", t.floatng);
        break;
      case kTokenTypeIdent:
        printf("id: '%s'\n", t.ident);
        break;
      case kTokenTypeInteger:
        printf("integer: '%" PRIu64 "'\n", t.integer);
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
        printf("string: '%s'\n", t.string);
        break;
    }
    free(l);
  }
}
