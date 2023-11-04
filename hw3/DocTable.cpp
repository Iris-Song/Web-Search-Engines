#include "DocTable.h"
void Document::print()
{
    std::cout << "docID:" << docID << " docNO:" << docNO
              << " url:" << url << " dataLen:" << dataLen
              << " wordnums:" << wordnums << std::endl;
}

DocTable::DocTable(/* args */)
{
    _totalDoc = 0;
    _DocTable.empty();
}

DocTable::~DocTable()
{
}

void DocTable::add(Document docIDitem)
{
    _DocTable.push_back(docIDitem);
}

void DocTable::Write()
{
    std::ofstream outfile;
    std::string path;
    if (FILEMODE == FILEMODE_ASCII)
    {
        path = "ASCII_" + std::string(DOC_TABLE_PATH);
        outfile.open(path);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        path = "BIN_" + std::string(DOC_TABLE_PATH);
        outfile.open(path,std::ofstream::binary);
    }
    
    for (std::vector<Document>::iterator iter = _DocTable.begin();
         iter != _DocTable.end(); ++iter)
    {
        outfile << iter->docID << " " << iter->docNO << " " << iter->dataLen << " "
                << iter->wordnums << " " << iter->url << " " << std::endl;
    }
    outfile.close();
}

void DocTable::print()
{
    for (std::vector<Document>::iterator it = _DocTable.begin(); it != _DocTable.end(); ++it)
    {
        it->print();
    }
}

void DocTable::calcAvgDataLen()
{
    double allLen = 0;
    for (std::vector<Document>::iterator it = _DocTable.begin(); it != _DocTable.end(); ++it)
    {
        allLen+=it->dataLen;
    }
    _avg_data_len = allLen/_DocTable.size();
}

void DocTable::LoadDocTable()
{

    _DocTable.clear();
    
    std::ifstream infile;
    std::string path;
    if (FILEMODE == FILEMODE_ASCII)
    {
        path = "ASCII_" + std::string(DOC_TABLE_PATH);
        infile.open(path);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        path = "BIN_" + std::string(DOC_TABLE_PATH);
        infile.open(path, std::ifstream::binary);
    }
    if(!infile)
    {
        std::cout<<"can not read "<<path<<std::endl;
        exit(0);
    }

    _DocTable.clear();
    while(!infile.eof())
    {
        Document newdoc;
        infile>>newdoc.docID>>newdoc.docNO
        >>newdoc.dataLen>>newdoc.wordnums>>newdoc.url;
        _DocTable.push_back(newdoc);
    }
    _totalDoc = _DocTable.size();
    calcAvgDataLen();
}