#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "huff.h"

typedef int index;

namespace modes{
    const int row_major=0;
    const int column_major=1;
    const int row_major_scan_1=8;
    const int row_major_scan_2=9;
    const int row_major_scan_3=10; //not yet implemented
    const int row_major_scan_4=11; //        ""
    const int column_major_scan_1=12;
    const int column_major_scan_2=13;
    const int column_major_scan_3=14; //not yet implemented
    const int column_major_scan_4=15; //        ""
    const int diag_uR_1=16;
    const int zig_zag_1=40;
    const int zig_zag_2=41;
}

class ImageOrderIterator{
    
    private:
        index x,y,x_size,y_size,i;
        uint8_t mode;
        bool valid=true;
        bool toggle;
        bool IsTop(){ return y==0; }
        bool IsBottom(){ return y==(y_size-1);}
        bool IsLeft(){return x==0;}
        bool IsRight(){return x==(x_size-1);}
        
        
        void next_row_major(){
            ++i;
            x=i%x_size;
            y=i/x_size;
        }
        void next_col_major(){
            ++i;
            x=i/y_size;
            y=i%y_size;
        }
        
        
        bool & zig_zag_1_dir=toggle; 
        const bool DL=false;
        const bool UR=true;
        
        void nextZigZag1(){
            if(IsBottom() and IsRight()) valid=false;
            if(zig_zag_1_dir==DL){
                if(IsBottom()){
                    ++x;
                    zig_zag_1_dir=UR;
                }else if(IsLeft()){
                    ++y;
                    zig_zag_1_dir=UR;
                }else{
                    --x;
                    ++y;
                }
            }else{
                if(IsRight()){
                    ++y;
                    zig_zag_1_dir=DL;
                }else if(IsTop()){
                    ++x;
                    zig_zag_1_dir=DL;
                }else{
                    ++x;
                    --y;
                }
                
            }
            
        }
        
        const bool L=false;
        const bool R=true;
        const bool D=false;
        const bool U=true;
        bool & scan_dir=toggle;
        
        void nextRowMajorScan1(){
            if(scan_dir==R){
                if(IsRight()){
                    ++y;
                    scan_dir=L;
                }else{
                    ++x;
                }
            }else{
                if(IsLeft()){
                    ++y;
                    scan_dir=R;
                }else{
                    --x;
                }
                
            }
            
        }
        
        void nextColMajorScan1(){
            if(scan_dir==U){
                if(IsTop()){
                    ++x;
                    scan_dir=D;
                }else{
                    --y;
                }
            }else{
                if(IsBottom()){
                    ++x;
                    scan_dir=U;
                }else{
                    ++y;
                }
                
            }
            
        }
        
        bool & diag_on_x=toggle;
        index past_init_pos;
        void nextDiagUR1(){
            if(not diag_on_x){
                if(IsTop() or IsRight()){
                    x=0;
                    ++past_init_pos;
                    y=past_init_pos;
                    
                    if(y==y_size){
                        --y;
                        ++x;
                        diag_on_x=true;
                        past_init_pos=1;
                    }
                }else{
                    ++x;
                    --y;
                }
            }else{
                if(IsTop() or IsRight()){
                    y=y_size-1;
                    ++past_init_pos;
                    x=past_init_pos;
                
                }else{
                    ++x;
                    --y;
                }
                
            }
        }
        
    public:
        ImageOrderIterator(index y_size_s, index x_size_s, uint8_t mode_s){
            x_size=x_size_s;
            y_size=y_size_s;
            mode=mode_s;
            
            switch (mode){
                
                case modes::row_major://row major
                    x=0;y=0;i=0;
                    break;
                    
                case modes::column_major: //col major
                    x=0;y=0;i=0;
                    break;
                
                case modes::zig_zag_1:
                    x=0;y=0;zig_zag_1_dir=DL;
                    break;
                case modes::zig_zag_2:
                    x=0;y=0;zig_zag_1_dir=UR;
                    break;
                
                case modes::row_major_scan_1:
                    x=0;y=0;scan_dir=R;
                    break;
                    
                case modes::row_major_scan_2:
                    x=x_size-1;y=0;scan_dir=L;
                    break;
                
                case modes::column_major_scan_1:
                    x=0;y=0;scan_dir=D;
                    break;
                
                case modes::column_major_scan_2:
                    x=0;y=y_size-1;scan_dir=U;
                    break;
                
                case modes::diag_uR_1:
                    x=0;y=0;
                    past_init_pos=0;
                    diag_on_x=false;
                    break;
                    
                default:
                    throw("Mode not implemented on init");
                
            }
            
        }
        
        index num(){
            if(valid) return x_size*y+x;
            else throw("Tried to access non existent iteration");
            
        }
        
        void next(){
            switch (mode){
                case modes::row_major:
                    next_row_major();
                    break;
                    
                case modes::column_major:
                    next_col_major();
                    break;
                
                case modes::zig_zag_1:
                case modes::zig_zag_2:
                    nextZigZag1();
                    break;
                
                case modes::row_major_scan_1:
                case modes::row_major_scan_2:
                    nextRowMajorScan1();
                    break;
                    
                case modes::column_major_scan_1:
                case modes::column_major_scan_2:
                    nextColMajorScan1();
                    break;
                    
                case modes::diag_uR_1:
                    nextDiagUR1();
                    break;
                
                default:
                    throw("Mode not implemented on next");
                
            }
            
        }
    
    
};

struct picture{
    std::vector<uint8_t> bitmap;
    index x_size, y_size;
    inline const index size(){
        return x_size*y_size;
    }
};

picture ImageToReorder(picture & image, uint8_t mode){
    picture reordered_image;
    reordered_image.bitmap.resize(image.size());
    reordered_image.x_size=image.x_size;
    reordered_image.y_size=image.y_size;
    ImageOrderIterator iterador(image.y_size,image.x_size,mode);
    for(int i=0;i<image.size();++i){
        reordered_image.bitmap[i]=image.bitmap[iterador.num()];
        iterador.next();
    }
    return reordered_image;
}

picture ReorderToImage(picture & reordered_image, uint8_t mode){
    picture image;
    image.bitmap.resize(reordered_image.size());
    image.x_size=reordered_image.x_size;
    image.y_size=reordered_image.y_size;
    ImageOrderIterator iterador(reordered_image.y_size,reordered_image.x_size,mode);
    for(int i=0;i<(reordered_image.size());++i){
        image.bitmap[iterador.num()]=reordered_image.bitmap[i];
        iterador.next();
    }
    return image;
}

picture FileToPicture(std::string filename){
    std::ifstream infile(filename,std::ios_base::binary);
    if(infile.get()!='P') throw("File is not a Netpbm bitmap");
    if(infile.get()!='5') throw("File is not greyscale");
    picture image;
    int color_depth;
    infile>>image.x_size>>image.y_size>>color_depth;
    infile.get(); //clear newline
    
    if(color_depth!=255) throw ("Wrong colordepth");
    image.bitmap.resize(image.x_size*image.y_size);
    for(int i=0;i<image.x_size*image.y_size;++i){
        image.bitmap[i]=infile.get();
    }
    infile.close();
    return image;
}

void PictureToFile(std::string filename, picture image){
    std::ofstream outfile(filename,std::ios_base::binary);
    outfile<<"P5"<<std::endl;
    outfile<<image.x_size<<' ';
    outfile<<image.y_size<<std::endl;
    outfile<<(int)255<<std::endl;
    for(int i=0;i<image.x_size*image.y_size;++i){
        outfile.put(image.bitmap[i]);
    }
    outfile.close();
}

void RLE_Encode(const std::vector<uint8_t> & bitmap,std::vector<huff_data> & data,std::vector<huff_data> & coefs){
    index max_pos=bitmap.size();
    index i=0;
    while(i<max_pos){
        huff_data count =0;
        huff_data value=bitmap[i];
        ++i;
        
        if (i != max_pos) while(count <255 and i<max_pos and bitmap[i]==value){
            ++count;
            ++i;
        }
                
        coefs.push_back(count);
        data.push_back(value);
    }
}

std::vector<uint8_t> RLE_Decode(const std::vector<huff_data> & data,const std::vector<huff_data> & coefs,index imagesize){
    std::vector<uint8_t> answer;
    index i=0;
    if(coefs.size()!=data.size()){
        throw("Bad RLE Encoded");
    }
    int m=0;
    answer.resize(imagesize);
    while(i<data.size()){
        index j=coefs[i]+1;
        for(index k=0;k<j;++k){
            answer[m]=data[i];
            ++m;
        }
        ++i;
        
    }
    return answer;
}

std::vector<uint8_t> DeltaEncode(const std::vector<uint8_t> & bitmap){
    uint8_t prev_pixel=0;
    std::vector<uint8_t> answer(bitmap.size());
    for(int i=0;i<bitmap.size();++i){
        answer[i]=bitmap[i]-prev_pixel;
        prev_pixel=bitmap[i];
    }
    return answer;
}

std::vector<uint8_t> DeltaDecode(const std::vector<uint8_t> & bitmap){
    uint8_t prev_pixel=0;
    std::vector<uint8_t> answer(bitmap.size());
    for(int i=0;i<bitmap.size();++i){
        answer[i]=bitmap[i]+prev_pixel;
        prev_pixel=answer[i];
    }
    return answer;
}


CodedFile Encode(picture image,uint8_t mode,uint8_t order){
    CodedFile answer;
    answer.mode=mode;
    answer.order=order;
    answer.x_size=image.x_size;
    answer.y_size=image.y_size;
    picture reordered_image=ImageToReorder(image,order);
    if(answer.isDelta()){
        image.bitmap=DeltaEncode(reordered_image.bitmap);
    }else{
        image=reordered_image;
    }
    if(answer.isRLE()){
        RLE_Encode(image.bitmap,answer.data,answer.coefs);
    }else{
        answer.data=image.bitmap;
    }
    return answer;
}

picture Decode(CodedFile coded_file){
    picture answer;
    answer.x_size=coded_file.x_size;
    answer.y_size=coded_file.y_size;
    std::vector<huff_data> temp;
    if(coded_file.isRLE()){
        temp=RLE_Decode(coded_file.data,coded_file.coefs,answer.size());
    }else{
        temp=coded_file.data;
    }
    if(coded_file.isDelta()){
        answer.bitmap=DeltaDecode(temp);
    }else{
        answer.bitmap=temp;
    }
    return ReorderToImage(answer,coded_file.order);
    
}


int main(int argc, char *argv[]){
    static const char *optString = "edRDH:p:";
    try{
        bool encode=false;
        if(argc>=3){
            int opt = getopt( argc, argv, optString );
            uint8_t mode=0x00;
            uint8_t order=0x00;
            uint8_t huffman=0x00;
            bool RLE=false;
            bool Delta=false;
            while(opt!=-1){
                switch(opt){
                    case 'e':
                        encode=true;
                        break;
                    case 'd':
                        break;
                    case 'R':
                        RLE=true;
                        break;
                    case 'D':
                        Delta=true;
                        break;
                    case 'H':
                        huffman=atoi(optarg);
                        break;
                    case 'p':
                        order=atoi(optarg);
                        
                }
                
                opt = getopt( argc, argv, optString );   
            }
            if(RLE) mode |= 0x02;
            if(Delta) mode |= 0x01;
            mode |= huffman<<6;
            
            if(optind==(argc-2)){
                std::string in_filename=argv[optind];
                std::string out_filename=argv[optind+1];
                if(encode){ 
                    CodedFile c_file=Encode(FileToPicture(in_filename),mode,order);
                    WriteData(c_file,out_filename);
                }else{
                    PictureToFile(out_filename,Decode(ReadData(in_filename)));
                }
            }else{
                std::cout<<"Usage: program -[e|d] -(D|R|H)(h_num)input_file output_file"<<std::endl;
            }
            
            
        }else{
            std::cout<<"Usage: program -[e|d] -(D|R|H)(h_num)input_file output_file"<<std::endl;
            
        }
    }catch(char const * msg){
        std::cerr << msg << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
