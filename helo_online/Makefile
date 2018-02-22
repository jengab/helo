CPP_FILES:=$(wildcard src/source/*.cpp)
SRC_PATTERN:=src/source/%.cpp
PUGI_PATTERN:=../pugi_build/%.cpp
PUGI_CPPS:=$(addprefix ../pugi_build/,$(notdir $(wildcard ../pugixml/src/*.cpp)))
PUGI_OBJS:=$(addprefix ../pugi_build/,$(notdir $(PUGI_CPPS:.cpp=.o)))
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o))) $(PUGI_OBJS)
LD_DIRS:=-L ../sqlitecpp_build/sqlite3 -L ../sqlitecpp_build
LD_FLAGS:=-lSQLiteCpp -lsqlite3
INCLUDE_DIRS:=-I ../SQLiteCpp/include -I ../SQLiteCpp/sqlite3 -I src/header -I ../pugi_build

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG -std=c++0x -Wall
else
	CXX_FLAGS:=-O3 -std=c++0x
endif

ifneq (,$(findstring Windows, $(OS)))
	LD_FLAGS+=-lssp -lws2_32 -lboost_thread-mgw63-mt-1_63 \
						-lboost_system-mgw63-mt-1_63 -lboost_locale-mgw63-mt-1_63 -liconv
	INCLUDE_DIRS+=-I D:\boost
	LD_DIRS+=-L D:/boost/stage/lib
	CMAKE_ARGS:=-G "MSYS Makefiles"
else
	LD_FLAGS+=-lpthread -ldl -lboost_thread -lboost_system -lboost_locale
	CMAKE_ARGS:=-G "Unix Makefiles"
ifeq ($(shell uname -s), Darwin)
	INCLUDE_DIRS+=-I /opt/local/include
endif
endif

.PHONY: clean
.PHONY: clean_3pp

helo_online: obj/ $(PUGI_CPPS) $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) -o $@ -std=c++0x $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

obj/%.o: $(SRC_PATTERN) $(PUGI_CPPS) ../sqlitecpp_build
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -std=c++0x -c $< -o $@

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
	rm -f helo_online

clean_3pp:
	rm -rf ../pugi_build
	rm -rf ../sqlitecpp_build
