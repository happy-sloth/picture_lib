#include "jpg_codec.h"
#include "jpg_common.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"


typedef int (*block_handler)(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);

typedef enum chunk_types_codes {
    CHUNK_TYPE_DQT = 0xFFDB,
    CHUNK_TYPE_SOF0 = 0xFFC0,
    CHUNK_TYPE_DHT = 0xFFC4,
    CHUNK_TYPE_SOS = 0xFFDA,
    CHUNK_TYPE_COMMENT = 0xFFFE
} chunk_types_codes_t;

typedef struct {
    uint16_t type;
    block_handler handler;
} handlers_table_item_t;

static int chunk_handler_dqt(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);
static int chunk_handler_sof0(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);
static int chunk_handler_dht(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);
static int chunk_handler_sos(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);
static int chunk_handler_comment(jpg_file_params_t *jpg_param, void* data, uint16_t data_size);

static int decode_data_flow(jpg_decoding_params_t *decoding_param, uint8_t*** matrixes);

static handlers_table_item_t s_block_handlers[] = {
    {.type = CHUNK_TYPE_DQT, .handler = chunk_handler_dqt},
    {.type = CHUNK_TYPE_SOF0, .handler = chunk_handler_sof0},
    {.type = CHUNK_TYPE_DHT, .handler = chunk_handler_dht},
    {.type = CHUNK_TYPE_SOS, .handler = chunk_handler_sos},
    {.type = CHUNK_TYPE_COMMENT, .handler = chunk_handler_comment},
};

int jpeg_codec_zigzag_to_matrix(int bytes_per_value, uint16_t ***matrix_ptr, uint8_t* l_array, size_t arr_size)
{
    uint16_t **b_matrix = NULL;

    b_matrix = calloc(8, sizeof(uint16_t*));
    for (int i = 0 ; i < 8 ; i++){
        b_matrix[i] = calloc(8, sizeof(uint16_t*));
    }

    int col = 0, row = 0;
    bool revers = false;
    b_matrix[row][col] = l_array[0];

    for (int i = 1 ; i < 64; i++){
        if ((row == 0 || row == 7)) {
            revers = row ? false : true;
            col++;
            b_matrix[row][col] = l_array[i];
            i++;
        } else if ((col == 7 || col == 0)){
            revers = col ? true : false;
            row++;
            b_matrix[row][col] = l_array[i];
            i++;
        }
        if (row == 7 && col == 7)
            break;
        if (revers){
            col = col-1 < 0 ? 0 : col-1;
            row = row+1 > 7 ? 7 : row+1;
            b_matrix[row][col] = l_array[i];
        } else {
            col = col+1 > 7 ? 7 : col+1;
            row = row-1 < 0 ? 0 : row-1;
            b_matrix[row][col] = l_array[i];
        }
    }

    if (matrix_ptr)
        *matrix_ptr = b_matrix;
    return 0;
}



int jpg_codec_file_dump(jpg_file_params_t *jpg_param)
{
    printf("=== Dump of jpg_file ===\n");
    printf("[DQT] Number of DQT tables: %d\n", jpg_param->dqt_tables_cnt);
    for (int i = 0; i < jpg_param->dqt_tables_cnt; i++){
        uint16_t **dqt_table = NULL;
        printf("[DQT] \tDQT table id: %d\n", jpg_param->dqt_param[i]->header.tbl_id);
        printf("[DQT] \tDQT table value size: %d byte\n", jpg_param->dqt_param[i]->header.tbl_value_size);
        jpeg_codec_zigzag_to_matrix(jpg_param->dqt_param[i]->header.tbl_value_size, &dqt_table, jpg_param->dqt_param[i]->table, 64);
        printf("[DQT] \tDQT table:\n");
        for (int k = 0; k < 8; k++){
            printf("[DQT] \t\t[");
            for (int j = 0; j < 8; j++){
                printf("%.4hX ", dqt_table[k][j]);
            }
            printf("]\n");
        }
        for (int i = 0; i < 8;i++)
            free(dqt_table[i]);
        
        free(dqt_table);
    }
    printf("\n");
    printf("[SOF0] Precision: %d\n", jpg_param->sof0->header.precision);
    printf("[SOF0] Image height: %d\n", jpg_param->sof0->header.height);
    printf("[SOF0] Image width: %d\n", jpg_param->sof0->header.width);
    printf("[SOF0] Number of channels: %d\n", jpg_param->sof0->header.channel_cnt);
    for (int i = 0; i < jpg_param->sof0->header.channel_cnt; i++){
        printf("[SOF0] \tChannel id: %d\n", jpg_param->sof0->channels[i].id);
        printf("[SOF0] \tHorizontal thinning: %d\n", jpg_param->sof0->channels[i].h_thinning);
        printf("[SOF0] \tVertical thinning: %d\n", jpg_param->sof0->channels[i].v_thinning);
        printf("[SOF0] \tDQT table id: %d\n\n", jpg_param->sof0->channels[i].dqt_id);
    }

    printf("\n");
    printf("[DHT] DHTs count: %d\n", jpg_param->dht_cnt);
    for (int i = 0; i < jpg_param->dht_cnt; i++){
        printf("[DHT] \tTable id: %d\n", jpg_param->dht[i]->header.table_id);
        printf("[DHT] \tCLASS: %s\n", jpg_param->dht[i]->header.table_class ? "AC" : "DC");
        printf("[DHT] \tCodes length counts:\n");
        printf("[DHT] \t");
        for (int j = 0; j < 16; j++){
            printf("[%d] ", jpg_param->dht[i]->header.codes_cnts_by_length[j]);
        }
        printf("\n");
        printf("[DHT] \tCodes: \n");
        printf("[DHT] \t");
        for (int k = 0; k < jpg_param->dht[i]->header.codes_value_cnt; k++){
            if (k%16 == 0)
                printf("\n[DHT] \t");
            printf("[0x%.2X] ", jpg_param->dht[i]->codes_value[k]);
        }
        printf("\n\n");
    }

    printf("\n");
    printf("[SOS] Channels count: %d\n", jpg_param->sos->channel_cnt);
    for (int i = 0; i < jpg_param->sos->channel_cnt; i++){
        printf("[SOS] \tChannel id: %d\n", jpg_param->sos->channels[i].channel_id);
        printf("[SOS] \tHuffman DC table id: %d\n", jpg_param->sos->channels[i].huffman_table_dc_id);
        printf("[SOS] \tHuffman AC table id: %d\n", jpg_param->sos->channels[i].huffman_table_ac_id);
        printf("\n");
    }

}

int jpg_codec_jpg_param_remove(jpg_file_params_t *jpg_param)
{
    return 0;
}

int jpg_codec_file_decode(FILE *jpg_file, void **out_pixel_array)
{
    jpg_file_params_t jpg_param = {};

    if (!jpg_file || !out_pixel_array){
        return -1;
    }
    
    uint16_t l_begin_marker = getc(jpg_file) << 8;
    l_begin_marker |= getc(jpg_file);

    if (l_begin_marker != 0xFFD8){
        return -2;
    }
  
    uint16_t l_section_marker = 0;
    // parse jpg file
    for (;;){
        if(l_section_marker != CHUNK_TYPE_SOS){
            l_section_marker = getc(jpg_file) << 8;
            l_section_marker |= getc(jpg_file);

            if(l_section_marker == 0xFFD9 || feof(jpg_file)){
                jpg_codec_jpg_param_remove(&jpg_param);
                return -6;
            }

            uint16_t l_section_size = getc(jpg_file) << 8;
            l_section_size |= getc(jpg_file);

            if (!l_section_size)
                continue;

            l_section_size -= sizeof(l_section_size);
            uint8_t *l_data = calloc(l_section_size, sizeof(uint8_t));
            int l_size = fread (l_data, sizeof(char), l_section_size, jpg_file);

            for(int i = 0; i < sizeof(s_block_handlers)/sizeof(handlers_table_item_t); i++){
                if (s_block_handlers[i].type == l_section_marker && s_block_handlers[i].handler){
                    s_block_handlers[i].handler(&jpg_param, l_data, l_section_size);
                    break;
                }
            }

            if (l_data)
                free(l_data);
        } else  {
            // let's compute remain data size
            // save current position
            unsigned long position = ftell(jpg_file);
            // go to end
            fseek(jpg_file, 0, SEEK_END);
            // save end position
            unsigned long end_position = ftell(jpg_file);
            // go to old position
            fseek(jpg_file, position, SEEK_SET);

            size_t remain_size = end_position - position;
            uint8_t* b_encoded_data = malloc(remain_size);

            int l_size = fread (b_encoded_data, sizeof(uint8_t), remain_size, jpg_file);
            for (int i = 0; i < remain_size - 1; i++){
                if (b_encoded_data[i+1] == 0xD9 && b_encoded_data[i] == 0xFF){
                    remain_size = (i - 1) * sizeof(uint8_t);
                    b_encoded_data = realloc(b_encoded_data, remain_size);
                    jpg_param.encoded_data = b_encoded_data;
                    jpg_param.encoded_data_size = remain_size;
                    break;
                } 
            }

            


            if (!jpg_param.encoded_data){
                free(b_encoded_data);
                jpg_codec_jpg_param_remove(&jpg_param);
                return -7;
            } else 
                break;
            
        }
    }

    jpg_codec_file_dump(&jpg_param);
    jpg_decoding_params_t jpg_decode_params = {};
    jpg_decode_params.dqt_tables_cnt = jpg_param.dqt_tables_cnt;
    jpg_decode_params.dqt_tables = calloc(sizeof(uint16_t**), jpg_param.dqt_tables_cnt);
    for (int i = 0; i < jpg_param.dqt_tables_cnt; i++){
        uint16_t **dqt_table = NULL;
        jpeg_codec_zigzag_to_matrix(jpg_param.dqt_param[i]->header.tbl_value_size, &dqt_table, jpg_param.dqt_param[i]->table, 64);
        jpg_decode_params.dqt_tables[i] = dqt_table;
    }

    jpg_decode_params.huffman_trees = calloc(sizeof(huffman_tree_t*), jpg_param.dht_cnt);
    jpg_decode_params.huffman_trees_cnt = jpg_param.dht_cnt;
    for(int i = 0; i < jpg_param.dht_cnt; i++)
        jpg_decode_params.huffman_trees[i] = huffman_tree_create(jpg_param.dht[i]->header.table_class, 
                                            jpg_param.dht[i]->header.table_id,
                                            jpg_param.dht[i]->header.codes_cnts_by_length, 
                                            jpg_param.dht[i]->codes_value, 
                                            jpg_param.dht[i]->header.codes_value_cnt);
    
    size_t sos_size = sizeof(uint8_t) + jpg_param.sos->channel_cnt * sizeof(jpg_sos_channel_t);
    jpg_decode_params.sos = calloc(1, sos_size);
    memcpy(jpg_decode_params.sos, jpg_param.sos, sos_size);

    size_t sof0_size = sizeof(jpg_param.sof0->header) + jpg_param.sof0->header.channel_cnt * sizeof(jpg_sof0_channel);
    jpg_decode_params.sof0 = calloc(1, sof0_size);
    memcpy(jpg_decode_params.sof0, jpg_param.sof0, sof0_size);

    jpg_decode_params.encoded_data = calloc(jpg_decode_params.encoded_data_size, sizeof(uint8_t));
    jpg_decode_params.encoded_data_size = jpg_param.encoded_data_size;
    memcpy(jpg_decode_params.encoded_data, jpg_param.encoded_data, jpg_decode_params.encoded_data_size);
    
    jpg_codec_jpg_param_remove(&jpg_param);



    return 0;
}


static int chunk_handler_dqt(jpg_file_params_t *jpg_param, void* data, uint16_t data_size)
{
    if (!data_size || !data || !jpg_param)
        return -1;

    jpg_dqt_t *l_buf_dqt = calloc(data_size - sizeof(uint8_t) + sizeof(l_buf_dqt->header), sizeof(uint8_t));

    uint8_t l_buf_params = *(uint8_t*)data;
    l_buf_dqt->header.tbl_value_size = (int)((l_buf_params & 0xF0) >> 4) + 1;
    l_buf_dqt->header.tbl_id = (int)(l_buf_params & 0x0F);

    memcpy(l_buf_dqt->table, data + sizeof(uint8_t), data_size - sizeof(uint8_t));

    jpg_param->dqt_tables_cnt++;
    jpg_param->dqt_param = (jpg_dqt_t **)realloc(jpg_param->dqt_param, jpg_param->dqt_tables_cnt * sizeof(jpg_dqt_t *));
    jpg_param->dqt_param[jpg_param->dqt_tables_cnt - 1] = l_buf_dqt;

    return 0;
}


static int chunk_handler_sof0(jpg_file_params_t *jpg_param, void* data, uint16_t data_size)
{
    if (!data_size || !data || !jpg_param)
        return -1;

    void* data_pointer = data;

    uint8_t precision = *(uint8_t*)data_pointer;
    uint16_t height = ((*(uint8_t*)(++data_pointer)) << 8 | (*(uint8_t*)(++data_pointer)));
    uint16_t width = ((*(uint8_t*)(++data_pointer)) << 8 | (*(uint8_t*)(++data_pointer)));
    uint8_t chanel_cnt = *(uint8_t*)(++data_pointer);

    jpg_sof0_t *l_buf_sof = calloc(sizeof(l_buf_sof->header) + chanel_cnt*sizeof(jpg_sof0_channel), sizeof(uint8_t));
    l_buf_sof->header.precision = precision;
    l_buf_sof->header.height = height;
    l_buf_sof->header.width = width;
    l_buf_sof->header.channel_cnt = chanel_cnt;

    for (int i = 0; i < chanel_cnt; i++){
        l_buf_sof->channels[i].id = *(uint8_t*)++data_pointer;
        l_buf_sof->channels[i].h_thinning = ((*(uint8_t*)++data_pointer & 0xF0) >> 4);
        l_buf_sof->channels[i].v_thinning = *(uint8_t*)data_pointer & 0x0F;
        l_buf_sof->channels[i].dqt_id = (*(uint8_t*)++data_pointer);
    }

    jpg_param->sof0 = l_buf_sof;

    return 0;
}

static int chunk_handler_dht(jpg_file_params_t *jpg_param, void* data, uint16_t data_size)
{
    if (!data_size || !data || !jpg_param)
        return -1;

    void* data_pointer = data;

    jpg_dht_t *l_buf_dht = calloc(1, sizeof(jpg_dht_t));

    l_buf_dht->header.table_class = (*(uint8_t*)data_pointer & 0xF0) >> 4;
    l_buf_dht->header.table_id = *(uint8_t*)data_pointer & 0x0F;
    data_pointer++;
    memcpy(l_buf_dht->header.codes_cnts_by_length, data_pointer, 16);
    data_pointer += 16;

    for(int i = 0; i < 16; i++){
        l_buf_dht->header.codes_value_cnt += l_buf_dht->header.codes_cnts_by_length[i];
    }

    l_buf_dht = (jpg_dht_t*)realloc(l_buf_dht, sizeof(l_buf_dht->header) + l_buf_dht->header.codes_value_cnt * sizeof(uint8_t));
    memcpy(l_buf_dht->codes_value, data_pointer, l_buf_dht->header.codes_value_cnt);

    jpg_param->dht_cnt++;
    jpg_param->dht = (jpg_dht_t **)realloc(jpg_param->dht, jpg_param->dht_cnt * sizeof(jpg_dht_t *));
    jpg_param->dht[jpg_param->dht_cnt - 1] = l_buf_dht;

    return 0;
}

static int chunk_handler_sos(jpg_file_params_t *jpg_param, void* data, uint16_t data_size)
{
    if (!data_size || !data || !jpg_param)
        return -1;

    void* data_pointer = data;

    uint8_t l_channels_cnt = *(uint8_t*)data_pointer;
    jpg_sos_t *l_buf_sos = calloc(1, sizeof(l_buf_sos->channel_cnt) + l_channels_cnt*sizeof(jpg_sos_channel_t));
    l_buf_sos->channel_cnt = l_channels_cnt;
    data_pointer++;
    for(int i = 0; i < l_channels_cnt; i++){
        l_buf_sos->channels[i].channel_id = *(uint8_t*)data_pointer;
        data_pointer++;
        l_buf_sos->channels[i].huffman_table_dc_id = (*(uint8_t*)data_pointer & 0xF0) >> 4;
        l_buf_sos->channels[i].huffman_table_ac_id = *(uint8_t*)data_pointer & 0x0F;
        data_pointer++;
    }

    jpg_param->sos = l_buf_sos;
    return 0;
}

static int chunk_handler_comment(jpg_file_params_t *jpg_param, void* data, uint16_t data_size)
{
    if (!data_size || !data || !jpg_param)
        return -1;

    char *l_comment = calloc(data_size + 1, sizeof(char));
    memcpy(l_comment, data, data_size);

    printf("Comment: %s\n", (char*)l_comment);
    free(l_comment);

    return 0;
}

static int decode_data_flow(jpg_decoding_params_t *decoding_param, uint8_t*** zigzag_matrix)
{
    uint8_t** l_zigzag_matrix = NULL;

    uint8_t b_zigzag_array = calloc(64, sizeof(uint8_t));
    uint8_t b_zigzag_pos = 0;


    huffman_tree_t *huffman_tree_current = decoding_param->huffman_trees;
    for (size_t i = 0; i < decoding_param->encoded_data_size; i++){
        for (int i = 0; i < 8; i++){

            // if (decoding_param->encoded_data[i] & (0x01 << i))
            
        }
    }


    return 0;
}