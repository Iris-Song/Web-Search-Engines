#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>

#define DATA_SOURCE_PATH "./msmarco-docs.trec.gz"
#define INDEX_FILE_FOLDER_PATH "./tempIndex/"

#define INDEX_CHUNK 409600 //400KB
#define POST_BYTES 10 // 2*uint_32(4) + 2*seperator
#define AVG_WORD_BYTES 6 //avg len of English word is 5

#define FILE_INDEX_CHUNK 20971520 //20MB
#define FILEMODE_ASCII 0
#define FILEMODE_BIN 1
#define FILEMODE 0


#define IS_DEBUG 1
#define IS_INDEX 1 //whether build index


#endif