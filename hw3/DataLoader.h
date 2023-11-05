#ifndef DATALOADER_H
#define DATALOADER_H

#include "config.h"
#include "zlib.h"
#include "DocTable.h"
#include "InvertedIndex.h"
#include "SortedPosting.h"
#include "Lexicon.h"
#include "Result.h"

class DataLoader
{
private:
    /* data */
    ResultList _resultList;
    int index_fd;

    std::string extractContent(std::string org, std::string bstr, std::string estr);
    std::string getFirstLine(std::string);
    uint32_t calcWordFreq(std::string, uint32_t); // calc (word,Freq) in TEXT
    void mergeIndex(uint32_t, uint32_t);
    double BM25_t_q(std::string term, uint32_t docID);
    double BM25_t_q(std::string term, uint32_t docID, uint32_t freq);
    uint32_t getFreq(std::string term, uint32_t docID);
    uint32_t nextGEQ(uint32_t pointer, uint32_t end,uint32_t k);
    void openList(uint32_t,uint32_t&,std::vector<uint32_t>&,std::vector<uint32_t>&,std::vector<uint32_t>&);
    void TAATQuery(std::vector<std::string> word_list, int type);
    std::vector<std::string> splitQuery(std::string);
    void decodeBlocks(std::string,double []);
    void decodeBlock(std::string,uint32_t,uint32_t,double []);
    std::vector<uint32_t> decodeChunk(uint32_t,uint32_t);
    void findTopKscores(double[],int);
    void updateScoreHash(std::string,std::map<uint32_t, double>&, bool);
    void decodeBlocks(std::string,std::map<uint32_t, double>&, bool);
    void decodeBlock(std::string,uint32_t&,std::map<uint32_t, double>&, bool);
    void findTopKscores(std::map<uint32_t, double>&,int);
    bool findDocID(uint32_t,uint32_t&,uint32_t&,uint32_t,uint32_t);
    
    
public:
    DocTable _DocTable;
    InvertedIndex _InvertedIndex;
    Lexicon _Lexicon;

    DataLoader();
    ~DataLoader();
    void read_data(const char *filepath);
    void mergeIndexToOne();
    void WriteDocTable();
    void WriteLexicon();
    void QueryLoop();
    void TestQuery();
    
};

#endif