#pragma once
#include <stdio.h>
#include "hashmap.h"
#include "tokenizer.h"
#include "expressions.h"
#include <stdbool.h>
#include "osstream.h"
#include "type.h"
#include "options.h"
#include "ownership.h"

#define CAKE_VERSION "0.6"


struct owner scope
{
    int scope_level;
    struct hash_map tags;
    struct hash_map variables;
        
    struct scope* next;
    struct scope* previous;        
};

void scope_destroy(implicit struct scope* obj_owner p);

struct scope_list
{
    struct scope* head;
    struct scope* tail;
};
void scope_list_push(struct scope_list* list, struct scope* s);
void scope_list_pop(struct scope_list* list);

struct report
{
    int error_count;
    int warnings_count;
    int info_count;
};




struct owner parser_ctx
{
    struct options options;
    
    /*
      file scope -> function params -> function -> inner scope
    */
    struct scope_list scopes;
    
    /*
    * Points to the function we're in. Or null in file scope.
    */
    struct declaration* p_current_function_opt;

    /*
    * Points to the try-block we're in. Or null.
    */
    const struct try_statement* p_current_try_statement_opt;

    FILE* sarif_file;

    struct token_list input_list;
    struct token* current;
    struct token* previous;
    int try_catch_block_index;
    int n_warnings;
    int n_errors;
    int n_info;
};

///////////////////////////////////////////////////////

void parser_ctx_destroy(implicit struct parser_ctx* obj_owner ctx);


struct token* parser_look_ahead(struct parser_ctx* ctx);

struct token* parser_match(struct parser_ctx* ctx);
int parser_match_tk(struct parser_ctx* ctx, enum token_type type);
struct token* parser_look_ahead(struct parser_ctx* ctx);
struct token* previous_parser_token(struct token* token);
struct declarator* find_declarator(struct parser_ctx* ctx, const char* lexeme, struct scope** ppscope_opt);
struct enumerator* find_enumerator(struct parser_ctx* ctx, const char* lexeme, struct scope** ppscope_opt);
struct map_entry* find_variables(struct parser_ctx* ctx, const char* lexeme, struct scope** ppscope_opt);

struct struct_or_union_specifier* find_struct_or_union_specifier(struct parser_ctx* ctx, const char* lexeme);
bool first_is(struct parser_ctx* ctx, enum token_type type);
void print_scope(struct scope_list* e);

char* CompileText(const char* options, const char* content);

void compiler_set_error_with_token(enum error error, struct parser_ctx* ctx, const struct token* p_token, const char* fmt, ...);
_Bool compiler_set_warning_with_token(enum warning w, struct parser_ctx* ctx, const struct token* p_token, const char* fmt, ...);
void compiler_set_info_with_token(enum warning w, struct parser_ctx* ctx, const struct token* p_token, const char* fmt, ...);

int compile(int argc, const char** argv, struct report* error);
struct declaration_list parse(struct parser_ctx* ctx, struct token_list* list, struct report* report);


void print_type_qualifier_flags(struct osstream* ss, bool* first, enum type_qualifier_flags e_type_qualifier_flags);

enum token_type parse_number(const char* lexeme, enum type_specifier_flags* flags_opt);
bool print_type_specifier_flags(struct osstream* ss, bool* first, enum type_specifier_flags e_type_specifier_flags);

enum storage_class_specifier_flags
{
    STORAGE_SPECIFIER_NONE = 0,
    STORAGE_SPECIFIER_TYPEDEF = 1 << 0,
    STORAGE_SPECIFIER_EXTERN = 1 << 1,
    STORAGE_SPECIFIER_STATIC = 1 << 2,
    STORAGE_SPECIFIER_THREAD_LOCAL = 1 << 3,
    STORAGE_SPECIFIER_AUTO = 1 << 4,
    STORAGE_SPECIFIER_REGISTER = 1 << 5,
    STORAGE_SPECIFIER_CONSTEXPR = 1 << 6,
    
    /*extra flag just to annotate this*/
    STORAGE_SPECIFIER_CONSTEXPR_STATIC = 1 << 7,
};

struct expression_ctx;

struct declaration_specifier
{
    /*
       declaration-specifier:
         storage-class-specifier
         type-specifier-qualifier
         function-specifier
    */
    struct storage_class_specifier* storage_class_specifier;

    struct type_specifier_qualifier* type_specifier_qualifier;
    
    struct function_specifier* function_specifier;

    struct declaration_specifier* next;
};

struct declaration_specifier* declaration_specifier(struct parser_ctx* ctx);


struct declaration_specifiers
{
    /*
     declaration-specifiers:
       declaration-specifier attribute-specifier-sequence opt
       declaration-specifier declaration-specifiers
    */

    /*cumulative flags*/
    enum attribute_flags  attributes_flags;
    enum type_specifier_flags type_specifier_flags;
    enum type_qualifier_flags type_qualifier_flags;
    enum storage_class_specifier_flags storage_class_specifier_flags;

    /*shortcuts*/
    struct struct_or_union_specifier* struct_or_union_specifier;
    struct enum_specifier* enum_specifier;
    struct declarator* typedef_declarator;
    struct typeof_specifier* typeof_specifier;

    struct token* first_token; /*not owner*/
    struct token* last_token; /*not owner*/

    struct declaration_specifier* head;
    struct declaration_specifier* tail;
};

void print_declaration_specifiers(struct osstream* ss, struct declaration_specifiers* p);
struct declaration_specifiers* declaration_specifiers(struct parser_ctx* ctx);

struct static_assert_declaration
{
    /*
     static_assert-declaration:
       "static_assert" ( constant-expression , string-literal ) ;
       "static_assert" ( constant-expression ) ;
    */
    
    struct token* first_token;
    struct token* last_token;
    struct expression* constant_expression;
    struct token* string_literal_opt;
};
struct static_assert_declaration* static_assert_declaration(struct parser_ctx* ctx);

struct attribute_specifier_sequence
{
    /*
     attribute-specifier-sequence:
       attribute-specifier-sequence opt attribute-specifier
    */

    struct token* first_token;
    struct token* last_token;
    enum attribute_flags  attributes_flags;
    struct attribute_specifier* head;
    struct attribute_specifier* tail;
};
struct attribute_specifier_sequence* attribute_specifier_sequence_opt(struct parser_ctx* ctx);

struct attribute_specifier
{
    /*
     attribute-specifier:
        [ [ attribute-list ] ]
    */
    struct token* first_token;
    struct token* last_token;
    struct attribute_list* attribute_list;
    struct attribute_specifier* next;
};

struct attribute_specifier* attribute_specifier(struct parser_ctx* ctx);
struct attribute* attribute(struct parser_ctx* ctx);


struct storage_class_specifier
{
    /*
     storage-class-specifier:
      "auto"
      "constexpr"
      "extern"
      "register"
      "static"
      "thread_local"
      "typedef"
    */
    enum storage_class_specifier_flags flags;
    struct token* token;
};

struct storage_class_specifier* storage_class_specifier(struct parser_ctx* ctx);

struct function_specifier
{
    /*
     function-specifier:
       inline
       _Noreturn
    */
    struct token* token;
};
struct function_specifier* function_specifier(struct parser_ctx* ctx);


struct typeof_specifier_argument
{
    /*
     typeof-specifier-argument:
       expression
       type-name
    */
    struct expression* expression;
    struct type_name* type_name;
};

struct typeof_specifier
{
    /*
     typeof-specifier:
       "typeof" ( typeof-specifier-argument )
    */
    struct token* first_token;
    struct token* last_token;
    struct typeof_specifier_argument* typeof_specifier_argument;
    struct type type;    
};

struct type_specifier
{
    /*
     type-specifier:
        "void"
        "char"
        "short"
        "int"
        "long"
        "float"
        "double"
        "signed"
        "unsigned"
        "_BitInt" ( constant-expression )
        "bool"
        "_Complex"
        "_Decimal32"
        "_Decimal64"
        "_Decimal128"
        atomic-type-specifier
        struct-or-union-specifier
        enum-specifier
        typedef-name
        typeof-specifier
    */
    enum type_specifier_flags flags;
    struct token* token;
    struct struct_or_union_specifier* struct_or_union_specifier;
    struct typeof_specifier* typeof_specifier;
    struct enum_specifier* enum_specifier;
    struct declarator* typedef_declarator;
    struct atomic_type_specifier* atomic_type_specifier;
};

struct type_specifier* type_specifier(struct parser_ctx* ctx);

struct init_declarator_list
{
    /*
     init-declarator-list:
       init-declarator
       init-declarator-list , init-declarator
    */
    struct init_declarator* head;
    struct init_declarator* tail;    
};

struct init_declarator_list init_declarator_list(struct parser_ctx* ctx,
    struct declaration_specifiers* p_declaration_specifiers,
    struct attribute_specifier_sequence* p_attribute_specifier_sequence_opt
    );

struct declaration
{
    /*
      declaration:
        declaration-specifiers init-declarator-list opt ;
        attribute-specifier-sequence declaration-specifiers init-declarator-list ;
        static_assert-declaration
        attribute-declaration
    */
    struct attribute_specifier_sequence* p_attribute_specifier_sequence_opt;
    
    struct static_assert_declaration* static_assert_declaration;

    struct declaration_specifiers* declaration_specifiers;
    struct init_declarator_list init_declarator_list;

    struct compound_statement* function_body;
    struct declarator* contract_declarator;

    struct token* first_token;
    struct token* last_token;

    struct declaration* next;
};

struct declaration* external_declaration(struct parser_ctx* ctx);

struct atomic_type_specifier
{
    /*
      atomic-type-specifier:
        "_Atomic" ( type-name )
    */
    struct token* token;
    struct type_name* type_name;
};

struct atomic_type_specifier* atomic_type_specifier(struct parser_ctx* ctx);

struct enumerator_list
{
    /*
     enumerator-list:
       enumerator
       enumerator-list , enumerator
    */
    struct enumerator* head;
    struct enumerator* tail;
};

struct enumerator_list enumerator_list(struct parser_ctx* ctx,
    struct enum_specifier*  p_enum_specifier
    );


struct enum_specifier
{
    /*
     enum-type-specifier:
       : specifier-qualifier-lis
    
     enum-specifier:
       "enum" attribute-specifier-sequence opt identifier opt enum-type-specifier opt  { enumerator-list }
       "enum" attribute-specifier-sequence opt identifier opt enum-type-specifier opt  { enumerator-list , }
       "enum" identifier enum-type-specifier opt
    */
    struct attribute_specifier_sequence* attribute_specifier_sequence_opt;
    struct specifier_qualifier_list* specifier_qualifier_list;
    

    struct enumerator_list enumerator_list;
    
    struct token* tag_token;
    struct token* first_token;
    /*points to the complete enum (can be self pointed)*/
    struct enum_specifier* complete_enum_specifier;
};

struct enum_specifier* enum_specifier(struct parser_ctx*);

struct member_declaration_list
{
    /*
     member-declaration-list:
       member-declaration
       member-declaration-list member-declaration
    */

    struct token* first_token; /*TODO ? necessary*/
    struct token* last_token;
    struct member_declaration* head;
    struct member_declaration* tail;
};

struct member_declaration_list member_declaration_list(struct parser_ctx* ctx);
struct member_declarator* find_member_declarator(struct member_declaration_list* list, const char* name);

struct struct_or_union_specifier
{
    /*
     struct-or-union-specifier:
       struct-or-union attribute-specifier-sequence opt identifier opt { member-declaration-list }
       struct-or-union attribute-specifier-sequence opt identifier
    */
    struct attribute_specifier_sequence* attribute_specifier_sequence_opt;
    struct member_declaration_list member_declaration_list;
    
    struct token* first_token;
    struct token* last_token;
    struct token* owner_token;

    /*
    * Token que possui tag da struct
    */
    struct token* tagtoken;

    char tag_name[200];
    /*geramos um tag name para anomimas, mas colocamos banonymousTag para true*/
    bool has_anonymous_tag;
    /*it was asked to show struct tag created for anonymous*/
    bool show_anonymous_tag;

    int scope_level; /*nivel escopo 0 global*/
    int visit_moved; /*nivel escopo 0 global*/
        
    /*
    * This points to the first struct_or_union_specifier that will have it´s
    * complete_struct_or_union_specifier_indirection pointing to the complete
    * struct_or_union_specifier.
    */
    struct struct_or_union_specifier* complete_struct_or_union_specifier_indirection;
};

struct struct_or_union_specifier* struct_or_union_specifier(struct parser_ctx* ctx);
bool struct_or_union_specifier_is_complete(struct struct_or_union_specifier* p_struct_or_union_specifier);
struct struct_or_union_specifier* get_complete_struct_or_union_specifier(struct struct_or_union_specifier* p_struct_or_union_specifier);

struct init_declarator
{
    /*
     init-declarator:
        declarator
        declarator = initializer
    */

    struct declarator* p_declarator;
    struct initializer* initializer;
    struct init_declarator* next;    
};

struct init_declarator* init_declarator(struct parser_ctx* ctx,
    struct declaration_specifiers* p_declaration_specifiers,
    struct attribute_specifier_sequence* p_attribute_specifier_sequence_opt
    );

struct initializer
{
    /*
     initializer:
       assignment-expression
       braced-initializer
    */
    struct token* first_token;
    struct designation* designation; /*auxiliar para list??*/
    struct braced_initializer* braced_initializer;
    struct expression* assignment_expression;
    struct initializer* next;
    struct token * move_token;
    /*
       cake extension
       int * p = [[cake::move]] p2;
    */
    struct attribute_specifier_sequence* p_attribute_specifier_sequence_opt;
};

struct initializer* initializer(struct parser_ctx* ctx);

enum declarator_flags
{
    DECLARATOR_UNINITIALIZED = 0,
    DECLARATOR_ISVALID = 1 << 1,    
    DECLARATOR_MUST_DESTROY = 1 << 3,
    DECLARATOR_MUST_FREE = 1 << 4
};

struct declarator
{
    /*
      declarator:
        pointer opt direct-declarator
    */

    struct token* first_token;
    struct token* last_token;


    struct pointer* pointer;
    struct direct_declarator* direct_declarator;

    
    struct declaration_specifiers* declaration_specifiers; //aponta para este cara
    struct specifier_qualifier_list* specifier_qualifier_list; //aponta
    struct token* name; //shortcut

    struct compound_statement* function_body;
           
    int num_uses; /*used to show not used warnings*/

    bool is_parameter_declarator;
    
    enum declarator_flags declarator_flags;    

    /*Já mastiga o tipo dele*/
    struct type type;    
};

enum type_specifier_flags declarator_get_type_specifier_flags(const struct declarator* p);

struct declarator;
void print_declarator(struct osstream* ss, struct declarator* declarator, bool is_abstract);

struct declarator* declarator(struct parser_ctx* ctx,
    struct specifier_qualifier_list* specifier_qualifier_list,
    struct declaration_specifiers* declaration_specifiers,
    bool abstract_acceptable,
    struct token** pptokenname);

struct array_declarator
{
    /*
     array-declarator:
        direct-declarator [ type-qualifier-list opt assignment-expression opt ]
        direct-declarator [ "static" type-qualifier-list opt assignment-expression ]
        direct-declarator [ type-qualifier-list "static" assignment-expression ]
        direct-declarator [ type-qualifier-listopt * ]
    */
    struct direct_declarator* direct_declarator;
    struct expression* assignment_expression;
    struct expression* expression;
    struct type_qualifier_list* type_qualifier_list_opt;
    
    struct token* token;
    struct token* static_token_opt;
};

/*
  Return a value > 0 if it has constant size
*/
unsigned long long  array_declarator_get_size(struct array_declarator* p_array_declarator);

struct function_declarator
{
    /*
     function-declarator:
       direct-declarator ( parameter-type-list opt )
    */
    struct direct_declarator* direct_declarator;
    struct scope parameters_scope; //usado para escopo parametros
    struct parameter_type_list* parameter_type_list_opt;
};

struct direct_declarator
{
    /*
     direct-declarator:
        identifier attribute-specifier-sequence opt
        ( declarator )
        array-declarator attribute-specifier-sequence opt
        function-declarator attribute-specifier-sequence opt
    */
    struct token* name_opt;
    struct declarator* declarator;
    struct array_declarator* array_declarator;
    struct function_declarator* function_declarator;
};

struct direct_declarator* direct_declarator(struct parser_ctx* ctx,
    struct specifier_qualifier_list* specifier_qualifier_list,
    struct declaration_specifiers* declaration_specifiers,
    bool abstract_acceptable,
    struct token** pptoken_name
    );

struct parameter_type_list
{
    /*
    parameter-type-list:
        parameter-list
        parameter-list , ...
        ...
    */
    bool is_var_args; /*(...)*/
    bool is_void;/*(void)*/
    struct parameter_list* parameter_list;
};

struct parameter_type_list* parameter_type_list(struct parser_ctx* ctx);

struct pointer
{
    /*
     pointer:
        * attribute-specifier-sequence opt type-qualifier-list opt
        * attribute-specifier-sequence opt type-qualifier-list opt pointer    
    */
    struct attribute_specifier_sequence* attribute_specifier_sequence_opt;
    struct type_qualifier_list* type_qualifier_list_opt;
    struct pointer* pointer;
};

struct pointer* pointer_opt(struct parser_ctx* ctx);

struct parameter_list
{
    /*
     parameter-list:
        parameter-declaration
        parameter-list , parameter-declaration
    */
    struct parameter_declaration* head;
    struct parameter_declaration* tail;
};
struct parameter_list* parameter_list(struct parser_ctx* ctx);

struct parameter_declaration
{
    /*
     parameter-declaration:
      attribute-specifier-sequence opt declaration-specifiers declarator
      attribute-specifier-sequence opt declaration-specifiers abstract-declarator opt
    */
    struct attribute_specifier_sequence* attribute_specifier_sequence_opt;
    
    struct declaration_specifiers* declaration_specifiers;
    struct declarator* declarator;
    struct token * implicit_token;
    struct parameter_declaration* next;
};
struct parameter_declaration* parameter_declaration(struct parser_ctx* ctx);

struct type_name
{
    /*
     type-name:
       specifier-qualifier-list abstract-declarator opt
    */

    struct token* first_token;
    struct token* last_token;
    struct specifier_qualifier_list* specifier_qualifier_list;
    struct declarator* declarator;
    struct type type;
};

struct type_name* type_name(struct parser_ctx* ctx);
void print_type_name(struct osstream* ss, struct type_name* p);

struct argument_expression
{
    struct token * move_token;
    struct expression* expression;
    struct argument_expression* next;
};

struct braced_initializer
{    
    /*
      { }
      { initializer-list }
      { initializer-list , }
    */

    struct token* first_token;
    struct initializer_list* initializer_list;
};
struct braced_initializer* braced_initializer(struct parser_ctx* ctx);

struct type_specifier_qualifier
{
    /*
     type-specifier-qualifier:
       type-specifier
       type-qualifier
       alignment-specifier
    */

    struct type_specifier* type_specifier;
    struct type_qualifier* type_qualifier;
    struct alignment_specifier* alignment_specifier;

    struct type_specifier_qualifier* next;
};
struct type_specifier_qualifier* type_specifier_qualifier(struct parser_ctx* ctx);


struct specifier_qualifier_list
{
    /*
      specifier-qualifier-list:
        type-specifier-qualifier attribute-specifier-sequence opt
        type-specifier-qualifier specifier-qualifier-list
    */

    /*cumulative flags*/
    enum type_specifier_flags type_specifier_flags;
    enum type_qualifier_flags type_qualifier_flags;

    /*shortcuts*/
    struct struct_or_union_specifier* struct_or_union_specifier;
    struct enum_specifier* enum_specifier;
    struct typeof_specifier* typeof_specifier;
    struct declarator* typedef_declarator;
    
    struct type_specifier_qualifier* head;
    struct type_specifier_qualifier* tail;
    struct token* first_token;
    struct token* last_token;

};

struct specifier_qualifier_list* specifier_qualifier_list(struct parser_ctx* ctx);
void print_specifier_qualifier_list(struct osstream* ss, bool* first, struct specifier_qualifier_list* p_specifier_qualifier_list);


struct alignment_specifier
{
    /*
     alignment-specifier:
       "alignas" ( type-name )
       "alignas" ( constant-expression )
    */
    struct token* token;
};
struct alignment_specifier* alignment_specifier(struct parser_ctx* ctx);


struct type_qualifier
{
    /*
     type-qualifier:
       "const"
       "restrict"
       "volatile"
       "_Atomic"
    */
    enum type_qualifier_flags flags;
    struct token* token;
    struct type_qualifier* next;
};

struct type_qualifier* type_qualifier(struct parser_ctx* ctx);

struct member_declaration
{
    /*
     member-declaration:
       attribute-specifier-sequence opt specifier-qualifier-list member-declarator-list opt ;
       static_assert-declaration
    */
    struct specifier_qualifier_list* specifier_qualifier_list;
    struct member_declarator_list* member_declarator_list_opt;

    struct static_assert_declaration* static_assert_declaration;

    struct member_declaration* next;
};

struct member_declaration* member_declaration(struct parser_ctx* ctx);

struct member_declarator
{
    /*    
     member-declarator:
       declarator
       declarator opt : constant-expression
    */
    
    struct declarator* declarator;
    struct expression* constant_expression;
    struct member_declarator* next;
};

struct member_declarator_list
{
    /*    
     member-declarator-list:
        member-declarator
        member-declarator-list , member-declarator
    */

    struct member_declarator* head;
    struct member_declarator* tail;
};

struct member_declarator_list* member_declarator_list(struct parser_ctx* ctx,
    struct specifier_qualifier_list* specifier_qualifier_list
    );

struct block_item_list
{
    /*
     block-item-list:
       block-item
       block-item-list block-item
    */
    struct block_item* head;
    struct block_item* tail;
};
struct block_item_list block_item_list(struct parser_ctx* ctx);


struct compound_statement
{
    /*
     compound-statement:
       { block-item-list opt }
    */
    struct token* first_token; /*{*/
    struct token* last_token; /*}*/

    struct block_item_list block_item_list;
};
struct compound_statement* compound_statement(struct parser_ctx* ctx);

struct defer_statement
{
    /*
     defer-statement: (extension)
       "defer" secondary-block     
    */
    struct token* first_token;
    struct token* last_token;
    struct secondary_block* secondary_block;
};

struct try_statement
{   
   /*
     try-statement: (extension)
      "try" secondary-block
      "try" secondary-block "catch" secondary-block
   */
    struct secondary_block* secondary_block;
    struct secondary_block* catch_secondary_block_opt;
    struct token* first_token; /*try*/
    struct token* last_token;
    struct token* catch_token_opt; /*catch*/
    /*Used to generate label names*/
    int try_catch_block_index;
};

struct try_statement* try_statement(struct parser_ctx* ctx);

struct selection_statement
{
    /*
      selection-statement:
        "if" ( expression ) secondary-block
        "if" ( expression ) secondary-block "else" secondary-block
        "switch" ( expression ) secondary-block
    */

    /*C++ 17 if with initialization extension*/
    struct init_declarator* init_declarator;

    struct expression* expression;
    struct secondary_block* secondary_block;
    struct secondary_block* else_secondary_block_opt;

    struct token* first_token;
    struct token* last_token;
    struct token* else_token_opt;    
};

struct selection_statement* selection_statement(struct parser_ctx* ctx);

struct iteration_statement
{
    /*
      iteration-statement:
        "while" ( expression ) secondary-block
        "do" secondary-block "while" ( expression ) ;
        "for" ( expression opt ; expression opt ; expression opt ) secondary-block
        "for" ( declaration expression opt ; expression opt ) secondary-block
        "repeat" secondary-block  (extension)
    */
    
    struct token* first_token;
    struct token* second_token; /*do {} while*/

    struct secondary_block* secondary_block;
    struct expression* expression1;
    struct expression* expression2;
};

struct iteration_statement* iteration_statement(struct parser_ctx* ctx);

struct jump_statement
{
    /*
     jump-statement:
       "goto" identifier ;
       "continue" ;
       "break" ;
       "return" expression opt ;
    */
    
    struct token* label;
    struct token* first_token;
    struct token* last_token;
    struct expression* expression_opt;
    
    int try_catch_block_index;
};

struct jump_statement* jump_statement(struct parser_ctx* ctx);

struct expression_statement
{
    /*
     expression-statement:
       expression opt;
       attribute-specifier-sequence expression ;
    */
    
    struct attribute_specifier_sequence* p_attribute_specifier_sequence_opt;
    struct expression* expression_opt;
};

struct expression_statement* expression_statement(struct parser_ctx* ctx);

struct block_item
{
    /*
     block-item:
      declaration
      unlabeled-statement
      label
    */
    struct token* first_token; //?necessary
    struct declaration* declaration;    
    struct unlabeled_statement* unlabeled_statement;
    struct label* label;

    struct block_item* next;
};
struct block_item* block_item(struct parser_ctx* ctx);

struct compound_statement* function_body(struct parser_ctx* ctx);

struct designator
{
    /*
     designator:
       [ constant-expression ]
       . identifier
    */
    struct expression* constant_expression_opt;
    struct token* token;
    struct designator* next;
};
struct designator* designator(struct parser_ctx* ctx);

struct initializer_list
{
    /*
      initializer-list:
        designation opt initializer
        initializer-list , designation opt initializer
    */
    struct token* first_token;
    struct initializer* head;
    struct initializer* tail;
    int size;
};
struct initializer_list* initializer_list(struct parser_ctx* ctx);

struct primary_block
{
    /*
       primary-block:
         compound-statement
         selection-statement
         iteration-statement
         defer-statement (extension)
         try-statement (extension)
    */
    
    struct compound_statement* compound_statement;
    struct selection_statement* selection_statement;
    struct iteration_statement* iteration_statement;
    struct defer_statement* defer_statement;
    struct try_statement* try_statement;
};

struct secondary_block
{
    /*
     secondary-block:
       statement
    */
    struct token* first_token;
    struct token* last_token;
    struct statement* statement;
};

struct unlabeled_statement
{
    /*
      unlabeled-statement:
        expression-statement
        attribute-specifier-sequence opt primary-block
        attribute-specifier-sequence opt jump-statement
     */

    struct expression_statement* expression_statement;
    struct primary_block* primary_block;
    struct jump_statement* jump_statement;
};

struct unlabeled_statement* unlabeled_statement(struct parser_ctx* ctx);

struct labeled_statement
{
    /*
     label statement:
       label statement
    */
    struct label* label;
    struct statement* statement;
};
struct labeled_statement* labeled_statement(struct parser_ctx* ctx);

struct statement
{    
    /*
     statement:
       labeled-statement
       unlabeled-statemen
    */
    struct labeled_statement* labeled_statement;
    struct unlabeled_statement* unlabeled_statement;
};
struct statement* statement(struct parser_ctx* ctx);

struct designator_list
{
    /*
     designator-list:
       designator
       designator-list designator
    */
    struct designator* head;
    struct designator* tail;
};

struct designator_list* designator_list(struct parser_ctx* ctx);

struct designation
{
    /*
     designation:
       designator-list =
    */
    struct token* token;
};
struct designation* designation(struct parser_ctx* ctx);


struct type_qualifier_list
{
    /*
     type-qualifier-list:
       type-qualifier
       type-qualifier-list type-qualifier
    */
    enum type_qualifier_flags flags;
    struct type_qualifier* head;
    struct type_qualifier* tail;
};

struct type_qualifier_list* type_qualifier_list(struct parser_ctx* ctx);

struct attribute_token
{
    enum attribute_flags attributes_flags;
    struct token* token;
};
struct attribute_token* attribute_token(struct parser_ctx* ctx);

struct attribute
{
    enum attribute_flags  attributes_flags;
    struct attribute_token* attribute_token;
    struct attribute_argument_clause* attribute_argument_clause;
    struct attribute_specifier* next;
};

struct attribute_list
{
    enum attribute_flags  attributes_flags;
    struct attribute* head;
    struct attribute* tail;
};
struct attribute_list* attribute_list(struct parser_ctx* ctx);

struct enumerator
{
    /*
      enumeration-constant:
        identifier
   
      enumerator:
        enumeration-constant attribute-specifier-sequence opt
        enumeration-constant attribute-specifier-sequence opt = constant-expression
    */
    
    struct token* token;
    struct attribute_specifier_sequence* attribute_specifier_sequence_opt;

    struct expression* constant_expression_opt;
    struct enum_specifier* enum_specifier;
    struct enumerator* next;
    long long value;    
};

struct enumerator* enumerator(struct parser_ctx* ctx, struct enum_specifier* p_enum_specifier);

struct attribute_argument_clause
{
    /*
     attribute-argument-clause:
       ( balanced-token-sequence opt )
    */
    struct token* token;
};

struct attribute_argument_clause* attribute_argument_clause(struct parser_ctx* ctx);

bool first_of_attribute(struct parser_ctx* ctx);

struct balanced_token_sequence
{
    struct balanced_token* head;
    struct balanced_token* tail;
};
struct balanced_token_sequence* balanced_token_sequence_opt(struct parser_ctx* ctx);

bool is_first_of_conditional_expression(struct parser_ctx* ctx);
bool first_of_type_name(struct parser_ctx* ctx);
bool first_of_type_name_ahead(struct parser_ctx* ctx);

struct argument_expression_list argument_expression_list(struct parser_ctx* ctx);

struct declaration_list
{
    struct declaration* head;
    struct declaration* tail;
};

struct declaration_list translation_unit(struct parser_ctx* ctx);
void declaration_list_destroy(struct declaration_list* list);

struct label
{
    /*
     label:
       attribute-specifier-sequence opt identifier :
       attribute-specifier-sequence opt "case" constant-expression :
       attribute-specifier-sequence opt "default" :
    */
    struct expression* constant_expression;
    struct token* name;
};

struct label* label(struct parser_ctx* ctx);

struct owner ast
{
    struct token_list token_list;
    struct declaration_list declaration_list;
};


struct ast get_ast(struct options* options, const char* filename, const char* source, struct report* report);
void ast_destroy(implicit struct ast* obj_owner ast);
struct type make_type_using_declarator(struct parser_ctx* ctx, struct declarator* pdeclarator);


