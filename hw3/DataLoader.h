#ifndef DATALOADER_H
#define DATALOADER_H

#include "config.h"
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

    std::string extractContent(std::string org, std::string bstr, std::string estr);
    std::string getFirstLine(std::string);
    uint32_t calcWordFreq(std::string, uint32_t); // calc (word,Freq) in TEXT
    void mergeIndex(uint32_t, uint32_t);
    
    double BM25_t_q(std::string term, uint32_t docID, uint32_t freq);
    uint32_t getFreq(std::string term, uint32_t docID);
    void openList(uint32_t,uint32_t&,std::vector<uint32_t>&,std::vector<uint32_t>&,std::vector<uint32_t>&);
    void TAATQuery(std::vector<std::string> word_list, int type);
    std::vector<std::string> splitQuery(std::string);
    void decodeBlocks(std::string,std::vector<double>&);
    void decodeBlock(std::string,uint32_t&,std::vector<double>&);
    std::vector<uint32_t> decodeChunk(uint32_t,uint32_t);
    void findTopKscores(std::vector<double>&,int);
    void updateScoreHash(std::string,std::map<uint32_t, double>&, bool);
    void decodeBlocks(std::string,std::map<uint32_t, double>&, bool);
    void decodeBlock(std::string,uint32_t&,std::map<uint32_t, double>&, bool);
    void findTopKscores(std::map<uint32_t, double>&,int);
    uint32_t calcMetaSize(uint32_t,std::vector<uint32_t>&,std::vector<uint32_t>&,std::vector<uint32_t>&);
    void updateSnippets(std::vector<std::string>, std::vector<uint32_t>);
    std::string findSnippets(uint32_t, std::vector<std::string>,  std::vector<uint32_t>);
    
public:
    DocTable _DocTable;
    InvertedIndex _InvertedIndex;
    Lexicon _Lexicon;

    DataLoader();
    ~DataLoader();
    void ReadData(const char *filepath);
    void mergeIndexToOne();
    void BuildLexicon();
    void WriteDocTable();
    void WriteLexicon();
    void QueryLoop();
    void TestQuery();
    void TestSnippets(std::string);
    
};

#endif