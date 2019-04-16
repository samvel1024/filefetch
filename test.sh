#!/usr/bin/env bash

set -e

cd $1
mkdir -p serve
echo "1234567890" >> serve/10byte.txt
echo "1234567890qwertyuiop" >> serve/20byte.txt

