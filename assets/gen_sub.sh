#!/bin/bash

# All times are in milliseconds

# When do the subtitles start?
let current_start=6153

# Number of subtitle lines (entre 455 et 2599 dans ma réserve de film VO en mai 2026)
let n_subtitles=RANDOM%2600+400

# Average interval between each subtitle
let interval_average=9400
# Largest interval gap
let interval_gap=60000

# Average length of a subtitle
let length_average=3000
# Largest length gap
let length_gap=10000

prev_fps=""
function ms_to_ts () {
  fps=$(sed 's/\(...\)$/.\1/' <<<${1})
  let h=$(date -d @${fps} +'%H')-1
  date -d @${fps} +"0$h:%M:%S.%3N"
}

# Ecrit un sous-titre au format suivant :
# Numéro_du_sous-titre
# XX:XX:XX,XXX --> XX:XX:XX,XXX
# Toujours les même mots, on s'en fout ...
function write_a_subtitle() {
  num=$1
  let length=RANDOM%length_average+RANDOM%length_gap+300
  let current_end=current_start+length

  echo "$i"
  echo "$(ms_to_ts ${current_start}) --> $(ms_to_ts ${current_end})"
  echo "Toujours les même mots, on s'en fout ..."
  echo

  let current_start=current_start+length+RANDOM%interval_average+RANDOM%interval_gap+300
}


for ((i=1; i < n_subtitles; i++))
do
  write_a_subtitle $i
done
