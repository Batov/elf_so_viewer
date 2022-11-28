#pragma once

/**
 * @brief      Fetch dependencies from ELF file (NEEDED fields)
 *
 * @param[in]  filename                 The filename to ELF-file
 * @param      dependencies             The dependencies - dependencie's names divided by \0 symbol
 * @param[in]  max_dependencies_length  The maximum dependencies length (with \0 symbols)
 *
 * dependencies == "dep1.so\0dep2.so\0dep3.so\0", result is 3
 *
 * @return     result >= 0 Count of dependencies, result < 0 error
 */
int elf_parser_get_dependencies(const char *filename, char *const dependencies, const size_t max_dependencies_length);
