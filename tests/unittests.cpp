#include "fuzzyjson.h"
#include "randomjson.h"
#include "simdjsonfuzzyparser.h"

void test_simdjson() {
    const int size = 1000;
    randomjson::RandomJson random_json(size);
    SimdjsonFuzzyParser simdjson_parser;
    assert(simdjson_parser.parse(random_json.get_json(), random_json.get_size()));
}

void test_fuzz() {
    fuzzyjson::FuzzyJson fuzzy("100");
    fuzzy.add_parser(std::make_unique<SimdjsonFuzzyParser>());
    fuzzy.fuzz();
}

int main() {

    test_simdjson();
    test_fuzz();
    return 0;
}