#ifndef DATALOADER_H
#define DATALOADER_H

#include "config.h"
#include "zlib.h"
#include "DocTable.h"
#include "InvertedIndex.h"
#include "SortedPosting.h"
#include "Lexicon.h"

class DataLoader
{
private:
    /* data */
    std::string extractContent(std::string org, std::string bstr, std::string estr);
    std::string getFirstLine(std::string);
    uint32_t calcWordFreq(std::string, uint32_t); // calc (word,Freq) in TEXT
    void mergeIndex(uint32_t, uint32_t);

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
};

#endif