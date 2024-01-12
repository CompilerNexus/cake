/*do not edit this file*/

#include <stdio.h>

#ifdef TEST
#define TESTCODE
#endif
int g_unit_test_error_count = 0;
int g_unit_test_success_count = 0;
#ifdef TESTCODE

/*forward declarations*/

/* tests from tokenizer.c*/
void test_lexeme_cmp(void);
void token_list_pop_front_test(void);
void token_list_pop_back_test(void);
int token_list_append_list_test(void);
void test_collect(void);
void test_va_opt_0(void);
void test_va_opt_1(void);
void test_va_opt_2(void);
void test_va_opt_3(void);
void test_va_opt_4(void);
void test_va_opt_5(void);
void test_va_opt_6(void);
void test_va_opt_7(void);
void concatenation_problem(void);
void test_va_opt_G2(void);
void test_va_opt(void);
void test_empty_va_args(void);
void test_va_args_single(void);
void test_va_args_extra_args(void);
void test_empty_va_args_empty(void);
void test_defined(void);
void testline(void);
void ifelse(void);
void T1(void);
int EXAMPLE5(void);
void recursivetest1(void);
void rectest(void);
void emptycall(void);
void semiempty(void);
void calling_one_arg_with_empty_arg(void);
void test_argument_with_parentesis(void);
void two_empty_arguments(void);
void simple_object_macro(void);
void test_one_file(void);
void test2(void);
void test3(void);
void tetris(void);
void recursive_macro_expansion(void);
void empty_and_no_args(void);
void test4(void);
void test_string(void);
void test6(void);
void testerror(void);
int test_expression(void);
int test_concatenation_o(void);
int test_concatenation(void);
void bad_test(void);
int test_spaces(void);
int test_stringfy(void);
int test_tokens(void);
int test_predefined_macros(void);
int test_utf8(void);
int test_line_continuation(void);

/* tests from tests.c*/
void parser_specifier_test(void);
void char_constants(void);
void array_item_type_test(void);
void take_address_type_test(void);
void parser_scope_test(void);
void parser_tag_test(void);
void string_concatenation_test(void);
void test_digit_separator(void);
void test_lit(void);
void type_test2(void);
void type_test3(void);
void crazy_decl(void);
void crazy_decl2(void);
void crazy_decl4(void);
void sizeof_not_evaluated(void);
void sizeof_array_test(void);
void sizeof_test(void);
void alignof_test(void);
void indirection_struct_size(void);
void traits_test(void);
void comp_error1(void);
void array_size(void);
void expr_type(void);
void expand_test(void);
void expand_test2(void);
void expand_test3(void);
void bigtest(void);
void literal_string_type(void);
void digit_separator_test(void);
void numbers_test(void);
void binary_digits_test(void);
void type_suffix_test(void);
void type_test(void);
void is_pointer_test(void);
void params_test(void);
void test_compiler_constant_expression(void);
void zerodiv(void);
void function_result_test(void);
void type_normalization(void);
void auto_test(void);
void visit_test_auto_typeof(void);
void enum_scope(void);
void const_member(void);
void register_struct_member(void);
void address_of_const(void);
void lvalue_test(void);
void simple_no_discard_test(void);
void simple_no_discard_test2(void);
void address_of_register(void);
void return_address_of_local(void);
void assignment_of_read_only_object(void);
void simple_move(void);
void simple_move_error(void);
void parameter_view(void);
void move_from_extern(void);
void owner_type_test(void);
void correct_move_assigment(void);
void no_explicit_move_required(void);
void no_explicit_move_with_function_result(void);
void cannot_ignore_owner_result(void);
void can_ignore_owner_result(void);
void move_not_necessary_on_return(void);
void explicit_move_not_required(void);
void error_using_temporary_owner(void);
void passing_view_to_owner(void);
void obj_owner_cannot_be_used_in_non_pointer(void);
void ownership_flow_test_null_ptr_at_end_of_scope(void);
void ownership_flow_test_pointer_must_be_deleted(void);
void ownership_flow_test_basic_pointer_check(void);
void ownership_flow_test_struct_member_missing_free(void);
void ownership_flow_test_struct_member_free(void);
void ownership_flow_test_move_inside_if(void);
void ownership_flow_test_goto_same_scope(void);
void ownership_flow_test_jump_labels(void);
void ownership_flow_test_owner_if_pattern_1(void);
void ownership_flow_test_owner_if_pattern_2(void);
void ownership_flow_test_missing_destructor(void);
void ownership_flow_test_no_warning(void);
void ownership_flow_test_moved_if_not_null(void);
void ownership_flow_test_struct_moved(void);
void ownership_flow_test_scope_error(void);
void ownership_flow_test_void_destroy(void);
void ownership_flow_test_void_destroy_ok(void);
void ownership_flow_test_moving_owner_pointer(void);
void ownership_flow_test_moving_owner_pointer_missing(void);
void ownership_flow_test_error(void);
void ownership_flow_test_setting_owner_pointer_to_null(void);
void ownership_flow_test_while_not_null(void);
void ownership_flow_test_if_state(void);
void ownership_types_test_error_owner(void);
void ownership_flow_test_if_variant(void);
void check_leaks_on_else_block(void);
void ownership_flow_test_two_ifs(void);
void ownership_no_name_parameter(void);
void ownership_flow_switch_case(void);
void state_inner_objects_preserved(void);
void owner_parameter_must_be_ignored(void);
void taking_address(void);
void taking_address_const(void);
void pointer_argument(void);
void do_while(void);
void switch_cases_state(void);
void switch_break(void);
void passing_non_owner(void);
void flow_analysis_else(void);
void moving_content_of_owner(void);
void switch_scope(void);
void swith_and_while(void);
void owner_to_non_owner(void);
void owner_to_non_owner_zero(void);
void incomplete_struct(void);
void switch_pop_problem(void);
void switch_pop2(void);
void scopes_pop(void);
void owner_moved(void);
void partially_owner_moved(void);
void use_after_destroy(void);
void obj_owner_must_be_from_addressof(void);
void discarding_owner(void);
void using_uninitialized(void);
void using_uninitialized_struct(void);
void zero_initialized(void);
void empty_initialized(void);
void calloc_state(void);
void malloc_initialization(void);
void valid_but_unkown_result(void);
void calling_non_const_func(void);
void calling_const_func(void);
void pointer_to_owner(void);
void socket_sample(void);
void return_object(void);
void return_bad_object(void);
void null_to_owner(void);
void return_true_branch(void);
void flow_tests(void);
void member(void);
void loop_leak(void);
void out_parameter(void);
void lvalue_required_1(void);
void lvalue_required_2(void);
void lvalue_required_3(void);
void lvalue_required_4(void);
void null_check_1(void);
void null_check_2(void);
void compound_literal_object(void);
void bounds_check1(void);
void bounds_check2(void);

/*end of forward declarations*/

int test_main(void)
{
g_unit_test_error_count = 0;
g_unit_test_success_count = 0;
    test_lexeme_cmp();
    token_list_pop_front_test();
    token_list_pop_back_test();
    token_list_append_list_test();
    test_collect();
    test_va_opt_0();
    test_va_opt_1();
    test_va_opt_2();
    test_va_opt_3();
    test_va_opt_4();
    test_va_opt_5();
    test_va_opt_6();
    test_va_opt_7();
    concatenation_problem();
    test_va_opt_G2();
    test_va_opt();
    test_empty_va_args();
    test_va_args_single();
    test_va_args_extra_args();
    test_empty_va_args_empty();
    test_defined();
    testline();
    ifelse();
    T1();
    EXAMPLE5();
    recursivetest1();
    rectest();
    emptycall();
    semiempty();
    calling_one_arg_with_empty_arg();
    test_argument_with_parentesis();
    two_empty_arguments();
    simple_object_macro();
    test_one_file();
    test2();
    test3();
    tetris();
    recursive_macro_expansion();
    empty_and_no_args();
    test4();
    test_string();
    test6();
    testerror();
    test_expression();
    test_concatenation_o();
    test_concatenation();
    bad_test();
    test_spaces();
    test_stringfy();
    test_tokens();
    test_predefined_macros();
    test_utf8();
    test_line_continuation();
    parser_specifier_test();
    char_constants();
    array_item_type_test();
    take_address_type_test();
    parser_scope_test();
    parser_tag_test();
    string_concatenation_test();
    test_digit_separator();
    test_lit();
    type_test2();
    type_test3();
    crazy_decl();
    crazy_decl2();
    crazy_decl4();
    sizeof_not_evaluated();
    sizeof_array_test();
    sizeof_test();
    alignof_test();
    indirection_struct_size();
    traits_test();
    comp_error1();
    array_size();
    expr_type();
    expand_test();
    expand_test2();
    expand_test3();
    bigtest();
    literal_string_type();
    digit_separator_test();
    numbers_test();
    binary_digits_test();
    type_suffix_test();
    type_test();
    is_pointer_test();
    params_test();
    test_compiler_constant_expression();
    zerodiv();
    function_result_test();
    type_normalization();
    auto_test();
    visit_test_auto_typeof();
    enum_scope();
    const_member();
    register_struct_member();
    address_of_const();
    lvalue_test();
    simple_no_discard_test();
    simple_no_discard_test2();
    address_of_register();
    return_address_of_local();
    assignment_of_read_only_object();
    simple_move();
    simple_move_error();
    parameter_view();
    move_from_extern();
    owner_type_test();
    correct_move_assigment();
    no_explicit_move_required();
    no_explicit_move_with_function_result();
    cannot_ignore_owner_result();
    can_ignore_owner_result();
    move_not_necessary_on_return();
    explicit_move_not_required();
    error_using_temporary_owner();
    passing_view_to_owner();
    obj_owner_cannot_be_used_in_non_pointer();
    ownership_flow_test_null_ptr_at_end_of_scope();
    ownership_flow_test_pointer_must_be_deleted();
    ownership_flow_test_basic_pointer_check();
    ownership_flow_test_struct_member_missing_free();
    ownership_flow_test_struct_member_free();
    ownership_flow_test_move_inside_if();
    ownership_flow_test_goto_same_scope();
    ownership_flow_test_jump_labels();
    ownership_flow_test_owner_if_pattern_1();
    ownership_flow_test_owner_if_pattern_2();
    ownership_flow_test_missing_destructor();
    ownership_flow_test_no_warning();
    ownership_flow_test_moved_if_not_null();
    ownership_flow_test_struct_moved();
    ownership_flow_test_scope_error();
    ownership_flow_test_void_destroy();
    ownership_flow_test_void_destroy_ok();
    ownership_flow_test_moving_owner_pointer();
    ownership_flow_test_moving_owner_pointer_missing();
    ownership_flow_test_error();
    ownership_flow_test_setting_owner_pointer_to_null();
    ownership_flow_test_while_not_null();
    ownership_flow_test_if_state();
    ownership_types_test_error_owner();
    ownership_flow_test_if_variant();
    check_leaks_on_else_block();
    ownership_flow_test_two_ifs();
    ownership_no_name_parameter();
    ownership_flow_switch_case();
    state_inner_objects_preserved();
    owner_parameter_must_be_ignored();
    taking_address();
    taking_address_const();
    pointer_argument();
    do_while();
    switch_cases_state();
    switch_break();
    passing_non_owner();
    flow_analysis_else();
    moving_content_of_owner();
    switch_scope();
    swith_and_while();
    owner_to_non_owner();
    owner_to_non_owner_zero();
    incomplete_struct();
    switch_pop_problem();
    switch_pop2();
    scopes_pop();
    owner_moved();
    partially_owner_moved();
    use_after_destroy();
    obj_owner_must_be_from_addressof();
    discarding_owner();
    using_uninitialized();
    using_uninitialized_struct();
    zero_initialized();
    empty_initialized();
    calloc_state();
    malloc_initialization();
    valid_but_unkown_result();
    calling_non_const_func();
    calling_const_func();
    pointer_to_owner();
    socket_sample();
    return_object();
    return_bad_object();
    null_to_owner();
    return_true_branch();
    flow_tests();
    member();
    loop_leak();
    out_parameter();
    lvalue_required_1();
    lvalue_required_2();
    lvalue_required_3();
    lvalue_required_4();
    null_check_1();
    null_check_2();
    compound_literal_object();
    bounds_check1();
    bounds_check2();
return g_unit_test_error_count;

}
#undef TESTCODE
#endif /*TEST*/
