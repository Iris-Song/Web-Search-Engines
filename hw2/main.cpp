#include "DataLoader.h"
int main()
{
    DataLoader dl;
    if (IS_INDEX){
        time_t index_begin = time(NULL);
        dl.read_data(DATA_SOURCE_PATH);
        time_t index_end = time(NULL);
        std::cout<<index_end-index_begin<<std::endl;
    }
    dl.mergeIndexToOne();
}