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
helo_offline: obj/ $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) $(OBJ_FILES) -o $@ -std=c++11 $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

obj/%.o: src/source/%.cpp ../SQLiteCpp/sqlitecpp_build
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -std=c++11 -c $< -o $@

obj/:
	mkdir -p obj

../SQLiteCpp/sqlitecpp_build:
	mkdir -p ../SQLiteCpp/sqlitecpp_build
	cd ../SQLiteCpp/sqlitecpp_build; cmake $(CMAKE_ARGS) ..; cmake --build .

clean:
	rm -rf obj
	rm -rf ../SQLiteCpp/sqlitecpp_build
	rm -f helo_offline

print:
	@echo $(OS)
