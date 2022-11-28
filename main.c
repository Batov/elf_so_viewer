#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf_parser.h"

#define MAX_DEPENENCIES_LENGTH (256)
#define MAX_DEPTH              (16)

/**
 * Path prefixes for libraries search
 */
static const char *prefixes[] = {"/lib/x86_64-linux-gnu/", "/usr/local/lib/", "/usr/local/lib/x86_64-linux-gnu"};

/**
 * Dependencies divided by \n symbol
 */
static char top_deps[MAX_DEPENENCIES_LENGTH] = {0};
static char *next_top_deps                   = top_deps;

/**
 * @brief      Finds dependencies. Fill global top_deps string
 *
 * @param[in]  full_path  The full path to ELF file
 * @param[in]  depth      The depth - recursion depth
 *
 * @return     0 - SUCCESS, -1 - ERROR
 */
static int find_dependencies(const char *full_path, size_t depth)
{
    int result = 0;

    if (depth >= MAX_DEPTH)
    {
        printf("It is so deep, try to increase MAX_DEPTH\n");
        result = -1;
        goto exit;
    }

    char *deps = malloc(MAX_DEPENENCIES_LENGTH);

    if (deps == NULL)
    {
        result = -1;
        goto exit;
    }

    // Get dependencies from ELF file
    int deps_count = elf_parser_get_dependencies(full_path, deps, MAX_DEPENENCIES_LENGTH);

    if (deps_count < 0)
    {
        result = -1;
        goto cleanup;
    }

    char *cur_dep_name = deps;
    for (size_t i = 0; i < deps_count; i++)
    {
        size_t cur_dep_name_length = strlen(cur_dep_name);

        if (strstr(top_deps, cur_dep_name) == NULL)
        {
            strcpy(next_top_deps, cur_dep_name);
            next_top_deps[cur_dep_name_length] = '\n';
            next_top_deps += cur_dep_name_length + 1;
        }

        // Try to find dependency full path
        const size_t prefixes_count = sizeof(prefixes) / sizeof(prefixes[0]);
        for (size_t prefix_idx = 0; prefix_idx < prefixes_count; prefix_idx++)
        {
            size_t cur_prefix_length        = strlen(prefixes[prefix_idx]);
            size_t cur_dep_full_path_length = cur_prefix_length + cur_dep_name_length + 1;

            char *cur_dep_full_path = malloc(cur_dep_full_path_length);

            if (cur_dep_full_path == NULL)
            {
                result = -1;
                goto cleanup;
            }

            strcpy(cur_dep_full_path, prefixes[prefix_idx]);
            strcpy(cur_dep_full_path + cur_prefix_length, cur_dep_name);

            bool is_cur_dep_exist = !!find_dependencies(cur_dep_full_path, depth + 1);

            free(cur_dep_full_path);

            // Dependency full path has been found, stop prefix search
            if (is_cur_dep_exist)
                break;
        }

        // Switch on next dependency (It's \0 symbol divided)
        cur_dep_name += strlen(cur_dep_name) + 1;
    }

cleanup:
    free(deps);

exit:
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

    if (result < 0)
    {
        printf("Unable to find dependencies for %s\n", argv[1]);
        goto exit;
    }

    printf("%s", top_deps);

exit:
    return result;
}
