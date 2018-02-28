CPP_FILES:=$(wildcard src/source/*.cpp)
SRC_PATTERN:=src/source/%.cpp
3PP_SRC:=../3pp
3PP_BUILD:=$(3PP_SRC)/build
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_DIRS:=-L $(3PP_BUILD)/sqlitecpp_build/sqlite3 -L $(3PP_BUILD)/sqlitecpp_build
LD_FLAGS:=-lSQLiteCpp -lsqlite3
INCLUDE_DIRS:=-I $(3PP_SRC)/SQLiteCpp/include -I $(3PP_BUILD)/pugi_build -I src/header

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG -std=c++0x -Wall
else
	CXX_FLAGS:=-O3 -std=c++0x
endif

ifeq (yes, $(COVERAGE))
	CXX_FLAGS:=-g -O0 --coverage -std=c++0x
endif

ifeq ($(MAKECMDGOALS), test)
	INCLUDE_DIRS+=-I ../lest/include -I ../FakeIt/single_header/standalone
	LD_FLAGS+=-lgcov
	CPP_FILES:=$(wildcard src/*/*.cpp)
	OBJ_FILES:=$(filter-out obj/main_offline.o, $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))) $(PUGI_OBJS)
	SRC_PATTERN:=src/*/%.cpp
endif

ifneq (,$(findstring Windows, $(OS)))
	LD_FLAGS+=-lssp
	INCLUDE_DIRS+=-I D:\boost
	CMAKE_ARGS:=-G "MSYS Makefiles"
else
	LD_FLAGS+=-lpthread -ldl
	CMAKE_ARGS:=-G "Unix Makefiles"
ifeq ($(shell uname -s), Darwin)
	INCLUDE_DIRS+=-I /opt/local/include
endif
endif

.PHONY: clean
.PHONY: clean_3pp
.PHONY: clean_all
.PHONY: test

helo_offline: $(3PP_BUILD)/pugi_build/pugixml.o obj/ $(OBJ_FILES)
	$(eval PUGI_OBJS=$(shell ls $(3PP_BUILD)/pugi_build/*.o))
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) $(PUGI_OBJS) -o $@ $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

test: obj/ $(OBJ_FILES) $(TEST_OBJ_FILES)
	rm -f obj/*.gcda
	$(CXX) $(OBJ_FILES) -o test_helo_offline $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)
	./test_helo_offline
ifeq ($(COVERAGE), yes)
	mkdir -p coverage
	lcov -q --base-directory src/source --directory ./obj --no-external -c --output-file coverage/app.info
	genhtml -q --output-directory coverage coverage/app.info
endif

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
	rm -f helo_offline
	rm -f test_helo_offline
	rm -rf coverage

clean_3pp:
	rm -rf $(3PP_BUILD)

clean_all: clean clean_3pp
	rm -rf $(3PP_SRC)
