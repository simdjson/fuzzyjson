#ifndef SIMDJSONFUZZYPARSER_H
#define SIMDJSONFUZZYPARSER_H

#include <string>

#include "fuzzyjsonparser.h"
#include "simdjson.h"
#include "simdjson.cpp"

class SimdjsonFuzzyParser : public fuzzyjson::FuzzyJsonParser {
    public:
    SimdjsonFuzzyParser() : FuzzyJsonParser("simdjson") {};
    bool parse(const char* json, int size) override
    {
        ParsedJson pj;
        bool allocation_is_successful = pj.allocateCapacity(size);
        const int res = json_parse(json, size, pj);
        /*
        std::fstream file("truc.json", std::ios::out | std::ios::binary);
        file.write(json, sizeof(char)*size);
        file.close();

        padded_string p = get_corpus("truc.json");
        ParsedJson pj2;
        bool allocation_is_successful2 = pj2.allocateCapacity(p.length());
        int res2 = json_parse(p, pj2);
        std::cout << res << res2 << " " << p.length() << std::endl;*/


        return (res!=0) && allocation_is_successful;
    }
};

#endif