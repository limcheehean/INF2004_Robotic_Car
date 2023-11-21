#ifndef BARCODE_INTERPRET_
#define BARCODE_INTERPRET_

#ifndef BARCODE_BUFFER_SIZE 
#define BARCODE_BUFFER_SIZE 9
#endif

#define BC_START_STOP 363 //0b101101011;

#define BC_A 246 //0b011110110
#define BC_B 438 //0b110110110

typedef struct{
    int key;
    char value;
} KeyValuePair ;

char get_value_for_key(KeyValuePair dict[], int dictSize, int key) {
    for (int i = 0; i < dictSize; i++) {
        if ( (dict[i].key & key) == dict[i].key) {
            return dict[i].value;
        }
    }
    // Return a default value or handle not found case accordingly
    return ' ';
}

char get_barcode_char(int key){
    static KeyValuePair barcode_dict[] = {
        {246, 'A'}, //0b011110110
        {438, 'B'}, //0b110110110
        {183, 'C'}, //0b010110111
        {486, 'D'}, //0b111100110
        {231, 'E'}, //0b011100111
        {423, 'F'}, //0b110100111
        {498, 'G'}, //0b111110010
        {243, 'H'}, //0b011110011
        {435, 'I'}, //0b110110011
        {483, 'J'}, //0b111100011
        {252, 'K'}, //0b011111100
        {444, 'L'}, //0b110111100
        {189, 'M'}, //0b010111101
        {492, 'N'}, //0b111101100
        {237, 'O'}, //0b011101101
        {429, 'P'}, //0b110101101
        {504, 'Q'}, //0b111111000
        {249, 'R'}, //0b011111001
        {441, 'S'}, //0b110111001
        {489, 'T'}, //0b111101001
        {126, 'U'}, //0b001111110
        {318, 'V'}, //0b100111110
        {63, 'W'}, //0b000111111
        {366, 'X'}, //0b101101110
        {111, 'Y'}, //0b001101111
        {303, 'Z'}  //0b100101111
    };
    return get_value_for_key(barcode_dict,26,key);
}



#endif
