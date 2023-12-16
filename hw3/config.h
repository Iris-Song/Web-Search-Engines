#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>
#include <queue>
#include <deque>
#include <iomanip>
#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include "zlib.h"

#define DATA_SOURCE_PATH "./msmarco-docs.trec.gz"
#define SNIPPETS_SOURCE_PATH "./msmarco-docs.trec"
#define INDEX_FILE_FOLDER_PATH "./tempb/"
#define FINAL_INDEX_PATH "index.idx"
#define LEXICON_PATH "lexicon.lex"
#define DOC_TABLE_PATH "docTable.dt"
#define FAKE_RESULT_PATH "./result/fake_result5.txt"
#define SNIPPET_FOLDER "./snippet/"

#define INDEX_CHUNK 1024 //1KB
#define POST_BYTES 10 // 2*uint_32(4) + 2*seperator
#define AVG_WORD_BYTES 12 //estimated
#define CHAR_END_TAG '\0'

#define FILE_INDEX_CHUNK 20971520 //20MB
#define FILEMODE_ASCII 0
#define FILEMODE_BIN 1
#define FILEMODE 1

#define POSTINGS_IN_BLOCK 64
#define BLOCK_SIZE 65536//64KB
#define MAX_METASIZE 8192//8KB
#define MAX_DOCID -1
#define CONJUNCTIVE 0
#define DISJUNCTIVE 1
#define RESULT_NUM 20
#define SNIPPETS_RANGE 50

#define TERM_NUM 7
#define SNIPPETS_TYPE 7
#define LINEAR_SNIPPETS 1
#define PREFIX_SNIPPETS 2
#define BM25_SNIPPETS 3
#define VECTOR_SNIPPETS 4
#define WEIGHT_SNIPPETS 5
#define KEYWORD_SNIPPETS 6
#define DUMP_SNIPPETS 7
#define SEPARATOR " :;,.\t\v\r\n\f[]{}()<>+-=*&^%$#@!~`\'\"|\\/?·\"：“”"
#define DOC_AVG_LEN 7111
#define DOC_NUM 3213835
#define MAX_SNIPPETS 200
#define MIN_KEYWORD 2
#define KEYWORD_PERCENT 0.25

#define IS_DEBUG 1
#define IS_INDEX 0 //whether build index
#define IS_MERGE 0 //whether merge index
#define IS_WRITE_PAGE 0//whether write Document Table(page table)
#define IS_WRITE_LEXICON 0 //whether write Lexicon Structure
#define IS_DELETE_TEMP 0
#define IS_BUILD 0
#define IS_RELOAD 0
#define IS_QUERY 0
#define IS_SNIPPETS 1

#endif