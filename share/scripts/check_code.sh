#!/bin/bash

set -ue

DIR=../../Teaching

function check_tab()
{
    echo 'TABs used in source code:'
    find $DIR -name "*.h" -o -name "*.cpp" | xargs grep -n $'\t'
    return 0
}

function check_character_code()
{
    echo 'character code other than UTF-8 or ASCII:'
    find $DIR -name "*.h" -o -name "*.cpp" | xargs nkf --guess | grep -v UTF-8 | grep -v ASCII
    return 0
}

function check_line_feed()
{
    echo 'line feed code (CR+LF):'
    find $DIR -name "*.h" -o -name "*.cpp" | xargs nkf --guess | grep CRLF
    return 0
}

function check_all()
{
    check_tab
    echo ''
    check_character_code
    echo ''
    check_line_feed
    return 0
}

while getopts "tcla" opt; do
    case "$opt" in
	t)
	    check_tab
	    ;;
	c)
	    check_character_code
	    ;;
	l)
	    check_line_feed
	    ;;
	a)
	    check_all
	    ;;
    esac
done

exit 0
