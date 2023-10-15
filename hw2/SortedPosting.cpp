#include "sortedPosting.h"

void SortedPosting::print()
{
    for (std::map<std::string,uint32_t>::iterator it=_sortedList.begin(); it!=_sortedList.end(); ++it){
        std::cout << "(" << it->first << "," << it->second << ")\n";
    }
}