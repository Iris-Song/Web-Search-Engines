#include "config.h"

class LexiconItem
{
public:
    uint32_t beginp; // begin offset
    uint32_t endp;   // end offset
    uint32_t docNum;
    void update(uint32_t, uint32_t, uint32_t);
};

class Lexicon
{
private:
    std::string LexiconPath;
    uint32_t calcDocNum(std::string); //calc Doc Num
    uint32_t WriteBlocks(std::string,std::ofstream &);
    

public:
    std::map<std::string, LexiconItem> _lexiconList;
    std::string IndexPath;
    Lexicon();
    ~Lexicon();
    bool Insert(std::string, uint32_t, uint32_t, uint32_t);
    void Build(std::string);
    void Write();
    void LoadLexiconList();
    
};