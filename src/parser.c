#include "parser.h"
#include "lexer.h"
#include "easy_access.h"
#include "file_reader.h"
#include "syn_tree.h"
#include "tokens.h"

parser_* init_parser(lexer_* lexer, char* active_file) {
    parser_* parser = calloc(1,sizeof(*parser));
    parser->PKG_INFO = calloc(1,sizeof(*parser->PKG_INFO));
    parser->active_file = active_file;

    parser->lexer = lexer;
    parser->current_token_info = get_next_token(lexer);
    parser->last_token_info = parser->current_token_info;
    //parser->PKG_INFO->all_imports = calloc(1,sizeof(*parser->PKG_INFO->all_imports));
    //parser->PKG_INFO->amount_of_imports = 0;
    parser->PKG_INFO->current_import_name = calloc(1,sizeof(char));
    //parser->PKG_INFO->export_values = calloc(1,sizeof(char*));
    //parser->PKG_INFO->imports = calloc(1,sizeof(parser->PKG_INFO->imports));

    return parser;
}

static inline parser_* gather_next_token(parser_* parser, int current_token_id) {
    if(parser->current_token_info->token_id==current_token_id) {
        parser->last_token_info = parser->current_token_info;
        parser->current_token_info = get_next_token(parser->lexer);
    } else {
        raise_error("\nTokens do not match(%s)\n\n",parser->current_token_info->token_value);
    }

    return parser;
}

/*
    Simple way to check basic operations:
    ':' and '='
*/
static inline void* check_operation(parser_* parser, char* PKG_, int throw_err) {

    if(parser->current_token_info->token_id==TOKEN_EQUALS) {
        gather_next_token(parser, TOKEN_EQUALS);
    }
    else if(parser->current_token_info->token_id == TOKEN_COLON) {
        gather_next_token(parser, TOKEN_COLON);
    }
    else {
        if(throw_err == 0) {
            raise_error("\nMissing key token ideal for %s\n\n",PKG_);
        }
    }
    return parser;
}

/*
    Imports in Jang is a very strict concept:
        When you import another .jang file, that jang file will be ran..fully..as if it is the main file
*/
// IDEALS NEEDED FOR THE SYN_TREE_ STRUCT
static int import_amount;
char** arr;
void** parser_current_state;
SYN_TREE_* IMPORT(parser_* parser, SYN_TREE_* syntax_tree) {
    if(!(arr)) {
        arr = calloc(1,sizeof(char**));
    }
    if(!(parser_current_state)) {
        parser_current_state = calloc(1,sizeof(struct parser**));
    }
    parser->lexer->is_import = 0;
    /* TODO: Finsish This. This is a very sloppy version */
    gather_next_token(parser, IMPORTS_KEYWORD);
    Tokens_* tokens = calloc(1,sizeof(*tokens));
    lexer_* lexer = init_lexer(read_file(strcat(parser->current_token_info->token_value,".jang")),tokens);

    if(strcmp(parser->current_token_info->token_value,parser->active_file)==0) raise_error("\nCannot import main file(line %d)\n\n",parser->lexer->current_line);

    /* STORING IMPORTED FILE NAME */
    parser->PKG_INFO->current_import_name = parser->current_token_info->token_value;

    parser_* parser__ = init_parser(lexer, parser->current_token_info->token_value);
    parse(parser__);

    /* Storing all the names */
    arr = realloc(
        arr,
        (import_amount+1)*sizeof(char*)
    );
    arr[import_amount] = parser->PKG_INFO->current_import_name;
    import_amount++;

    parser->PKG_INFO->amount_of_imports = import_amount;
    /*parser->PKG_INFO->all_imports = realloc(
        parser->PKG_INFO->all_imports,
        (import_amount+1)*sizeof(*parser->PKG_INFO->all_imports)
    );
    parser->PKG_INFO->all_imports = arr;*/

    /* Setting up syntax tree */
    syntax_tree->import_names = arr;
    syntax_tree->amount_of_imports = parser->PKG_INFO->amount_of_imports;
    syntax_tree->parser_import_information = parser_current_state;

    if(parser->current_token_info->token_id == TOKEN_SEMI) gather_next_token(parser, TOKEN_SEMI);

    free(arr);
    return syntax_tree;
}

// Ideals needed for the PKG keyword Syntax Tree struct
static int amount_of_packages;
char** pkg_names;
void** curent_parser_state;
SYN_TREE_* Jang_Pkg_Setup(parser_* parser, SYN_TREE_* syntax_tree) {
    gather_next_token(parser, _PKG_KEYWORD);
        
    if(parser->current_token_info->token_id == TOKEN_LEFT_CURL) {
        gather_next_token(parser, TOKEN_LEFT_CURL);

        if(parser->current_token_info->token_id == TOKEN_RIGHT_CURL) gather_next_token(parser, TOKEN_RIGHT_CURL);
        else raise_error("\nMissing closing '}'(Line: %d,Characters: %d, `%s`)\n\n",parser->lexer->current_line,parser->lexer->character_number,parser->active_file);
    } else {
        raise_error("\nInvalid PKG format. Expected 'PKG{', found '%s'(Token Id Reported: '%d', Line %d, Characters: %d, `%s`)\n\n",parser->current_token_info->token_value,parser->current_token_info->token_id,parser->lexer->current_line,parser->lexer->character_number-1/* character_number-1 will be where the incorrect punctation is at */,parser->active_file);
    }
    return syntax_tree;
}

static inline void* parse_id(parser_* parser) {

    return parser;
}

SYN_TREE_* parse(parser_* parser) {
    switch(parser->current_token_info->token_id) {
        case IMPORTS_KEYWORD: return IMPORT(parser, init_syntax_tree(TREE_PKG));
        case _PKG_KEYWORD: return Jang_Pkg_Setup(parser,init_syntax_tree(TREE_PKG));
        case TOKEN_ID: return parse_id(parser);
        default: break;
    }
    return init_syntax_tree(TREE_EOF);
}
