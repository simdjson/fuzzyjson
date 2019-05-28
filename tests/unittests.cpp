#include "fuzzyjson.h"
#include "randomjson.h"
#include "simdjsonfuzzyparser.h"
#include "rapidjsonfuzzyparser.h"
#include "mockfuzzyparser.h"

void test_simdjson(randomjson::RandomJson& random_json) {
    fuzzyjson::SimdjsonFuzzyParser simdjson_parser;
    assert(simdjson_parser.parse(random_json.get_json(), random_json.get_size()));
}

void test_rapidjson(randomjson::RandomJson& random_json) {
    fuzzyjson::RapidjsonFuzzyParser rapidjson_parser;
    assert(rapidjson_parser.parse(random_json.get_json(), random_json.get_size()));
}

void test_fuzz() {
    fuzzyjson::FuzzyJson fuzzy(std::string(TESTS_DIR)+"/test.json");
    auto mock1_parser = std::make_unique<fuzzyjson::MockFuzzyParser>("Mock1", fuzzyjson::FuzzyParserResult::ok);
    auto mock2_parser = std::make_unique<fuzzyjson::MockFuzzyParser>("Mock2", fuzzyjson::FuzzyParserResult::other_error);
    fuzzy.add_parser(std::move(mock1_parser));
    fuzzy.add_parser(std::move(mock2_parser));
    fuzzy.fuzz();
}


int main() {
    //randomjson::RandomJson random_json("test.json");
    //test_rapidjson(random_json);
    //test_simdjson(random_json);
    test_fuzz();
    return 0;
}