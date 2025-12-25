/**
 * Simple tool to create conversion table
 */

#include <gammu.h>

int main(int argc, char *argv[])
{
	unsigned char InputBuffer[10000], Buffer[10000];
	FILE *file;
	int size, i, j = 0;

    if (argc != 2) {
        printf("Usage: makeconverttable FILE\n");
        return 1;
    }

	file = fopen(argv[1], "rb");
	if (file == NULL) {
        printf("Failed to open file: %s\n", argv[1]);
        return 2;
    }
	size = fread(InputBuffer, 1, 10000 - 1, file);
	fclose(file);

	InputBuffer[size] = 0;
	InputBuffer[size + 1] = 0;

	ReadUnicodeFile(Buffer, InputBuffer);

	for (i = 0; i < ((int)UnicodeLength(Buffer)); i++) {
		j++;
		if (j == 100) {
			printf("\"\\\n\"");
			j = 0;
		}
		printf("\\x%02x\\x%02x", Buffer[i * 2], Buffer[i * 2 + 1]);
	}
	printf("\\x00\\x00");
    return 0;
}
