#!/bin/bash

function RESsvg2h () {
  fne=${1%.*}
  fnd=${fne##*/}
  vnm=${fnd%.*}_svg_data
  def=${vnm^^}_H
  echo -ne "#ifndef ${def}\n#define ${def}\nconst char* ${vnm} = "
  sed 's/"/\\"/g;s/\(.*\)/"\1"/' "$1" | tr -d '\n' | sed 's/$/;/;s/""/" \\\n "/g'
  echo -ne "\n#endif /* ${def} */"
}

function svg2h () {
  fne=${1%.*}
  fnd=${fne##*/}
  vnm=${fnd%.*}_svg_data
  def=${vnm^^}_H
  echo -ne "#ifndef ${def}\n#define ${def}\nconst char* ${vnm} = R\"SVG("
  cat "$1"
  echo ')SVG";'
  echo -ne "\n#endif /* ${def} */"
}

svg2h "$1"
