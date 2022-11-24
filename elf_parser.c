#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <elf.h>

#include "elf_parser.h"

static const char elf_magic[] = ELFMAG;

static int read_from_file(FILE *elf_file, void *dst, size_t offset, size_t length)
{
	int result = 0;

	int fseek_result = fseek(elf_file, offset, SEEK_SET);
	if (fseek_result)
	{
		result = -1;
		goto exit;
	}

	int fread_result = fread(dst, length, 1, elf_file);
	if (fread_result != 1)
		result = -1;

exit:
	return result;
}

int elf_parser_get_dependencies(const char * const filename, char* dependencies[], size_t dependencies_count, char* rpath)
{
	int result = 0;

	FILE *elf_file = NULL;
	Elf64_Shdr* sections_header_table = NULL;
	char* dynamic_strings_section = NULL;
	Elf64_Dyn* dynamic_section = NULL;

	elf_file = fopen(filename, "rb");
	if (elf_file == NULL)
	{
		printf("Unable to open %s\r\n", filename);
		result = -1;
		goto exit;
	}

	Elf32_Ehdr header = {0};
	int read_result = read_from_file(elf_file, &header, 0, sizeof(header));
	if (read_result)
	{
		printf("Unable to read header from %s\r\n", filename);
		result = -1;
		goto cleanup;
	}

	bool magic_is_ok = !strncmp(header.e_ident, elf_magic, strlen(elf_magic));
	if (!magic_is_ok)
	{
		printf("%s is not ELF file\r\n", filename);
		result = -1;
		goto cleanup;
	}

	bool is_64 = header.e_ident[EI_CLASS] == ELFCLASS64;
	if (is_64)
	{
		Elf64_Ehdr header_64 = {0};
		read_result = read_from_file(elf_file, &header_64, 0, sizeof(header_64));
		if (read_result)
		{
			printf("Unable to read header (64-bit) from %s\r\n", filename);
			result = -1;
			goto cleanup;
		}

		sections_header_table = calloc(header_64.e_shnum, header_64.e_shentsize);
		if (sections_header_table == NULL)
		{
			printf("Unable to allocate memory for sections header table\r\n");
			result = -1;
			goto cleanup;
		}

		read_result = read_from_file(elf_file, sections_header_table, header_64.e_shoff, header_64.e_shentsize*header_64.e_shnum);
		if (read_result)
		{
			printf("Unable to read sections header table from %s\r\n", filename);
			result = -1;
			goto cleanup;
		}

		Elf64_Shdr *dynamic_section_header = NULL;
		for(size_t i = 0; i<header_64.e_shnum; i++)
		{
			if (sections_header_table[i].sh_type == SHT_DYNAMIC)
			{
				dynamic_section_header = &(sections_header_table[i]);
				break;
			}
		}

		Elf64_Shdr *dynamic_strings_section_header = &(sections_header_table[dynamic_section_header->sh_link]);
		dynamic_strings_section = malloc(dynamic_strings_section_header->sh_size);
		if (dynamic_strings_section == NULL)
		{
			printf("Unable to allocate memory for dynamic strings section\r\n");
			result = -1;
			goto cleanup;
		}

		read_result = read_from_file(elf_file, dynamic_strings_section, dynamic_strings_section_header->sh_offset, dynamic_strings_section_header->sh_size);
		if (read_result)
		{
			printf("Unable to read dynamic strings section from %s\r\n", filename);
			result = -1;
			goto cleanup;
		}

		dynamic_section = malloc(dynamic_section_header->sh_size);
		if (dynamic_section == NULL)
		{
			printf("Unable to allocate memory for dynamic section\r\n");
			result = -1;
			goto cleanup;
		}

		read_result = read_from_file(elf_file, dynamic_section, dynamic_section_header->sh_offset, dynamic_section_header->sh_size);
		if (read_result)
		{
			printf("Unable to read dynamic section from %s\r\n", filename);
			result = -1;
			goto cleanup;
		}

		assert((dynamic_section_header->sh_size % sizeof(Elf64_Dyn)) == 0);
		size_t dynamic_section_items_count = dynamic_section_header->sh_size / sizeof(Elf64_Dyn);

		for(size_t i = 0; i<dynamic_section_items_count; i++)
		{
			if (dynamic_section[i].d_tag == DT_NEEDED)
			{
				printf("%s\r\n", dynamic_strings_section + dynamic_section[i].d_un.d_val);
			}
		}

	}
	else
	{
		printf("No 32-bit support\r\n");
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