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
    std::string url; // url
    void print();
};


// assign document IDs (doc IDs) to pages
class DocTable
{
private:
    /* data */
    uint32_t _totalDoc;
    std::vector<Document> _DocTable;
    
public:
    DocTable(/* args */);
    ~DocTable();
    void add(Document docIDitem);
    void write();
    void print();
};

#endif
