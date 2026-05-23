#!/bin/bash

for i in *.srt
do
  ns=$(dos2unix -q -O "$i" | grep -E ^[0-9]+$ | wc -l)
  echo "$ns $i"
done
