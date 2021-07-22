#define INPUT_FILEPATH    "./input/input_original.txt"
#define OUTPUT_FILEPATH    "./output/output-in-bin.txt"


#define entwidth 32
#define entheight 32
#define depth  1

#define npixel (entheight * (entwidth + 1))

#define nchar (npixel + 3)
#define nbit (nchar * 8)

#define density  300
#define whitepix  0

#define ndigit 1934

#ifdef tiffout
// TIFF HEADER CODES
    #define HEADER_ENDIAN_LITTLE            0x4949
    #define HEADER_ENDIAN_BIG               0x4D4D
    #define HEADER_MAGIC_NUMBER             0x002A

// TIFF TAG ID CODES
    #define TAG_WIDTH                       0x0100 
    #define TAG_HEIGHT                      0x0101
    #define TAG_BITS_PER_SAMPLE             0x0102
    #define TAG_COMPRESSION                 0x0103
    #define TAG_INTERPRETATION              0x0106
    #define TAG_STRIP_OFFSETS               0x0111
    #define TAG_ROWS_PER_STRIP              0x0116
    #define TAG_STRIP_BYTE_COUNTS           0x0117
    #define TAG_X_RESOLUTION                0x011a
    #define TAG_Y_RESOLUTION                0x011b
    #define TAG_RESOLUTION_UNIT             0x0153

    #define NUM_TAGS                            11
    #define TIFF_END                    0x00000000
#endif

//ntot = 1934
//nlower = 0
//nupper = 0
//nparag = 0
//firstdigit = 0
//lastdigit = 0
//firstlower = 0
//lastlower = 0
//firstupper = 0
//lastupper = 0
//firstparag = 0
//lastparag = 0