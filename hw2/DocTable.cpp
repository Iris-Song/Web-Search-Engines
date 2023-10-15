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

void DocTable::write()
{
}

void DocTable::print()
{
    for(std::vector<Document>::iterator it=_DocTable.begin(); it!=_DocTable.end(); ++it)
    {
        it->print();
    }
}