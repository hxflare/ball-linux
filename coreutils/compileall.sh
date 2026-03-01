#!/bin/bash
DIR="${1:-.}"
for file in "$DIR"/*.c; do
    [ -f "$file" ] || { echo "No .c files found in $DIR"; exit 1; }
    name="${file%.c}"
    echo "Compiling $file -> $name"
    gcc -Wall -Wextra -static -o "$name" "$file" ../btools.c || echo "Compilation failed : $file"
done

echo "compiled"