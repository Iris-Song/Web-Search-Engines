#include "InvertedIndex.h"

InvertedIndex::InvertedIndex()
{
    HashWord.clear();
    allIndexSize = 0;
    indexFileNum = 0;

    if (!IS_INDEX){
        countIndexFileNum();
        return;
    }

    // if folder exist,delete old file;
    // else create
    std::string IndexFoldPath = INDEX_FILE_FOLDER_PATH;

    if (std::filesystem::exists(IndexFoldPath))
    {
        if (!clearIndexFolder())
        {
            throw "cannot clear Index Folder!";
        }
    }
    else
    {
        if (!creatIndexFolder())
        {
            throw "cannot create Index Folder!";
        };
    }
}

InvertedIndex::~InvertedIndex()
{
}

bool InvertedIndex::creatIndexFolder()
{
    std::string IndexFoldPath = INDEX_FILE_FOLDER_PATH;
    return std::filesystem::create_directory(IndexFoldPath);
}

bool InvertedIndex::clearIndexFolder()
{
    std::string IndexFoldPath = INDEX_FILE_FOLDER_PATH;
    for (const auto &entry : std::filesystem::directory_iterator(IndexFoldPath))
    {
        if (!std::filesystem::is_directory(entry.path()))
        {
            if (!std::filesystem::remove(entry.path()))
            {
                return false;
            };
        }
    }
    return true;
}

void InvertedIndex::countIndexFileNum()
{
    indexFileNum = 0;
    std::string IndexFoldPath = INDEX_FILE_FOLDER_PATH;
    for (const auto &entry : std::filesystem::directory_iterator(IndexFoldPath))
    {
        if (!std::filesystem::is_directory(entry.path()))
        {
            indexFileNum+=1;
        }
    }
}

// inset (docID,freq) in wordHash[word]
void InvertedIndex::Insert(std::string word, uint32_t docID, uint32_t freq)
{
    // judge if have enough space to put
    bool isWrite = false;
    if (HashWord.count(word))
    {
        if (allIndexSize + POST_BYTES >= FILE_INDEX_CHUNK)
            isWrite = true;
        else
        {
            HashWord[word].push_back(std::pair<uint32_t, uint32_t>(docID, freq));
            allIndexSize += POST_BYTES;
        }
    }
    else
    {
        if (allIndexSize + POST_BYTES + AVG_WORD_BYTES >= FILE_INDEX_CHUNK)
            isWrite = true;
        else
        {
            std::vector<std::pair<uint32_t, uint32_t>> newVec;
            newVec.push_back(std::pair<uint32_t, uint32_t>(docID, freq));
            HashWord[word] = newVec;
            allIndexSize += POST_BYTES + AVG_WORD_BYTES;
        }
    }

    if (isWrite) // INDEX CHUNK is full, need write out.
    {
        Write();
        Clear();
        std::vector<std::pair<uint32_t, uint32_t>> newVec;
        newVec.push_back(std::pair<uint32_t, uint32_t>(docID, freq));
        HashWord[word] = newVec;
        allIndexSize += POST_BYTES + AVG_WORD_BYTES;
    }
}

std::string InvertedIndex::getIndexFilePath()
{
    indexFileNum += 1;
    if (FILEMODE == FILEMODE_ASCII)
        return std::string(INDEX_FILE_FOLDER_PATH) + "ASCII_" + std::to_string(indexFileNum-1) + ".txt";
    else if (FILEMODE == FILEMODE_BIN)
        return std::string(INDEX_FILE_FOLDER_PATH) + "BIN_" + std::to_string(indexFileNum-1) + ".bin";
}

void InvertedIndex::Write()
{
    std::string path = getIndexFilePath();
    std::ofstream outfile;
    if (FILEMODE == FILEMODE_ASCII)
    {
        outfile.open(path);
        for (std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>>::iterator it = HashWord.begin();
             it != HashWord.end(); ++it)
        {
            outfile << it->first << ":";
            for (std::vector<std::pair<uint32_t, uint32_t>>::iterator iter = it->second.begin();
                 iter != it->second.end(); ++iter)
            {
                outfile << iter->first << " " << iter->second;
            }
            outfile << std::endl;
        }
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
    }
    outfile.close();
}

void InvertedIndex::Clear()
{
    allIndexSize = 0;
    HashWord.clear();
}