#ifndef FUZZYJSONPARSER_H
#define FUZZYJSONPARSER_H

#include <string>
#include <unordered_map>

namespace fuzzyjson
{

enum class FuzzyParserResult
{
    ok,
    number_error,
    string_error,
    encoding_error,
    other_error,
};

std::unordered_map<FuzzyParserResult, std::string> result_to_string {
        { FuzzyParserResult::ok , "ok"},
        { FuzzyParserResult::number_error, "number_error" },
        { FuzzyParserResult::string_error, "string_error" },
        { FuzzyParserResult::encoding_error, "encoding_error" },
        { FuzzyParserResult::other_error, "other_error" },
    };

class FuzzyJsonParser 
{
    public:
    FuzzyJsonParser(std::string name) : name(name) {};
    virtual FuzzyParserResult parse(char* const json, int size) = 0;
    virtual ~FuzzyJsonParser() {};
    std::string get_name() { return name; }
    virtual std::string get_result_string() = 0;
    //virtual std::string get_commentary() = 0; // More information about the result. The original error code, for example
    private:
    std::string name;
};

}

#endif