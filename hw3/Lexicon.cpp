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

void LexiconItem::update(uint32_t bgp, uint32_t edp, uint32_t dn, uint32_t bn)
{
    beginp = bgp;
    endp = edp;
    docNum = dn;
    blockNum = bn;
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

bool Lexicon::Insert(std::string word, uint32_t beginp,
                     uint32_t endp, uint32_t docNum, uint32_t blockNum)
{
    if (word.empty())
        return false;
    LexiconItem lexItem;
    lexItem.update(beginp, endp, docNum, blockNum);
    _lexiconList[word] = lexItem;
    return true;
}

uint32_t Lexicon::WriteBlocks(std::string term, uint32_t &docNum, std::string arr, std::ofstream &outfile)
{
    // get docID_list,freq_list
    size_t beginpos = 0;
    std::vector<std::vector<uint8_t>> docID_list;
    std::vector<std::vector<uint8_t>> freq_list;
    std::vector<uint32_t> metadata_last_docID;
    std::vector<uint32_t> metadata_docID_block_sizes;
    std::vector<uint32_t> metadata_freq_block_sizes;

    uint32_t freq, docID;
    bool isOk = true;
    uint32_t prevdocID = 0;

    while (isOk)
    {
        size_t space, comma;
        try
        {
            space = arr.find(" ", beginpos);
            comma = arr.find(",", space);
            docID = std::stoi(arr.substr(beginpos, space - beginpos));

            if (comma != std::string::npos)
            {
                freq = std::stoi(arr.substr(space + 1, comma - space - 1));
            }
            else
            {
                freq = std::stoi(arr.substr(space + 1));
                isOk = false;
            }

            if (docID < prevdocID)
            {
                std::cout << "nooo" << std::endl;
            }
            // if (term == "0")
            // {
            //     std::cout << docID << " " << freq << std::endl;
            // }
            std::vector<uint8_t> endocID = varbyte_encode(docID - prevdocID);
            std::vector<uint8_t> enFreq = varbyte_encode(freq);
            docID_list.push_back(endocID);
            freq_list.push_back(enFreq);
            prevdocID = docID;
            if (docID_list.size() % POSTINGS_IN_BLOCK == 0 || isOk == false)
            {
                metadata_last_docID.push_back(docID);
                prevdocID = 0;
            }
        }
        catch (...)
        {
            std::cout << arr.substr(beginpos, space - beginpos) << " " << arr.substr(space + 1, comma - space - 1);
            std::cout << arr.substr(beginpos, space - beginpos).length() << std::endl;
            std::cout << arr.substr(space + 1, comma - space - 1).length() << std::endl;
            continue;
        }
        beginpos = comma + 1;
    }

    docNum = docID_list.size();

    // get metadata_docID_block_sizes, metadata_freq_block_sizes
    uint32_t docIDsize = 0, freqSize = 0;
    for (int i = 0; i < docID_list.size(); i++)
    {
        docIDsize += docID_list[i].size();
        freqSize += freq_list[i].size();

        if ((i + 1) % POSTINGS_IN_BLOCK == 0 || i == docID_list.size() - 1)
        {
            metadata_docID_block_sizes.push_back(docIDsize);
            metadata_freq_block_sizes.push_back(freqSize);
            docIDsize = 0;
            freqSize = 0;
        }
    }

    // write blocks

    int blocks_num = metadata_last_docID.size();
    int pblocks = 0;

    uint32_t blockNum = 0;

    while (pblocks < blocks_num)
    {
        uint32_t nowbyte = 4; // block_len
        int pbeginblock = pblocks;
        while (nowbyte <= BLOCK_SIZE && pblocks < blocks_num)
        {
            uint32_t newsize = 4 * 3 + metadata_docID_block_sizes[pblocks] + metadata_freq_block_sizes[pblocks];
            if (nowbyte + newsize > BLOCK_SIZE)
            {
                break;
            }
            pblocks += 1;
            nowbyte += newsize;
        }

        // write a block
        // if(term=="0")
        // std::cout<<"write a block"<<std::endl;
        // write metadata
        blockNum += 1;
        uint32_t block_len = pblocks - pbeginblock;
        outfile.write(reinterpret_cast<const char *>(&block_len), sizeof(uint32_t));
        // if(term=="0")
        // std::cout<<"metadata size :"<<block_len<<std::endl;
        // if(term=="0")
        // std::cout<<"lastdocID :";
        for (int i = pbeginblock; i < pblocks; i++)
        {
            outfile.write(reinterpret_cast<const char *>(&metadata_last_docID[i]), sizeof(uint32_t));
            // if(term=="0")
            // std::cout<<metadata_last_docID[i]<<" ";
        }
        // if(term=="0")std::cout<<std::endl;
        for (int i = pbeginblock; i < pblocks; i++)
        {
            outfile.write(reinterpret_cast<const char *>(&metadata_docID_block_sizes[i]), sizeof(uint32_t));
            // if(term=="0")
            // std::cout<<metadata_docID_block_sizes[i]<<" ";
        }
        // if(term=="0")std::cout<<std::endl;
        for (int i = pbeginblock; i < pblocks; i++)
        {
            outfile.write(reinterpret_cast<const char *>(&metadata_freq_block_sizes[i]), sizeof(uint32_t));
            // if(term=="0")
            // std::cout<<metadata_freq_block_sizes[i]<<" ";
        }
        // if(term=="0")std::cout<<std::endl;
        // write docID blocks & freq blocks
        for (int i = pbeginblock; i < pblocks; i++)
        {
            for (int j = i * POSTINGS_IN_BLOCK; j < (i + 1) * POSTINGS_IN_BLOCK && j < docID_list.size(); j++)
            {
                std::vector<uint8_t> endocID = docID_list[j];
                for (int k = 0; k < endocID.size(); k++)
                {
                    outfile.write(reinterpret_cast<const char *>(&endocID[k]), sizeof(uint8_t));
                }
            }
            for (int j = i * POSTINGS_IN_BLOCK; j < (i + 1) * POSTINGS_IN_BLOCK && j < docID_list.size(); j++)
            {
                std::vector<uint8_t> enFreq = freq_list[j];
                for (int k = 0; k < enFreq.size(); k++)
                {
                    outfile.write(reinterpret_cast<const char *>(&enFreq[k]), sizeof(uint8_t));
                }
            }
        }
        // file left blocks in 0
        // uint8_t zero = 0;
        // for (int i = 0; i < BLOCK_SIZE - nowbyte; i++)
        // {
        //     outfile.write(reinterpret_cast<const char *>(&zero), sizeof(uint8_t));
        // }
    }
    
    return blockNum;
}

void Lexicon::Build(std::string path)
{
    std::ifstream infile;
    std::ofstream outfile;

    infile.open(path, std::ifstream::binary);
    outfile.open(IndexPath, std::ofstream::binary);
    uint32_t beginp = 0, endp;
    std::string line;

    while (!infile.eof())
    {
        std::getline(infile, line);
        std::string word = line.substr(0, line.find(":"));
        if (!word.length())
        {
            break;
        }
        std::string arr = line.substr(line.find(":") + 1);

        uint32_t docNum;
        uint32_t blockNum = WriteBlocks(word, docNum, arr, outfile);

        // update Lexicon
        endp = outfile.tellp();
        if (blockNum > 1)
        {
            std::cout << word << " " << beginp << " " << endp << " "
                      << docNum << " " << blockNum << std::endl;
        }
        Insert(word, beginp, endp, docNum, blockNum);
        beginp = endp;
    }

    infile.close();
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
                << iter->second.docNum << " " << iter->second.blockNum << std::endl;
    }
    outfile.close();
}

void Lexicon::LoadLexiconList()
{
    std::ifstream infile;
    if (FILEMODE == FILEMODE_ASCII)
    {
        infile.open(LexiconPath);
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
        infile.open(LexiconPath, std::ofstream::binary);
    }
    if (!infile)
    {
        std::cout << "can not read " << LexiconPath << std::endl;
        exit(0);
    }
    _lexiconList.clear();
    while (!infile.eof())
    {
        std::string term;
        LexiconItem li;
        infile >> term;
        infile >> li.beginp >> li.endp >> li.docNum >> li.blockNum;
        _lexiconList[term] = li;
    }
    std::cout << "There are " << _lexiconList.size() << " words in Lexicon Structure" << std::endl;
}
