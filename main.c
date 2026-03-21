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
  JSON_ARRAY,
  JSON_OBECT,
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

String_View* sv_dup(String_View isv) {
  String_View* osv = (String_View*)malloc(sizeof(String_View));
  osv->count = isv.count;
  osv->data = strdup(isv.data);
  return osv;
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

void ParseTokens(Tokens tokens, JSON_Elements* root) {
    for (size_t t = 0; t < tokens.count; ++t) {
      Token token = tokens.items[t];
      switch (token.kind) {
      case TOKEN_STRING: {
        if (tokens.items[t+1].kind == TOKEN_COLON) {
          JSON_Element* j = (JSON_Element*)malloc(sizeof(JSON_Element));
          j->key = *sv_dup(token.value);
          da_append(root, *j);
        } else {
          if (root->count > 0) {
            JSON_Element* j = &da_last(root);
            j->kind = JSON_STRING;
            j->value.jstring = *sv_dup(token.value);
          }
        }
      } break;
      case TOKEN_NUM: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);
          j->kind = JSON_NUM;
          char* endPtr;
          j->value.jnum = strtod(token.value.data, &endPtr);
          if (endPtr == token.value.data) {
            nob_log(ERROR, "Could not convert %s into a double!", token.value.data);
          }
        }
      } break;
      case TOKEN_BOOL: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);
          j->kind = JSON_BOOL;
          j->value.jbool = strcmp(token.value.data, "true") == 0 ? true : false;
        }
      } break;
      case TOKEN_NULL: {
        if (root->count > 0) {
          JSON_Element* j = &da_last(root);;
          j->kind = JSON_NULL;
        }
      } break;
      case TOKEN_OPEN_SQ: {} break;
      case TOKEN_CLOSE_SQ: {} break;
      case TOKEN_OPEN_CURLY: {} break;
      case TOKEN_CLOSE_CURLY: {} break;
      case TOKEN_COLON: {} break;
      case TOKEN_COMMA: {} break;
      default: {}
      }
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

        if (c == '[') {
            AppendToken(tokens, TOKEN_OPEN_SQ, "[");
        } else if (c == ']') {
            AppendToken(tokens, TOKEN_CLOSE_SQ, "]");
        } else if (c == '{') {
            AppendToken(tokens, TOKEN_OPEN_CURLY, "{");
        } else if (c == '}') {
            AppendToken(tokens, TOKEN_CLOSE_CURLY, "}");
        } else if (c == '"') {
            continue;
        } else if (c == ':') {
            AppendToken(tokens, TOKEN_COLON, ":");
        } else if (c == ',') {
            AppendToken(tokens, TOKEN_COMMA, ",");
        } else if (isspace(c)) {
            continue;
        } else if (c >= '0' && c <= '9') {
            String_Builder n = {0};
            sb_append(&n, c); 
            while (i < file_data.count) {
                c = file_data.items[++i];
                if ((c >= '0' && c <= '9') || c == '.') {
                    sb_append(&n, c);
                } else {
                    --i;
                    break;
                }
            }
            const char* s = sb_to_cstr(&n);
            AppendToken(tokens, TOKEN_NUM, s);
            free(n.items);
        } else {
            String_Builder n = {0};
            sb_append(&n, c);
            while (i < file_data.count) {
                c = file_data.items[++i];
                /*if (c == '"' || c == ',' || 
                    c == '{' || c == '}' || 
                    c == '[' || c == ']' ||
                    isspace(c)) {*/
                if (c == '"' ||
                    sb_eq_jbool(n) ||
                    sb_eq_jnull(n)) {
                    --i;
                    break;
                }
                sb_append(&n, c);
            }
            const char* s = sb_to_cstr(&n);
            if (strcmp(s, "true") == 0 || strcmp(s, "false") == 0) {
                AppendToken(tokens, TOKEN_BOOL, s);
            } else if (strcmp(s, "null") == 0) {
                AppendToken(tokens, TOKEN_NULL, s);
            } else {
                AppendToken(tokens, TOKEN_STRING, s);
            }
            free(n.items);
        }
    }

}


int main(void) {
    const char* src_file_path = "./64KB2.json";

    String_Builder sb = {0};
    if (!read_entire_file(src_file_path, &sb)) return 1;

    Tokens tokens = {0};
    JSON_Elements json = {0};
    
    Tokenize(sb, &tokens); 
    ParseTokens(tokens, &json);
    /*
    for (size_t i = 0; i < tokens.count; ++i) {
        nob_log(INFO, SV_Fmt, SV_Arg(tokens.items[i].value));
    }
    */

    nob_log(INFO, "%ld", json.count);
    for (size_t i = 0; i < json.count; ++i) {
        JSON_Element j = json.items[i];
        if (j.kind == JSON_STRING) {
          nob_log(INFO, "\nkey: %s\nvalue: "SV_Fmt, j.key.data, SV_Arg(j.value.jstring));
        } else if (j.kind == JSON_NUM) {
          nob_log(INFO, "\nkey: %s\nvalue: %f", j.key.data, j.value.jnum);
        } else if (j.kind == JSON_BOOL) {
          nob_log(INFO, "\nkey: %s\nvalue: %d", j.key.data, j.value.jbool);
        } else if (j.kind == JSON_NULL) {
          nob_log(INFO, "\nkey: %s\nvalue: null", j.key.data);
        }
    }

    return 0;
}
