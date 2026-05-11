#!/bin/bash

# Exemple avec subadjust.cpp depuis la racine du dépôt :
# Génération initiale du .pot
# mkdir -p src/locale
# xgettext --keyword=_ --language=C++ --add-comments --sort-output -o src/locale/subadjust.pot src/subadjust.cpp
# xgettext --keyword=_ --language=C++ --add-comments --sort-output -o src/locale/options.pot src/options.cpp
# Et custo du .pot : Project-Id-Version, Report-Msgid-Bugs-To and charset
#
# Génération du .po
# mkdir -p src/locale/fr
# msginit --input=src/locale/subadjust.pot --locale=fr --output=src/locale/fr/subadjust.po
# Ajout des autres .pot et traduction du tout
#
# Génération du .mo
# mkdir -p locale/fr/LC_MESSAGES
# msgfmt --output-file=locale/fr/LC_MESSAGES/subadjust.mo src/locale/fr/subadjust.po
#
# for i in *.{h,cpp}
#do
#  echo $i
#  xgettext --keyword=_ --language=C++ --add-comments --sort-output -o "locale/${i/.*}.pot" "$i"
#done

#  xgettext --keyword=_ --language=C++ --add-comments --sort-output -o src/locale/all.pot src/*.{cpp,h}


# ==> S'assurer que subadjust_ui.cpp et .h soit créés
cd src
while read i
do
  fcod="UTF-8"
  if file $i >/dev/null | grep ASCII; then
    fcod=ASCII
  fi

  echo "$i == ${fcod}"
  xgettext --keyword=_  --keyword=_S  --keyword=gettext --language=C++ --from-code=${fcod} --add-comments -o "locale/${i/.*}.pot" "$i"
done < <(grep -sl -e '_(' -e '_S(' *.{h,cpp})

cd locale
cat *.pot | grep msgid | sort -u
grep msgid fr/subadjust.po | sort -u
Comparer dans Excel le '^cat' avec le '^grep' pour repérer dans le cat ce qui est manquant dans le grep
Compléter fr/subadjust.po avec ce répérés comme manquant dans Excel.
