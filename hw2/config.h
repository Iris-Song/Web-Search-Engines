#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>

#define DATA_SOURCE_PATH "./msmarco-docs.trec.gz"
#define INDEX_FILE_FOLDER_PATH "./temp/"
#define FINAL_INDEX_PATH "index.idx"
#define LEXICON_PATH "lexicon.lex"
#define DOC_TABLE_PATH "docTable.dt"

#define INDEX_CHUNK 409600 //400KB
#define POST_BYTES 10 // 2*uint_32(4) + 2*seperator
#define AVG_WORD_BYTES 12 //estimated

#define FILE_INDEX_CHUNK 20971520 //20MB
#define FILEMODE_ASCII 0
#define FILEMODE_BIN 1
#define FILEMODE 1


#define IS_DEBUG 1
#define IS_INDEX 1 //whether build index
#define IS_MERGE 1 //whether merge index
#define IS_WRITE_PAGE 1 //whether write Document Table(page table)
#define IS_WRITE_LEXICON 1 //whether write Lexicon Structure
#define IS_DELETE_TEMP 0

#endif