#!/bin/bash

# Run from the project root directory as follows:
# ./scripts/check_format.sh sdk tests 

printf "Running clang-format...\n"

RET_CODE=0

function format() {
    for f in $(find $@ -name '*.h' -or -name '*.hpp' -or -name '*.c' -or -name '*.cpp'); do 
        case "${f}" in
            sdk/src/thirdparty/*)
                ;;
            tests/external/*)
                ;;    
            *)
                clang-format -i --dry-run --Werror --style=file ${f};
                ret=$?
                if [ $ret -ne 0 ]; then
                    RET_CODE=$ret
                fi
                ;;
        esac
    done

    echo "~~~ $@ directory checked ~~~";
}

# Check all of the arguments first to make sure they're all directories
for dir in "$@"; do
    if [ ! -d "${dir}" ]; then
        echo "${dir} is not a directory";
    else
        format ${dir};
    fi
done

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

if [ $RET_CODE -eq 0 ]; then
    printf "${GREEN}Everything up to standard${NC}\n"
else
    printf "${RED}Not up to formatting standard${NC}\n"
    echo "Try running run_format.sh to format all files."
fi

exit $RET_CODE