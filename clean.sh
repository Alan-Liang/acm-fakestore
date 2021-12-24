#!/bin/bash

DATABASES=(author_books.dat books.dat keyword_books.dat name_books.dat users.dat log_cmd.bin log_trade.bin)

for db in ${DATABASES[@]}; do rm -f $db; done

rm -f data/*
