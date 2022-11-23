#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
		Elf64_Ehdr header_64 = {0};
		bytes_from_file = fread(&header_64, sizeof(header_64), 1, p_elf_file);
		assert(bytes_from_file == 1);
		magic_is_ok = !strncmp(header.e_ident, elf_magic, strlen(elf_magic));
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


