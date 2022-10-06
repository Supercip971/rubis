#ifndef _MULIB_STR_UTILS_H
#define _MULIB_STR_UTILS_H
#include <ctype.h>

#include <stddef.h>
#include <stdbool.h>

#define STR_TABLE_NOT_FOUNDED (-1)

bool mustr_is_string_a_number(const char *str);

/* from a string table, return if we founded an table entry that match `entry`,
 * if no entry is founded, it will return STR_TABLE_NOT_FOUNDED */
int mustr_find_table_entry(const char *entry, const char **table,
						   size_t table_size);

#endif /* _MULIB_STR_UTILS_H */
