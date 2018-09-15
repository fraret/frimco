#ifndef BINARYFILEIF_H
#define BINARYFILEIF_H


#include <fstream>
#include <string>

typedef int index;

class BinaryFileIf{
    private:
        uint8_t next_bit=0;
        uint8_t next_bit_remaining=0;
        std::ifstream instream;
    
    public:
        bool GetBit();
        uint8_t get();
        BinaryFileIf(const std::string& filename);
        void close();
        index GetTextIndex();
};

#endif // BINARYFILEIF_H
