#pragma once

#include <stddef.h>

int elf_parser_get_dependencies(const char * const filename, char* dependencies[], size_t dependencies_count, char* rpath);