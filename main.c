#include <stdio.h>

#include "elf_parser.h"

int main(int argc, char **argv)
{
	int ret_value = 0;
	
	if (argc != 2)
	{
		printf("Usage: elf_so_viewer elf_filename\r\n");
		ret_value = -1;
		goto exit;
	}

	int get_dependencies_result = elf_parser_get_dependencies(argv[1], NULL, 0, NULL);
	
	printf("OK: %d\r\n", get_dependencies_result);
	
exit:
	return ret_value;
}


