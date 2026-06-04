
SRC_DIR=src
UNAME=$(shell uname)

# OS is defined only under Windows
ifeq (${OS},Windows_NT)
SYS_VER=${OS}_$(shell powershell -Command '(Get-WmiObject -class Win32_OperatingSystem).Version')
ECHO=/bin/echo
PATH:=/ucrt64/bin:${PATH}
else # If not defined then set it to "uname -s" value
ifeq (${OS},)
OS=$(shell uname -s)
ECHO=/bin/echo
endif
endif

# Available at least under Fedora, Debian, Ubuntu, Arch and even under msys2
OS_ID=$(shell sed -n 's/^ID=//p' /etc/os-release)

ifeq ($(OS),Linux)
ifeq ($(OS_ID),arch)
SYS_VER=Arch_Linux_$(shell sed -n 's/^VERSION_ID=//p' /etc/os-release)
else
ifeq ($(OS_ID),opensuse-tumbleweed)
SYS_VER=openSUSE_Tumbleweed_$(shell sed -n 's/^VERSION_ID=//p' /etc/os-release)
else
SYS_VER=$(shell lsb_release -irs | sed 'N;s/\n/_/' | sed 's/ /_/g')
endif
endif

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown')
DECORATION=
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown')
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
PLATFORM="${SYS_VER} $(shell uname -m)"

PREFIX=subadjust
SRCS=${PREFIX}_ui.cpp ${PREFIX}.cpp themes.cpp utils.cpp subs.cpp Fl_Time_Input.cpp Fl_Time_Picker.cpp file_features.cpp edit_features.cpp log.cpp pref.cpp export.cpp options.cpp place.cpp fonts.cpp my_ask.cpp
SRCS:=$(addprefix  ${SRC_DIR}/,${SRCS})

ifeq ($(findstring NT-, $(UNAME)),)
#FLTK_DIR=/opt/fltk/1.5
FLTK_DIR=/opt/fltk/1.4.5
EXEXT=
TARGET_DIR=build/gcc/linux
ifeq ($(BUILD_SYS),)
BUILD_SYS=gcc
endif
MAGICK=convert
else
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/18/Community/MSBuild/Current/Bin/amd64/MSBuild.exe	
MAGICK=magick
EXEXT=.exe
FLTK_DIR=/c/fltk/1.4.5
TARGET_DIR=build/msvc/win/x64/Release
ifeq ($(BUILD_SYS),)
BUILD_SYS=msvc
endif
endif

ifeq ($(MAKECMDGOALS),gcc)
undefine MSBUILD
MAGICK=magick
EXEXT=.exe
FLTK_DIR=/ucrt64
TARGET_DIR=build/gcc/win
BUILD_SYS=gcc
ifeq ($(MSYSTEM),CLANG64)
CPPFLAGS += -I /clang64/include/c++
CXX=clang++
endif
LDFLAGS += -static -g
#LDFLAGS += -static-libgcc -static-libstdc++
CPPFLAGS += -DUNICODE -D_UNICODE 
endif

CPPFLAGS += -I src

FLTK_CONFIG=${FLTK_DIR}/bin/fltk-config
FLUID=${FLTK_DIR}/bin/fluid
PATH:=${FLTK_DIR}/bin:${PATH}
TARGET=${TARGET_DIR}/${PREFIX}${EXEXT}

.PHONY: FORCE

all : ${SRC_DIR}/${PREFIX}_ui.cpp ${TARGET} locale/fr/LC_MESSAGES/subadjust.mo locale/es/LC_MESSAGES/subadjust.mo

locale/fr/LC_MESSAGES/subadjust.mo : src/locale/fr/subadjust.po
	@mkdir -p $(@D)
	msgfmt --output-file=$@ $<

locale/es/LC_MESSAGES/subadjust.mo : src/locale/es/subadjust.po
	@mkdir -p $(@D)
	msgfmt --output-file=$@ $<

gcc : ${TARGET}

assets/QuickDoc.jpg : assets/QuickDoc.svg
	${MAGICK} $< $@

GXX_COMPILE=/ucrt64/bin/g++.exe -std=c++23 -Wall -pedantic -Wextra -Wno-comment  -I/ucrt64/include -march=nocona -msahf -mtune=generic -O2 -pipe -Wp,-D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wp,-D__USE_MINGW_ANSI_STDIO=1 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUNICODE -D_UNICODE  -I src -static -g 
GXX_LINK_OPT=-mwindows /ucrt64/lib/libfltk_images.a /ucrt64/lib/libjpeg.a /ucrt64/lib/libpng.a /ucrt64/lib/libz.a /ucrt64/lib/libfltk_gl.a -lglu32 -lopengl32 /ucrt64/lib/libfltk.a -lole32 -luuid -lcomctl32 -lws2_32 -lwinspool -lintl -liconv
font_gui${EXEXT} : assets/font_gui.cpp
	${GXX_COMPILE} assets/font_gui.cpp ${GXX_LINK_OPT} -o font_gui.exe

font_list${EXEXT} : assets/font_list.cpp
	${GXX_COMPILE} assets/font_list.cpp ${GXX_LINK_OPT} -o font_list${EXEXT}

sunset_calculator${EXEXT} : assets/sunset_calculator.cpp
	${GXX_COMPILE} assets/sunset_calculator.cpp ${GXX_LINK_OPT} -o sunset_calculator${EXEXT}

gen_subs${EXEXT} : assets/gen_subs.cpp
	${GXX_COMPILE} assets/gen_subs.cpp ${GXX_LINK_OPT} -o gen_subs${EXEXT}


SETUP_PKG=${PREFIX}-${VERSION}-${SYS_VER}.zip

assets/${SETUP_PKG} : README.md ${TARGET}
	@( strip ${TARGET} | true  ) >/dev/null 2>&1
	@( upx ${TARGET} | true  ) >/dev/null 2>&1
	@mkdir -p assets/setup
	pandoc -V geometry:paperwidth=210mm -V geometry:paperheight=297mm -V geometry:margin=1cm -o assets/setup/README.pdf README.md
	@cp -R locale assets/setup
	@cp ${TARGET} assets/setup
ifeq (${OS},Windows_NT)
	@cp ${TARGET_DIR}/*.dll assets/setup
endif
	@cd assets/setup && zip -rq ../${SETUP_PKG} .
	@echo "Package assets/${SETUP_PKG} is ready"
	@rm -rf assets/setup

setup : assets/${SETUP_PKG}

deliv : assets/${SETUP_PKG}
	@echo "Delivering it to github."
	@./assets/github_release.sh $<


# FLUID file rules
FL_FLUID_DEP=$(shell sed -n 's?^#include "\(.*\)"?src/\1?p' ${SRC_DIR}/Fl_Fluid.h | tr '\n' ' ')
#${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_ui.cpp : assets/svg_icons/*.svg ${SRC_DIR}/Fl_Hover.H ${SRC_DIR}/Fl_I18n.H ${SRC_DIR}/Fl_I18n.H ${SRC_DIR}/Fl_Time_Input.H ${SRC_DIR}/${PREFIX}_ui.fl
${SRC_DIR}/${PREFIX}_ui.cpp : assets/svg_icons/*.svg ${SRC_DIR}/Fl_Fluid.h ${FL_FLUID_DEP} ${SRC_DIR}/${PREFIX}_ui.fl
	@echo "Fluid code generation"
	@sed -i 's?image {\.\.\(.*\)/assets?image {../assets?' src/subadjust_ui.fl
	cd ${SRC_DIR} && ${FLUID} -c -o .cpp ${PREFIX}_ui.fl


${SRC_DIR}/${PREFIX}.cpp ${SRC_DIR}/file_features.cpp ${SRC_DIR}/edit_features.cpp ${SRC_DIR}/pref.cpp : ${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_icon.h ${SRC_DIR}/${PREFIX}.ico

${SRC_DIR}/${PREFIX}.ico : ${SRC_DIR}/${PREFIX}.svg
	${MAGICK} -density 256x256 -background none $< -define icon:auto-resize=128,96,64,48,32,16 -colors 256 $@
#	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@


ifeq ($(BUILD_SYS),msvc)
OBJS=$(SRCS:.cpp=.obj)
#OBJS:=$(addprefix  ${TARGET_DIR}/,${OBJS})
OBJS:=$(subst ${SRC_DIR}/,${TARGET_DIR}/,${OBJS})
MSVC_SLN=${PREFIX}.slnx
${TARGET} : ${SRC_DIR}/app_info_check.txt ${SRC_DIR}/app_info.h ${SRCS}
	@echo "Building with Visual Studio"
	@${MSBUILD} ${MSVC_SLN} -p:Configuration=Release
	@echo "Build of ${TARGET} with Visual Studio is OK"
	@echo "${SRCS}"
else
OBJS=$(SRCS:.cpp=.o)
#OBJS:=$(addprefix  ${TARGET_DIR}/,${OBJS})
OBJS:=$(subst ${SRC_DIR}/,${TARGET_DIR}/,${OBJS})
CC  = $(shell ${FLTK_CONFIG} --cc)
CXX = $(shell ${FLTK_CONFIG} --cxx)
CXXFLAGS += -std=c++23
CXXFLAGS += -Wall -pedantic -Wextra -Wno-comment # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
CXXFLAGS += $(shell ${FLTK_CONFIG} --use-gl --use-images --cxxflags )
# Optim
#CXXFLAGS += -Oz
#LDFLAGS += -fno-rtti
#LDLIBS = $(shell ${FLTK_CONFIG} --use-gl --use-images --ldflags )
LDLIBS += $(shell ${FLTK_CONFIG} --use-gl --use-images --ldstaticflags | sed 's/.dll//g' )
ifeq (${OS},Windows_NT)
LDLIBS += -lintl -liconv
endif
LINK     = $(CXX)
${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
	@echo "Build of ${TARGET} with G++ is OK"
	@echo "${SRCS}"
endif

${TARGET_DIR}/${PREFIX}.d ${TARGET_DIR}/options.d : ${SRC_DIR}/app_info.h
	@echo "OK header dependency for $@"

ALL_SRCS=$(wildcard ${SRC_DIR}/*.cpp) $(wildcard assets/*.cpp) $(wildcard ${SRC_DIR}/*.h) $(wildcard ${SRC_DIR}/*.H)
format :
	@echo "Formatting with clang, the following files: ${ALL_SRCS}"
	@clang-format -style="{ BasedOnStyle: Microsoft, ColumnLimit: 256, IndentWidth: 2, TabWidth: 2, UseTab: Never }" --sort-includes -i ${ALL_SRCS}
# , AllowShortIfStatementsOnASingleLine: AllIfsAndElse 

# Génération du app_info.h intégré dans l'appli
${SRC_DIR}/app_info.h : ${SRC_DIR}/app_info_check.txt
	@${ECHO} -e "Building C++ header $@"
	@${ECHO} -e "#ifndef APP_INFO_H\n#define APP_INFO_H\nstruct\n{\n  std::string name, version, decoration, commit, created_at, platform;\n} app_info = {\"${PREFIX}\", \"${VERSION}\", \"${DECORATION}\", \"${COMMIT}\", \"${ISO8601}\", \"${PLATFORM}\"};\n#endif" >$@
	dos2unix $@

# Génération du app_info.json intégré dans le paquetage
${SRC_DIR}/app_info.json : ${SRC_DIR}/app_info_check.txt
	@${ECHO} -e "Building json file $@"
	@${ECHO} -e '{ "name":"${PREFIX}", "version":"${VERSION}", "decoration":"${DECORATION}", "commit":"${COMMIT}","created_at":"${ISO8601}, "platform":"${PLATFORM}" }' >$@
	dos2unix $@

# Pour regénérer silencieusement app_info.h et app_info.json dès qu'un des champs app_info ou decoration ou commit, est modifié.
${SRC_DIR}/app_info_check.txt : FORCE
	@${ECHO} -e "Version:${VERSION}, decoration:${DECORATION}, commit:${COMMIT}, platform:${PLATFORM}" >$@.new
	@-( if [ ! -f $@ ]; then cp $@.new $@; sleep 0.4; fi )
	@-( if diff $@.new $@ >/dev/null 2>&1; then rm -f $@.new; else mv -f $@.new $@; rm -f ${PREFIX}.iss ${PREFIX}-standalone.iss; fi )

cfg : 
	@echo "ECHO: ${ECHO}"
	@echo "Building TARGET [${TARGET}] for system [${UNAME}] with built tool [${BUILD_SYS}]"
	@echo "fltk tools come from ${FLTK_DIR}"
	@which ${FLTK_CONFIG}
	@echo "CPPFLAGS: ${CPPFLAGS}"
	@echo "LINK.cc: ${LINK.cc}"
	@echo "SRCS: ${SRCS}"
	@echo "OBJS: ${OBJS}"
	@echo "LOADLIBES: ${LOADLIBES}"
	@echo "LDLIBS: ${LDLIBS}"

help :
	@echo "$(shell tput bold; tput smul)What to do to build and/or deliver a new version of ${PREFIX} ?$(shell tput sgr0)"
	@echo "  I - $(shell tput bold)Under Windows the app may be compiled with gcc and/or Visual Studio.$(shell tput sgr0)"
	@echo "    1. Under the Windows explorer, double-click on the 'subadjust.sln' file, if you have Visual Studio already installed."
	@echo "    2. Under MSYS2 bash, by simply running the 'make' command and if Visual Studio is set according to the content of the MSBUILD variable then it will be the default compiler."
	@echo "    3. If you have a correctly set Visual Studio configuration but want to force the use of gcc, then run 'make gcc' to build the binaries with gcc."
	@echo " II - $(shell tput bold)On any other build system (Linux) gcc is the default.$(shell tput sgr0)"
	@echo "III - $(shell tput bold)Prepare build and delivery$(shell tput sgr0)"
	@echo "$(shell tput sitm)  If, for instance, you are in a develop branch and want to merge its last commit to master, then do the following:$(shell tput sgr0)"
	@echo "    1-Display commit  : git log -1 --oneline # or git show --oneline -s"
	@echo "    2-Set a variable  : ck=\044(git log -1 --oneline | sed 's/ .*//')"
	@echo "    3-Go to master    : git checkout master"
	@echo "    4-Retrieve commit : git cherry-pick \044ck"
	@echo "    5-Back to develop : git checkout develop # To go on with your developmements, if needed."
	@echo " IV - $(shell tput bold)Build and delivery steps$(shell tput sgr0)"
	@echo "    1-Check remote tags : git ls-remote --tags origin"
	@echo "    2-Check local tags  : git describe --abbrev=0 --tags"
	@echo "    3-New version tag   : git tag -a X.Y.Z-nom_de_la_prerelease -m 'commentaire' # De préférence un tag annoté (-a)."
	@echo "    4-Push a tag        : git push --tags"
	@echo "    5-Build application : make ... (the only mandatory step before delivery)"
	@echo "    6-Build the setup   : make setup # (Only a zip creation for now, could be improved by creating a 'real' Windows and/or Linux setup)"
	@echo "    7-Delivery          : make deliv (may be run directly after 'make ...' step."
	@echo "  V - $(shell tput bold)For versioning$(shell tput sgr0)"
	@echo "    Respect Semantic Versioning in the delivery tags (see https://semver.org)."
	@echo "    A short example according to the pattern 'MAJOR.MINOR.PATCH-pre_release+metadata' might be something like : 0.1.1-pre_alpha_release+Latest.bug.fixes.on.sha.965c58e"
	@echo "  VI - $(shell tput bold)From time to time$(shell tput sgr0)"
	@echo "     Run: 'scripts/update_webviews.sh' to check the versions of webview and eventually MS Webview2."
	@echo " VII - $(shell tput bold)For more details$(shell tput sgr0)"
	@echo "     Have a look at the following files : header.mk, Makefile, rules.mk, webapp.vcxproj and webapp-console.vcxproj."

clean :
	 rm -rf build/msvc build/gcc
	 rm -f $(OBJS) $(SRCS:.cpp=.d) ${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_ui.cpp ${SRC_DIR}/*_icon.h ${SRC_DIR}/app_info_check.txt ${SRC_DIR}/app_info.h ${SRC_DIR}/app_info.json *~

ifneq ($(MAKECMDGOALS),clean)
# Implicit rule for building dep file from .cpp
${TARGET_DIR}/%.d: ${SRC_DIR}/%.cpp
	@echo Checking header dependencies from $<
	@mkdir -p ${TARGET_DIR}
	@$(COMPILE.cpp) -isystem /usr/include -MM $< >> tmp.d$$
	@test -s tmp.d$$ && ( echo -n "${TARGET_DIR}/" > $@; cat tmp.d$$ >> $@ )
	@rm -f tmp.d$$

%.png : %.svg
	inkscape --export-type=png --export-background-opacity=0 $<

UC = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')
#	sed 's/"/\\"/g;s/\(.*\)/"\1" \\//' -e '$ s/.$//' $2 >>$4; 
define svg2h
	${ECHO} -ne '#ifndef $3\n#define $3\nconst char* $1_svg_data=R"SVG(' >$4; \
	cat $2 >>$4; \
	${ECHO} -ne ")SVG\";\n#endif /* $3 */\n\n" >> $4
endef

${SRC_DIR}/%_icon.h : ${SRC_DIR}/%.svg
	@$(call svg2h,$*,$<,$(call UC,$*)_SVG_DATA_H,$@)

ifeq ($(BUILD_SYS),gcc)
${TARGET_DIR}/%.o: ${SRC_DIR}/%.cpp
	@mkdir -p ${TARGET_DIR}
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
endif
# Inclusion of the dependency files '.d'
ifdef OBJS
-include $(OBJS:.o=.d)
endif
endif

