#!/bin/bash

if [ $# -ne 1 ]; then
	echo "1 argument (database name) is required." 1>&2
	exit 1
fi

DBNAME=$1

rm -i ${DBNAME}

echo 'creating tables for database ...'
sqlite3 ${DBNAME} < define_tables.sql
echo 'done'

