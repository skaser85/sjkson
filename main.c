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

JSON_Elements* make_json_elements() {
  JSON_Elements* j = (JSON_Elements*)malloc(sizeof(JSON_Element));
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

JSON_Element* make_json_string(const char* key, const char* value) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_STRING, &keysv);
  j->value.jstring = sv_from_cstr(value);
  return j;
}

JSON_Element* make_json_num(const char* key, double num) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_NUM, &keysv);
  j->value.jnum = num;
  return j;
}

JSON_Element* make_json_bool(const char* key, bool b) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_BOOL, &keysv);
  j->value.jbool = b;
  return j;
}

JSON_Element* make_json_null(const char* key) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_NULL, &keysv);
  return j;
}

JSON_Element* make_json_object(const char* key) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_OBJECT, &keysv);
  return j;
}

JSON_Element* make_json_array(const char* key) {
  String_View keysv = sv_from_cstr(key);
  JSON_Element* j = make_json_element(JSON_ARRAY, &keysv);
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

JSON_Element* get_last(JSON_Elements* root) {
  if (root->count == 0)
    return NULL;
  return &da_last(root);
}
/*
JSON_Elements* ParseTokens(Tokens* tokens) {
  JSON_Elements* root = make_json_elements();
  Token* t = token_get_next(tokens);
  while (t) {
    switch (t->kind) {
      case TOKEN_STRING: {
        Token* next = token_peek_next(tokens);
        if (!next) {
          nob_log(ERROR, "Could not peek the next token!");
          return root;
        }
        // check if this STRING is a key
        if (next->kind == TOKEN_COLON) {
          JSON_Element* j = make_json_element(JSON_NONE, &t->value);
          da_append(root, *j);
        } else {
          JSON_Element* last = get_last(root);
          if (last && last->kind == JSON_NONE) {
            last->kind = JSON_STRING;
            last->value.jstring = *sv_dup(t->value);
          } else {
            JSON_Element* j = make_json_element(JSON_STRING, NULL);
            j->value.jstring = *sv_dup(t->value);
            da_append(root, *j);
          }
        }
      } break;
      case TOKEN_NUM: {
        JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_NUM;
          last->value.jnum = sv_to_double(t->value); 
        } else {
          JSON_Element* j = make_json_element(JSON_NUM, NULL);
          j->value.jnum = sv_to_double(t->value);
          da_append(root, *j);
        }
      } break;
      case TOKEN_BOOL: {
        JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_BOOL;
          last->value.jbool = strcmp(t->value.data, "true") == 0 ? true : false;
        } else {
          JSON_Element* j = make_json_element(JSON_BOOL, NULL);
          j->value.jbool = strcmp(t->value.data, "true") == 0 ? true : false;
          da_append(root, *j);
        }
      } break;
      case TOKEN_NULL: {
        JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_NULL;
        } else {
          JSON_Element* j = make_json_element(JSON_NULL, NULL);
          da_append(root, *j);
        }
      } break;
      case TOKEN_OPEN_SQ: {
        JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_ARRAY;
          last->value.jarray = ParseTokens(tokens);
        } else {
          JSON_Element* j = make_json_element(JSON_ARRAY, NULL);
          j->value.jobject = ParseTokens(tokens);
          da_append(root, *j);
        }
      } break;
      case TOKEN_CLOSE_SQ: {
        return root;
      } break;
      case TOKEN_OPEN_CURLY: {
        JSON_Element* last = get_last(root);
        if (last && last->kind == JSON_NONE) {
          last->kind = JSON_OBJECT;
          last->value.jobject = ParseTokens(tokens);
        } else {
          JSON_Element* j = make_json_element(JSON_OBJECT, NULL);
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
*/
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

void get_spaces(String_Builder* sb, size_t indent_amt) {
  for (size_t i = 0; i < indent_amt; ++i) {
    sb_appendf(sb, " ");
  }
}

void print_sb(String_Builder sb) {
  String_View sv = sb_to_sv(sb);
  nob_log(INFO, SV_Fmt, SV_Arg(sv));
}

void print_json_element(JSON_Element j, size_t indent_amt) {
  String_Builder sb = {0};
  get_spaces(&sb, indent_amt);
  if (j.key.count > 0)
    sb_appendf(&sb, "\""SV_Fmt"\": ", SV_Arg(j.key));
  switch (j.kind) {
    case JSON_STRING: sb_appendf(&sb, "\""SV_Fmt"\"", SV_Arg(j.value.jstring)); break;
    case JSON_NUM: sb_appendf(&sb, "%f", j.value.jnum); break;
    case JSON_BOOL: sb_appendf(&sb, "%s", j.value.jbool ? "true" : "false"); break;
    case JSON_NULL: sb_appendf(&sb, "null"); break;
    default: {};
  }
  print_sb(sb);
}

void print_json_collection(JSON_Element* j, size_t indent_amt);

void print_json(JSON_Element* json, size_t indent_amt) {
  switch(json->kind) {
    case JSON_ARRAY: print_json_collection(json, indent_amt); break;
    case JSON_OBJECT: print_json_collection(json, indent_amt); break;
    default: print_json_element(*json, indent_amt);
  }
}

void print_json_collection(JSON_Element* j, size_t indent_amt) {
  char opening = j->kind == JSON_ARRAY ? '[' : '{'; 
  char closing = j->kind == JSON_ARRAY ? ']' : '}';
  JSON_Elements* els = j->kind == JSON_ARRAY ? j->value.jarray : j->value.jobject;

  String_Builder sb = {0};
  get_spaces(&sb, indent_amt);
  if (j->key.count > 0) { 
    sb_appendf(&sb, "\""SV_Fmt"\": %c", SV_Arg(j->key), opening);
  } else {
    sb_append(&sb, opening);
  }
  print_sb(sb);
  
  indent_amt += PRETTY_PRINT_SPACES_AMT;
  for (size_t i = 0; i < els->count; ++i) {
    print_json(&els->items[i], indent_amt);
  }
  indent_amt -= PRETTY_PRINT_SPACES_AMT;
  
  sb.count = 0;
  get_spaces(&sb, indent_amt);
  sb_append(&sb, closing);
  print_sb(sb);
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
/*
JSON_Element* parse_json_file(const char* src_file_path) {
  String_Builder sb = {0};
  if (!read_entire_file(src_file_path, &sb)) return NULL;

  Tokens tokens = {0};
  Tokenize(sb, &tokens); 
  
  JSON_Elements* json = ParseTokens(&tokens);

  return json;
}
*/
void append_to_json(JSON_Elements* root, JSON_Element* j) {
  da_append(root, *j);
}

typedef struct {
  JSON_Elements** items;
  size_t count;
  size_t capacity;
} Roots;

JSON_Elements* push_root(Roots* roots, JSON_Elements* new_root) {
  da_append(roots, new_root);
  return new_root;
}

JSON_Elements* pull_root(Roots* roots) {
  if (roots->count == 0) return NULL;
  roots->count -= 1;
  if (roots->count == 0) return NULL;
  return da_last(roots);
}

JSON_Element* test_make_json() {
  Roots roots = {0};
  
  JSON_Element* r = make_json_array("");
  JSON_Elements* root = push_root(&roots, r->value.jarray);

  JSON_Element* o1 = make_json_object("");
  append_to_json(root, o1);
  root = push_root(&roots, o1->value.jobject);

  append_to_json(root, make_json_string("name", "France"));
  append_to_json(root, make_json_string("capital", "Paris"));
  append_to_json(root, make_json_num("population", 67364357));
  append_to_json(root,  make_json_num("area", 551695));
  append_to_json(root, make_json_string("currency", "Euro"));
  JSON_Element* lang = make_json_array("languages");
  root = push_root(&roots, lang->value.jarray);
  append_to_json(root, make_json_string("", "French")); 
  root = pull_root(&roots);
  append_to_json(root, lang);
  append_to_json(root, make_json_string("region", "Europe"));
  append_to_json(root, make_json_string("subregion", "Western Europe"));
  append_to_json(root, make_json_string("subregion", "https://upload.wikimedia.org/wikipedia/commons/c/c3/Flag_of_France.svg"));

  root = pull_root(&roots);

  return r;
}

int main(void) {
  JSON_Element* json = test_make_json();
  print_json(json, 0);

  return 0;
}
