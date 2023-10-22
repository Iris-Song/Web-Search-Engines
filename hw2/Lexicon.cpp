#include "Lexicon.h"

// Function to encode a uint32 value using Varbyte encoding
std::vector<uint8_t> varbyte_encode(uint32_t value)
{
    std::vector<uint8_t> encoded;
    while (value > 0)
    {
        // Extract the 7 least significant bits
        uint8_t byte = value & 0x7F;
        // Set the high bit to indicate more bytes if needed
        if (value > 0x7F)
        {
            byte |= 0x80;
        }
        encoded.push_back(byte);
        // Shift the value to the right by 7 bits
        value >>= 7;
    }
    return encoded;
}

// Function to decode a Varbyte encoded sequence into a uint32 value
uint32_t varbyte_decode(const std::vector<uint8_t> &encoded)
{
    uint32_t value = 0;
    for (size_t i = 0; i < encoded.size(); ++i)
    {
        // Extract the 7 least significant bits from the byte
        uint32_t byte = encoded[i] & 0x7F;
        // Add the extracted bits to the result
        value |= (byte << (7 * i));
        // If the high bit is not set, it's the last byte
        if (!(encoded[i] & 0x80))
        {
            break;
        }
    }
    return value;
}

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

uint32_t Lexicon::WriteBitArray(std::string arr, std::ofstream &outfile)
{
    size_t beginpos = 0;
    uint32_t freq, docID;
    bool isOk = true;
    uint32_t docNum = 0;
    // uint32_t prev_docID = 0;
    while (isOk)
    {
        size_t space, comma;
        try
        {
            space = arr.find(" ", beginpos);
            comma = arr.find(",", space);
            docID = std::stoi(arr.substr(beginpos, space - beginpos));

            if (comma != std::string::npos)
                freq = std::stoi(arr.substr(space + 1, comma - space - 1));
            else
            {
                freq = std::stoi(arr.substr(space + 1));
                isOk = false;
            }
        }
        catch (...)
        {
            std::cout << arr.substr(beginpos, space - beginpos) << " " << arr.substr(space + 1, comma - space - 1);
            std::cout << arr.substr(beginpos, space - beginpos).length() << std::endl;
            std::cout << arr.substr(space + 1, comma - space - 1).length() << std::endl;
            std::cout << docNum << std::endl;
            continue;
        }
        
        std::vector<uint8_t> endocID = varbyte_encode(docID);
        std::vector<uint8_t> enFreq = varbyte_encode(freq);
        for (int i = 0; i < endocID.size(); i++)
        {
            outfile.write(reinterpret_cast<const char *>(&endocID[i]), sizeof(uint8_t));
        }
        for (int i = 0; i < enFreq.size(); i++)
        {
            outfile.write(reinterpret_cast<const char *>(&enFreq[i]), sizeof(uint8_t));
        }

        // prev_docID = docID;
        beginpos = comma + 1;
        docNum += 1;
        
    }
    return docNum;
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
                    outfile << word1 << ":" << arr1 << "," << arr2 << std::endl;
                }
                else
                {
                    outfile << word1 << ":" << arr2 << "," << arr1 << std::endl;
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
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        infile1.open(path1, std::ifstream::binary);
        infile2.open(path2, std::ifstream::binary);
        outfile.open(IndexPath, std::ofstream::binary);
        uint32_t beginp, endp;

        std::string line1, line2;
        std::string word1, word2;

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
                if (arr1.empty() || arr2.empty())
                    break;
                uint32_t docNum1, docNum2;

                if (docID1 < docID2)
                {
                    docNum1 = WriteBitArray(arr1, outfile);
                    docNum2 = WriteBitArray(arr2, outfile);
                }
                else
                {
                    docNum2 = WriteBitArray(arr2, outfile);
                    docNum1 = WriteBitArray(arr1, outfile);
                }

                // update Lexicon
                endp = outfile.tellp();
                Insert(word1, beginp, endp, docNum1 + docNum2);
                beginp = endp;

                // update line1 and line2
                std::getline(infile1, line1);
                std::getline(infile2, line2);
                word1 = line1.substr(0, line1.find(":"));
                word2 = line2.substr(0, line2.find(":"));
            }
            else if (word1.compare(word2) < 0)
            {
                std::string arr1 = line1.substr(line1.find(":") + 1);
                if (arr1.empty())
                    break;

                // write line1 only
                uint32_t docNum = WriteBitArray(arr1, outfile);

                // update Lexicon
                endp = outfile.tellp();
                Insert(word1, beginp, endp, docNum);
                beginp = endp;

                // update line1
                std::getline(infile1, line1);
                word1 = line1.substr(0, line1.find(":"));
            }
            else
            {
                std::string arr2 = line2.substr(line2.find(":") + 1);
                if (arr2.empty())
                    break;

                // write line2 only
                uint32_t docNum = WriteBitArray(arr2, outfile);

                // update Lexicon
                endp = outfile.tellp();
                Insert(word2, beginp, endp, docNum);
                beginp = endp;

                // update line2
                std::getline(infile2, line2);
                word2 = line2.substr(0, line2.find(":"));
            }
        }
        
        if (infile1)
        {
            std::string arr1 = line1.substr(line1.find(":") + 1);
            if (!arr1.empty())
            {
                uint32_t docNum = WriteBitArray(arr1, outfile);

                // update Lexicon

                endp = outfile.tellp();
                Insert(word1, beginp, endp, docNum);
                beginp = endp;
            }
        }
        if (infile2)
        {
            std::string arr2 = line2.substr(line2.find(":") + 1);
            if (!arr2.empty())
            {
                uint32_t docNum = WriteBitArray(arr2, outfile);

                // update Lexicon
                if (docNum != -1)
                {
                    endp = outfile.tellp();
                    Insert(word2, beginp, endp, docNum);
                    beginp = endp;
                }
            }
        }

        while (infile1)
        {
            std::string arr1 = line1.substr(line1.find(":") + 1);
            if (arr1.empty())
                break;
            std::getline(infile1, line1);

            uint32_t docNum = WriteBitArray(arr1, outfile);

            // update Lexicon
            if (docNum != -1)
                break;
            endp = outfile.tellp();
            Insert(word1, beginp, endp, docNum);
            beginp = endp;
        }
        while (infile2)
        {
            std::string arr2 = line2.substr(line2.find(":") + 1);
            if (arr2.empty())
                break;

            std::getline(infile2, line2);

            uint32_t docNum = WriteBitArray(arr2, outfile);

            // update Lexicon
            endp = outfile.tellp();
            Insert(word2, beginp, endp, docNum);
            beginp = endp;
        }
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