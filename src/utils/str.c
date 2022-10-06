#include <utils/str.h>
#include <string.h>

bool
mustr_is_string_a_number(const char *str)
{
	size_t length = strlen(str);

	for (size_t i = 0; i < length; i++)
	{
		if (!(isdigit(str[i]) || str[i] == '-'))
		{
			return false;
		}
	}

	return true;
}

int
mustr_find_table_entry(const char *entry_name, const char **table,
					   size_t table_size)
{
	for (size_t i = 0; i < table_size; i++)
	{
		if (strcmp(table[i], entry_name) == 0)
		{
			return i;
		}
	}
	return STR_TABLE_NOT_FOUNDED;
}
