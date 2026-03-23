#include <stdio.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

typedef enum {
  TOKEN_STRING,
  TOKEN_NUM,
  TOKEN_BOOL,
  TOKEN_NULL,
  TOKEN_OPEN_SQ,
  TOKEN_CLOSE_SQ,
  TOKEN_OPEN_CURLY,
  TOKEN_CLOSE_CURLY,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_COUNT
} Token_Kind;

typedef struct {
  Token_Kind kind;
  String_View value;
} Token;

typedef struct {
  Token* items;
  size_t count;
  size_t capacity;
} Tokens;

typedef enum {
  JSON_NONE,
  JSON_ARRAY,
  JSON_OBJECT,
  JSON_STRING,
  JSON_NUM,
  JSON_BOOL,
  JSON_NULL,
  JSON_COUNT
} JSON_Kind;

const char* get_json_kind(JSON_Kind kind) {
  switch (kind) {
    case JSON_NONE: return "none";
    case JSON_ARRAY: return "array";
    case JSON_OBJECT: return "object";
    case JSON_STRING: return "string";
    case JSON_NUM: return "num";
    case JSON_BOOL: return "bool";
    case JSON_NULL: return "null";
    default: return "";
  }
}

typedef struct JSON_Element JSON_Element;
typedef struct JSON_Elements JSON_Elements;

struct JSON_Element {
  JSON_Kind kind;
  String_View key;
  union {
    JSON_Elements* jarray;
    JSON_Elements* jobject;
    String_View jstring;
    double jnum;
    bool jbool;
  } value;
}; 

struct JSON_Elements {
  JSON_Element* items;
  size_t count;
  size_t capacity;
  bool open;
};

const char* get_token_kind(Token_Kind kind) {
  switch (kind) { 
    case TOKEN_STRING: return "string";
    case TOKEN_NUM: return "num";
    case TOKEN_BOOL: return "bool";
    case TOKEN_NULL: return "null";
    case TOKEN_OPEN_SQ: return "open square bracket";
    case TOKEN_CLOSE_SQ: return "close square bracket";
    case TOKEN_OPEN_CURLY: return "open curly bracket";
    case TOKEN_CLOSE_CURLY: return "close curly brakcet";
    case TOKEN_COLON: return "colon";
    case TOKEN_COMMA: return "comma";
    default: return "";
  }
}
String_View* sv_dup(String_View isv) {
  String_View* osv = (String_View*)malloc(sizeof(String_View));
  osv->count = isv.count;
  osv->data = strdup(isv.data);
  return osv;
}

JSON_Elements* make_json_elements();

JSON_Element* make_json_element(JSON_Kind kind, String_View* key) {
  JSON_Element* j = (JSON_Element*)malloc(sizeof(JSON_Element));
  memset(j, 0, sizeof(*j));
  j->kind = kind;
  if (kind == JSON_ARRAY)
    j->value.jarray = make_json_elements();
  if (kind == JSON_OBJECT)
    j->value.jobject = make_json_elements();
  if (key && key->count > 0)
    j->key = *sv_dup(*key);
  return j;
}

JSON_Elements* make_json_elements() {
  JSON_Elements* j = (JSON_Elements*)malloc(sizeof(JSON_Element));
  memset(j, 0, sizeof(*j));
  j->open = true;
  return j;
}

bool sb_eq_jbool(String_Builder sb) {
  if (sb.count != 4 && sb.count != 5) return false;
  String_View sv = sb_to_sv(sb);
  const char* str = temp_sv_to_cstr(sv);
  if (sb.count == 4)
    return strcmp(str, "true") == 0;
  return strcmp(str, "false") == 0;
}

bool sb_eq_jnull(String_Builder sb) {
  if (sb.count != 4) return false;
  String_View sv = sb_to_sv(sb);
  return strcmp(temp_sv_to_cstr(sv), "null") == 0;
}

Token* token_get_next(Tokens* tokens) {
  if (tokens->count == 0) return NULL;
  Token* t = &tokens->items[0];
  tokens->count--;
  if (tokens->count > 0)
    tokens->items = &tokens->items[1];
  return t;
}

Token* token_peek_next(Tokens* tokens) {
  if (tokens->count == 0) return NULL;
  return &tokens->items[0];
}

void token_skip(Tokens* tokens) {
  Token* t = token_get_next(tokens);
  UNUSED(t);
}

void ParseTokens(Tokens* tokens, JSON_Elements* root) {
  Token* t = token_get_next(tokens);
  while (t) {
    switch (t->kind) {
      case TOKEN_STRING: {
        Token* next = token_peek_next(tokens);
        if (!next) {
          nob_log(ERROR, "Could not peek the next token!");
          return;
        }
        // check if this STRING is a key
        if (next->kind == TOKEN_COLON) {
          JSON_Element* j = make_json_element(JSON_NONE, &t->value);
          da_append(root, *j);
        } else {
          JSON_Element* j = &da_last(root);
          if (j->kind == JSON_NONE && j->key.count > 0) {
            j->kind = JSON_STRING;
            j->value.jstring = *sv_dup(t->value);
          } else if (j->kind == JSON_ARRAY) {
            JSON_Element* e = make_json_element(JSON_STRING, &t->value);
            da_append(j->value.jarray, *e);
          }
        }
      } break;
      case TOKEN_NUM: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);
          j->kind = JSON_NUM;
          char* endPtr;
          j->value.jnum = strtod(t->value.data, &endPtr);
          if (endPtr == t->value.data) {
            nob_log(ERROR, "Could not convert %s into a double!", t->value.data);
          }
        }
      } break;
      case TOKEN_BOOL: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);
          j->kind = JSON_BOOL;
          j->value.jbool = strcmp(t->value.data, "true") == 0 ? true : false;
        }
      } break;
      case TOKEN_NULL: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);
          j->kind = JSON_NULL;
        }
      } break;
      case TOKEN_OPEN_SQ: {
        if (root->count > 0) {
          // TODO: make this work next       
        } else {
          JSON_Element* j = make_json_element(JSON_ARRAY, NULL); 
          da_append(root, *j);
          ParseTokens(tokens, j->value.jarray);
        }
      } break;
      case TOKEN_CLOSE_SQ: {
        root->open = false;
        return;
      } break;
      case TOKEN_OPEN_CURLY: {
        if (root->count > 0) {
          JSON_Element* last = &da_last(root);
          assert(root->open && "Root JSON Object is not open!");
          if (last->kind == JSON_NONE) {
            last->kind = JSON_OBJECT;
            last->value.jobject = make_json_elements();
            ParseTokens(tokens, last->value.jobject);
          }
        } else {
          JSON_Element* j = make_json_element(JSON_OBJECT, NULL);
          da_append(root, *j);
          ParseTokens(tokens, j->value.jobject);
        }
      } break;
      case TOKEN_CLOSE_CURLY: {
        root->open = false;
        return;
      } break;
      case TOKEN_COLON: {} break;
      case TOKEN_COMMA: {} break;
      default: {}
    }
    t = token_get_next(tokens);
  }
}

const char* sb_to_cstr(String_Builder *sb) {
  char* s = (char*)malloc(sizeof(char)*sb->count+1);
  for (size_t i = 0; i < sb->count; ++i){
    s[i] = sb->items[i];
  }
  s[sb->count] = '\0';
  return (const char*)s;
}

void AppendToken(Tokens* tokens, Token_Kind kind, const char* value) {
  Token* t = (Token*)malloc(sizeof(Token));
  t->kind = kind;
  t->value = sv_from_cstr(value);
  da_append(tokens, *t);
}

void Tokenize(String_Builder file_data, Tokens* tokens) {

  for (size_t i = 0; i < file_data.count; ++i) {
    char c = file_data.items[i];

    if (isspace(c))
      continue;

    if (c == '[') {
      AppendToken(tokens, TOKEN_OPEN_SQ, "[");
    } else if (c == ']') {
      AppendToken(tokens, TOKEN_CLOSE_SQ, "]");
    } else if (c == '{') {
      AppendToken(tokens, TOKEN_OPEN_CURLY, "{");
    } else if (c == '}') {
      AppendToken(tokens, TOKEN_CLOSE_CURLY, "}");
    } else if (c == ':') {
      AppendToken(tokens, TOKEN_COLON, ":");
    } else if (c == ',') {
      AppendToken(tokens, TOKEN_COMMA, ",");
    } else if (c >= '0' && c <= '9') {
      String_Builder n = {0};
      sb_append(&n, c); 
      while (i < file_data.count) {
        c = file_data.items[++i];
        if ((c >= '0' && c <= '9') || c == '.') {
          sb_append(&n, c);
        } else {
          break;
        }
      }
      const char* s = sb_to_cstr(&n);
      AppendToken(tokens, TOKEN_NUM, s);
      free(n.items);
    } else if (c == '"') {
      String_Builder n = {0};
      while (i < file_data.count) {
        c = file_data.items[++i];
        if (c == '"') {
          break;
        }
        sb_append(&n, c);
      }
      const char* s = sb_to_cstr(&n);
      AppendToken(tokens, TOKEN_STRING, s);
      free(n.items);
    } else if (c == 'n' || c == 't' || c == 'f') {
      // check for null, true or false
      String_Builder n = {0};
      sb_append(&n, c);
      while (i < file_data.count) {
        c = file_data.items[++i];
        sb_append(&n, c);
        if (sb_eq_jnull(n) || sb_eq_jbool(n)) {
          break;
        }
      }
      const char* s = sb_to_cstr(&n);
      Token_Kind k = TOKEN_BOOL;
      if (strcmp(s, "null") == 0)
        k = TOKEN_NULL;
      AppendToken(tokens, k, s);
      free(n.items);
    }
  }

}

void print_json(JSON_Elements json) {
  if (json.count == 0) return;
  for (size_t i = 0; i < json.count; ++i) {
    JSON_Element j = json.items[i];
    switch (j.kind) {
    case JSON_STRING: nob_log(INFO, "\nkey: "SV_Fmt"\nvalue: "SV_Fmt, SV_Arg(j.key), SV_Arg(j.value.jstring)); break;
    case JSON_NUM: nob_log(INFO, "\nkey: "SV_Fmt"\nvalue: %f", SV_Arg(j.key), j.value.jnum); break;
    case JSON_BOOL: nob_log(INFO, "\nkey: "SV_Fmt"\nvalue: %d", SV_Arg(j.key), j.value.jbool); break;
    case JSON_NULL: nob_log(INFO, "\nkey: "SV_Fmt"\nvalue: null", SV_Arg(j.key)); break;
    case JSON_ARRAY: print_json(*j.value.jarray); break;
    case JSON_OBJECT: print_json(*j.value.jobject); break;
    default: {};
    }
  }
}


void print_tokens(Tokens tokens) {
  if (tokens.count == 0) return;
  Token* token = token_get_next(&tokens);
  size_t i = 0;
  while (token) {
    nob_log(INFO, "\nkind: %s\nvalue: "SV_Fmt, get_token_kind(token->kind), SV_Arg(token->value));
    token = token_get_next(&tokens);
    if (i > 10)
      break;
    ++i;
  }
}

int main(void) {
  const char* src_file_path = "./products.json";

  String_Builder sb = {0};
  if (!read_entire_file(src_file_path, &sb)) return 1;

  Tokens tokens = {0};
  Tokenize(sb, &tokens); 
  //print_tokens(tokens);
  
  JSON_Elements json = {0};
  ParseTokens(&tokens, &json);
  print_json(json);

  return 0;
}
