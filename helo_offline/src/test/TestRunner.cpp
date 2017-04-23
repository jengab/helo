#include "lest/lest.hpp"

extern const lest::tests logParserSuite;
extern const lest::tests clusterSuite;

int main(int argc, char* argv[]) {
    int ret = lest::run(logParserSuite, argc, argv);
    ret += lest::run(clusterSuite, argc, argv);
    return ret;
}
