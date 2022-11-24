#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "elf_parser.h"

#define MAX_DEPENENCIES_LENGTH (256)

static const char *prefixes[] = {
		"/lib/x86_64-linux-gnu/", "/lib"
	};


static int find_dependencies(const char * full_path, size_t depth)
{
	int result = 0;

	char *dependencies = malloc(MAX_DEPENENCIES_LENGTH+1);

	int get_dependencies_result = elf_parser_get_dependencies(full_path, dependencies, MAX_DEPENENCIES_LENGTH);

	if (get_dependencies_result < 0)
	{
		result = -1;
		goto exit;
	}

	char * next_dependency = dependencies;
	for (size_t i = 0; i < get_dependencies_result; i++)
	{
		for (size_t prefix_idx = 0; prefix_idx < 1; prefix_idx++)
		{
			size_t path_length = strlen(prefixes[prefix_idx]);
			size_t full_path_length = path_length + strlen(next_dependency) + 1;

			char *dependency_full_path = malloc(full_path_length);

			if (dependency_full_path == NULL)
				goto exit;

			strcpy(dependency_full_path, prefixes[prefix_idx]);
			strcpy(dependency_full_path + path_length, next_dependency);

			for (int i = 0; i < depth; ++i)
			{
				printf("    ");
			}

			printf("%s\n", dependency_full_path);

			int ret = find_dependencies(dependency_full_path, depth+1);

			free(dependency_full_path);
		}

		next_dependency += strlen(next_dependency) + 1;
	}

exit:
	free(dependencies);

	return result;
}



int main(int argc, char **argv)
{
	int result = 0;
	
	if (argc != 2)
	{
		printf("Usage: elf_so_viewer elf_filename\r\n");
		result = -1;
		goto exit;
	}

	result = find_dependencies(argv[1], 0);

	
exit:
	return result;
}


