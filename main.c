#include <stdio.h>
#include "jpg_codec.h"

int main(int argc, char** argv)
{
    FILE *output = NULL;
    output = fopen("./60669610479084c8ecd776f5a2acd10c.jpg", "rb");
    
    // output = fopen("/mnt/e/Фото и видео архив/На печать/жипег/20230507_102255.jpg", "rb");
    void* l_array = NULL;

    int l_ret_status = jpg_codec_file_decode(output, &l_array);

    printf("ret_status = %d\n", l_ret_status);
    if (output)
        fclose(output);
	return 0;
}