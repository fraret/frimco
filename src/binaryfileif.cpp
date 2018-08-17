#include "binaryfileif.h"


BinaryFileIf::BinaryFileIf(const std::string& filename){
    instream.open(filename, std::ios_base::binary);
    next_bit=0;
    next_bit_remaining=0;
}

bool BinaryFileIf::GetBit(){
    if(next_bit_remaining==0){
        next_bit=instream.get();
        next_bit_remaining=8;
    }
    bool result= (next_bit & 0x80)>>7;
    next_bit=next_bit<<1;
    --next_bit_remaining;
    return result;
}

uint8_t BinaryFileIf::get(){
    next_bit_remaining=0;
    next_bit=0;
    return instream.get();
}


void BinaryFileIf::close(){
    instream.close();
}

index BinaryFileIf::GetTextIndex(){
    next_bit_remaining=0;
    next_bit=0;
    index i;
    instream>>i;
    instream.get();
    return i;
}
