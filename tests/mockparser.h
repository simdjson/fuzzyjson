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
    , last_result(result)
    {
    };

    ~MockParser() = default;
    
    ParsingResult parse(char* const json, int size) override
    {
        return last_result;
    }

    std::string get_result_string() override {
        return result_to_string.at(last_result);
    }

    private:
    ParsingResult last_result;
};

}

#endif