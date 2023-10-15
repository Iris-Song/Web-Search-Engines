#include "config.h"
#include <filesystem>

class InvertedIndex
{
public:
uint32_t allIndexSize;
std::map<std::string,std::vector<std::pair<uint32_t,uint32_t>>> HashWord;
uint32_t indexFileNum; //record write file num 

InvertedIndex();
~InvertedIndex();
std::string getIndexFilePath();
std::string getIndexFilePath(uint32_t);
void Insert(std::string,uint32_t,uint32_t); 
void Clear();
void Write();

private:
bool creatIndexFolder();
bool clearIndexFolder(bool);
void countIndexFileNum();

};