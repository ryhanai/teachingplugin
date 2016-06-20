#!/bin/bash

DBNAME=teachingDB.sqlite3

rm -i ${DBNAME}

echo 'creating tables for database ...'
sqlite3 ${DBNAME} < define_tables.sql
echo 'done'

