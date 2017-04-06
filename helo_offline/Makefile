CPP_FILES:=$(wildcard src/source/*.cpp)
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_DIRS:=-L ../SQLiteCpp/sqlitecpp_build/sqlite3 -L ../SQLiteCpp/sqlitecpp_build
LD_FLAGS:=-lSQLiteCpp -lsqlite3
INCLUDE_DIRS:=-I ../SQLiteCpp/include -I src/header

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG
else
	CXX_FLAGS:=-O3
endif

ifeq (yes, $(COVERAGE))
	CXX_FLAGS:=-g -O0 --coverage
	TEST_FLAGS:=-lgcov
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
.PHONY: test

test: src/test/LogParserTests.cpp obj/ $(OBJ_FILES)
	rm -f obj/*.gcda
	$(CXX) $< $(filter-out obj/main_offline.o, $(OBJ_FILES)) -o $@ -std=c++0x $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS) -I ../lest/include -I ../FakeIt/single_header/standalone $(TEST_FLAGS)
	./test
ifeq ($(COVERAGE), yes)
	mkdir -p ../coverage
	lcov -q --base-directory src/source --directory ./obj --no-external -c --output-file ../coverage/app.info
	genhtml -q --output-directory ../coverage ../coverage/app.info
endif

helo_offline: obj/ $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) -o $@ -std=c++0x $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

obj/%.o: src/source/%.cpp ../SQLiteCpp/sqlitecpp_build
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -std=c++0x -c $< -o $@

obj/:
	mkdir -p obj

../SQLiteCpp/sqlitecpp_build:
	mkdir -p ../SQLiteCpp/sqlitecpp_build
	cd ../SQLiteCpp/sqlitecpp_build; cmake $(CMAKE_ARGS) ..; cmake --build .

clean:
	rm -rf obj
	rm -rf ../SQLiteCpp/sqlitecpp_build
	rm -f helo_offline
