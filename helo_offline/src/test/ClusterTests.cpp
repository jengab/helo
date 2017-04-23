#include "cluster.h"
#include "lest/lest.hpp"

static const lest::test _clusterSuite[] {
    CASE("Dummy1") {
        EXPECT(1 == 0);
    },
    CASE("Dummy2") {
        EXPECT(1 == 1);
    }
};

extern const lest::tests clusterSuite(_clusterSuite, _clusterSuite + sizeof(_clusterSuite) / sizeof(*_clusterSuite));
