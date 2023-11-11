#ifndef DocTable_H
#define DocTable_H
#include "config.h"

class Document
{
public:
    uint32_t docID; // doc id
    std::string docNO; // doc no
    uint32_t dataLen; // the number of data file containing this page
    uint32_t wordnums; // number of words
    z_off_t gzp; //pointer in gzfile
    std::string url; // url
    void print();
};


// assign document IDs (doc IDs) to pages
class DocTable
{
private:
    /* data */
    void calcAvgDataLen();
    
public:
    uint32_t _totalDoc;
    uint32_t _avg_data_len;
    std::vector<Document> _DocTable;
    DocTable(/* args */);
    ~DocTable();
    void add(Document docIDitem);
    void Write();
    void print();
    void LoadDocTable();
};

#endif
