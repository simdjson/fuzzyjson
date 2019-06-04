#ifndef MOCKPARSER_H
#define MOCKPARSER_H

#include <string>

#include "parser.h"

namespace fuzzyjson
{

class MockParser : public Parser
{
    public:
    MockParser(std::string name, ParsingResult result)
    : Parser(name)
    , result(result)
    {
    };

    ~MockParser() = default;
    
    std::unique_ptr<Traverser> parse(char* const json, int size) override
    {
        return std::make_unique<Traverser>(get_name(), result);
    }

    ParsingResult result;
};

}

#endif