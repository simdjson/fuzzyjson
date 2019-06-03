#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

namespace fuzzyjson
{

enum class ParsingResult
{
    ok,
    number_error,
    string_error,
    encoding_error,
    other_error,
};

std::unordered_map<ParsingResult, std::string> result_to_string {
    { ParsingResult::ok , "ok"},
    { ParsingResult::number_error, "number_error" },
    { ParsingResult::string_error, "string_error" },
    { ParsingResult::encoding_error, "encoding_error" },
    { ParsingResult::other_error, "other_error" },
};

class Parser 
{
    public:
    Parser(std::string name) : name(name) {};
    virtual ParsingResult parse(char* const json, int size) = 0;
    virtual ~Parser() {};
    std::string get_name() { return name; }
    virtual std::string get_result_string() = 0;
    //virtual std::string get_commentary() = 0; // More information about the result. The original error code, for example
    private:
    std::string name;
};

}

#endif