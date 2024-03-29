#include <queue>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "binaryfileof.h"
#include "binaryfileif.h"
#include "huff.h"


struct node{  //Definition of a hufmman tree node.
    node *left, *right;
    huff_data value;
    index freq,age; //Age is just a parameter to know the order in which the nodes were created to then order them if freq is equal. TODO: I think it is not used. See wheter I am right or not, 
    bool leaf;
    node(huff_data data,index freq_f, index age_f){ //initialization for leaf
        left=NULL;
        right=NULL;
        age=age_f;
        value=data;
        freq=freq_f;
        leaf=true;
    }
    node(node *left_f, node *right_f,index age_f){ //initalization for not-leaf node
        left=left_f;
        right=right_f;
        age=age_f;
        freq=left->freq+right->freq; //Huffman: frequency is the sum of freq of the children
        value=huff_null;
        leaf=false;
    }
    node(){ //initialization for null. TODO: Check wheter this is used and delete it/replace places where it is used if possible. 
        left=NULL;
        right=NULL;
        value=huff_null;
        leaf=false;
    }
    node(huff_data data){ //Initialization without freq. TODO: Chek where this is used and find a better way if possible.
        left=NULL;
        right=NULL;
        value=data;
        leaf=true;
    }
};


struct more_frequent{ // 2019-01-04: I believe this is a comparator for some sort of sorting function

    bool operator()(node* a, node* b){
        if((a->freq)>(b->freq)) return true;   
        else if((a->freq)==(b->freq)){
            if ((a->age)>(b->age)) return true;
            else return false;
        }else return false;
    }
};


bool CodedFile::isRLE(){
    return mode & 0x02;
}
bool CodedFile::isDelta(){
    return mode & 0x01;
}


std::vector<std::pair<huff_data,index>> DataToDataAndFreqs(const std::vector<huff_data> & data){
    std::vector<index> freqs(256*sizeof(huff_data),0);
    for(const huff_data & value:data){
        ++freqs[value];
    }
    std::vector<std::pair<huff_data,index>> answer;
    for(index i=0;i<freqs.size();++i){
        if(freqs[i]!=0){
            answer.push_back({i,freqs[i]});
        }
    }
    return answer;
}


node* CreateHuffTree(const std::vector<std::pair<huff_data,index>> & data_and_freqs){
    std::priority_queue<node*,std::vector<node*>,more_frequent> p_queue;
    for(auto & p: data_and_freqs){
        p_queue.push(new node(p.first,p.second,0));
    }
    index i=1;
    while(p_queue.size()>1){
        node* first=p_queue.top();
        p_queue.pop();
        node* second=p_queue.top();
        
        node* temp;
        if(first->age>second->age){
            temp=second;
            second=first;
            first=temp;
        }
        p_queue.pop();
        p_queue.push(new node(first,second,i));
        ++i;
    }
    
    return p_queue.top();
}

node* CreateHuffTree(const std::vector<huff_data> & vector){
    return CreateHuffTree(DataToDataAndFreqs(vector));
}

bool canon(std::pair<huff_data,huff_data> & a, std::pair<huff_data,huff_data> & b){
        if(a.second<b.second) return true;
        return false;
}

void increment(std::vector<bool> & vec){
    //2019-01-04: It seems that, given a vector of bools, it looks for the last false, makes it true and makes everything after it false. WHY?
    index last=vec.size()-1;
    while(vec[last]){
        vec[last]=false;
        --last;
    }
    vec[last]=true;
    
}

void HuffTreeToMap(node* tree, huff_data current_code_length, std::vector<std::pair<huff_data,huff_data>> & pairs){
    if(not (tree->leaf)){
        ++current_code_length;
        HuffTreeToMap(tree->left,current_code_length,pairs);
        HuffTreeToMap(tree->right,current_code_length,pairs);
        --current_code_length;
    }else{
        pairs.push_back({tree->value,current_code_length});
    }
    
}

std::pair<std::map<huff_data,std::vector<bool>>,std::vector<std::pair<huff_data,huff_data>>> HuffTreeToMap(node* tree){
    
    huff_data temp=0;
    std::vector<std::pair<huff_data,huff_data>> pairs;
    HuffTreeToMap(tree,temp,pairs);
    
    std::map<huff_data,std::vector<bool>> answer;
    std::vector<bool> nums;
    
    
    std::sort(pairs.begin(),pairs.end(),canon);
    
    
    
    for(auto & p:pairs){
        if(nums.size()) increment(nums);
        while(p.second>nums.size()) nums.push_back(0);
        answer[p.first]=nums;
        
        
    }
    return {answer,pairs};
}



std::vector<huff_data> MapToVector(const std::map<huff_data,std::vector<bool>> & map_f, const std::vector<std::pair<huff_data,huff_data>> & pairs){
    /* 
     * 
     * 
     */
    
    if(pairs.size()==1) throw ("Unary Huffman tree not supported yet");
    
    std::vector<huff_data> freq;
    huff_data max_seen_length=0;
    for(const std::pair<const huff_data,const huff_data> & p:pairs){
        const huff_data & length=p.second;
        while(max_seen_length<length){
            freq.push_back(0);
            ++max_seen_length;
        }
        ++freq[length-1];
    }
    
    
    for(auto & p:pairs){
        freq.push_back(p.first);
        
    }
    
    return freq;
    
}

std::vector<huff_data> MapToVector(std::pair<std::map<huff_data,std::vector<bool>>,std::vector<std::pair<huff_data,huff_data>>> & thing){
    return MapToVector(thing.first,thing.second);
    
}

huff_data ReadHuffNum(BinaryFileIf & infile){
    static_assert(sizeof(huff_data)==1, "Multi-byte huffman not yet implemented");
    return infile.get();
}

void WriteHuffNum(huff_data num, BinaryFileOf & outfile){
    static_assert(sizeof(huff_data)==1, "Multi-byte huffman not yet implemented");
    outfile.put(num);
    return;
}

void FillLayer(huff_data & nums, huff_data layer, node* tree, BinaryFileIf & infile){
    if(tree->leaf) return;
    if(layer==0){
        if(nums>=2){
            tree->left=new node(ReadHuffNum(infile));
            tree->right=new node(ReadHuffNum(infile));
            nums-=2;
        }else if (nums==1){
            tree->left=new node(ReadHuffNum(infile));
            tree->right=new node();
            --nums;
        }else{
            tree->left=new node();
            tree->right=new node();
        }
        
    }else{
        FillLayer(nums,layer-1,tree->left,infile);
        FillLayer(nums,layer-1,tree->right,infile);
    }
    
    
}

node* TreeFromFile(BinaryFileIf & infile){
    std::vector<huff_data> lengths;
    huff_data remaining_codes=2;
    huff_data data_num=0;
    while(remaining_codes){
        huff_data next_num=ReadHuffNum(infile);
        lengths.push_back(next_num);
        remaining_codes-=next_num;
        data_num+=next_num;
        remaining_codes*=2;
        
    }
    
    
    node* tree = new node();
    huff_data layer=0;
    for(auto next_num:lengths){
        FillLayer(next_num,layer,tree,infile);
        ++layer;
    }
    
    
    return tree;
}



void VectorToFile(const std::vector<huff_data> & vector_f,BinaryFileOf & outfile){
    for(const auto & num:vector_f){
        WriteHuffNum(num,outfile);
    }
}

void DataToFile(const std::vector<huff_data> & data, std::map<huff_data,std::vector<bool>> & huff_map, BinaryFileOf & outfile){
    for(const huff_data & value:data){
        const std::vector<bool> & code=huff_map[value];
        for(const bool & bit:code){
            outfile.PutBit(bit);
        }
    }
    
}

std::vector<huff_data> FileToData(BinaryFileIf & infile, node* tree,index num){
    std::vector<huff_data> answer(num);
    node* current=tree;
    index i=0;
    while(i<num){
        if(current->leaf){
            answer[i]=current->value;
            current=tree;
            ++i;
        }else{
            bool bit=infile.GetBit();
            if(bit){
                current=current->right;
            }else{
                current=current->left;
            }
        }
        
    }
    return answer;
}


index ReadIndex(BinaryFileIf & infile){
    static_assert(sizeof(index)==4,"Maximum symbols is fixed to uint32 at the moment");
    index datalength=0;
    datalength|= infile.get()<<24;
    datalength|= infile.get()<<16;
    datalength|= infile.get()<<8;
    datalength|= infile.get();
    return datalength;
}

void WriteIndex(index num, BinaryFileOf & outfile){
    static_assert(sizeof(index)==4,"Maximum symbols is fixed to uint32 at the moment");
    outfile.put(num>>24);
    outfile.put(num>>16);
    outfile.put(num>>8);
    outfile.put(num);
    
}


void DestroyTree(node* tree){
    if(tree->leaf){
        delete tree;
    }else{
        DestroyTree(tree->left);
        DestroyTree(tree->right);
        delete tree;
    }
}

CodedFile ReadData(std::string & filename){
    CodedFile answer;
    BinaryFileIf infile(filename);
    if(infile.get()!='E' and infile.get()!='5'){
        std::cerr<<"File has not the magic number"<<std::endl;
    }else infile.get();
    infile.get();
    answer.mode=infile.get();
    answer.order=infile.get();
    infile.get(); //0A
    answer.x_size=ReadIndex(infile);
    answer.y_size=ReadIndex(infile);
    
    node* tree=NULL;
    if((answer.mode & 0x80)){//Uses a single Huffman table or two
        if(infile.get()!=0xFE) throw ("No huffman table");
        tree =TreeFromFile(infile);
    }
    uint8_t temp=infile.get();
    if(temp!=0xFF){
        std::cout<<(int)temp<<std::endl;
        
        throw ("No data");
    }
    
    int datalength=ReadIndex(infile);
        
    if((answer.mode & 0x80)){//Uses a single Huffman table or two
        answer.data=FileToData(infile,tree,datalength);
    }else{
        answer.data.resize(datalength);
        for(index i=0;i<datalength;++i){
            answer.data[i]=infile.get();
        }
    }
    if(answer.isRLE()){
        if(answer.mode & 0x40){
            if(tree!=NULL) DestroyTree(tree);
            if(infile.get()!=0xFE) throw ("No second huffman table");
            tree =TreeFromFile(infile);
        }
        
        if(infile.get()!=0xFD){
            throw ("No runs");
        }
        int runlength=ReadIndex(infile);
        
        
        if((answer.mode & 0x80 or answer.mode & 0x40)){//Uses huffman
            answer.coefs=FileToData(infile,tree,runlength);
            DestroyTree(tree);
        }else{
            answer.coefs.resize(runlength);
            for(index i=0;i<runlength;++i){
                answer.coefs[i]=infile.get();
            }
        }
    }
    infile.close();
    return answer;
    
    
}

void WriteData(CodedFile encoded_file, std::string & filename){
    BinaryFileOf outfile(filename);
    outfile.put('E');outfile.put('5');outfile.put('\n');
    outfile.put(encoded_file.mode);
    outfile.put(encoded_file.order); outfile.put('\n');
    WriteIndex(encoded_file.x_size,outfile);
    WriteIndex(encoded_file.y_size,outfile);
    
    node* tree=NULL;
    std::pair<std::map<huff_data,std::vector<bool>>,std::vector<std::pair<huff_data,huff_data>>> thing;
    
    if((encoded_file.mode & 0x80)){//Uses a single Huffman table or two
        
        if(encoded_file.mode & 0x40){//two huff
            tree=CreateHuffTree(encoded_file.data);
        }else{
            std::vector<huff_data> quantify=encoded_file.data;
            quantify.insert(quantify.end(),encoded_file.coefs.begin(),encoded_file.coefs.end());
            tree=CreateHuffTree(quantify);
        }
        std::vector<bool> temp;
        thing=HuffTreeToMap(tree);
        
        outfile.put(0xFE);
        VectorToFile(MapToVector(thing),outfile);
    }
    outfile.put(0xFF);
    WriteIndex(encoded_file.data.size(),outfile);
    
    if((encoded_file.mode & 0x80)){//Uses a single Huffman table or two
        DataToFile(encoded_file.data,thing.first,outfile);
    }else{
        for(auto & el : encoded_file.data){
            outfile.put(el);
        }
    }
    
    if(encoded_file.isRLE()){
        if(encoded_file.mode & 0x40){ //If This has a second codification
            tree=CreateHuffTree(encoded_file.coefs);
            thing.first.clear();
            std::vector<bool> temp;
            thing=HuffTreeToMap(tree);
            outfile.put(0xFE);
            VectorToFile(MapToVector(thing),outfile);
        }
        outfile.put(0xFD);
        WriteIndex(encoded_file.coefs.size(),outfile);
        
        if((encoded_file.mode & 0x80) or (encoded_file.mode & 0x40)){//Uses huffman
            DataToFile(encoded_file.coefs,thing.first,outfile);
        }else{
            for(auto & el : encoded_file.coefs){
            outfile.put(el);
            }
        }
    }
    
    outfile.close();
    
    
    
}
