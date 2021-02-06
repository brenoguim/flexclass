#!/bin/bash
# Script used to parse coverage information and generate a coverage report
# using genhtml and lcov. Runs in the current directory.
# $1: Directory where to put the HTML report
set -e
DESTINATION="$1"
if [[ -z "$DESTINATION" ]]; then
    echo -e """ Usage:
    $(basename $0) <destination>
"""
    exit 1
fi

if ! which lcov >/dev/null; then
    echo "ERROR: lcov is required to use this script. Install it with \"sudo apt install lcov\"."
    exit 1
fi

if ! which genhtml >/dev/null; then
    echo "ERROR: genhtml is required to use this script. Install it with \"sudo apt install lcov\"."
    exit 1
fi

lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info '*/flexclass/external/*' --output-file coverage.info
lcov --remove coverage.info '*/flexclass/tests/*' --output-file coverage.info

genhtml coverage.info --output-directory "$DESTINATION"
