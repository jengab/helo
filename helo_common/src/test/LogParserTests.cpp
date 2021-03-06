#include "LogParser.h"
#include "lest/lest.hpp"

static const lest::test _logParserSuite[] {
    CASE("parse: Word determined correctly") {
        std::wstring str(L"WordTrial");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Word);
    },
    CASE("parse: Positive number determined correctly") {
        std::wstring str(L"1234567890");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: Negative number determined correctly") {
        std::wstring str(L"-1234567890");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: Pi is number") {
        std::wstring str(L"3.141592654");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: coma separated floating is number") {
        std::wstring str(L"3,141592654");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: Upper hexa determined correctly") {
        std::wstring str(L"0x1234567890ABCDEF");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: Lower hexa determined correctly") {
        std::wstring str(L"0xabcdef1234567890");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Number);
    },
    CASE("parse: Wrong hexa is not number") {
        std::wstring str(L"0x123fg");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret != Number);
    },
    CASE("parse: Hybrid found if first character is not alnum") {
        std::wstring str(L"#ThisIsHybrid123");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Hybrid);
    },
    CASE("parse: Hybrid found if middle character is not alnum") {
        std::wstring str(L"ThisIs&Hybrid123");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Hybrid);
    },
    CASE("parse: Hybrid found if last character is not alnum") {
        std::wstring str(L"ThisIsHybrid123!");
        wordtype ret = LogParser(0, L"[\\s+]").parse(str);
        EXPECT(ret == Hybrid);
    },
    CASE("parse: Hybrid is identified with number prefix") {
        std::wstring str(L"123xc456");
        wordtype ret = LogParser(0, L"[\\s]+").parse(str);
        EXPECT(ret == Hybrid);
    },
};

extern const lest::tests logParserSuite(_logParserSuite,
                                  _logParserSuite + sizeof(_logParserSuite) / sizeof(*_logParserSuite));
