
#ifndef SJKJSON_H_
#define SJKJSON_H_

#ifndef SJKJSONDEF
#define SJKJSONDEF
#endif

#include <stdio.h>
#include <stdbool.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define PRETTY_PRINT_SPACES_AMT 4

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
} SJKJSON_Token_Kind;

typedef struct {
  SJKJSON_Token_Kind kind;
  String_View value;
} SJKJSON_Token;

typedef struct {
  SJKJSON_Token* items;
  size_t count;
  size_t capacity;
} SJKJSON_Tokens;

typedef enum {
  JSON_NONE,
  JSON_ARRAY,
  JSON_OBJECT,
  JSON_STRING,
  JSON_NUM,
  JSON_BOOL,
  JSON_NULL,
  JSON_COUNT
} SJKJSON_JSON_Kind;

typedef struct SJKJSON_JSON_Element SJKJSON_JSON_Element;
typedef struct SJKJSON_JSON_Elements SJKJSON_JSON_Elements;

typedef struct {
  SJKJSON_JSON_Elements** items;
  size_t count;
  size_t capacity;
} SJKJSON_Roots;

struct SJKJSON_JSON_Element {
  SJKJSON_JSON_Kind kind;
  String_View key;
  union {
    SJKJSON_JSON_Elements* jarray;
    SJKJSON_JSON_Elements* jobject;
    String_View jstring;
    double jnum;
    bool jbool;
  } value;
}; 

struct SJKJSON_JSON_Elements {
  SJKJSON_JSON_Element* items;
  size_t count;
  size_t capacity;
};

SJKJSONDEF SJKJSON_JSON_Element* parse_json_file(const char* src_file_path);
SJKJSONDEF void SJKJSON_dump_json(String_Builder* sb, SJKJSON_JSON_Element* json, size_t indent_amt);
SJKJSONDEF void append_to_json(SJKJSON_JSON_Elements* root, SJKJSON_JSON_Element* j);
SJKJSONDEF SJKJSON_JSON_Elements* push_root(SJKJSON_Roots* roots, SJKJSON_JSON_Elements* new_root);
SJKJSONDEF SJKJSON_JSON_Elements* pull_root(SJKJSON_Roots* roots); 
SJKJSONDEF bool write_sb_to_file(const char* file_path, String_Builder sb);
#endif // SJKJSON_H_

// ---------------PRIVATE---------------

const char* get_token_kind(SJKJSON_Token_Kind kind) {
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

const char* get_json_kind(SJKJSON_JSON_Kind kind) {
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


String_View* sv_dup(String_View isv) {
  String_View* osv = (String_View*)malloc(sizeof(String_View));
  osv->count = isv.count;
  osv->data = strdup(isv.data);
  return osv;
}

SJKJSON_JSON_Elements* make_json_elements();

SJKJSON_JSON_Element* make_json_element(SJKJSON_JSON_Kind kind, String_View* key) {
  SJKJSON_JSON_Element* j = (SJKJSON_JSON_Element*)malloc(sizeof(SJKJSON_JSON_Element));
  memset(j, 0, sizeof(*j));
  j->kind = kind;
  switch (kind) {
    case JSON_ARRAY: j->value.jarray = make_json_elements(); break;
    case JSON_OBJECT: j->value.jobject = make_json_elements(); break;
    case JSON_STRING: j->value.jstring = sv_from_parts("", 0); break;
    case JSON_NUM: j->value.jnum = 0; break;
    case JSON_BOOL: j->value.jbool = false; break;
    default: {};
  }
  if (key && key->count > 0)
    j->key = *sv_dup(*key);
  return j;
}

SJKJSON_JSON_Elements* make_json_elements() {
  SJKJSON_JSON_Elements* j = (SJKJSON_JSON_Elements*)malloc(sizeof(SJKJSON_JSON_Element));
  memset(j, 0, sizeof(*j));
  return j;
}

String_View* make_sv(const char* str) {
  String_View* sv = (String_View*)malloc(sizeof(String_View));
  memset(sv, 0, sizeof(*sv));
  sv->data = strdup(str);
  sv->count = strlen(str)-1;
  return sv;
}

SJKJSON_JSON_Element* make_json_string(const char* key, const char* value) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_STRING, &keysv);
  j->value.jstring = sv_from_cstr(value);
  return j;
}

SJKJSON_JSON_Element* make_json_num(const char* key, double num) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_NUM, &keysv);
  j->value.jnum = num;
  return j;
}

SJKJSON_JSON_Element* make_json_bool(const char* key, bool b) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_BOOL, &keysv);
  j->value.jbool = b;
  return j;
}

SJKJSON_JSON_Element* make_json_null(const char* key) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_NULL, &keysv);
  return j;
}

SJKJSON_JSON_Element* make_json_object(const char* key) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_OBJECT, &keysv);
  return j;
}

SJKJSON_JSON_Element* make_json_array(const char* key) {
  String_View keysv = sv_from_cstr(key);
  SJKJSON_JSON_Element* j = make_json_element(JSON_ARRAY, &keysv);
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

SJKJSON_Token* token_get_next(SJKJSON_Tokens* tokens) {
  if (tokens->count == 0) return NULL;
  SJKJSON_Token* t = &tokens->items[0];
  tokens->count--;
  if (tokens->count > 0)
    tokens->items = &tokens->items[1];
  return t;
}

SJKJSON_Token* token_peek_next(SJKJSON_Tokens* tokens) {
  if (tokens->count == 0) return NULL;
  return &tokens->items[0];
}

void token_skip(SJKJSON_Tokens* tokens) {
  SJKJSON_Token* t = token_get_next(tokens);
  UNUSED(t);
}

double sv_to_double(String_View sv) {
  const char* str = temp_sv_to_cstr(sv); 
  char* endPtr;
  double d = strtod(str, &endPtr);
  if (endPtr == str) {
    nob_log(ERROR, "Could not convert %s into a double!", str);
    return 0;
  }
  return d;
}

String_View* sv_empty() {
  String_View* sv = (String_View*)malloc(sizeof(String_View));
  memset(sv, 0, sizeof(*sv));
  return sv;
}

SJKJSON_JSON_Element* get_last(SJKJSON_JSON_Elements* root) {
  if (root->count == 0)
    return NULL;
  return &da_last(root);
}

SJKJSON_JSON_Elements* ParseTokens(SJKJSON_Tokens* tokens) {
  SJKJSON_JSON_Elements* root = make_json_elements();
  SJKJSON_Token* t = token_get_next(tokens);
  while (t) {
    switch (t->kind) {
      case TOKEN_STRING: {
        SJKJSON_Token* next = token_peek_next(tokens);
        if (!next) {
          nob_log(ERROR, "Could not peek the next token!");
          return root;
        }
        // check if this STRING is a key
        if (next->kind == TOKEN_COLON) {
          SJKJSON_JSON_Element* j = make_json_element(JSON_NONE, &t->value);
          da_append(root, *j);
        } else {
          SJKJSON_JSON_Element* last = get_last(root);
          if (last && last->kind == JSON_NONE) {
            last->kind = JSON_STRING;
            last->value.jstring = *sv_dup(t->value);
          } else {
            SJKJSON_JSON_Element* j = make_json_element(JSON_STRING, NULL);
            j->value.jstring = *sv_dup(t->value);
            da_append(root, *j);
          }
        }
      } break;
      case TOKEN_NUM: {
        SJKJSON_JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_NUM;
          last->value.jnum = sv_to_double(t->value); 
        } else {
          SJKJSON_JSON_Element* j = make_json_element(JSON_NUM, NULL);
          j->value.jnum = sv_to_double(t->value);
          da_append(root, *j);
        }
      } break;
      case TOKEN_BOOL: {
        SJKJSON_JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_BOOL;
          last->value.jbool = strcmp(t->value.data, "true") == 0 ? true : false;
        } else {
          SJKJSON_JSON_Element* j = make_json_element(JSON_BOOL, NULL);
          j->value.jbool = strcmp(t->value.data, "true") == 0 ? true : false;
          da_append(root, *j);
        }
      } break;
      case TOKEN_NULL: {
        SJKJSON_JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_NULL;
        } else {
          SJKJSON_JSON_Element* j = make_json_element(JSON_NULL, NULL);
          da_append(root, *j);
        }
      } break;
      case TOKEN_OPEN_SQ: {
        SJKJSON_JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_ARRAY;
          last->value.jarray = ParseTokens(tokens); 
        } else {
          SJKJSON_JSON_Element* j = make_json_element(JSON_ARRAY, NULL);
          j->value.jarray = ParseTokens(tokens);
          da_append(root, *j);
        }
      } break;
      case TOKEN_CLOSE_SQ: {
        return root;
      } break;
      case TOKEN_OPEN_CURLY: {
        SJKJSON_JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_OBJECT;
          last->value.jobject = ParseTokens(tokens);
        } else {
          SJKJSON_JSON_Element* j = make_json_element(JSON_OBJECT, NULL);
          j->value.jobject = ParseTokens(tokens);
          da_append(root, *j);
        }
      } break;
      case TOKEN_CLOSE_CURLY: {
        return root;
      } break;
      case TOKEN_COLON: {} break;
      case TOKEN_COMMA: {} break;
      default: {}
    }
    t = token_get_next(tokens);
  }

  return root; 
}

const char* sb_to_cstr(String_Builder *sb) {
  char* s = (char*)malloc(sizeof(char)*sb->count+1);
  for (size_t i = 0; i < sb->count; ++i){
    s[i] = sb->items[i];
  }
  s[sb->count] = '\0';
  return (const char*)s;
}

void AppendToken(SJKJSON_Tokens* tokens, SJKJSON_Token_Kind kind, const char* value) {
  SJKJSON_Token* t = (SJKJSON_Token*)malloc(sizeof(SJKJSON_Token));
  t->kind = kind;
  t->value = sv_from_cstr(value);
  da_append(tokens, *t);
}

void Tokenize(String_Builder file_data, SJKJSON_Tokens* tokens) {

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
      SJKJSON_Token_Kind k = TOKEN_BOOL;
      if (strcmp(s, "null") == 0)
        k = TOKEN_NULL;
      AppendToken(tokens, k, s);
      free(n.items);
    }
  }

}

void get_spaces(String_Builder* sb, size_t indent_amt) {
  for (size_t i = 0; i < indent_amt; ++i) {
    sb_appendf(sb, " ");
  }
}

void print_sb(String_Builder sb) {
  String_View sv = sb_to_sv(sb);
  nob_log(INFO, SV_Fmt, SV_Arg(sv));
}

void dump_json_element(String_Builder* sb, SJKJSON_JSON_Element j, size_t indent_amt) {
  get_spaces(sb, indent_amt);
  if (j.key.count > 0)
    sb_appendf(sb, "\""SV_Fmt"\": ", SV_Arg(j.key));
  switch (j.kind) {
    case JSON_STRING: sb_appendf(sb, "\""SV_Fmt"\"", SV_Arg(j.value.jstring)); break;
    case JSON_NUM: sb_appendf(sb, "%f", j.value.jnum); break;
    case JSON_BOOL: sb_appendf(sb, "%s", j.value.jbool ? "true" : "false"); break;
    case JSON_NULL: sb_appendf(sb, "null"); break;
    default: {};
  }
}

void dump_json_collection(String_Builder* sb, SJKJSON_JSON_Element* j, size_t indent_amt);

void dump_json_collection(String_Builder* sb, SJKJSON_JSON_Element* j, size_t indent_amt) {
  char opening = j->kind == JSON_ARRAY ? '[' : '{'; 
  char closing = j->kind == JSON_ARRAY ? ']' : '}';
  SJKJSON_JSON_Elements* els = j->kind == JSON_ARRAY ? j->value.jarray : j->value.jobject;

  get_spaces(sb, indent_amt);
  if (j->key.count > 0) { 
    sb_appendf(sb, "\""SV_Fmt"\": %c", SV_Arg(j->key), opening);
  } else {
    sb_append(sb, opening);
  }
  sb_append(sb, '\n');

  indent_amt += PRETTY_PRINT_SPACES_AMT;
  for (size_t i = 0; i < els->count; ++i) {
    SJKJSON_dump_json(sb, &els->items[i], indent_amt);
    sb_appendf(sb, ",\n");
  }
  indent_amt -= PRETTY_PRINT_SPACES_AMT;

  if (sb->count > 1) {
    char last[2];
    last[0] = sb->items[sb->count-2];
    last[1] = sb->items[sb->count-1];
    if (strcmp(last, ",\n") == 0) {
      sb->items[sb->count-2] = '\n';
      sb->count -= 1;
    }
  }

  get_spaces(sb, indent_amt);
  sb_append(sb, closing);
}

void dump_json_elements(String_Builder* sb, SJKJSON_JSON_Elements* j, size_t indent_amt) {
  for (size_t i = 0; i < j->count; ++i) {
    SJKJSON_JSON_Element* e = &j->items[i];
    switch (e->kind) {
      case JSON_ARRAY: {
         
      } break;
      case JSON_OBJECT: {
        dump_json_elements(sb, e->value.jobject, indent_amt);
      } break;
      default: dump_json_element(sb, *e, indent_amt);
    }
    if (i + 1 < j->count) {
        sb_appendf(sb, ",\n");
    }
  }  
}

void print_tokens(SJKJSON_Tokens tokens) {
  if (tokens.count == 0) return;
  SJKJSON_Token* token = token_get_next(&tokens);
  size_t i = 0;
  while (token) {
    nob_log(INFO, "\nkind: %s\nvalue: "SV_Fmt, get_token_kind(token->kind), SV_Arg(token->value));
    token = token_get_next(&tokens);
    if (i > 10)
      break;
    ++i;
  }
}

#ifdef SJKJSON_IMPLEMENTATION

SJKJSONDEF SJKJSON_JSON_Element* SJKJSON_parse_json_file(const char* src_file_path) {
  String_Builder sb = {0};
  if (!read_entire_file(src_file_path, &sb)) return NULL;

  SJKJSON_Tokens tokens = {0};
  Tokenize(sb, &tokens); 

  SJKJSON_JSON_Elements* json = ParseTokens(&tokens);

  SJKJSON_JSON_Element* j = NULL;

  if (json->count > 0) {
    if (json->items[0].kind == JSON_ARRAY) {
      j = make_json_element(JSON_ARRAY, sv_empty());
      j->value.jarray = json; 
    } else if (json->items[0].kind == JSON_OBJECT) {
      j = make_json_element(JSON_OBJECT, sv_empty());
      j->value.jarray = json; 
    }
  }

  return j;
}

SJKJSONDEF void SJKJSON_dump_json(String_Builder* sb, SJKJSON_JSON_Element* json, size_t indent_amt) {
  switch(json->kind) {
    case JSON_ARRAY: dump_json_collection(sb, json, indent_amt); break;
    case JSON_OBJECT: dump_json_collection(sb, json, indent_amt); break;
    default: dump_json_element(sb, *json, indent_amt);
  }
}

SJKJSONDEF void SJKJSON_append_to_json(SJKJSON_JSON_Elements* root, SJKJSON_JSON_Element* j) {
  da_append(root, *j);
}

SJKJSONDEF SJKJSON_JSON_Elements* SJKJSON_push_root(SJKJSON_Roots* roots, SJKJSON_JSON_Elements* new_root) {
  da_append(roots, new_root);
  return new_root;
}

SJKJSONDEF SJKJSON_JSON_Elements* SJKJSON_pull_root(SJKJSON_Roots* roots) {
  if (roots->count == 0) return NULL;
  roots->count -= 1;
  if (roots->count == 0) return NULL;
  return da_last(roots);
}

SJKJSONDEF bool SJKJSON_write_sb_to_file(const char* file_path, String_Builder sb) {
  return write_entire_file(file_path, sb.items, sb.count);
}

#endif // SJKJSON_IMPLEMENTATION

#ifndef SJKJSON_NO_PREFIX
#define SJKJSON_NO_PREFIX
  #ifndef SJKJSON_YES_PREFIX
    #define JSON_Element SJKJSON_JSON_Element
    #define JSON_Elements SJKJSON_JSON_Elements
    #define Roots SJKJSON_Roots
    #define parse_json_file SJKJSON_parse_json_file
    #define dump_json SJKJSON_dump_json
    #define write_sb_to_file SJKJSON_write_sb_to_file 
    #define append_to_json SJKJSON_append_to_json
    #define push_root SJKJSON_push_root
    #define pull_root SJKJSON_pull_root
  #endif // SJKJSON_YES_PREFIX
#endif // SJKJSON_NO_PREFIX


