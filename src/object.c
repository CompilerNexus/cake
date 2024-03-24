#include "ownership.h"
#include "object.h"
#include "parser.h"
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include "console.h"


void object_state_to_string(enum object_state e)
{
    bool first = true;

    printf("\"");
    if (e & OBJECT_STATE_UNINITIALIZED)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("uninitialized");
    }

    if (e & OBJECT_STATE_NOT_NULL &&
        e & OBJECT_STATE_NULL)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("maybe-null");
    }
    else if (e & OBJECT_STATE_NOT_NULL)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("not-null");
    }
    else if (e & OBJECT_STATE_NULL)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("null");
    }

    if (e & OBJECT_STATE_NOT_ZERO &&
        e & OBJECT_STATE_ZERO)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("any");
    }
    else if (e & OBJECT_STATE_ZERO)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("zero");
    }
    else if (e & OBJECT_STATE_NOT_ZERO)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("not-zero");
    }

    if (e & OBJECT_STATE_MOVED)
    {
        if (first)
            first = false;
        else
            printf(" or ");
        printf("moved");
    }

    printf("\"");

}

struct object* object_get_pointed_object(const struct object* p)
{
    if (p != NULL)
    {
        if (p->pointed2)
            return p->pointed2;
        if (p->pointed_ref)
            return p->pointed_ref;
    }
    return NULL;
}
void object_swap(struct object* a, struct object* b)
{
    struct object temp = *a;
    *a = *b;
    *b = temp;
}

void object_delete(struct object* owner opt p)
{
    if (p)
    {
        object_destroy(p);
        free(p);
    }
}

void object_destroy(struct object* obj_owner p)
{
    object_delete(p->pointed2);
    objects_destroy(&p->members);
    object_state_stack_destroy(&p->object_state_stack);
}


void object_state_stack_destroy(struct object_state_stack* obj_owner p)
{
    free(p->data);
}

int object_state_stack_reserve(struct object_state_stack* p, int n)
{
    if (n > p->capacity)
    {
        if ((size_t)n > (SIZE_MAX / (sizeof(p->data[0]))))
        {
            return EOVERFLOW;
        }

        void* owner pnew = realloc(p->data, n * sizeof(p->data[0]));
        if (pnew == NULL) return ENOMEM;
        static_set(p->data, "moved");
        p->data = pnew;
        p->capacity = n;
    }
    return 0;
}

int object_state_stack_push_back(struct object_state_stack* p, enum object_state e, const char* name, int state_number)
{
    if (p->size == INT_MAX)
    {
        return EOVERFLOW;
    }

    if (p->size + 1 > p->capacity)
    {
        int new_capacity = 0;
        if (p->capacity > (INT_MAX - p->capacity / 2))
        {
            /*overflow*/
            new_capacity = INT_MAX;
        }
        else
        {
            new_capacity = p->capacity + p->capacity / 2;
            if (new_capacity < p->size + 1)
            {
                new_capacity = p->size + 1;
            }
        }

        int error = object_state_stack_reserve(p, new_capacity);
        if (error != 0)
        {
            return error;
        }
    }

    p->data[p->size].state = e;
    p->data[p->size].name = name;
    p->data[p->size].state_number = state_number;
    p->size++;

    return 0;
}

void objects_destroy(struct objects* obj_owner p) /*unchecked*/
{
    for (int i = 0; i < p->size; i++)
    {
        object_destroy(&p->data[i]);
    }
    free(p->data);
}

int objects_reserve(struct objects* p, int n)
{
    if (n > p->capacity)
    {
        if ((size_t)n > (SIZE_MAX / (sizeof(p->data[0]))))
        {
            return EOVERFLOW;
        }

        void* owner pnew = realloc(p->data, n * sizeof(p->data[0]));
        if (pnew == NULL) return ENOMEM;

        static_set(p->data, "moved"); //p->data was moved to pnew

        p->data = pnew;
        p->capacity = n;
    }
    return 0;
}

int objects_push_back(struct objects* p, struct object* obj_owner p_object)
{
    if (p->size == INT_MAX)
    {
        object_destroy(p_object);
        return EOVERFLOW;
    }

    if (p->size + 1 > p->capacity)
    {
        int new_capacity = 0;
        if (p->capacity > (INT_MAX - p->capacity / 2))
        {
            /*overflow*/
            new_capacity = INT_MAX;
        }
        else
        {
            new_capacity = p->capacity + p->capacity / 2;
            if (new_capacity < p->size + 1)
            {
                new_capacity = p->size + 1;
            }
        }

        int error = objects_reserve(p, new_capacity);
        if (error != 0)
        {
            object_destroy(p_object);
            return error;
        }
    }

    p->data[p->size] = *p_object; /*COPIED*/


    p->size++;

    return 0;
}
struct object_name_list
{
    const char* name;
    struct object_name_list* previous;
};

bool has_name(const char* name, struct object_name_list* list)
{
    struct object_name_list* p = list;

    while (p)
    {
        if (strcmp(p->name, name) == 0)
        {
            return true;
        }
        p = p->previous;
    }
    return false;
}

struct object make_object_core(struct type* p_type,
    struct object_name_list* list,
    int deep,
    const struct declarator* p_declarator_opt,
    const struct expression* p_expression_origin)
{
    //assert((p_declarator_opt == NULL) != (p_expression_origin == NULL));
    if (p_declarator_opt == NULL)
    {
        assert(p_expression_origin != NULL);
    }
    if (p_expression_origin == NULL)
    {
        assert(p_declarator_opt != NULL);
    }



    struct object obj = { 0 };
    obj.p_expression_origin = p_expression_origin;
    obj.declarator = p_declarator_opt;

    if (p_type->struct_or_union_specifier)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            obj.state = OBJECT_STATE_NOT_APPLICABLE;

            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            struct object_name_list l = { 0 };
            l.name = p_struct_or_union_specifier->tag_name;
            l.previous = list;
            //int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            char* tag = NULL;
                            if (p_member_declarator->declarator->type.struct_or_union_specifier)
                            {
                                tag = p_member_declarator->declarator->type.struct_or_union_specifier->tag_name;
                            }
                            else if (p_member_declarator->declarator->type.next &&
                                p_member_declarator->declarator->type.next->struct_or_union_specifier)
                            {
                                tag = p_member_declarator->declarator->type.next->struct_or_union_specifier->tag_name;

                            }

                            if (tag && has_name(tag, &l))
                            {
                                struct object member_obj = { 0 };
                                member_obj.p_expression_origin = p_expression_origin;
                                member_obj.declarator = p_declarator_opt;
                                member_obj.state = OBJECT_STATE_NOT_APPLICABLE;
                                objects_push_back(&obj.members, &member_obj);
                            }
                            else
                            {
                                struct object member_obj =
                                    make_object_core(&p_member_declarator->declarator->type,
                                        &l,
                                        deep,
                                        p_declarator_opt,
                                        p_expression_origin);
                                objects_push_back(&obj.members, &member_obj);
                            }

                            //member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                else
                {
                    if (p_member_declaration->specifier_qualifier_list &&
                        p_member_declaration->specifier_qualifier_list->struct_or_union_specifier)
                    {
                        //struct object obj = {0};
                        //obj.state = OBJECT_STATE_STRUCT;
                        //objects_push_back(&obj.members, &obj);


                        struct type t = { 0 };
                        t.category = TYPE_CATEGORY_ITSELF;
                        t.struct_or_union_specifier = p_member_declaration->specifier_qualifier_list->struct_or_union_specifier;
                        t.type_specifier_flags = TYPE_SPECIFIER_STRUCT_OR_UNION;
                        struct object member_obj = make_object_core(&t, &l, deep, p_declarator_opt, p_expression_origin);
                        objects_push_back(&obj.members, &member_obj);
                        type_destroy(&t);
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
        }
    }


    else if (type_is_array(p_type))
    {
        //p_object->state = flags;
        //if (p_object->members_size > 0)
        //{
        //    //not sure if we instanticate all items of array
        //    p_object->members[0].state = flags;
        //}
    }
    else if (type_is_pointer(p_type))
    {
        obj.state = OBJECT_STATE_NOT_APPLICABLE;

        if (deep < 1)
        {
            struct type t2 = type_remove_pointer(p_type);
            if (!type_is_void(&t2))
            {
                struct object* owner p_object = calloc(1, sizeof(struct object));
                *p_object = make_object_core(&t2, list, deep + 1, p_declarator_opt, p_expression_origin);
                obj.pointed2 = p_object;
            }

            type_destroy(&t2);
            //(*p_deep)++;
        }
    }
    else
    {
        //assert(p_object->members_size == 0);
        //p_object->state = flags;
        obj.state = OBJECT_STATE_NOT_APPLICABLE;
    }

    return obj;
}

struct object make_object(struct type* p_type,
    const struct declarator* p_declarator_opt,
    const struct expression* p_expression_origin)
{
    //assert(p_token_position);
    struct object_name_list list = { .name = "" };
    return make_object_core(p_type, &list, 0, p_declarator_opt, p_expression_origin);
}

void object_push_empty(struct object* object, const char* name, int state_number)
{
    object_state_stack_push_back(&object->object_state_stack, 0, name, state_number);

    if (object_get_pointed_object(object))
    {
        object_push_empty(object_get_pointed_object(object), name, state_number);
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_push_empty(&object->members.data[i], name, state_number);
    }
}
void object_push_copy_current_state(struct object* object, const char* name, int state_number)
{

    object_state_stack_push_back(&object->object_state_stack, object->state, name, state_number);

    if (object_get_pointed_object(object))
    {
        object_push_copy_current_state(object_get_pointed_object(object), name, state_number);
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_push_copy_current_state(&object->members.data[i], name, state_number);
    }

}

void object_pop_states(struct object* object, int n)
{

    if (object->object_state_stack.size < n)
    {
        //assert(false);
        return;
    }

    object->object_state_stack.size =
        object->object_state_stack.size - n;

    if (object_get_pointed_object(object))
    {
        object_pop_states(object_get_pointed_object(object), n);
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_pop_states(&object->members.data[i], n);
    }

}

void object_restore_state(struct object* object, int state_to_restore)
{
    assert(state_to_restore > 0);

    //0 zero is top of stack
    //1 is the before top
    int index = object->object_state_stack.size - state_to_restore;
    if (index >= 0 && index < object->object_state_stack.size)
    {
    }
    else
    {
        //assert(false);
        return;
    }

    enum object_state sstate = object->object_state_stack.data[index].state;
    object->state = sstate;

    if (object_get_pointed_object(object))
    {
        object_restore_state(object_get_pointed_object(object), state_to_restore);
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_restore_state(&object->members.data[i], state_to_restore);
    }
}

void print_object_core(int ident, struct type* p_type, struct object* p_object, const char* previous_names, bool is_pointer, bool short_version)
{
    if (p_object == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            if (p_object == NULL)
            {
                printf("%*c", ident, ' ');
                printf("%s %s\n", previous_names, "-");
                return;
            }
            //obj.state = OBJECT_STATE_STRUCT;

            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {


                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;
                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            const char* name = p_member_declarator->declarator->name ? p_member_declarator->declarator->name->lexeme : "";

                            char buffer[200] = { 0 };
                            if (is_pointer)
                                snprintf(buffer, sizeof buffer, "%s->%s", previous_names, name);
                            else
                                snprintf(buffer, sizeof buffer, "%s.%s", previous_names, name);


                            print_object_core(ident + 1, &p_member_declarator->declarator->type,
                                &p_object->members.data[member_index], buffer,
                                type_is_pointer(&p_member_declarator->declarator->type), short_version);

                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                else
                {
                    //char buffer[200] = {0};
                    //if (is_pointer)
                    //  snprintf(buffer, sizeof buffer, "%s", previous_names, "");
                    //else
                    //  snprintf(buffer, sizeof buffer, "%s", previous_names, "");

                    struct type t = { 0 };
                    t.category = TYPE_CATEGORY_ITSELF;
                    t.struct_or_union_specifier = p_member_declaration->specifier_qualifier_list->struct_or_union_specifier;
                    t.type_specifier_flags = TYPE_SPECIFIER_STRUCT_OR_UNION;

                    print_object_core(ident + 1, &t, &p_object->members.data[member_index], previous_names, false, short_version);

                    member_index++;
                    type_destroy(&t);
                }
                p_member_declaration = p_member_declaration->next;
            }

        }
    }
    //else if (type_is_array(p_type))
    //{
        //p_object->state = flags;
        //if (p_object->members_size > 0)
        //{
        //    //not sure if we instanticate all items of array
        //    p_object->members[0].state = flags;
        //}
    //}
    else if (type_is_pointer(p_type))
    {
        struct type t2 = type_remove_pointer(p_type);
        printf("%*c", ident, ' ');
        if (p_object)
        {
            if (short_version)
            {
                printf("%s == ", previous_names);
                object_state_to_string(p_object->state);
            }
            else
            {
                printf("%p:%s == ", p_object, previous_names);
                printf("{");
                for (int i = 0; i < p_object->object_state_stack.size; i++)
                {
                    printf(LIGHTCYAN);
                    printf("(#%d %s)", p_object->object_state_stack.data[i].state_number, p_object->object_state_stack.data[i].name);
                    object_state_to_string(p_object->object_state_stack.data[i].state);
                    printf(RESET);
                    printf(",");
                }
                //printf("*");
                printf(LIGHTMAGENTA);
                printf("(current)");
                object_state_to_string(p_object->state);
                printf(RESET);
                printf("}");
            }
            printf("\n");


            if (object_get_pointed_object(p_object))
            {
                char buffer[200] = { 0 };
                if (type_is_struct_or_union(&t2))
                {
                    snprintf(buffer, sizeof buffer, "%s", previous_names);
                }
                else
                {
                    snprintf(buffer, sizeof buffer, "*%s", previous_names);
                }



                print_object_core(ident + 1, &t2, object_get_pointed_object(p_object), buffer, is_pointer, short_version);
            }
            else
            {
                //printf("%s %s\n");
            }
        }
        type_destroy(&t2);
    }
    else
    {
        printf("%*c", ident, ' ');
        if (p_object)
        {
            if (short_version)
            {
                printf("%s == ", previous_names);
                object_state_to_string(p_object->state);
            }
            else
            {
                printf("%p:%s == ", p_object, previous_names);
                printf("{");
                for (int i = 0; i < p_object->object_state_stack.size; i++)
                {
                    printf("(#%d %s)", p_object->object_state_stack.data[i].state_number, p_object->object_state_stack.data[i].name);
                    object_state_to_string(p_object->object_state_stack.data[i].state);
                    printf(",");
                }
                object_state_to_string(p_object->state);
                printf("}");
            }


            printf("\n");
        }
    }


}

enum object_state state_merge(enum object_state before, enum object_state after)
{
    enum object_state e = before | after;


    return e;
}

int object_set_state_from_current(struct object* object, int state_number)
{
    for (int i = object->object_state_stack.size - 1; i >= 0; i--)
    {
        if (object->object_state_stack.data[i].state_number == state_number)
        {
            object->object_state_stack.data[i].state = object->state;
            break;
        }
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_set_state_from_current(&object->members.data[i], state_number);
    }

    struct object* pointed = object_get_pointed_object(object);
    if (pointed)
    {
        object_set_state_from_current(pointed, state_number);
    }
    return 1;
}

int object_restore_current_state_from(struct object* object, int state_number)
{
    for (int i = object->object_state_stack.size - 1; i >= 0; i--)
    {
        if (object->object_state_stack.data[i].state_number == state_number)
        {
            object->state = object->object_state_stack.data[i].state;
            break;
        }
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_restore_current_state_from(&object->members.data[i], state_number);
    }

    struct object* pointed = object_get_pointed_object(object);
    if (pointed)
    {
        object_restore_current_state_from(pointed, state_number);
    }
    return 1;
}

int object_merge_current_state_with_state_number(struct object* object, int state_number)
{
    for (int i = object->object_state_stack.size - 1; i >= 0; i--)
    {
        if (object->object_state_stack.data[i].state_number == state_number)
        {
            object->object_state_stack.data[i].state |= object->state;
            break;
        }
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_merge_current_state_with_state_number(&object->members.data[i], state_number);
    }

    struct object* pointed = object_get_pointed_object(object);
    if (pointed)
    {
        object_merge_current_state_with_state_number(pointed, state_number);
    }
    return 1;
}

int object_merge_current_state_with_state_number_or(struct object* object, int state_number)
{
    for (int i = object->object_state_stack.size - 1; i >= 0; i--)
    {
        if (object->object_state_stack.data[i].state_number == state_number)
        {
            object->object_state_stack.data[i].state |= object->state;
            break;
        }
    }

    for (int i = 0; i < object->members.size; i++)
    {
        object_merge_current_state_with_state_number_or(&object->members.data[i], state_number);
    }

    struct object* pointed = object_get_pointed_object(object);
    if (pointed)
    {
        object_merge_current_state_with_state_number_or(pointed, state_number);
    }
    return 1;
}

void object_get_name(const struct type* p_type,
    const struct object* p_object,
    char* outname,
    int out_size);


void print_object(struct type* p_type, struct object* p_object, bool short_version)
{
    if (p_object == NULL)
    {
        printf("null object");
        return;
    }
    char name[100] = { 0 };
    object_get_name(p_type, p_object, name, sizeof name);



    print_object_core(0, p_type, p_object, name, type_is_pointer(p_type), short_version);
}

void set_object(
    struct type* p_type,
    struct object* p_object,
    enum object_state flags);

void set_object_state(
    struct parser_ctx* ctx,
    struct type* p_type,
    struct object* p_object,
    const struct type* p_source_type,
    const struct object* p_object_source,
    const struct token* error_position)
{
    if (p_object_source == NULL)
    {
        return;
    }
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }


    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {

                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size &&
                                member_index < p_object_source->members.size)
                            {
                                set_object_state(ctx,
                                    &p_member_declarator->declarator->type,
                                    &p_object->members.data[member_index],
                                    &p_member_declarator->declarator->type,//&p_object_source->members.data[member_index].p_declarator_opt->type,
                                    &p_object_source->members.data[member_index],
                                    error_position);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
        }
        else
        {
            assert(p_object->members.size == 0);
            p_object->state = p_object_source->state;
        }
    }
    else if (type_is_array(p_type))
    {
        p_object->state = p_object_source->state;
        if (p_object->members.size > 0)
        {
            //not sure if we instantiate all items of array
            p_object->members.data[0].state = p_object_source->members.data[0].state;
        }
    }
    else if (type_is_pointer(p_type))
    {
        if (p_object_source)
        {
            if (p_object_source->state == OBJECT_STATE_UNINITIALIZED)
            {
                char buffer[100] = { 0 };
                object_get_name(p_source_type, p_object_source, buffer, sizeof buffer);
                compiler_diagnostic_message(W_OWNERSHIP_FLOW_UNINITIALIZED,
                    ctx,
                    error_position,
                    "source object '%s' is uninitialized", buffer);
            }
            else if (p_object_source->state & OBJECT_STATE_UNINITIALIZED)
            {
                char buffer[100] = { 0 };
                object_get_name(p_source_type, p_object_source, buffer, sizeof buffer);

                compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                    ctx,
                    error_position,
                    "source object '%s' may be uninitialized", buffer);
            }

            if (type_is_any_owner(p_type) &&
                type_is_any_owner(p_source_type))
            {
                if (p_object_source->state == OBJECT_STATE_MOVED)
                {
                    char buffer[100] = { 0 };
                    object_get_name(p_source_type, p_object_source, buffer, sizeof buffer);

                    compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                        ctx,
                        error_position,
                        "source object '%s' have been moved", buffer);
                }
                else if (p_object_source->state & OBJECT_STATE_MOVED)
                {
                    char buffer[100] = { 0 };
                    object_get_name(p_source_type, p_object_source, buffer, sizeof buffer);

                    compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                        ctx,
                        error_position,
                        "source object '%s' may have been moved", buffer);
                }
            }

        }


        if (type_is_any_owner(p_type))
        {
            p_object->state = p_object_source->state;
        }
        else
        {
            //MOVED state is not applicable to non owner objects
            p_object->state = p_object_source->state & ~OBJECT_STATE_MOVED;
        }


        if (object_get_pointed_object(p_object))
        {
            struct type t2 = type_remove_pointer(p_type);
            if (object_get_pointed_object(p_object_source))
            {
                set_object_state(ctx, &t2, object_get_pointed_object(p_object),
                    p_source_type,
                    object_get_pointed_object(p_object_source), error_position);
            }
            else
            {
                set_object(&t2, object_get_pointed_object(p_object),
                    OBJECT_STATE_NULL | OBJECT_STATE_NOT_NULL);
            }
            type_destroy(&t2);
        }
    }
    else
    {


        //assert(p_object->members.size == 0); //enum?
        p_object->state = p_object_source->state;
    }
}


void set_direct_state(
    struct type* p_type,
    struct object* p_object,
    enum object_state flags)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {

                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                set_direct_state(&p_member_declarator->declarator->type, &p_object->members.data[member_index], flags);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
        }
        else
        {
            assert(p_object->members.size == 0);
            p_object->state = flags;
        }
    }

    if (type_is_pointer(p_type))
    {
        if (flags == OBJECT_STATE_ZERO)
        {
            /*zero for pointers is null*/
            p_object->state = OBJECT_STATE_NULL;
        }
        else
        {
            p_object->state = flags;
        }
    }
    else
    {
        p_object->state = flags;
    }
}

void set_object(
    struct type* p_type,
    struct object* p_object,
    enum object_state flags)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }


    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {

                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                set_object(&p_member_declarator->declarator->type, &p_object->members.data[member_index], flags);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
        }
        else
        {
            assert(p_object->members.size == 0);
            p_object->state = flags;
        }
    }
    else if (type_is_array(p_type))
    {
        p_object->state = flags;
        if (p_object->members.size > 0)
        {
            //not sure if we instantiate all items of array
            p_object->members.data[0].state = flags;
        }
    }
    else if (type_is_pointer(p_type))
    {
        p_object->state = flags;

        if (object_get_pointed_object(p_object))
        {
            struct type t2 = type_remove_pointer(p_type);
            if (type_is_out(&t2))
            {
                flags = OBJECT_STATE_UNINITIALIZED;
            }
            set_object(&t2, object_get_pointed_object(p_object), flags);
            type_destroy(&t2);
        }
    }
    else
    {
        //assert(p_object->members.size == 0); //enum?
        p_object->state = flags;
    }
}

void object_set_nothing(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                object_set_nothing(&p_member_declarator->declarator->type, &p_object->members.data[member_index]);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
            return;
        }
    }

    if (type_is_pointer(p_type))
    {
        p_object->state = 0;

        if (object_get_pointed_object(p_object))
        {
            struct type t2 = type_remove_pointer(p_type);
            object_set_uninitialized(&t2, object_get_pointed_object(p_object));
            type_destroy(&t2);
        }
    }
    else
    {
        p_object->state = 0;
    }
}
void object_set_uninitialized(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                object_set_uninitialized(&p_member_declarator->declarator->type, &p_object->members.data[member_index]);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
            return;
        }
    }

    if (type_is_pointer(p_type))
    {
        p_object->state = OBJECT_STATE_UNINITIALIZED;

        if (object_get_pointed_object(p_object))
        {
            struct type t2 = type_remove_pointer(p_type);
            object_set_nothing(&t2, object_get_pointed_object(p_object));
            type_destroy(&t2);
        }
    }
    else
    {
        p_object->state = OBJECT_STATE_UNINITIALIZED;
    }
}


void checked_empty(struct parser_ctx* ctx,
    struct type* p_type,
    struct object* p_object,
    const struct token* position_token)
{
    if (p_object == NULL)
    {
        return;
    }
    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier ?
            p_struct_or_union_specifier->member_declaration_list.head :
            NULL;

        /*
        *  Some parts of the object needs to be moved..
        *  we need to print error one by one
        */
        int member_index = 0;
        while (p_member_declaration)
        {
            if (p_member_declaration->member_declarator_list_opt)
            {
                struct member_declarator* p_member_declarator =
                    p_member_declaration->member_declarator_list_opt->head;
                while (p_member_declarator)
                {

                    if (p_member_declarator->declarator)
                    {
                        checked_empty(ctx, &p_member_declarator->declarator->type,
                            &p_object->members.data[member_index],
                            position_token);

                        member_index++;
                    }
                    p_member_declarator = p_member_declarator->next;
                }
            }
            p_member_declaration = p_member_declaration->next;
        }
    }

    if (type_is_any_owner(p_type))
    {
        if ((p_object->state & OBJECT_STATE_MOVED) ||
            (p_object->state & OBJECT_STATE_UNINITIALIZED) ||
            (p_object->state & OBJECT_STATE_NULL))
        {
        }
        else if (p_object->state & OBJECT_STATE_NOT_NULL)
        {
            struct type t = type_remove_pointer(p_type);
            struct object* pointed = object_get_pointed_object(p_object);
            if (pointed)
                checked_empty(ctx, &t, pointed, position_token);
            type_destroy(&t);
        }
        else
        {
            char name[200] = { 0 };
            object_get_name(p_type, p_object, name, sizeof name);
            compiler_diagnostic_message(W_OWNERSHIP_FLOW_MOVED,
                ctx,
                position_token,
                "object '%s' it not empty",
                name);
        }
    }
}

void object_set_moved(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                object_set_moved(&p_member_declarator->declarator->type, &p_object->members.data[member_index]);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
            return;
        }
    }

    if (type_is_pointer(p_type))
    {
        p_object->state = OBJECT_STATE_MOVED;

        if (object_get_pointed_object(p_object))
        {
            struct type t2 = type_remove_pointer(p_type);
            object_set_nothing(&t2, object_get_pointed_object(p_object));
            type_destroy(&t2);
        }
    }
    else
    {
        p_object->state = OBJECT_STATE_MOVED;
    }
}

void object_set_unknown(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                object_set_unknown(&p_member_declarator->declarator->type, &p_object->members.data[member_index]);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
            return;
        }
    }

    if (type_is_pointer(p_type))
    {
        p_object->state = OBJECT_STATE_NULL | OBJECT_STATE_NOT_NULL;

        struct object* pointed = object_get_pointed_object(p_object);
        if (pointed)
        {
            struct type t2 = type_remove_pointer(p_type);
            object_set_unknown(&t2, pointed);
            type_destroy(&t2);
        }
    }
    else
    {
        p_object->state = OBJECT_STATE_ZERO | OBJECT_STATE_NOT_ZERO;
    }
}


void object_set_zero(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL || p_type == NULL)
    {
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        if (p_struct_or_union_specifier)
        {
            struct member_declaration* p_member_declaration =
                p_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_member_declaration)
            {
                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;

                    while (p_member_declarator)
                    {
                        if (p_member_declarator->declarator)
                        {
                            if (member_index < p_object->members.size)
                            {
                                object_set_zero(&p_member_declarator->declarator->type, &p_object->members.data[member_index]);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
            return;
        }
    }

    if (type_is_pointer(p_type))
    {
        p_object->state = OBJECT_STATE_NULL;

        if (object_get_pointed_object(p_object))
        {
            /*
              if the pointer is null, there is no pointed object
            */
            struct type t2 = type_remove_pointer(p_type);
            object_set_nothing(&t2, object_get_pointed_object(p_object));
            type_destroy(&t2);
        }
    }
    else
    {
        p_object->state = OBJECT_STATE_ZERO;
    }
}

//returns true if all parts that need to be moved weren't moved.
bool object_check(struct type* p_type, struct object* p_object)
{
    if (p_object == NULL)
    {
        return false;
    }
    if (p_type->type_qualifier_flags & TYPE_QUALIFIER_VIEW)
    {
        return false;
    }

    if (!type_is_any_owner(p_type))
    {
        return false;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier->member_declaration_list.head;
        int possible_need_destroy_count = 0;
        int need_destroy_count = 0;
        int member_index = 0;
        while (p_member_declaration)
        {
            if (p_member_declaration->member_declarator_list_opt)
            {
                struct member_declarator* p_member_declarator =
                    p_member_declaration->member_declarator_list_opt->head;
                while (p_member_declarator)
                {

                    if (p_member_declarator->declarator)
                    {
                        if (type_is_owner(&p_member_declarator->declarator->type))
                        {
                            possible_need_destroy_count++;
                        }

                        if (object_check(&p_member_declarator->declarator->type,
                            &p_object->members.data[member_index]))
                        {
                            need_destroy_count++;
                        }
                        member_index++;
                    }
                    p_member_declarator = p_member_declarator->next;
                }
            }
            p_member_declaration = p_member_declaration->next;
        }

        return need_destroy_count > 1 && (need_destroy_count == possible_need_destroy_count);
    }
    else
    {
        bool should_had_been_moved = false;
        if (type_is_pointer(p_type))
        {
            should_had_been_moved = (p_object->state & OBJECT_STATE_NOT_NULL);
        }
        else
        {
            if (p_object->state == OBJECT_STATE_UNINITIALIZED ||
                p_object->state == OBJECT_STATE_MOVED ||
                p_object->state == OBJECT_STATE_NOT_NULL ||
                p_object->state == (OBJECT_STATE_UNINITIALIZED | OBJECT_STATE_MOVED))
            {
            }
            else
            {
                should_had_been_moved = true;
            }
        }

        return should_had_been_moved;
    }

    return false;
}

void object_get_name_core(
    const struct type* p_type,
    const struct object* p_object,
    const struct object* p_object_target,
    const char* previous_names,
    char* outname,
    int out_size)
{
    if (p_object == NULL)
    {
        return;
    }

    if (p_object == p_object_target)
    {
        snprintf(outname, out_size, "%s", previous_names);
        return;
    }

    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier->member_declaration_list.head;

        int member_index = 0;
        while (p_member_declaration)
        {
            if (p_member_declaration->member_declarator_list_opt)
            {
                struct member_declarator* p_member_declarator =
                    p_member_declaration->member_declarator_list_opt->head;
                while (p_member_declarator)
                {

                    if (p_member_declarator->declarator)
                    {
                        const char* name = p_member_declarator->declarator->name ? p_member_declarator->declarator->name->lexeme : "";
                        char buffer[200] = { 0 };
                        if (type_is_pointer(p_type))
                            snprintf(buffer, sizeof buffer, "%s->%s", previous_names, name);
                        else
                            snprintf(buffer, sizeof buffer, "%s.%s", previous_names, name);

                        object_get_name_core(
                            &p_member_declarator->declarator->type,
                            &p_object->members.data[member_index],
                            p_object_target,
                            buffer,
                            outname,
                            out_size);

                        member_index++;
                    }
                    p_member_declarator = p_member_declarator->next;
                }
            }
            p_member_declaration = p_member_declaration->next;
        }

    }
    else
    {
        if (type_is_pointer(p_type))
        {
            char buffer[100] = { 0 };
            snprintf(buffer, sizeof buffer, "%s", previous_names);

            struct type t2 = type_remove_pointer(p_type);
            if (type_is_owner(&t2))
            {
                object_get_name_core(
                    &t2,
                    object_get_pointed_object(p_object),
                    p_object_target,
                    buffer,
                    outname,
                    out_size);
            }
            type_destroy(&t2);
        }
    }
}


void object_get_name(const struct type* p_type,
    const struct object* p_object,
    char* outname,
    int out_size)
{
    if (p_object->declarator)
    {
        const char* root_name = p_object->declarator->name ? p_object->declarator->name->lexeme : "?";
        const struct object* root = &p_object->declarator->object;

        object_get_name_core(&p_object->declarator->type, root, p_object, root_name, outname, out_size);
    }
    else if (p_object->p_expression_origin)
    {
        const char* root_name = "expression";
        const struct object* root = p_object;//->declarator->object;

        object_get_name_core(p_type, root, p_object, root_name, outname, out_size);
    }
    else
    {
        outname[0] = '?';
        outname[1] = '\0';
    }
}

void checked_moved(struct parser_ctx* ctx,
    struct type* p_type,
    struct object* p_object,
    const struct token* position_token)
{
    if (p_object == NULL)
    {
        return;
    }
    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier->member_declaration_list.head;

        /*
        *  Some parts of the object needs to be moved..
        *  we need to print error one by one
        */
        int member_index = 0;
        while (p_member_declaration)
        {
            if (p_member_declaration->member_declarator_list_opt)
            {
                struct member_declarator* p_member_declarator =
                    p_member_declaration->member_declarator_list_opt->head;
                while (p_member_declarator)
                {

                    if (p_member_declarator->declarator)
                    {
                        checked_moved(ctx, &p_member_declarator->declarator->type,
                            &p_object->members.data[member_index],
                            position_token);

                        member_index++;
                    }
                    p_member_declarator = p_member_declarator->next;
                }
            }
            p_member_declaration = p_member_declaration->next;
        }
    }
    else
    {
        if (type_is_pointer(p_type) && !type_is_any_owner(p_type))
        {
            struct type t2 = type_remove_pointer(p_type);
            checked_moved(ctx,
                &t2,
                object_get_pointed_object(p_object),
                position_token);
            type_destroy(&t2);
        }

        if (p_object->state & OBJECT_STATE_MOVED)
        {
            struct token* name_pos = p_object->declarator->name ? p_object->declarator->name : p_object->declarator->first_token;
            const char* parameter_name = p_object->declarator->name ? p_object->declarator->name->lexeme : "?";


            char name[200] = { 0 };
            object_get_name(p_type, p_object, name, sizeof name);
            if (compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                ctx,
                position_token,
                "parameter '%s' is leaving scoped with a moved object '%s'",
                parameter_name,
                name))
            {
                compiler_diagnostic_message(W_LOCATION, ctx, name_pos, "parameter", name);
            }
        }

        if (p_object->state & OBJECT_STATE_UNINITIALIZED)
        {
            struct token* name_pos = p_object->declarator->name ? p_object->declarator->name : p_object->declarator->first_token;
            const char* parameter_name = p_object->declarator->name ? p_object->declarator->name->lexeme : "?";

            char name[200] = { 0 };
            object_get_name(p_type, p_object, name, sizeof name);
            if (compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                ctx,
                position_token,
                "parameter '%s' is leaving scoped with a uninitialized object '%s'",
                parameter_name,
                name))
            {
                compiler_diagnostic_message(W_LOCATION, ctx, name_pos, "parameter", name);
            }
        }
    }
}


void checked_read_object(struct parser_ctx* ctx,
    struct type* p_type,
    struct object* p_object,
    const struct token* position_token,
    bool check_pointed_object)
{
    if (p_object == NULL)
    {
        return;
    }
    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier ?
            p_struct_or_union_specifier->member_declaration_list.head :
            NULL;

        /*
        *  Some parts of the object needs to be moved..
        *  we need to print error one by one
        */
        int member_index = 0;
        while (p_member_declaration)
        {
            if (p_member_declaration->member_declarator_list_opt)
            {
                struct member_declarator* p_member_declarator =
                    p_member_declaration->member_declarator_list_opt->head;
                while (p_member_declarator)
                {

                    if (p_member_declarator->declarator)
                    {
                        checked_read_object(ctx, &p_member_declarator->declarator->type,
                            &p_object->members.data[member_index],
                            position_token,
                            check_pointed_object);

                        member_index++;
                    }
                    p_member_declarator = p_member_declarator->next;
                }
            }
            p_member_declaration = p_member_declaration->next;
        }
    }
    else
    {
        if (type_is_pointer(p_type) &&
            check_pointed_object &&
            p_object->state & OBJECT_STATE_NOT_NULL /*we don't need to check pointed object*/
            )
        {
            struct type t2 = type_remove_pointer(p_type);
            checked_read_object(ctx,
                &t2,
                object_get_pointed_object(p_object),
                position_token,
                true);
            type_destroy(&t2);
        }

        if (p_object->state & OBJECT_STATE_MOVED)
        {
            //struct token* name_pos = p_object->declarator->name ? p_object->declarator->name : p_object->declarator->first_token;
            //const char* parameter_name = p_object->declarator->name ? p_object->declarator->name->lexeme : "?";

            char name[200] = { 0 };
            object_get_name(p_type, p_object, name, sizeof name);
            compiler_diagnostic_message(W_OWNERSHIP_FLOW_MOVED,
                ctx,
                position_token,
                "object '%s' was moved",
                name);
        }

        if (p_object->state & OBJECT_STATE_UNINITIALIZED)
        {
            char name[200] = { 0 };
            object_get_name(p_type, p_object, name, sizeof name);
            compiler_diagnostic_message(W_OWNERSHIP_FLOW_UNINITIALIZED,
                ctx,
                position_token,
                "uninitialized object '%s'",
                name);
        }
    }
}


void visit_object(struct parser_ctx* ctx,
    struct type* p_type,
    struct object* p_object,
    const struct token* position_token,
    const char* previous_names,
    bool is_assigment)
{
    if (p_object == NULL)
    {
        return;
    }
    if (p_type->type_qualifier_flags & TYPE_QUALIFIER_VIEW)
    {
        return;
    }

    if (!type_is_any_owner(p_type))
    {
        if (p_type->storage_class_specifier_flags & STORAGE_SPECIFIER_PARAMETER)
        {
            //for view parameters we need to check if they left something moved..
            checked_moved(ctx,
                p_type,
                p_object,
                position_token);

        }
        return;
    }


    if (p_type->struct_or_union_specifier && p_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_type->struct_or_union_specifier);

        struct member_declaration* p_member_declaration =
            p_struct_or_union_specifier->member_declaration_list.head;

        if (object_check(p_type, p_object))
        {
            /*
            *  All parts of the object needs to be moved, so instead of
            *  describing each part we will just say that the object should
            *  have been moved.
            */
            const struct token* const name = p_object->declarator->name ? p_object->declarator->name : p_object->declarator->first_token;
            if (compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                ctx,
                name,
                "object '%s' was not moved/destroyed",
                previous_names))
            {

                if (p_object->declarator)
                    compiler_diagnostic_message(W_LOCATION, ctx, position_token, "end of '%s' scope", previous_names);
            }
        }
        else
        {
            /*
            *  Some parts of the object needs to be moved..
            *  we need to print error one by one
            */

            int member_index = 0;
            while (p_member_declaration)
            {

                if (p_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_member_declarator =
                        p_member_declaration->member_declarator_list_opt->head;
                    while (p_member_declarator)
                    {

                        if (p_member_declarator->declarator)
                        {
                            const char* name = p_member_declarator->declarator->name ? p_member_declarator->declarator->name->lexeme : "?";

                            char buffer[200] = { 0 };
                            if (type_is_pointer(p_type))
                                snprintf(buffer, sizeof buffer, "%s->%s", previous_names, name);
                            else
                                snprintf(buffer, sizeof buffer, "%s.%s", previous_names, name);

                            visit_object(ctx, &p_member_declarator->declarator->type,
                                &p_object->members.data[member_index],
                                position_token,
                                buffer,
                                is_assigment);

                            member_index++;
                        }
                        p_member_declarator = p_member_declarator->next;
                    }
                }
                p_member_declaration = p_member_declaration->next;
            }
        }


    }
    else
    {
        const char* name = previous_names;
        const struct token* position = NULL;
        if (p_object->declarator)
            position = p_object->declarator->name ? p_object->declarator->name : p_object->declarator->first_token;
        else if (p_object->p_expression_origin)
            position = p_object->p_expression_origin->first_token;
        else
        {
            assert(false);
        }

        if (name[0] == '\0')
        {
            /*function arguments without name*/
            name = "?";
        }
        bool should_had_been_moved = false;


        /*
           Despite the name OBJECT_STATE_NOT_NULL does not means null, it means
           the reference is not referring an object, the value could be -1 for instance.
        */
        if (type_is_pointer(p_type))
        {
            should_had_been_moved = (p_object->state & OBJECT_STATE_NOT_NULL);
        }
        else
        {
            if (p_object->state == OBJECT_STATE_UNINITIALIZED ||
                p_object->state == OBJECT_STATE_MOVED ||
                p_object->state == OBJECT_STATE_NULL)
            {
            }
            else
            {
                should_had_been_moved = true;
            }
        }


        if (type_is_pointer(p_type))
        {
            if (should_had_been_moved)
            {
                char buffer[100] = { 0 };
                snprintf(buffer, sizeof buffer, "%s", previous_names);

                struct type t2 = type_remove_pointer(p_type);
                if (type_is_owner(&t2))
                {
                    visit_object(ctx,
                        &t2,
                        object_get_pointed_object(p_object),
                        position_token,
                        buffer,
                        is_assigment);
                }
                type_destroy(&t2);
            }

        }


        if (should_had_been_moved)
        {
            if (type_is_obj_owner(p_type))
            {

            }
            else
            {
                if (type_is_pointer(p_type))
                {
                    struct type t2 = type_remove_pointer(p_type);
                    bool pointed_is_out = type_is_out(&t2);
                    type_destroy(&t2);

                    if (!pointed_is_out)
                    {
                        if (is_assigment)
                        {
                            compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                                ctx,
                                position_token,
                                "memory pointed by '%s' was not released before assignment.",
                                name);
                        }
                        else
                        {
                            compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                                ctx,
                                position,
                                "memory pointed by '%s' was not released.",
                                name);
                            if (p_object->declarator)
                            {
                                compiler_diagnostic_message(W_LOCATION, ctx, position_token, "end of '%s' scope", name);
                            }
                        }
                    }
                }
                else
                {
                    if (is_assigment)
                    {
                        compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                            ctx,
                            position_token,
                            "previous members of '%s' were not moved before this assignment.",
                            name);
                    }
                    else
                    {
                        compiler_diagnostic_message(W_OWNERSHIP_FLOW_MISSING_DTOR,
                            ctx,
                            position,
                            "object '%s' was not moved.",
                            name);
                        if (p_object->declarator)
                        {
                            compiler_diagnostic_message(W_LOCATION, ctx, position_token, "end of '%s' scope", name);
                        }
                    }
                }
            }
        }
    }

}


void object_assignment(struct parser_ctx* ctx,
    struct object* p_source_obj_opt,
    struct type* p_source_obj_type,

    struct object* p_dest_obj_opt,
    struct type* p_dest_obj_type,

    const struct token* error_position,
    bool bool_source_zero_value, //TODO can be removed
    enum object_state source_state_after,
    enum assigment_type assigment_type)
{


    if (assigment_type == ASSIGMENT_TYPE_OBJECTS)
    {
        assert(p_dest_obj_type);
        if (type_is_owner(p_dest_obj_type))
        {
            if (p_dest_obj_opt)
            {
                char buffer[100] = { 0 };
                object_get_name(p_dest_obj_type, p_dest_obj_opt, buffer, sizeof buffer);
                visit_object(ctx,
                    p_dest_obj_type,
                    p_dest_obj_opt,
                    error_position,
                    buffer,
                    true);
            }
            else
            {
                /*TODO should not happen but it is happening*/
            }
        }
    }



    if (p_dest_obj_opt)
    {
        if (bool_source_zero_value)
        {
            object_set_zero(p_dest_obj_type, p_dest_obj_opt);
        }
        else
        {
            if (p_source_obj_opt)
            {
                set_object_state(ctx, p_dest_obj_type, p_dest_obj_opt, p_source_obj_type, p_source_obj_opt, error_position);
            }
            else
            {
                object_set_unknown(p_dest_obj_type, p_dest_obj_opt);
            }
        }

    }


    if (type_is_any_owner(p_source_obj_type) &&
        !type_is_owner(p_dest_obj_type) &&
        p_source_obj_type->storage_class_specifier_flags & STORAGE_SPECIFIER_FUNCTION_RETURN)
    {
        /*
        int main()
        {
           struct X * p = (struct X * owner) malloc(1);
        }
        */

        compiler_diagnostic_message(W_OWNERSHIP_MISSING_OWNER_QUALIFIER,
            ctx,
            error_position,
            "Object must be owner qualified.");
    }

    if (type_is_any_owner(p_dest_obj_type) &&
        type_is_any_owner(p_source_obj_type) &&
        type_is_pointer(p_source_obj_type))
    {
        if (type_is_void_ptr(p_dest_obj_type))
        {
            if (p_source_obj_opt)
            {
                struct type t2 = type_remove_pointer(p_source_obj_type);
                const char* name = p_source_obj_opt->declarator ?
                    p_source_obj_opt->declarator->name->lexeme :
                    "?";

                visit_object(ctx,
                    &t2,
                    object_get_pointed_object(p_source_obj_opt),
                    error_position,
                    name,
                    true);
                p_source_obj_opt->state = source_state_after;
                type_destroy(&t2);
            }
        }
        else if (type_is_obj_owner(p_dest_obj_type))
        {
            if (type_is_owner(p_source_obj_type))
            {
                if (object_get_pointed_object(p_source_obj_opt))
                {
                    struct type t = type_remove_pointer(p_source_obj_type);
                    set_object(&t, object_get_pointed_object(p_source_obj_opt), source_state_after);
                    type_destroy(&t);
                }
            }
            else if (type_is_obj_owner(p_source_obj_type))
            {
                if (object_get_pointed_object(p_source_obj_opt))
                {
                    struct type t = type_remove_pointer(p_source_obj_type);
                    set_object(&t, object_get_pointed_object(p_source_obj_opt), source_state_after);
                    type_destroy(&t);
                }
            }
        }
        else
        {
            if (p_source_obj_opt)
            {
                set_object(p_source_obj_type, p_source_obj_opt, source_state_after);
            }
        }
    }
    else if (type_is_any_owner(p_dest_obj_type) && type_is_any_owner(p_source_obj_type))
    {
        /*everthing is moved*/
        if (p_source_obj_opt)
            set_object(p_source_obj_type, p_source_obj_opt, source_state_after);
    }
    else if (type_is_obj_owner(p_dest_obj_type))
    {
        if (p_source_obj_type->address_of)
        {
            struct type t = type_remove_pointer(p_source_obj_type);
            set_object(&t, p_source_obj_opt->pointed_ref, source_state_after);
            type_destroy(&t);
        }
    }
    else
    {
        /*nothing changes*/
    }

}

bool object_is_zero_or_null(const struct object* p_object)
{
    return (p_object->state == OBJECT_STATE_NULL) ||
        (p_object->state == OBJECT_STATE_ZERO);
}

/*
   This function must check and do the flow assignment of
   a = b
*/
void object_assignment3(struct parser_ctx* ctx,
    const struct token* error_position,
    enum assigment_type assigment_type,
    bool check_uninitialized_b,
    struct type* p_a_type, struct object* p_a_object,
    struct type* p_b_type, struct object* p_b_object)
{
    if (p_a_object == NULL || p_b_object == NULL)
    {
        return;
    }
    type_print(p_a_type);
    printf(" = ");
    type_print(p_b_type);
    printf("\n");

    /*general check for copying uninitialized object*/
    if (check_uninitialized_b && p_b_object->state & OBJECT_STATE_UNINITIALIZED)
    {
        //a = b where b is uninitialized
        char buffer[100] = { 0 };
        object_get_name(p_b_type, p_b_object, buffer, sizeof buffer);
        if (assigment_type == ASSIGMENT_TYPE_PARAMETER)
        {
            if (!type_is_out(p_a_type))
            {
                compiler_diagnostic_message(W_OWNERSHIP_FLOW_UNINITIALIZED,
                            ctx,
                            error_position,
                            "passing an uninitialized argument '%s' object", buffer);
            }
        }
        else if (assigment_type == ASSIGMENT_TYPE_RETURN)
        {
            compiler_diagnostic_message(W_OWNERSHIP_FLOW_UNINITIALIZED,
                        ctx,
                        error_position,
                        "returning an uninitialized '%s' object", buffer);
        }
        else
        {
            compiler_diagnostic_message(W_OWNERSHIP_FLOW_UNINITIALIZED,
                        ctx,
                        error_position,
                        "reading an uninitialized '%s' object", buffer);
        }

        return;
    }

    /*general check passing possible null to non opt*/
    if (type_is_pointer(p_a_type) &&
        !type_is_opt(p_a_type) &&
        p_b_object->state & OBJECT_STATE_NULL)
    {
#if 0
        char buffer[100] = { 0 };
        object_get_name(p_b_type, p_b_object, buffer, sizeof buffer);

        compiler_diagnostic_message(W_NON_NULL,
                   ctx,
                   error_position,
                   "assignment of possible null object '%s' to non-opt pointer", buffer);
#endif //nullchecks disabled for now
}

    if (type_is_owner(p_a_type) && type_is_pointer(p_a_type))
    {
        /*owner must be empty before assignment = 0*/
        checked_empty(ctx, p_a_type, p_a_object, error_position);

        if (object_is_zero_or_null(p_b_object))
        {
            //a = nullpr
            object_set_zero(p_a_type, p_a_object);
            return;
        }
    }

    if (type_is_void_ptr(p_a_type) && type_is_pointer(p_b_type))
    {
        if (type_is_owner(p_a_type) && object_get_pointed_object(p_b_object))
        {
            struct type t = type_remove_pointer(p_b_type);
            checked_empty(ctx, &t, object_get_pointed_object(p_b_object), error_position);
            type_destroy(&t);
            if (assigment_type == ASSIGMENT_TYPE_PARAMETER)
                object_set_uninitialized(p_b_type, p_b_object);
            else
                object_set_moved(p_b_type, p_b_object);
        }
        return;
    }

    if (type_is_pointer(p_a_type) && type_is_pointer(p_b_type))
    {
        p_a_object->state = p_b_object->state;

        checked_read_object(ctx, p_b_type, p_b_object, error_position, true);

        if (type_is_owner(p_a_type))
        {
            if (assigment_type == ASSIGMENT_TYPE_PARAMETER)
                object_set_uninitialized(p_b_type, p_b_object);
            else
                object_set_moved(p_b_type, p_b_object);
        }

        return;
    }

    if (p_a_type->struct_or_union_specifier && p_a_object->members.size > 0)
    {
        struct struct_or_union_specifier* p_a_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_a_type->struct_or_union_specifier);

        struct struct_or_union_specifier* p_b_struct_or_union_specifier =
            get_complete_struct_or_union_specifier(p_b_type->struct_or_union_specifier);

        if (p_a_struct_or_union_specifier && p_b_struct_or_union_specifier)
        {
            struct member_declaration* p_a_member_declaration =
                p_a_struct_or_union_specifier->member_declaration_list.head;

            struct member_declaration* p_b_member_declaration =
                p_b_struct_or_union_specifier->member_declaration_list.head;

            int member_index = 0;
            while (p_a_member_declaration && p_b_member_declaration)
            {
                if (p_a_member_declaration->member_declarator_list_opt)
                {
                    struct member_declarator* p_a_member_declarator =
                        p_a_member_declaration->member_declarator_list_opt->head;

                    struct member_declarator* p_b_member_declarator =
                        p_b_member_declaration->member_declarator_list_opt->head;

                    while (p_a_member_declarator && p_b_member_declarator)
                    {
                        if (p_a_member_declarator->declarator &&
                            p_b_member_declarator->declarator)
                        {
                            if (member_index < p_a_object->members.size &&
                                member_index < p_b_object->members.size)
                            {

                                struct type* p_a_member_type = &p_a_member_declarator->declarator->type;
                                struct object* p_a_member_object = &p_a_object->members.data[member_index];

                                struct type* p_b_member_type = &p_b_member_declarator->declarator->type;
                                struct object* p_b_member_object = &p_b_object->members.data[member_index];

                                object_assignment3(ctx,
                                    error_position,
                                    assigment_type,
                                    check_uninitialized_b,
                                    p_a_member_type, p_a_member_object,
                                    p_b_member_type, p_b_member_object);
                            }
                            else
                            {
                                //TODO BUG union?                                
                            }
                            member_index++;
                        }
                        p_a_member_declarator = p_a_member_declarator->next;
                        p_b_member_declarator = p_b_member_declarator->next;
                    }
                }
                p_a_member_declaration = p_a_member_declaration->next;
                p_b_member_declaration = p_b_member_declaration->next;
            }
            return;
        }
    }

    p_a_object->state = p_b_object->state;
    if (type_is_owner(p_a_type))
    {
        if (assigment_type == ASSIGMENT_TYPE_PARAMETER)
            object_set_uninitialized(p_b_type, p_b_object);
        else
            object_set_moved(p_b_type, p_b_object);
    }
}
