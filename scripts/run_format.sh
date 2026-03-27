#!/bin/bash

# Run from the project root directory as follows:
# ./scripts/run_format.sh sdk tests 

printf "Running clang-format...\n"

function format() {
    for f in $(find $@ -name '*.h' -or -name '*.hpp' -or -name '*.c' -or -name '*.cpp'); do 
        case "${f}" in
            sdk/src/thirdparty/*)
                ;;
            tests/external/*)
                ;;    
            *)
                echo "format ${f}";
                clang-format -i --style=file ${f};
                ;;
        esac
    done

    echo "~~~ $@ directory formatted ~~~";
}

# Check all of the arguments first to make sure they're all directories
for dir in "$@"; do
    if [ ! -d "${dir}" ]; then
        echo "${dir} is not a directory";
    else
        format ${dir};
    fi
done

GREEN='\033[0;32m'
NC='\033[0m'

printf "${GREEN}Done. All files were formatted (if required).${NC}\n"