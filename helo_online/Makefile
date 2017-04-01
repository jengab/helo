CPP_FILES:=$(wildcard src/source/*.cpp)
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
INCLUDE_DIRS:=-I ../SQLiteCpp/include -I ../SQLiteCpp/sqlite3 -I src/header
LD_DIRS:=-L ../SQLiteCpp/sqlitecpp_build/sqlite3 -L ../SQLiteCpp/sqlitecpp_build 
LD_FLAGS:=-lSQLiteCpp -lsqlite3 

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG
else
	CXX_FLAGS:=-O3
endif

ifneq (,$(findstring Windows, $(OS)))
	LD_FLAGS+=-lssp -lws2_32 -lboost_regex-mgw63-mt-1_63 -lboost_thread-mgw63-mt-1_63 \
						-lboost_system-mgw63-mt-1_63 -lboost_locale-mgw63-mt-1_63 -liconv
	INCLUDE_DIRS+=-I D:\boost
	LD_DIRS+=-L D:/boost/stage/lib
	CMAKE_ARGS:=-G "MSYS Makefiles"
else
	LD_FLAGS+=-lpthread -ldl -lboost_regex -lboost_thread -lboost_system -lboost_locale
	CMAKE_ARGS:=-G "Unix Makefiles"
endif

.PHONY: clean
helo_online: obj/ $(OBJ_FILES)
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
	rm -f helo_online
	rm -rf ../SQLiteCpp/sqlitecpp_build
