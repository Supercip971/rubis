#ifndef _MULIB_ARG_PARSER_H
#define _MULIB_ARG_PARSER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <ds/vec.h>
/* true when you have it and false when the argument don't exist */
#define MUARG_FLAG_BOOL (1 << 1)

/* for example --name "hey" */
#define MUARG_FLAG_STRING (1 << 2)

/* force the use of a possible result, must use
 * CREATE_MUARG_WITH_POSSIBLE_RESULT_LIST */
#define MUARG_FLAG_USE_ONLY_POSSIBLE_RESULT (1 << 3)

/* force the use of an int: example --memory 4096 */
#define MUARG_FLAG_INT (1 << 4)

#define MUARG_NO_CALLBACK NULL

#define MUARG_NO_SHORTNAME (0)

#define MUARG_ERROR	  1
#define MUARG_SUCCESS 0

struct muarg_header
{
	/* must not be null */
	const char *app_name;

	/* can be NULL, you can have for exemple "[FILE] -o [OUTPUT]",
					 in the help you will have: {app_name} [FILE] -o [OUTPUT]*/
	const char *usage;

	const char *help_info; /* more information if you need it */
	const char *version;   /* can be null */

	size_t argument_count;
	struct muarg_argument_config *argument_list;
};

struct muarg_argument_status
{
	bool is_called; /* true if you have the argument in the command line
					   arguments */
	const char *input;
	long value; /* only with MUARG_FLAG_INT */
};

#define MUARG_DEFAULT_ARGUMENT_STATUS                                          \
	{                                                                          \
		false, "", 0                                                           \
	}

struct muarg_argument_config
{
	uint32_t flag;

	const char *name;	 // must not be NULL
	char short_name;	 // can be 0

	const char *help_msg;	 // can be NULL

	size_t arg_enum_count;
	const char **arg_enum;

	void (*callback)(struct muarg_header *);

	struct muarg_argument_status
		status; /* status is updated after muarg_eval() */
};

struct muarg_result
{

	int raw_argument_count; /* argc */
	char **raw_arguments;	/* argv */

	vec_str_t string_list; /* everything that is not an option like in "gcc
							  hello.c" we have hello.c */

	size_t argument_count; /* argument count in argument_list */
	struct muarg_argument_config
		*argument_list; /* same argument list in header */

	int has_error; /* see MUARG_SUCCESS or MUARG_ERROR */
};

#define MUARG_CREATE_HEADER(app_name, app_argument_help, help_information,     \
							version, arg_list)                                 \
	{                                                                          \
		app_name, app_argument_help, help_information, version,                \
			sizeof(arg_list) / sizeof(arg_list[0]), arg_list                   \
	}

#define MUARG_BOOL(name, short_name, help_msg, callback)                       \
	{                                                                          \
		MUARG_FLAG_BOOL, name, short_name, help_msg, 0, NULL, callback,        \
			MUARG_DEFAULT_ARGUMENT_STATUS                                      \
	}

#define MUARG_STRING(name, short_name, help_msg, callback)                     \
	{                                                                          \
		MUARG_FLAG_STRING, name, short_name, help_msg, 0, NULL, callback,      \
			MUARG_DEFAULT_ARGUMENT_STATUS                                      \
	}

#define MUARG_INT(name, short_name, help_msg, callback)                        \
	{                                                                          \
		MUARG_FLAG_INT, name, short_name, help_msg, 0, NULL, callback,         \
			MUARG_DEFAULT_ARGUMENT_STATUS                                      \
	}

#define MUARG_ENUM(name, short_name, help_msg, enums, callback)                \
	{                                                                          \
		(MUARG_FLAG_USE_ONLY_POSSIBLE_RESULT | MUARG_FLAG_STRING), name,       \
			short_name, help_msg, sizeof((enums)) / sizeof((enums)[0]),        \
			(enums), callback, MUARG_DEFAULT_ARGUMENT_STATUS                   \
	}

struct muarg_result muarg_eval(struct muarg_header *info, int argc,
							   char **argv);

void muarg_destroy_arg_result(struct muarg_result *result);

void muarg_show_help(struct muarg_header *info);

void
muarg_show_help_option_possible_results(struct muarg_argument_config *option);

void muarg_show_version(struct muarg_header *info);

#define MUARG_HELP()                                                           \
	MUARG_BOOL("help", 'h', "display help information", muarg_show_help)

#define MUARG_VERSION()                                                        \
	MUARG_BOOL("version", 'v', "display the program version",                  \
			   muarg_show_version)

/* get an argument status from its name */

/* aka --name */
struct muarg_argument_status *
muarg_status_from_name(struct muarg_result *result, const char *name);

/* aka -name */
struct muarg_argument_status *
muarg_status_from_short_name(struct muarg_result *result, char short_name);

#endif /* !_MULIB_ARG_PARSER_H */
