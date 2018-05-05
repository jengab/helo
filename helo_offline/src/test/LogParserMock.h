#include "LogParser.h"

class LogParserMock : public LogParser {
protected:
    void ProcessHybrid(std::wstring&) const override {}
public:
    LogParserMock(size_t headerLen, const std::wstring& regex) : LogParser(headerLen, regex){}
};
