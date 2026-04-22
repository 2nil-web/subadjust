
SRC_DIR=src
UNAME=$(shell uname)

# OS is defined only under Windows
ifeq (${OS},Windows_NT)
SYS_VER=${OS}_$(shell powershell -Command '(Get-WmiObject -class Win32_OperatingSystem).Version')
ECHOE=echo -e
PATH:=/ucrt64/bin:${PATH}
else # If not defined then set it to "uname -s" value
ifeq (${OS},)
OS=$(shell uname -s)
ECHOE=echo
endif
endif

# Available at least under Fedora, Debian, Ubuntu, Arch and even under msys2
OS_ID=$(shell sed -n 's/^ID=//p' /etc/os-release)

ifeq ($(OS),Linux)
ifeq ($(OS_ID),arch)
SYS_VER=Arch_Linux_$(shell sed -n 's/^VERSION_ID=//p' /etc/os-release)
else
SYS_VER=$(shell lsb_release -irs | sed 'N;s/\n/_/' | sed 's/ /_/g')
endif
endif

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown')
COPYRIGHT=Copyright © D. LALANNE - MIT License - No warranty of any kind.
DECORATION=
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown')
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
PLATFORM="${SYS_VER} $(shell uname -m)"

PREFIX=subadjust
SRCS=${PREFIX}_ui.cpp ${PREFIX}.cpp themes.cpp utils.cpp subs.cpp Fl_Time_Input.cpp Fl_Time_Picker.cpp file_features.cpp edit_features.cpp log.cpp prefs.cpp options.cpp feed.cpp place.cpp
SRCS:=$(addprefix  ${SRC_DIR}/,${SRCS})

ifeq ($(findstring NT-, $(UNAME)),)
#FLTK_DIR=/opt/fltk-1.5
FLTK_DIR=/opt/fltk-1.4.4
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
FLTK_DIR=/c/fltk-1.4.4
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

all : ${TARGET} #assets/QuickDoc.jpg

gcc : ${TARGET}

assets/QuickDoc.jpg : assets/QuickDoc.svg
	${MAGICK} $< $@


SETUP_PKG=${PREFIX}-${VERSION}-${SYS_VER}.zip

assets/${SETUP_PKG} : README.md ${TARGET}
	@( strip ${TARGET} | true  ) >/dev/null 2>&1
	@( upx ${TARGET} | true  ) >/dev/null 2>&1
	@mkdir -p assets/setup
	@pandoc -V geometry:paperwidth=210mm -V geometry:paperheight=297mm -V geometry:margin=1cm -o assets/setup/README.pdf README.md
	@cp ${TARGET} assets/setup
	@cd assets/setup && zip -rq ../${SETUP_PKG} .
	@echo "Package assets/${SETUP_PKG} is ready"
	@rm -rf assets/setup

setup : assets/${SETUP_PKG}

deliv : assets/${SETUP_PKG}
	@echo "Delivering it to github."
	@./assets/github_release.sh $<

# FLUID file rules
${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_ui.cpp : ${SRC_DIR}/${PREFIX}_ui.fl
	@echo "Fluid Gen"
	cd ${SRC_DIR} && ${FLUID} -c -o .cpp ${PREFIX}_ui.fl

${SRC_DIR}/${PREFIX}.cpp ${SRC_DIR}/file_features.cpp ${SRC_DIR}/edit_features.cpp ${SRC_DIR}/prefs.cpp : ${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_icon.h ${SRC_DIR}/${PREFIX}.ico

#	@fold -w 253 ${PREFIX}.svg | sed -e 's/"/\\"/g;s/\(.*\)/"\1" \\/' >>${PREFIX}_icon.h
${SRC_DIR}/${PREFIX}_icon.h : ${SRC_DIR}/${PREFIX}.svg
	@echo -n "const char *svg_data=" >$@
	@sed -e 's/"/\\"/g;s/\(.*\)/"\1" \\/' $< >>$@
	@echo ";" >>$@

${SRC_DIR}/${PREFIX}.ico : ${SRC_DIR}/${PREFIX}.svg
	${MAGICK} -density 256x256 -background none $< -define icon:auto-resize=128,96,64,48,32,16 -colors 256 $@
#	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@


ifeq ($(BUILD_SYS),msvc)
OBJS=$(SRCS:.cpp=.obj)
#OBJS:=$(addprefix  ${TARGET_DIR}/,${OBJS})
OBJS:=$(subst ${SRC_DIR}/,${TARGET_DIR}/,${OBJS})
MSVC_SLN=${PREFIX}.slnx
${TARGET} : ${SRC_DIR}/app_info_check.txt ${SRC_DIR}/app_info.h ${SRCS}
	@${MSBUILD} ${MSVC_SLN} -p:Configuration=Release
	@echo "${TARGET} OK"
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
LINK     = $(CXX)
${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

${TARGET_DIR}/${PREFIX}.d ${TARGET_DIR}/options.d : ${SRC_DIR}/app_info.h

ALL_SRCS=$(wildcard ${SRC_DIR}/*.cpp) $(wildcard ${SRC_DIR}/*.h)
format :
	@echo "Formatting with clang, the following files: ${ALL_SRCS}"
	@clang-format -style="{ BasedOnStyle: Microsoft, ColumnLimit: 256, IndentWidth: 2, TabWidth: 2, UseTab: Never }" --sort-includes -i ${ALL_SRCS}
# , AllowShortIfStatementsOnASingleLine: AllIfsAndElse 

# Génération du app_info.h intégré dans l'appli
${SRC_DIR}/app_info.h : ${SRC_DIR}/app_info_check.txt
	@${ECHOE} "Building C++ header $@"
	@${ECHOE} "#ifndef APP_INFO_H\n#define APP_INFO_H\nstruct\n{\n  std::string name, version, copyright, decoration, commit, created_at, platform;\n} app_info = {\"${PREFIX}\", \"${VERSION}\", \"${COPYRIGHT}\", \"${DECORATION}\", \"${COMMIT}\", \"${ISO8601}\", \"${PLATFORM}\"};\n#endif" >$@
	dos2unix $@

# Génération du app_info.json intégré dans le paquetage
${SRC_DIR}/app_info.json : ${SRC_DIR}/app_info_check.txt
	@${ECHOE} "Building json file $@"
	@${ECHOE} -e '{ "name":"${PREFIX}", "version":"${VERSION}", copyright:"${COPYRIGHT}", "decoration":"${DECORATION}", "commit":"${COMMIT}","created_at":"${ISO8601}, "platform":"${PLATFORM}" }' >$@
	dos2unix $@

# Pour regénérer silencieusement app_info.h et app_info.json dès qu'un des champs app_info ou copyright ou decoration ou commit, est modifié.
${SRC_DIR}/app_info_check.txt : FORCE
	@${ECHOE} "Version:${VERSION}, copyright:${COPYRIGHT}, decoration:${DECORATION}, commit:${COMMIT}, platform:${PLATFORM}" >$@.new
	@-( if [ ! -f $@ ]; then cp $@.new $@; sleep 0.4; fi )
	@-( if diff $@.new $@ >/dev/null 2>&1; then rm -f $@.new; else mv -f $@.new $@; rm -f ${PREFIX}.iss ${PREFIX}-standalone.iss; fi )

cfg : 
	@echo "Building TARGET [${TARGET}] for system [${UNAME}] with built tool [${BUILD_SYS}]"
	@echo "fltk tools come from ${FLTK_DIR}"
	@echo "CPPFLAGS: ${CPPFLAGS}"
	@echo "LINK.cc: ${LINK.cc}"
	@echo "SRCS: ${SRCS}"
	@echo "OBJS: ${OBJS}"
	@echo "LOADLIBES: ${LOADLIBES}"
	@echo "LDLIBS: ${LDLIBS}"

clean :
	 rm -rf build/msvc build/gcc
	 rm -f $(OBJS) $(SRCS:.cpp=.d) ${SRC_DIR}/${PREFIX}_ui.h ${SRC_DIR}/${PREFIX}_ui.cpp ${SRC_DIR}/app_info_check.txt ${SRC_DIR}/app_info.h ${SRC_DIR}/app_info.json *~

ifneq ($(MAKECMDGOALS),clean)
# Implicit rule for building dep file from .c
${TARGET_DIR}/%.d: ${SRC_DIR}/%.cpp
	@echo Checking header dependencies from $<
	@mkdir -p ${TARGET_DIR}
	@echo -n "${TARGET_DIR}/" > $@
	@$(COMPILE.cpp) -isystem /usr/include -MM $< >> $@

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

