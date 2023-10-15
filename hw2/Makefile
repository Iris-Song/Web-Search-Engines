CC = g++
CXX_FLAGS = -std=c++17 -w 

BIN = Indexer

$(BIN): main.o DataLoader.o DocTable.o InvertedIndex.o SortedPosting.o
	$(CC) -o $@ $^ -lz
main.o: main.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
DataLoader.o: DataLoader.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $< 
DocTable.o: DocTable.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
InvertedIndex.o: InvertedIndex.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
SortedPosting.o: SortedPosting.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
	
.phony:
clean:
	rm -f *.o $(BIN)
	rm -f Indexer