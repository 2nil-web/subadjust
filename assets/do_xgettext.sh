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
