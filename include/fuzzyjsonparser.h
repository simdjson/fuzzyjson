#ifndef FUZZYJSONPARSER_H
#define FUZZYJSONPARSER_H

#include <string>

namespace fuzzyjson {
enum class FuzzyParserResult {
    ok,
    number_error,
    string_error,
    encoding_error,
    other_error,
};

class FuzzyJsonParser {
    public:
    FuzzyJsonParser(std::string name) : name(name) {};
    virtual FuzzyParserResult parse(char* json, int size) = 0;
    //virtual FuzzyParserResult get_fuzzy_error() = 0;
    //virtual std::string get_commentary() = 0; // More information about the result. The original error code, for example
    private:
    std::string name;
};
}

#endif