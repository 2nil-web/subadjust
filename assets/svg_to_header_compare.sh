#!/bin/bash

/mingw64/bin/magick reset.svg reset.svg.to.xpm
wc -c reset.svg.to.xpm
/mingw64/bin/magick reset.svg.to.xpm reset.svg.to.xpm.h
wc -c reset.svg.to.xpm.h
/mingw64/bin/magick reset.svg reset.svg.to.h
wc -c reset.svg.to.h

/mingw64/bin/magick reset.svg reset.svg.to.png
wc -c reset.svg.to.png
/mingw64/bin/magick reset.svg.to.png reset.svg.to.png.xpm
wc -c reset.svg.to.png.xpm
/mingw64/bin/magick reset.svg.to.png reset.svg.to.png.h
wc -c reset.svg.to.png.h

/mingw64/bin/magick reset.svg reset.svg.to.pnm
wc -c reset.svg.to.pnm
/mingw64/bin/magick reset.svg.to.png reset.svg.to.pnm.xpm
wc -c reset.svg.to.pnm.xpm
/mingw64/bin/magick reset.svg.to.png reset.svg.to.pnm.h
wc -c reset.svg.to.pnm.h

#rm reset.svg.to.*

