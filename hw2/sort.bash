#!/bin/bash

if [[ -z "$1" ]]; then
    echo "Usage: $0 <last_index>"
    exit 1
fi

NUM_THREADS=32

for ((i=0; i<$1; i++)); do
    file="temp/BIN_${i}.txt"

    if [[ -f "$file" ]]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        sort --parallel=$NUM_THREADS -t: -k1 1 "$file" > "temp/${i}.${extension}"
        echo "sorted and saved: $file"
    else
        echo "err: $file cannot be found."
    fi
done