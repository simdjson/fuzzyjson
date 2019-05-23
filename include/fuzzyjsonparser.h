#ifndef FUZZYJSONPARSER_H
#define FUZZYJSONPARSER_H

#include <string>

namespace fuzzyjson {
class FuzzyJsonParser {
    public:
    FuzzyJsonParser(std::string name) : name(name) {};
    virtual bool parse(const char* json, int size) = 0; // returns whether the parsing is successful or not
    private:
    std::string name;
};
}

#endif