#include "binaryfileof.h"

void BinaryFileOf::PutBit(bool bit){
    next_bit=next_bit<<1;
    next_bit+=bit;
    --next_bit_remaining;
    if(not next_bit_remaining){
        outstream.put(next_bit);
        next_bit=0;
        next_bit_remaining=8;
    }
            
}

BinaryFileOf::BinaryFileOf(const std::string& filename){
    outstream.open(filename, std::ios_base::binary);
    next_bit=0;
    next_bit_remaining=8;
}

void BinaryFileOf::put(uint8_t char_f){
    if(next_bit_remaining==8){
        outstream.put(char_f);
    }else{
        next_bit=next_bit<<next_bit_remaining;
        outstream.put(next_bit);
        next_bit=0;
        next_bit_remaining=8;
        outstream.put(char_f);
    }
    
}

void BinaryFileOf::close(){
    if(next_bit_remaining==8){
        outstream.close();
    }else{
        next_bit=next_bit<<next_bit_remaining;
        outstream.put(next_bit);
        next_bit=0;
        next_bit_remaining=8;
        outstream.close();
    }
}
