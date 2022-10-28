#include <ctype.h>
#include <stdio.h>
#include <utils/str.h>
#include <arg-parse/arg-parse.h>

static struct muarg_argument_config *
find_argument_from_name(struct muarg_argument_config *array,
                        size_t argument_count, const char *name)
{
    for (size_t i = 0; i < argument_count; i++)
    {
        if (strcmp(array[i].name, name) == 0)
        {
            return &array[i];
        }
    }

    return NULL;
}

static struct muarg_argument_config *
find_argument_from_short_name(struct muarg_argument_config *array,
                              size_t argument_count, char name)
{
    for (size_t i = 0; i < argument_count; i++)
    {
        if (array[i].short_name != MUARG_NO_SHORTNAME &&
            array[i].short_name == name)
        {
            return &array[i];
        }
    }

    return NULL;
}

static bool
is_a_valid_result(const char *str, struct muarg_argument_config *argument)
{
    for (size_t i = 0; i < argument->arg_enum_count; i++)
    {
        if (strcmp(str, argument->arg_enum[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

/* add string argument to the last entry of muarg_result.option_list */
static int
parse_string_argument(char *next_argument,
                      struct muarg_argument_config *argument)
{
    if (next_argument == NULL)
    {
        printf("parameter: %s require a string argument \n", argument->name);
        return MUARG_ERROR;
    }

    argument->status.input = next_argument;

    if (argument->flag & MUARG_FLAG_USE_ONLY_POSSIBLE_RESULT)
    {
        if (is_a_valid_result(next_argument, argument))
        {
            return MUARG_SUCCESS;
        }

        printf(
            "%s is not a valid result for %s, must use these possible results:",
            next_argument, argument->name);

        muarg_show_help_option_possible_results(argument);
        printf("\n");

        return MUARG_ERROR;
    }
    return MUARG_SUCCESS;
}
static int
parse_int_argument(char *next_argument, struct muarg_argument_config *argument)
{
    if (next_argument == NULL)
    {
        printf("parameter: %s require a int argument \n", argument->name);
        return MUARG_ERROR;
    }

    if (!mustr_is_string_a_number(next_argument))
    {
        printf("parameter: %s require a int argument (not a string)\n",
               argument->name);
        return MUARG_ERROR;
    }

    argument->status.input = next_argument;
    argument->status.value = atol(next_argument);
    return MUARG_SUCCESS;
}

static int
parse_string_value(struct muarg_result *final, int argv_id)
{
    int res = vec_push(&final->string_list, final->raw_arguments[argv_id]);
    if (res != 0)
    {
        printf("vector push error \n");
        return MUARG_ERROR;
    }
    return MUARG_SUCCESS;
}

static int
parse_single_argument(struct muarg_result *result, int *argv_id,
                      struct muarg_header *option)
{
    char *current_argv = result->raw_arguments[*argv_id];
    struct muarg_argument_config *argument = NULL;

    if (strncmp("--", current_argv, 2) == 0) // long name
    {
        argument = find_argument_from_name(
            option->argument_list, option->argument_count, current_argv + 2);
    }
    else if (current_argv[0] == '-') // short name
    {
        if (strlen(current_argv) > 2)
        {
            printf("error: argument %s is not recognised\n", current_argv);
            return MUARG_ERROR;
        }
        argument = find_argument_from_short_name(
            option->argument_list, option->argument_count, *(current_argv + 1));
    }
    else
    {
        return parse_string_value(result, *argv_id);
    }

    if (argument == NULL)
    {
        printf("unknown argument: %s \n", current_argv);
        return MUARG_ERROR;
    }

    char *next_argument = NULL;
    if (*argv_id + 1 < result->raw_argument_count)
    {
        next_argument = result->raw_arguments[*argv_id + 1];
    }

    argument->status.is_called = true;

    if (argument->flag & MUARG_FLAG_STRING ||
        argument->flag & MUARG_FLAG_USE_ONLY_POSSIBLE_RESULT)
    {
        if (parse_string_argument(next_argument, argument) == MUARG_ERROR)
        {
            return MUARG_ERROR;
        }
        (*argv_id)++;
    }

    else if (argument->flag & MUARG_FLAG_INT)
    {
        if (parse_int_argument(next_argument, argument) == MUARG_ERROR)
        {
            return MUARG_ERROR;
        }
        (*argv_id)++;
    }

    if (argument->callback != NULL)
    {
        argument->callback(option);
    }
    return MUARG_SUCCESS;
}

struct muarg_result
muarg_eval(struct muarg_header *info, int argc, char **argv)
{
    struct muarg_result result;
    result.raw_argument_count = argc;
    result.raw_arguments = argv;
    result.argument_count = info->argument_count;
    result.argument_list = info->argument_list;
    result.has_error = MUARG_SUCCESS;
    vec_init(&result.string_list);
    for (int i = 1; i < argc; i++)
    {
        result.has_error |= parse_single_argument(&result, &i, info);
    }

    return result;
}

void muarg_destroy_arg_result(struct muarg_result *result)
{
    vec_deinit(&result->string_list);
}

/* output: {element_1|element_2|element_3} for example */
void muarg_show_help_option_possible_results(struct muarg_argument_config *option)
{
    printf("{");

    for (size_t i = 0; i < option->arg_enum_count; i++)
    {
        printf("%s", option->arg_enum[i]);
        if (i < option->arg_enum_count - 1)
        {
            printf("|");
        }
    }
    printf("}\n");
}

static void
show_help_for_option(struct muarg_argument_config *option)
{
    if (option->help_msg == NULL)
    {
        return;
    }

    if (option->name != NULL)
    {
        printf("\t --%-10s", option->name);
    }
    else if (option->short_name != MUARG_NO_SHORTNAME)
    {
        printf("\t -%-10c", option->short_name);
    }

    if (option->flag & MUARG_FLAG_USE_ONLY_POSSIBLE_RESULT)
    {
        muarg_show_help_option_possible_results(option);
        printf("\t%-15s\t  ", ""); // realign everything
    }
    else if (option->flag & MUARG_FLAG_STRING)
    {
        printf("%-5s", "{str}");
    }
    else if (option->flag & MUARG_FLAG_INT)
    {
        printf("%-5s", "{int}");
    }
    else
    {
        printf("%-5s", "");
    }

    printf(" %s \n", option->help_msg);
}

void muarg_show_help(struct muarg_header *info)
{
    if (info->usage != NULL)
    {
        printf("Usage: %s %s \n", info->app_name, info->usage);
    }
    if (info->argument_count != 0)
    {

        printf("Options: \n");

        for (size_t i = 0; i < info->argument_count; i++)
        {
            show_help_for_option(&info->argument_list[i]);
        }
    }

    /* display even more information */
    if (info->help_info != NULL)
    {
        printf("\n %s \n", info->help_info);
    }
}

void muarg_show_version(struct muarg_header *info)
{
    printf("%s version %s \n", info->app_name, info->version);
}

struct muarg_argument_status *
muarg_status_from_name(struct muarg_result *result, const char *name)
{
    for (size_t i = 0; i < result->argument_count; i++)
    {
        if (strcmp(result->argument_list[i].name, name) == 0)
        {
            return &result->argument_list[i].status;
        }
    }
    return NULL;
}

struct muarg_argument_status *
muarg_status_from_short_name(struct muarg_result *result, char short_name)
{
    for (size_t i = 0; i < result->argument_count; i++)
    {
        if (result->argument_list[i].short_name == short_name)
        {
            return &result->argument_list[i].status;
        }
    }
    return NULL;
}
