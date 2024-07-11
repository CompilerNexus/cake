/*
 *  This file is part of cake compiler
 *  https://github.com/thradams/cake
*/

#pragma once

#include "type.h"
#include  "tokenizer.h"
#include "ownership.h"

struct parser_ctx;

enum expression_type
{
    PRIMARY_IDENTIFIER,
    

    PRIMARY_EXPRESSION_ENUMERATOR,
    PRIMARY_EXPRESSION_DECLARATOR,
    PRIMARY_EXPRESSION_STRING_LITERAL,
    PRIMARY_EXPRESSION__FUNC__, /*predefined identifier __func__ */
    PRIMARY_EXPRESSION_CHAR_LITERAL,
    PRIMARY_EXPRESSION_PREDEFINED_CONSTANT, /*true false*/
    PRIMARY_EXPRESSION_GENERIC,
    PRIMARY_EXPRESSION_NUMBER,
    PRIMARY_EXPRESSION_PARENTESIS,

    POSTFIX_EXPRESSION_FUNCTION_LITERAL,
    POSTFIX_EXPRESSION_COMPOUND_LITERAL,

    POSTFIX_FUNCTION_CALL, // ( ) 
    POSTFIX_ARRAY, // [ ]
    POSTFIX_DOT, // .
    POSTFIX_ARROW, // .
    POSTFIX_INCREMENT,
    POSTFIX_DECREMENT,


    UNARY_EXPRESSION_SIZEOF_EXPRESSION,
    UNARY_EXPRESSION_SIZEOF_TYPE,    
    
    UNARY_EXPRESSION_TRAITS,
    UNARY_EXPRESSION_IS_SAME,
    UNARY_DECLARATOR_ATTRIBUTE_EXPR,
    UNARY_EXPRESSION_ALIGNOF,
    UNARY_EXPRESSION_ASSERT,

    UNARY_EXPRESSION_INCREMENT,
    UNARY_EXPRESSION_DECREMENT,

    UNARY_EXPRESSION_NOT,
    UNARY_EXPRESSION_BITNOT,
    UNARY_EXPRESSION_NEG,
    UNARY_EXPRESSION_PLUS,
    UNARY_EXPRESSION_CONTENT,
    UNARY_EXPRESSION_ADDRESSOF,

    CAST_EXPRESSION,

    MULTIPLICATIVE_EXPRESSION_MULT,
    MULTIPLICATIVE_EXPRESSION_DIV,
    MULTIPLICATIVE_EXPRESSION_MOD,

    ADDITIVE_EXPRESSION_PLUS,
    ADDITIVE_EXPRESSION_MINUS,

    SHIFT_EXPRESSION_RIGHT,
    SHIFT_EXPRESSION_LEFT,

    RELATIONAL_EXPRESSION_BIGGER_THAN,
    RELATIONAL_EXPRESSION_LESS_THAN,
    RELATIONAL_EXPRESSION_BIGGER_OR_EQUAL_THAN,
    RELATIONAL_EXPRESSION_LESS_OR_EQUAL_THAN,

    EQUALITY_EXPRESSION_EQUAL,
    EQUALITY_EXPRESSION_NOT_EQUAL,

    AND_EXPRESSION,
    EXCLUSIVE_OR_EXPRESSION,
    INCLUSIVE_OR_EXPRESSION,
    
    LOGICAL_OR_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    ASSIGNMENT_EXPRESSION,

    CONDITIONAL_EXPRESSION,
};

struct argument_expression_list
{
    /*
     argument-expression-list:
        assignment-expression
        argument-expression-list , assignment-expression
    */
    struct argument_expression* _Owner _Opt head;
    struct argument_expression* _Opt tail;
};

void argument_expression_list_destroy(struct argument_expression_list * _Obj_owner p);
void argument_expression_list_push(struct argument_expression_list * list, struct argument_expression* _Owner p);

struct generic_association
{
    /*
     generic-association:
       type-name : assignment-expression
       "default" : assignment-expression
    */

    struct type type;
    struct type_name* _Owner _Opt p_type_name;
    struct expression* _Owner expression;

    struct token* first_token;
    struct token* last_token;

    struct generic_association* _Owner _Opt next;
};

void generic_association_delete(struct generic_association* _Owner _Opt p);

struct generic_assoc_list
{
    struct generic_association* _Owner _Opt head;
    struct generic_association* _Opt tail;
};

void generic_assoc_list_add(struct generic_assoc_list * p, struct generic_association* _Owner item);
void generic_assoc_list_destroy(struct generic_assoc_list * _Obj_owner p);

struct generic_selection
{
    /*
      generic-selection:
        "_Generic" ( assignment-expression , generic-assoc-list )
    */


    /*
      Extension
      generic-selection:
        "_Generic" ( generic-argument, generic-assoc-list )

        generic-argument:
          assignment-expression
          type-name
    */


    struct expression* _Owner _Opt expression;
    struct type_name* _Owner _Opt type_name;
    /*
    * Points to the matching expression
    */
    struct expression* p_view_selected_expression;

    struct generic_assoc_list generic_assoc_list;
    struct token* first_token;
    struct token* last_token;
};

void generic_selection_delete(struct generic_selection * _Owner _Opt p);

enum constant_value_type {
    TYPE_NOT_CONSTANT,
    TYPE_LONG_LONG,
    TYPE_DOUBLE,
    TYPE_UNSIGNED_LONG_LONG
};

struct constant_value {       
    enum constant_value_type type;
    union {
        unsigned long long ullvalue;
        long long llvalue;
        double dvalue;
    };
};

struct constant_value make_constant_value_double(double d, bool disabled);
struct constant_value make_constant_value_ull(unsigned long long d, bool disabled);
struct constant_value make_constant_value_ll(long long d, bool disabled);

struct constant_value constant_value_op(const struct constant_value* a, const  struct constant_value* b, int op);
unsigned long long constant_value_to_ull(const struct constant_value* a);
long long constant_value_to_ll(const struct constant_value* a);
long long constant_value_to_ll(const struct constant_value* a);
bool constant_value_to_bool(const struct constant_value* a);
bool constant_value_is_valid(const struct constant_value* a);
void constant_value_to_string(const struct constant_value* a, char buffer[], int sz);

struct expression
{
    enum expression_type expression_type;
    struct type type;    

    struct constant_value constant_value;

    struct type_name* _Owner _Opt type_name; 
    
    struct braced_initializer* _Owner _Opt braced_initializer;
    struct compound_statement* _Owner _Opt compound_statement; //function literal (lambda)
    struct generic_selection* _Owner _Opt generic_selection; //_Generic

    struct token* first_token;
    struct token* last_token;

    
    /*se expressão for um identificador ele aponta para declaração dele*/
    struct declarator* _Opt declarator;
    int member_index; //used in post_fix .

    /*se for POSTFIX_FUNCTION_CALL post*/
    struct argument_expression_list argument_expression_list; //este node eh uma  chamada de funcao

    struct expression* _Owner _Opt condition_expr;
    struct expression* _Owner _Opt left;
    struct expression* _Owner _Opt right;

    bool is_assignment_expression;
 };

//built-in semantics
bool expression_is_malloc(const struct expression* p);
bool expression_is_calloc(const struct expression* p);

void expression_delete(struct expression* _Owner _Opt p);

struct expression* _Owner _Opt assignment_expression(struct parser_ctx* ctx);
struct expression* _Owner _Opt expression(struct parser_ctx* ctx);
struct expression* _Owner _Opt constant_expression(struct parser_ctx* ctx, bool show_error_if_not_constant);
bool expression_is_subjected_to_lvalue_conversion(const struct expression*);
bool expression_is_zero(const struct expression*);
bool expression_is_lvalue(const struct expression* expr);


bool expression_is_null_pointer_constant(const struct expression* expression);
void expression_evaluate_equal_not_equal(const struct expression* left,
    const struct expression* right,
    struct expression* result,
    int op,
    bool disabled);

void check_diferent_enuns(struct parser_ctx* ctx,
                                 const struct token* operator_token,
                                 struct expression* left,
                                 struct expression* right,
                                 const char * message);

void check_assigment(struct parser_ctx* ctx,
    struct type* left_type,
    struct expression* right,
    enum assigment_type assigment_type);
