CPP_FILES:=$(wildcard src/source/*.cpp)
SRC_PATTERN:=src/source/%.cpp
PUGI_PATTERN:=../pugi_build/%.cpp
PUGI_CPPS:=$(addprefix ../pugi_build/,$(notdir $(wildcard ../pugixml/src/*.cpp)))
PUGI_OBJS:=$(addprefix ../pugi_build/,$(notdir $(PUGI_CPPS:.cpp=.o)))
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o))) $(PUGI_OBJS)
LD_DIRS:=-L ../sqlitecpp_build/sqlite3 -L ../sqlitecpp_build
LD_FLAGS:=-lSQLiteCpp -lsqlite3
INCLUDE_DIRS:=-I ../SQLiteCpp/include -I src/header -I ../pugi_build

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
.PHONY: test

helo_offline: obj/ $(PUGI_CPPS) $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) -o helo_offline $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

test: obj/ $(OBJ_FILES) $(TEST_OBJ_FILES)
	rm -f obj/*.gcda
	$(CXX) $(OBJ_FILES) -o test_helo_offline $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)
	./test_helo_offline
ifeq ($(COVERAGE), yes)
	mkdir -p coverage
	lcov -q --base-directory src/source --directory ./obj --no-external -c --output-file coverage/app.info
	genhtml -q --output-directory coverage coverage/app.info
endif

obj/%.o: $(SRC_PATTERN) $(PUGI_CPPS) ../sqlitecpp_build
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

obj/:
	mkdir -p obj

../pugi_build/%.o: $(PUGI_PATTERN)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

../pugi_build/%.cpp:
	mkdir -p ../pugi_build
	find ../pugixml/src -regex ".*\.\(hpp\|cpp\)" -exec cp {} ../pugi_build \;
	sed -i '/\/\/ #define PUGIXML_WCHAR_MODE/c\#define PUGIXML_WCHAR_MODE' ../pugi_build/pugiconfig.hpp

../sqlitecpp_build:
	mkdir -p ../sqlitecpp_build
	cd ../sqlitecpp_build; cmake $(CMAKE_ARGS) ../SQLiteCpp; cmake --build .

clean:
	rm -rf obj
	rm -f helo_offline
	rm -f test_helo_offline
	rm -rf coverage

clean_3pp:
	rm -rf ../sqlitecpp_build
	rm -rf ../pugi_build
