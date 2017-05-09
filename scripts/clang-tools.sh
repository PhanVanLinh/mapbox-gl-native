#!/usr/bin/env bash

set -e
set -o pipefail

CLANG_TIDY=${CLANG_TIDY:-$(scripts/mason.sh PREFIX clang-tidy VERSION 4.0.0)/bin/clang-tidy}
CLANG_APPLY=${CLANG_APPLY:-$(scripts/mason.sh PREFIX clang-tidy VERSION 4.0.0)/bin/clang-apply-replacements}
CLANG_FORMAT=${CLANG_FORMAT:-$(scripts/mason.sh PREFIX clang-format VERSION 4.0.0)/bin/clang-format}

for CLANG_FILE in "${CLANG_TIDY} ${CLANG_APPLY} ${CLANG_FORMAT}"; do
    command -v ${CLANG_TIDY} > /dev/null 2>&1 || {
        echo "Can't find ${CLANG_FILE} in PATH."
        if [ -z ${CLANG_FILE} ]; then
            echo "Alternatively, you can manually set ${!CLANG_FILE@}."
        fi
        exit 1
    }
done

cd $1

CDUP=$(git rev-parse --show-cdup)

function run_clang_tidy() {
    echo "Running clang-tidy on $0..."
    if [[ -n $1 ]] && [[ $1 == "--fix" ]]; then
        OUTPUT=$(${CLANG_TIDY} -p=${PWD} -export-fixes=${EXPORT_FIXES} -fix -fix-errors ${0} 2>/dev/null)
        if [[ -n $OUTPUT ]]; then
            echo "Caught clang-tidy warning/error:"
            echo -e "$OUTPUT"
        fi
    else
        OUTPUT=$(${CLANG_TIDY} -p=${PWD} ${0} 2>/dev/null)
        if [[ -n $OUTPUT ]]; then
            echo "Caught clang-tidy warning/error:"
            echo -e "$OUTPUT"
            rm -rf ${EXPORT_FIXES}
            exit 1
        fi
    fi
}

function run_clang_apply_replacements() {
    echo "Running clang-apply-replacements..."
    ${CLANG_APPLY} -format ${EXPORT_FIXES}
    rm -rf ${EXPORT_FIXES}
}

function run_clang_format() {
    echo "Running clang-format on $0..."
    ${CLANG_FORMAT} -i ${CDUP}/$0
}

export CLANG_TIDY CLANG_APPLY CLANG_FORMAT
export -f run_clang_tidy run_clang_format run_clang_apply_replacements
export EXPORT_FIXES=$(mktemp -d)

echo "Running clang checks... (this might take a while)"

if [[ -n $2 ]] && [[ $2 == "--diff" ]]; then
    DIFF_FILES=$(for file in `git diff origin/master --name-only | grep "pp$"`; do echo $file; done)
    if [[ -n $DIFF_FILES ]]; then
        echo "${DIFF_FILES}" | xargs -I{} -P ${JOBS} bash -c 'run_clang_tidy --fix' {}
        bash -c "run_clang_apply_replacements"
        # XXX disabled until we run clang-format over the entire codebase.
        #echo "${DIFF_FILES}" | xargs -I{} -P ${JOBS} bash -c 'run_clang_format' {}
        git diff --quiet || {
            echo "Changes were made to source files - please review them before committing."
            exit 1
        }
    fi
    echo "All looks good!"
else
    git ls-files "${CDUP}/src/mbgl/*.cpp" "${CDUP}/platform/*.cpp" "${CDUP}/test/*.cpp" | \
        xargs -I{} -P ${JOBS} bash -c 'run_clang_tidy --fix' {}
    bash -c "run_clang_apply_replacements"
fi
