#include "config.h"
#include <filesystem>

class InvertedIndex
{
public:
uint32_t allIndexSize;
std::map<std::string,std::vector<std::pair<uint32_t,uint32_t>>> HashWord;
uint32_t indexFileNum; //record write file num
void Insert(std::string,uint32_t,uint32_t); 
void Read(std::string);
std::string getIndexFilePath();
InvertedIndex();
~InvertedIndex();

private:
bool creatIndexFolder();
bool clearIndexFolder();
void countIndexFileNum();
void Write();
void Clear();
};