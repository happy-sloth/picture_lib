

#pragma pack(push, 1)
typedef struct jpg_comment {
    uint16_t size;
    char data[];
}jpg_comment_t;

typedef struct jpg_dqt {
    struct {
        int tbl_value_size;
        int tbl_id;
    } header;
    uint8_t table[];
}jpg_dqt_t;

typedef struct {

} dqt_table;

typedef struct jpg_sof0_channel{
    int id;
    int h_thinning;
    int v_thinning;
    int dqt_id;
}jpg_sof0_channel;

typedef struct jpg_sof0 {
    struct{
        uint8_t precision;
        uint16_t height;
        uint16_t width;
        uint8_t chanel_cnt;
    } header;
    jpg_sof0_channel channels[];
}jpg_sof0_t;

typedef struct {
    struct {
        uint8_t table_class;
        uint8_t table_id;
        uint8_t codes_cnts_by_length[16];
        uint8_t codes_value_cnt;
    } header;
    uint8_t codes_value[];
} jpg_dht_t;

typedef struct {
    uint8_t channel_id;
    uint8_t huffman_table_dc_id;
    uint8_t huffman_table_ac_id;
} jpg_sos_channel_t;

typedef struct {
    uint8_t channel_cnt;
    jpg_sos_channel_t channels[];
} jpg_sos_t;

typedef struct {

} rgb_pixel;

typedef struct {

} YCbCr_pixel;


typedef struct jpg_file_params {
    int dqt_tables_cnt;
    jpg_dqt_t **dqt_param;
    jpg_sof0_t *sof0;
    int dht_cnt;
    jpg_dht_t **dht;
    jpg_sos_t *sos;
    uint8_t *encoded_data;
} jpg_file_params_t;

typedef struct jpg_decoding_params {
    int dqt_tables_cnt;
    uint16_t ***dqt_tables;
    jpg_sos_t *sos;
    
} jpg_decoding_params_t;
#pragma pack(pop)