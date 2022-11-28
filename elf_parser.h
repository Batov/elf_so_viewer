#pragma once

int elf_parser_get_dependencies(const char *const filename,
                                char *const dependencies,
                                const size_t max_dependencies_length);
