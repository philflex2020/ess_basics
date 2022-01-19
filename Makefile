PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")


# we'll use this to determine if we build or not.
# to build
# export PATH=/usr/local/gcc-9.2.0/bin:$PATH
# make
# to run  
# export LD_LIBRARY_PATH=/usr/local/gcc-9.2.0/lib64:$LD_LIBRARY_PATH
# /usr/local/bin/dnp3_interface/dnp3_outstation config/<some_config>
# dirname `which g++`

 
BINS = dnp3_utils
BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
#SLIBS_DIR = /usr/local/gcc-9.2.0/lib64/
SLIBS_DIR = /usr/local/gcc-9.1.0/lib64/
WGPP = $(shell expr `which g++`)

XLIBS_DIR = $(shell expr `dirname $(WGPP)`)/../lib64


INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)


CPPFLAGS += -std=c++17 -g
CPPFLAGS += -Wall -pedantic -Werror

# BUILD_MODE = debug
ifeq ($(BUILD_MODE),debug)
	CPPFLAGS += -g -DFPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPFLAGS += -DFPS_TEST_MODE
	LIBS += -lgtest -lgmock
else
	BUILD_MODE=release
	CPPFLAGS += -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIST = $(addprefix $(BUILD_DIR), $(BINS))
LIBS = -L/usr/lib -L$(OBJ_DIR) -L$(XLIBS_DIR) -L$(LIBS_DIR) -L/usr/lib64/ -Wl,--build-id -Wl,-rpath,$(XLIBS_DIR):$(LIBS_DIR):/usr/lib64/ -lstdc++ -lcjson -lfims 
#-lopendnp3 -lopenpal -lasiopal -lasiodnp3 -lpthread
LIBS += -lssl -lssl3
LIBS += -lcrypto
LIBS += -lsimdjson

BLIST = build
BLIST += $(OBJ_DIR)libdmap.so 
BLIST += $(LIST)

GCC_GT_4805 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \> 40805)
     BLIST = okbuild
     BLIST += build 
     BLIST += $(OBJ_DIR)libdmap.so 
     BLIST += $(LIST)


#all:	
all:	$(BLIST)

$(OBJ_DIR)libdmap.so:	$(OBJ_DIR)libdmap.o 
	$(CXX) -shared -o $@ $^ $(INCLUDES) $(LIBS)

$(OBJ_DIR)libdmap.o:	$(PROJECT_ROOT)src/dnp3_utils.cpp $(PROJECT_ROOT)include/dnp3_utils.h
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)


$(BUILD_DIR)dnp3_utils: $(OBJ_DIR)dnp3_utils_main.o 
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap

 
$(OBJ_DIR)%.o: $(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $< $(INCLUDES_BUILD)

.PHONY: build nobuild okbuild
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)
nobuild:
	mkdir -p NO_$(BUILD_DIR) $(OBJ_DIR)
okbuild:
	mkdir -p OK_$(BUILD_DIR) $(OBJ_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)dnp3_utils

.PHONY: install
install:
	cp $(BUILD_DIR)dnp3_utils $(BIN_DIR)
	cp $(OBJ_DIR)*.so $(LIBS_DIR)
