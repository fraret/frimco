#ifndef BINARYFILEOF_H
#define BINARYFILEOF_H



#include <fstream>
#include <string>

class BinaryFileOf{
    private:
        uint8_t next_bit=0;
        uint8_t next_bit_remaining=8;
        //as oldput;
        std::ofstream outstream;
    
    public:
        void PutBit(bool bit);
        void put(uint8_t char_f);
        BinaryFileOf(const std::string& filename);
        void close();
        
};

#endif // BINARYFILEOF_H
