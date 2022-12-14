#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <elf.h>

#include "elf_parser.h"

#define FROM_FILE_START (0x00)

static const char elf_magic[] = ELFMAG;

/**
 * @brief      Read from file.
 *
 * @param      elf_file  The elf file (already opened)
 * @param      dst       The destination
 * @param[in]  offset    The offset in file
 * @param[in]  length    The length of data
 *
 * @return     0 - SUCCESS, -1 - ERROR
 */
static int read_from_file(FILE *elf_file, void *dst, size_t offset, size_t length)
{
    int result = 0;

    int fseek_result = fseek(elf_file, offset, SEEK_SET);
    if (fseek_result)
    {
        result = -1;
        goto exit;
    }

    const size_t count = 1;
    int fread_result   = fread(dst, length, count, elf_file);
    if (fread_result != count)
        result = -1;

exit:
    return result;
}

int elf_parser_get_dependencies(const char *filename, char *dependencies, const size_t max_dependencies_length)
{
    int result = 0;

    FILE *elf_file                    = NULL;
    Elf64_Shdr *sections_header_table = NULL;
    char *dynamic_strings_section     = NULL;
    Elf64_Dyn *dynamic_section        = NULL;

    elf_file = fopen(filename, "rb");
    if (elf_file == NULL)
    {
        result = -1;
        goto exit;
    }

    Elf64_Ehdr header_64 = {0};
    int read_result      = read_from_file(elf_file, &header_64, FROM_FILE_START, sizeof(header_64));
    if (read_result)
    {
        printf("Unable to read header from %s\n", filename);
        result = -1;
        goto cleanup;
    }

    bool magic_is_ok = !memcmp(header_64.e_ident, elf_magic, sizeof(elf_magic) - 1);
    if (!magic_is_ok)
    {
        printf("%s is not ELF file\n", filename);
        result = -1;
        goto cleanup;
    }

    bool is_64 = header_64.e_ident[EI_CLASS] == ELFCLASS64;
    if (is_64)
    {
        sections_header_table = calloc(header_64.e_shnum, header_64.e_shentsize);
        if (sections_header_table == NULL)
        {
            printf("Unable to allocate memory for sections header table\n");
            result = -1;
            goto cleanup;
        }

        read_result = read_from_file(elf_file, sections_header_table, header_64.e_shoff,
                                     header_64.e_shentsize * header_64.e_shnum);
        if (read_result)
        {
            printf("Unable to read sections header table from %s\n", filename);
            result = -1;
            goto cleanup;
        }

        // Try to find DYNAMIC section header
        Elf64_Shdr *dynamic_section_header = NULL;
        for (size_t i = 0; i < header_64.e_shnum; i++)
        {
            if (sections_header_table[i].sh_type == SHT_DYNAMIC)
            {
                dynamic_section_header = &(sections_header_table[i]);
                break;
            }
        }

        if (dynamic_section_header == NULL)
        {
            printf("No dynamic section\n");
            result = -1;
            goto cleanup;
        }

        // Go to dynamic strings section by link field from dynamic section header
        Elf64_Shdr *dynamic_strings_section_header = &(sections_header_table[dynamic_section_header->sh_link]);
        dynamic_strings_section                    = malloc(dynamic_strings_section_header->sh_size);
        if (dynamic_strings_section == NULL)
        {
            printf("Unable to allocate memory for dynamic strings section\n");
            result = -1;
            goto cleanup;
        }

        read_result = read_from_file(elf_file, dynamic_strings_section, dynamic_strings_section_header->sh_offset,
                                     dynamic_strings_section_header->sh_size);
        if (read_result)
        {
            printf("Unable to read dynamic strings section from %s\n", filename);
            result = -1;
            goto cleanup;
        }

        dynamic_section = malloc(dynamic_section_header->sh_size);
        if (dynamic_section == NULL)
        {
            printf("Unable to allocate memory for dynamic section\n");
            result = -1;
            goto cleanup;
        }

        read_result = read_from_file(elf_file, dynamic_section, dynamic_section_header->sh_offset,
                                     dynamic_section_header->sh_size);
        if (read_result)
        {
            printf("Unable to read dynamic section from %s\n", filename);
            result = -1;
            goto cleanup;
        }

        assert((dynamic_section_header->sh_size % sizeof(Elf64_Dyn)) == 0);
        size_t dynamic_section_items_count = dynamic_section_header->sh_size / sizeof(Elf64_Dyn);
        size_t dependencies_length         = 0;

        // Try to find NEEDED field
        for (size_t i = 0; i < dynamic_section_items_count; i++)
        {
            if (dynamic_section[i].d_tag == DT_NEEDED)
            {
                char *needed_name              = dynamic_strings_section + dynamic_section[i].d_un.d_val;
                size_t needed_name_bytes_count = strlen(needed_name) + 1;    // with \0

                // If dependencies output buffer has enough space for current needed name
                if (dependencies_length + needed_name_bytes_count <= max_dependencies_length)
                {
                    strcpy(dependencies, needed_name);
                    dependencies_length += needed_name_bytes_count;
                    dependencies += needed_name_bytes_count;
                    result++;
                }
                else
                {
                    printf("Dependencies list is so long for %s\n", filename);
                    break;
                }
            }
        }
    }
    else
    {
        printf("No 32-bit support\n");
        result = -1;
    }

cleanup:
    if (elf_file)
        fclose(elf_file);

    if (sections_header_table)
        free(sections_header_table);

    if (dynamic_strings_section)
        free(dynamic_strings_section);

    if (dynamic_section)
        free(dynamic_section);

exit:
    return result;
}
