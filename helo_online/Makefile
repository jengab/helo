HELO_COMMON:=../helo_common
CPP_FILES:=$(wildcard src/source/*.cpp)
COMMON_CPP_FILES:=$(wildcard $(HELO_COMMON)/src/source/*.cpp)
SRC_PATTERN:=src/source/%.cpp
COMMON_SRC_PATTERN:=$(HELO_COMMON)/src/source/%.cpp
3PP_SRC:=../3pp
3PP_BUILD:=$(3PP_SRC)/build
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
COMMON_OBJ_FILES:=$(addprefix $(HELO_COMMON)/obj/,$(notdir $(COMMON_CPP_FILES:.cpp=.o)))
LD_DIRS:=-L $(3PP_BUILD)/sqlitecpp_build/sqlite3 -L $(3PP_BUILD)/sqlitecpp_build
LD_FLAGS:=-lSQLiteCpp -lsqlite3
INCLUDE_DIRS:=-I $(3PP_SRC)/SQLiteCpp/include -I $(3PP_SRC)/SQLiteCpp/sqlite3 -I $(3PP_BUILD)/pugi_build -I src/header -I $(HELO_COMMON)/src/header

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG -std=c++0x -Wall
else
	CXX_FLAGS:=-O3 -std=c++0x
endif

ifneq (,$(findstring Windows, $(OS)))
	LD_FLAGS+=-lssp -lws2_32 -lboost_system-mgw63-mt-1_63 -liconv
	INCLUDE_DIRS+=-I D:\boost
	LD_DIRS+=-L D:/boost/stage/lib
	CMAKE_ARGS:=-G "MSYS Makefiles"
	EXEC_NAME:=helo_online.exe
else
	LD_FLAGS+=-lpthread -ldl -lboost_system
	CMAKE_ARGS:=-G "Unix Makefiles"
	EXEC_NAME:=helo_online
ifeq ($(shell uname -s), Darwin)
	INCLUDE_DIRS+=-I /opt/local/include
endif
endif

.PHONY: clean
.PHONY: clean_3pp
.PHONY: clean_all
.PHONY: test

$(EXEC_NAME): $(3PP_BUILD)/pugi_build/pugixml.o obj/ $(OBJ_FILES) $(COMMON_OBJ_FILES)
	$(eval PUGI_OBJS=$(shell ls $(3PP_BUILD)/pugi_build/*.o))
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) $(COMMON_OBJ_FILES) $(PUGI_OBJS) -o $@ $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

$(HELO_COMMON)/obj/%.o:
	cd $(HELO_COMMON); make DBG=$(DBG) COVERAGE=$(COVERAGE) $(MAKECMDGOALS)

obj/%.o: $(SRC_PATTERN) $(3PP_BUILD)/pugi_build/pugixml.cpp $(3PP_BUILD)/sqlitecpp_build
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

obj/:
	mkdir -p obj

$(3PP_BUILD)/pugi_build/pugixml.o: $(3PP_BUILD)/pugi_build/pugixml.cpp
	$(CXX) $(CXX_FLAGS) -c $(shell ls $(3PP_BUILD)/pugi_build/*.cpp) -o $@

$(3PP_BUILD)/pugi_build/pugixml.cpp: $(3PP_SRC)
	mkdir -p $(3PP_BUILD)/pugi_build
	find $(3PP_SRC)/pugixml/src -regex ".*\.\(hpp\|cpp\)" -exec cp {} $(3PP_BUILD)/pugi_build \;
	sed -i '/\/\/ #define PUGIXML_WCHAR_MODE/c\#define PUGIXML_WCHAR_MODE' $(3PP_BUILD)/pugi_build/pugiconfig.hpp

$(3PP_BUILD)/sqlitecpp_build: $(3PP_SRC)
	mkdir -p $(3PP_BUILD)/sqlitecpp_build
	cd $(3PP_BUILD)/sqlitecpp_build; cmake $(CMAKE_ARGS) ../../SQLiteCpp; cmake --build .

$(3PP_SRC):
	mkdir -p $(3PP_SRC)
	git submodule update --init --force --recursive

clean:
	rm -rf obj
	rm -rf $(HELO_COMMON)/obj
	rm -f $(EXEC_NAME)

clean_3pp:
	rm -rf $(3PP_BUILD)

clean_all: clean clean_3pp
	rm -rf $(3PP_SRC)
