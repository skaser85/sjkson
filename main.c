#include <stdio.h>

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
    TOKEN_DQ,
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
            AppendToken(tokens, TOKEN_DQ, "\"");
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
                if (c == '"') {
                    --i;
                    break;
                }
                sb_append(&n, c);
            }
            const char* s = sb_to_cstr(&n);
            if (strcmp(s, "true") == 0 || strcmp(s, "false") == 0) {
                AppendToken(tokens, TOKEN_BOOL, s);
            } else {
                AppendToken(tokens, TOKEN_STRING, s);
            }
            free(n.items);
        }
    }

}


int main(void) {
    const char* src_file_path = "./64KB.json";

    String_Builder sb = {0};
    if (!read_entire_file(src_file_path, &sb)) return 1;

    Tokens tokens = {0};
    
    Tokenize(sb, &tokens); 

    for (size_t i = 0; i < tokens.count; ++i) {
        nob_log(INFO, SV_Fmt, SV_Arg(tokens.items[i].value));
    }

    return 0;
}
