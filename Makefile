TARGET=elf_so_viewer

all:
	gcc -Wall -I. main.c elf_parser.c -o $(TARGET)

clean:
	rm $(TARGET)
