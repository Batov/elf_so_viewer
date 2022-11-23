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
	int magic_is_ok = !strncmp(header.e_ident, elf_magic, strlen(elf_magic));

	
	if (magic_is_ok)
		printf("Magic\r\n");
	else
		printf("No magic\r\n");
	
	fclose(p_elf_file);
	printf("OK\r\n");
	
exit:
	return ret_value;
}


