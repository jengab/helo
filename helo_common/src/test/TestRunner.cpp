#include "lest/lest.hpp"

extern const lest::tests logParserSuite;

int main(int argc, char* argv[]) {
    int ret = lest::run(logParserSuite, argc, argv);
    return ret;
}
