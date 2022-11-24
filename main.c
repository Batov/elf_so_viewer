#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <elf.h>

int main(int argc, char **argv)
{
	int ret_value = 0;
	
	if (argc != 2)
	{
		printf("Usage: elf_so_viewer elf_filename\r\n");
		ret_value = -1;
		goto exit;
	}
	
	FILE *p_elf_file = fopen(argv[1], "rb");
	if (p_elf_file == NULL)
	{
		printf("Unable to open %s\r\n", argv[1]);
		ret_value = -1;
		goto exit;
	}
	
	Elf32_Ehdr header = {0};
	
	int bytes_from_file = fread(&header, sizeof(header), 1, p_elf_file);
	assert(bytes_from_file == 1);
	
	const char elf_magic[] = ELFMAG;
	bool magic_is_ok = !strncmp(header.e_ident, elf_magic, strlen(elf_magic));
	
	bool is_64 = header.e_ident[EI_CLASS] == ELFCLASS64;
	
	if (is_64)
	{
		int ret = fseek(p_elf_file, 0, SEEK_SET);
		assert(ret == 0);

		Elf64_Ehdr header_64 = {0};
		bytes_from_file = fread(&header_64, sizeof(header_64), 1, p_elf_file);
		assert(bytes_from_file == 1);
		magic_is_ok = !strncmp(header.e_ident, elf_magic, strlen(elf_magic));

		Elf64_Shdr* section_header_table = calloc(header_64.e_shnum, header_64.e_shentsize);
		assert(section_header_table != NULL);

		ret = fseek(p_elf_file, header_64.e_shoff, SEEK_SET);
		assert(ret == 0);
		bytes_from_file = fread(section_header_table, header_64.e_shentsize, header_64.e_shnum, p_elf_file);
		assert(bytes_from_file == header_64.e_shnum);

		Elf64_Shdr *strings_table = &(section_header_table[header_64.e_shstrndx]);


		char* strings_section = malloc(strings_table->sh_size);
		assert(strings_section != NULL);
		ret = fseek(p_elf_file, strings_table->sh_offset, SEEK_SET);
		assert(ret == 0);
		bytes_from_file = fread(strings_section, strings_table->sh_size, 1, p_elf_file);
		assert(bytes_from_file == 1);

		Elf64_Shdr *dynamic_section = NULL;

		for(uint32_t i = 0; i<header_64.e_shnum; i++)
		{
			if (section_header_table[i].sh_type == SHT_DYNAMIC)
			{
				printf("[%u] Name: %s\r\n", i, strings_section+section_header_table[i].sh_name);
				printf("  Type: 0x%x\r\n", section_header_table[i].sh_type);
				printf("  Size: 0x%lx %ld\r\n", section_header_table[i].sh_size, section_header_table[i].sh_size);
				printf("  Offset: 0x%lx\r\n", section_header_table[i].sh_offset);
				printf("  Link: 0x%x\r\n", section_header_table[i].sh_link);
				printf("  Entry size: 0x%lx\r\n", section_header_table[i].sh_entsize);

				dynamic_section = &(section_header_table[i]);
			}
		}

		Elf64_Shdr *dynamic_strings_section = &(section_header_table[dynamic_section->sh_link]);
		char* dyn_strings_section = malloc(dynamic_strings_section->sh_size);
		assert(dyn_strings_section != NULL);
		ret = fseek(p_elf_file, dynamic_strings_section->sh_offset, SEEK_SET);
		assert(ret == 0);
		bytes_from_file = fread(dyn_strings_section, dynamic_strings_section->sh_size, 1, p_elf_file);
		assert(bytes_from_file == 1);

		Elf64_Dyn* dynamic_section_array = malloc(dynamic_section->sh_size);
		assert(dynamic_section_array != NULL);
		ret = fseek(p_elf_file, dynamic_section->sh_offset, SEEK_SET);
		assert(ret == 0);
		bytes_from_file = fread(dynamic_section_array, dynamic_section->sh_size, 1, p_elf_file);
		assert(bytes_from_file == 1);

		assert((dynamic_section->sh_size % sizeof(Elf64_Dyn)) == 0);
		size_t dyn_count = dynamic_section->sh_size / sizeof(Elf64_Dyn);


		for(uint32_t i = 0; i<dyn_count; i++)
		{
			if ((dynamic_section_array[i].d_tag == DT_NEEDED) || (dynamic_section_array[i].d_tag == DT_RPATH))
			{
				printf("Tag: 0x%lx\r\n", dynamic_section_array[i].d_tag);
				printf("Value: 0x%lx %s\r\n", dynamic_section_array[i].d_un.d_val, dyn_strings_section + dynamic_section_array[i].d_un.d_val);
			}
		}
	}


	if (magic_is_ok && is_64)
		printf("Magic\r\n");
	else
		printf("No magic\r\n");
	
	fclose(p_elf_file);
	printf("OK\r\n");
	
exit:
	return ret_value;
}


