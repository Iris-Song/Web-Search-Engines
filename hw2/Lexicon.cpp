#include "Lexicon.h"

void LexiconItem::update(uint32_t bgp, uint32_t edp, uint32_t dn)
{
    beginp = bgp;
    endp = edp;
    docNum = dn;
}

Lexicon::Lexicon()
{
    LexiconPath = (FILEMODE == FILEMODE_ASCII) ? "ASCII_" + std::string(LEXICON_PATH) : "BIN_" + std::string(LEXICON_PATH);
    IndexPath = (FILEMODE == FILEMODE_ASCII) ? "ASCII_" + std::string(FINAL_INDEX_PATH) : "BIN_" + std::string(FINAL_INDEX_PATH);
}

Lexicon::~Lexicon()
{
}

uint32_t Lexicon::calcDocNum(std::string idxWordList)
{
    uint32_t spaceNum = 0;
    for (size_t i = 0; i < idxWordList.length(); i++)
    {
        if (idxWordList[i] == ' ')
        {
            spaceNum += 1;
        }
    }
    return (spaceNum + 1) / 2;
}

bool Lexicon::Insert(std::string word, uint32_t beginp, uint32_t endp, uint32_t docNum)
{
    if (word.empty())
        return false;
    LexiconItem lexItem;
    lexItem.update(beginp, endp, docNum);
    _lexiconList[word] = lexItem;
    return true;
}

void Lexicon::Build(std::string path1, std::string path2)
{
    std::ifstream infile1;
    std::ifstream infile2;
    std::ofstream outfile;

    if (FILEMODE == FILEMODE_ASCII)
    {
        infile1.open(path1);
        infile2.open(path2);
        outfile.open(IndexPath);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        infile1.open(path1, std::ifstream::binary);
        infile2.open(path2, std::ifstream::binary);
        outfile.open(IndexPath, std::ofstream::binary);
    }

    std::string line1, line2;
    std::string word1, word2;
    uint32_t beginp, endp;

    while (infile1 && infile2)
    {
        if (word1.empty() && word2.empty())
        {
            beginp = outfile.tellp();
            std::getline(infile1, line1);
            std::getline(infile2, line2);
            word1 = line1.substr(0, line1.find(":"));
            word2 = line2.substr(0, line2.find(":"));
        }
        if (word1.compare(word2) == 0)
        {
            // merge word1 and word2
            std::string arr1 = line1.substr(line1.find(":") + 1);
            std::string arr2 = line2.substr(line2.find(":") + 1);
            std::string docID1 = arr1.substr(0, arr1.find(" "));
            std::string docID2 = arr2.substr(0, arr2.find(" "));
            if (docID1 < docID2)
            {
                outfile << word1 << ":" << arr1 << " " << arr2 << std::endl;
            }
            else
            {
                outfile << word1 << ":" << arr2 << " " << arr1 << std::endl;
            }

            // update Lexicon
            endp = outfile.tellp();
            Insert(word1, beginp, endp, calcDocNum(arr1) + calcDocNum(arr2));
            beginp = endp;

            // update line1 and line2
            std::getline(infile1, line1);
            std::getline(infile2, line2);
            word1 = line1.substr(0, line1.find(":"));
            word2 = line2.substr(0, line2.find(":"));
        }
        else if (word1.compare(word2) < 0)
        {
            // write line1 only
            outfile << line1 << std::endl;

            // update Lexicon
            endp = outfile.tellp();
            Insert(word1, beginp, endp, calcDocNum(line1));
            beginp = endp;

            // update line1
            std::getline(infile1, line1);
            word1 = line1.substr(0, line1.find(":"));
        }
        else
        {
            // write line2 only
            outfile << line2 << std::endl;

            // update Lexicon
            endp = outfile.tellp();
            Insert(word2, beginp, endp, calcDocNum(line2));
            beginp = endp;

            // update line2
            std::getline(infile2, line2);
            word2 = line2.substr(0, line2.find(":"));
        }
    }

    if (infile1)
    {
        outfile << line1 << std::endl;
        // update Lexicon
        endp = outfile.tellp();
        Insert(word1, beginp, endp, calcDocNum(line1));
        beginp = endp;
    }
    if (infile2)
    {
        outfile << line2 << std::endl;
        // update Lexicon
        endp = outfile.tellp();
        Insert(word2, beginp, endp, calcDocNum(line2));
        beginp = endp;
    }

    while (infile1)
    {
        std::getline(infile1, line1);
        outfile << line1 << std::endl;

        // update Lexicon
        endp = outfile.tellp();
        Insert(word1, beginp, endp, calcDocNum(line1));
        beginp = endp;
    }
    while (infile2)
    {
        std::getline(infile2, line2);
        outfile << line2 << std::endl;

        // update Lexicon
        endp = outfile.tellp();
        Insert(word2, beginp, endp, calcDocNum(line2));
        beginp = endp;
    }

    infile1.close();
    infile2.close();
    outfile.close();
}

void Lexicon::Write()
{
    std::ofstream outfile;
    if (FILEMODE == FILEMODE_ASCII)
    {
        outfile.open(LexiconPath);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        outfile.open(LexiconPath, std::ofstream::binary);
    }

    for (std::map<std::string, LexiconItem>::iterator iter = _lexiconList.begin();
         iter != _lexiconList.end(); ++iter)
    {
        outfile << iter->first << " " << iter->second.beginp << " " << iter->second.endp << " "
                << iter->second.docNum << std::endl;
    }
    outfile.close();
}