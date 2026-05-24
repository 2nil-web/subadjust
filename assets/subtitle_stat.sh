#!/bin/bash

TZ=utc

function hms_to_ms () {
  IFS=':' read -r h m ms <<<"$*"
  [[ $h =~ ^0*(.+)$ ]] && h=${BASH_REMATCH[1]}
  [[ $m =~ ^0*(.+)$ ]] && m=${BASH_REMATCH[1]}
  [[ $ms =~ ^0*(.+)$ ]] && ms=${BASH_REMATCH[1]}

  echo $((h*3600000+m*60000+ms))
}

echo "num;start;end;dur;dur/sub;file"
for i in *.srt
do
  tss=$(dos2unix -q -O "$i" | sed -n 's/^\(..:..:..\),\(...\).*-->.*\(..:..:..\),\(...\).*$/\1\2 \3\4/p')
  n=$(wc -l <<<"$tss")
  read ts1 te1 ts2 te2 <<<$(sed '1b;$b;d' <<<"$tss" | tr '\n' ' ')
  ms1=$(hms_to_ms "$ts1")
  ms2=$(hms_to_ms "$te2")
  let d=ms2-ms1
  let ds=d/n
  echo "$n;$ms1;$ms2;$d;$ds;$i"
done
