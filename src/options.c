#include "options.h"
#include <string.h>
#include "console.h"
#include <stdio.h>
#include <assert.h>

static struct w {
    enum compiler_warning w;
    const char* name;
}
s_warnings[] = {
    {W_UNUSED_VARIABLE, "unused-variable"},
    {W_DEPRECATED, "deprecated"},
    {W_ENUN_COMPARE,"enum-compare"},
    {W_NON_NULL, "nonnull"},
    {W_ADDRESS, "address"},
    {W_UNUSED_PARAMETER, "unused-parameter"},
    {W_DECLARATOR_HIDE, "hide-declarator"},
    {W_TYPEOF_ARRAY_PARAMETER, "typeof-parameter"},
    {W_ATTRIBUTES, "attributes"},
    {W_UNUSED_VALUE, "unused-value"}
};

enum compiler_warning  get_warning_flag(const char* wname, bool* p_disable_warning)
{
    const bool disable_warning = (wname[2] == 'n' && wname[3] == 'o');
    *p_disable_warning = disable_warning;

    for (int j = 0; j < sizeof(s_warnings) / sizeof(s_warnings[0]); j++)
    {
        if (disable_warning)
        {
            if (strncmp(s_warnings[j].name, wname + 5, strlen(s_warnings[j].name)) == 0)
            {
                return s_warnings[j].w;
            }
        }
        else
        {
            if (strncmp(s_warnings[j].name, wname + 2, strlen(s_warnings[j].name)) == 0)
            {
                return s_warnings[j].w;
            }
        }
    }
    return 0;
}

const char* get_warning_name(enum compiler_warning w)
{
    int lower_index = 0;
    int upper_index = sizeof(s_warnings) / sizeof(s_warnings[0]) - 1;

    while (lower_index <= upper_index)
    {
        const int mid = (lower_index + upper_index) / 2;
        const int cmp = w - s_warnings[mid].w;

        if (cmp == 0)
        {
            return s_warnings[mid].name;
        }
        else if (cmp < 0)
        {
            upper_index = mid - 1;
        }
        else
        {
            lower_index = mid + 1;
        }
    }

    assert(false);
    return "";
}

int fill_options(struct options* options,
    int argc,
    const char** argv)
{

    /*
       default at this moment is same as -Wall
    */
    options->enabled_warnings_stack[0] = ~0;


    /*first loop used to collect options*/
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
            continue;

        if (strcmp(argv[i], "-no-output") == 0)
        {
            options->no_output = true;
            continue;
        }

        if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
            {
                strcpy(options->output, argv[i + 1]);
                i++;
            }
            else
            {
                //ops
            }
            continue;
        }

        if (strcmp(argv[i], "-E") == 0)
        {
            options->preprocess_only = true;
            continue;
        }
        if (strcmp(argv[i], "-remove-comments") == 0)
        {
            options->remove_comments = true;
            continue;
        }
        if (strcmp(argv[i], "-direct-compilation") == 0 ||
            strcmp(argv[i], "-rm") == 0)
        {
            options->direct_compilation = true;
            continue;
        }
        if (strcmp(argv[i], "-n") == 0)
        {
            options->check_naming_conventions = true;
            continue;
        }
        if (strcmp(argv[i], "-fi") == 0)
        {
            options->format_input = true;
            continue;
        }


        if (strcmp(argv[i], "-st") == 0)
        {
            options->do_static_analisys = true;
            continue;
        }


        if (strcmp(argv[i], "-fo") == 0)
        {
            options->format_ouput = true;
            continue;
        }

        if (strcmp(argv[i], "-no-discard") == 0)
        {
            options->nodiscard_is_default = true;
            continue;
        }


        //
        if (strcmp(argv[i], "-target=c89") == 0)
        {
            options->target = LANGUAGE_C89;
            continue;
        }

        if (strcmp(argv[i], "-target=c99") == 0)
        {
            options->target = LANGUAGE_C99;
            continue;
        }
        if (strcmp(argv[i], "-target=c11") == 0)
        {
            options->target = LANGUAGE_C11;
            continue;
        }
        if (strcmp(argv[i], "-target=c2x") == 0)
        {
            options->target = LANGUAGE_C2X;
            continue;
        }
        if (strcmp(argv[i], "-target=cxx") == 0)
        {
            options->target = LANGUAGE_CXX;
            continue;
        }



        //
        if (strcmp(argv[i], "-std=c99") == 0)
        {
            options->input = LANGUAGE_C99;
            continue;
        }
        if (strcmp(argv[i], "-std=c11") == 0)
        {
            options->input = LANGUAGE_C11;
            continue;
        }
        if (strcmp(argv[i], "-std=c2x") == 0)
        {
            options->input = LANGUAGE_C2X;
            continue;
        }
        if (strcmp(argv[i], "-std=cxx") == 0)
        {
            options->input = LANGUAGE_CXX;
            continue;
        }

        //warnings
        if (argv[i][1] == 'W')
        {
            if (strcmp(argv[i], "-Wall") == 0)
            {
                options->enabled_warnings_stack[0] = ~0;
                continue;
            }

            bool disable_warning = 0;
            enum compiler_warning  w = get_warning_flag(argv[i], &disable_warning);

            if (disable_warning)
            {
                options->enabled_warnings_stack[0] &= ~w;
            }
            else
            {
                options->enabled_warnings_stack[0] |= w;
            }
            continue;
        }
    }
    return 0;
}


void print_help()
{
    const char* options =
        "cake [options] source1.c source2.c ...\n"
        "\n"
        WHITE "SAMPLES\n" RESET
        "\n"
        WHITE "    cake source.c\n" RESET
        "    Compiles source.c and ouputs /out/source.c\n"
        "\n"
        WHITE "    cake -target=C11 source.c\n" RESET
        "    Compiles source.c and ouputs C11 code at /out/source.c\n"
        "\n"
        WHITE "OPTIONS\n" RESET
        "\n"
        WHITE "  -I                   " RESET " Adds a directory to the list of directories searched for include files \n"
        "                        (On windows, if you run cake at the visual studio command prompt cake \n"
        "                        uses the same include files used by msvc )\n"
        "\n"
        WHITE "  -no-output            " RESET "Cake will not generate ouput\n"
        "\n"
        WHITE "  -D                    " RESET "Defines a preprocessing symbol for a source file \n"
        "\n"
        WHITE "  -E                    " RESET "Copies preprocessor output to standard output \n"
        "\n"
        WHITE "  -o name.c             " RESET "Defines the ouput name. used when we compile one file\n"
        "\n"
        WHITE "  -remove-comments      " RESET "Remove all comments from the ouput file \n"
        "\n"
        WHITE "  -direct-compilation   " RESET "output without macros/preprocessor parts\n"
        "\n"
        WHITE "  -target=standard      " RESET "Output target C standard (c89, c99, c11, c2x, cxx) \n"
        "                        C99 is the default and C89 (ANSI C) is the minimum target \n"
        "\n"
        WHITE "  -std=standard         " RESET "Assume that the input sources are for standard (c89, c99, c11, c2x, cxx) \n"
        "                        (not implented yet, input is considered C23)                    \n"
        "\n"
        WHITE "  -n                    " RESET "Check naming conventions (it is hardcoded for its own naming convention)\n"
        "\n"
        WHITE "  -fi                   " RESET "Format input (format before language convertion)\n"
        "\n"
        WHITE "  -fo                   " RESET "Format output (format after language convertion, result parsed again)\n"
        "\n"
        WHITE "  --no-discard          " RESET "Makes [[nodiscard]] default implicitly \n"
        "\n"
        WHITE "  -Wname -Wno-name      " RESET "Enables or disable warning\n"

        "\n"
        "More details at http://thradams.com/cake/manual.html\n"
        ;

    printf("%s", options);
}
