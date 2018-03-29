#include <lest/lest.hpp>
#include <trompeloeil.hpp>

extern const lest::tests clusterSuite;

int main(int argc, char* argv[]) {
    std::ostream& stream = std::cout;

    trompeloeil::set_reporter([&stream](trompeloeil::severity s, const char* file, unsigned long line,
                std::string const& msg) {
        if (s == trompeloeil::severity::fatal) {
            throw lest::message{"", lest::location{ line ? file : "[file/line unavailable]", int(line) }, "", msg };
        } else {
            stream << lest::location{ line ? file : "[file/line unavailable]", int(line) } << ": " << msg;
        }
    });
    int ret = lest::run(clusterSuite, argc, argv, stream);
    return ret;
}
