#! /usr/bin/env bash

script_prefix='@SCRIPT_PREFIX@'
data_prefix='@DATA_PREFIX@'
export OTTER_TRACE_PATH="${data_prefix}/integration-tests"

__run() {
    printf "Running integration tests under ${script_prefix} ...\n"

    mkdir -p "$OTTER_TRACE_PATH"
    for file in ${script_prefix}/integration-tests/*; do
        echo "Running test: ${file}"
        export OTTER_TRACE_NAME="${file##*/}"
        echo " ╔═══ << ${OTTER_TRACE_NAME} >> ═══"
        $file 2>/dev/null | while IFS= read -r line; do
            echo " ║ $line"
        done
        echo " ╚═══"
    done
    printf "Integration tests completed.\n"
}

__list() {
    find "$OTTER_TRACE_PATH" -type f -name *.otf2
}

case "$1" in
    run)
        __run
        ;;
    list)
        __list
        ;;
    *)
        echo "Usage: $0 {run|list}"
        exit 1
        ;;
esac
