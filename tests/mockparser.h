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
    , traverser(name, result)
    {
    };

    ~MockParser() = default;
    
    Traverser parse(char* const json, int size) override
    {
        return traverser;
    }

    Traverser traverser;
};

}

#endif