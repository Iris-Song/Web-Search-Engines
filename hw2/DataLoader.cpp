#include "DataLoader.h"

DataLoader::DataLoader()
{
}

DataLoader::~DataLoader()
{
}

std::string DataLoader::extractContent(std::string org, std::string bstr, std::string estr)
{
    size_t begin_tag_len = bstr.length();

    size_t start_pos = org.find(bstr);
    size_t end_pos = org.find(estr);

    return org.substr(start_pos + begin_tag_len, end_pos - start_pos - begin_tag_len);
}

std::string DataLoader::getFirstLine(std::string str)
{
    size_t endpos = str.find("\n");
    return str.substr(0, endpos);
}

uint32_t DataLoader::calcWordFreq(std::string text, uint32_t docID)
{
    // delete first row - url
    size_t beginpos = text.find("\n") + 1;
    text = text.substr(beginpos);
    std::string sep = " :;,.\t\v\r\n\f[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”";
    std::string word;
    SortedPosting sortedPosting;
    for (size_t i = 0; i < text.length(); i++)
    {
        if (sep.find(text[i]) == std::string::npos)
        {
            word += text[i];
        }
        else
        {
            if (word.length())
            {
                if (!sortedPosting._sortedList.count(word))
                {
                    sortedPosting._sortedList[word] = 1;
                }
                else
                {
                    sortedPosting._sortedList[word] += 1;
                }
            }
            word.clear();
        }
    }

    if (IS_DEBUG & 0)
    {
        sortedPosting.print();
    }

    for (std::map<std::string, uint32_t>::iterator it = sortedPosting._sortedList.begin();
         it != sortedPosting._sortedList.end(); it++)
    {
        _InvertedIndex.Insert(it->first, docID, it->second);
    }
    return sortedPosting._sortedList.size();
}

void DataLoader::read_data(const char *filepath)
{
    gzFile gzfile = gzopen(filepath, "r");
    int size = INDEX_CHUNK;
    char *buffer = (char *)malloc(size + 1);

    uint32_t docID = 0;
    std::string docContent;

    while (!gzeof(gzfile))
    {
        int readlen = gzread(gzfile, buffer, size);
        buffer[readlen] = '\0';

        docContent += buffer;
        size_t pos;

        while ((pos = docContent.find("</DOC>")) != std::string::npos)
        {
            Document doc;
            const int CONST_DOC_LEN = 6; // len of '</DOC>'
            doc.docID = docID;
            doc.docNO = extractContent(docContent.substr(0, pos), "<DOCNO>", "</DOCNO>");
            std::string fullText = extractContent(docContent.substr(0, pos), "<TEXT>\n", "</TEXT>");
            doc.dataLen = sizeof(fullText);
            doc.url = getFirstLine(fullText);
            doc.wordnums = calcWordFreq(fullText, docID);
            _DocTable.add(doc);

            if (IS_DEBUG)
                if (docID % 1000 == 0)
                {
                    std::cout << docID << std::endl;
                };

            docContent = docContent.substr(pos + CONST_DOC_LEN);
            docID += 1;
            // break;
        }
        // break;
    }

    _DocTable.print();
    gzclose(gzfile);
    gzfile = NULL;
}

void DataLoader::mergeIndex(uint32_t fileIndex1, uint32_t fileIndex2)
{
    std::string dst_mergePath = _InvertedIndex.getIndexFilePath();
    std::ifstream infile1;
    std::ifstream infile2;
    std::ofstream outfile;
    if (FILEMODE == FILEMODE_ASCII)
    {
        std::string file1 = std::string(INDEX_FILE_FOLDER_PATH) + "ASCII_" + std::to_string(fileIndex1) + ".txt";
        std::string file2 = std::string(INDEX_FILE_FOLDER_PATH) + "ASCII_" + std::to_string(fileIndex2) + ".txt";
        infile1.open(file1);
        infile2.open(file2);
        outfile.open(dst_mergePath);

        std::string line1, line2;
        std::string word1, word2;
        std::string arr1, arr2;
        while (infile1 && infile2)
        {
            if (word1.empty() && word2.empty())
            {
                std::getline(infile1, line1);
                std::getline(infile2, line2);
                word1 = line1.substr(0, line1.find(":"));
                word2 = line2.substr(0, line2.find(":"));
            }
            if (word1 == word2)
            {
                // merge word1 and word2
                arr1 = line1.substr(line1.find(":") + 1);
                arr2 = line2.substr(line2.find(":") + 1);
                outfile << word1 << ":" << arr1 << " " << arr2 << std::endl;
                // update line1 and line2
                std::getline(infile1, line1);
                std::getline(infile2, line2);
                word1 = line1.substr(0, line1.find(":"));
                word2 = line2.substr(0, line2.find(":"));
            }
            else if (word1 < word2)
            {
                // write line1 only
                outfile << line1 << std::endl;
                // update line1
                std::getline(infile1, line1);
                word1 = line1.substr(0, line1.find(":"));
            }
            else
            {
                // write line2 only
                outfile << line2 << std::endl;
                // update line2
                std::getline(infile2, line2);
                word2 = line2.substr(0, line2.find(":"));
            }
        }

        if (infile1)
        {
            outfile << line1 << std::endl;
        }
        if (infile2)
        {
            outfile << line2 << std::endl;
        }

        while (infile1)
        {
            std::getline(infile1, line1);
            outfile << line1 << std::endl;
        }
        while (infile2)
        {
            std::getline(infile2, line2);
            outfile << line2 << std::endl;
        }
    }
    else if (FILEMODE == FILEMODE_BIN)
    {
    }
    infile1.close();
    infile2.close();
    outfile.close();
}

void DataLoader::mergeIndexToOne()
{
    uint32_t mergedIndexNum = 0;
    uint32_t leftIndexNum;
    while ((leftIndexNum = _InvertedIndex.indexFileNum - mergedIndexNum) >= 2)
    {
        mergeIndex(mergedIndexNum, mergedIndexNum);
        mergedIndexNum += 2;
    }
}