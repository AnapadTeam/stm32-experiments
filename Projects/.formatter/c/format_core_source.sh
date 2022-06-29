#!/bin/bash

DIRECTORY_TO_FORMAT="Core/"

if [[ ! -d "$DIRECTORY_TO_FORMAT" ]]; then
    echo "This script must be called from the project directory (e.g. where the \"Core\" directory is)."
    exit 1
fi

find "$DIRECTORY_TO_FORMAT" -iname *.c -o -iname *.h | xargs clang-format -i --style="{BasedOnStyle: LLVM, ColumnLimit: 120, IndentWidth: 4, IndentCaseLabels: true}"

