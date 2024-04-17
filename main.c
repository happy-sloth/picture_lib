#include <stdio.h>
#include "jpg_codec.h"

int main(int argc, char** argv)
{
    // if (argc < 2){
    //     printf("Input file is not specified.");
    //     return -1;
    // }

    FILE *output = NULL;
    // output = fopen(argv[1], "rb");
    // output = fopen("/home/happy-sloth/pet/picture_lib/1234.jpg", "rb");
    // output = fopen("/home/happy-sloth/pet/picture_lib/60669610479084c8ecd776f5a2acd10c.jpg", "rb");
    output = fopen("/home/happy-sloth/pet/picture_lib/test3.jpg", "rb");
    // 
    
    if (!output){
        printf("Can't find file %s\n", argv[1]);
        return -1;
    }
    void* l_array = NULL;

    int l_ret_status = jpg_codec_file_decode(output, &l_array);

    printf("ret_status = %d\n", l_ret_status);
    if (output)
        fclose(output);
	return 0;
}