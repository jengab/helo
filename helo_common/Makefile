CPP_FILES:=$(wildcard src/source/*.cpp)
SRC_PATTERN:=src/source/%.cpp
OBJ_FILES:=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
INCLUDE_DIRS:=-I src/header
3PP_SRC:=../3pp/

ifeq (yes, $(DBG))
	CXX_FLAGS:=-g -O0 -D DEBUG -std=c++14 -Wall
else
	CXX_FLAGS:=-O3 -std=c++14
endif

ifeq (yes, $(COVERAGE))
	CXX_FLAGS:=-g -O0 --coverage -std=c++14
endif

ifeq ($(MAKECMDGOALS), test)
	INCLUDE_DIRS+=-I $(3PP_SRC)/lest/include -I $(3PP_SRC)/FakeIt/single_header/standalone
	LD_FLAGS+=-lgcov
	CPP_FILES:=$(wildcard src/*/*.cpp)
	OBJ_FILES:=$(filter-out obj/main_offline.o, $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))) $(PUGI_OBJS)
	SRC_PATTERN:=src/*/%.cpp
endif

ifneq (,$(findstring Windows, $(OS)))
	LD_FLAGS+=-lssp
	TEST_EXEC_NAME:=test_helo_common.exe
else
	LD_FLAGS+=-lpthread -ldl
	TEST_EXEC_NAME:=test_helo_common
ifeq ($(shell uname -s), Darwin)
	INCLUDE_DIRS+=-I /opt/local/include
endif
endif

.PHONY: clean
.PHONY: test
.PHONY: all

all: $(OBJ_FILES)

obj/%.o: $(SRC_PATTERN) obj/
	$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

obj/:
	mkdir -p obj

test: $(TEST_EXEC_NAME)
	rm -f obj/*.gcda
	./$(TEST_EXEC_NAME) -p
ifeq ($(COVERAGE), yes)
	mkdir -p coverage
	lcov -q --base-directory src/source --directory ./obj --no-external -c --output-file coverage/app.info
	genhtml -q --output-directory coverage coverage/app.info
endif

$(TEST_EXEC_NAME): obj/ $(OBJ_FILES) $(TEST_OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(TEST_EXEC_NAME) $(INCLUDE_DIRS) $(LD_DIRS) $(LD_FLAGS)

clean:
	rm -rf obj
	rm -f $(TEST_EXEC_NAME)
	rm -rf coverage
