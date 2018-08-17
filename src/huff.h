#ifndef HUFF
#define HUFF

#include <vector>


typedef uint8_t huff_data;
const huff_data huff_null=-1;
typedef int index;


struct CodedFile{
    uint8_t mode;
    uint8_t order;
    index x_size,y_size;
    std::vector<huff_data> data;
    std::vector<huff_data> coefs;
    bool isRLE();
    bool isDelta();
    
};


CodedFile ReadData(std::string & filename);
void WriteData(CodedFile encoded_file, std::string & filename);










#endif
