#include "fuzzyjson.h"
#include "randomjson.h"
#include "simdjsonparser.h"
#include "rapidjsonparser.h"
#include "mockparser.h"

void test_simdjson(randomjson::RandomJson& random_json) {
    fuzzyjson::SimdjsonParser simdjson_parser;
    fuzzyjson::ParsingResult result = simdjson_parser.parse(random_json.get_json(), random_json.get_size());
    assert(result == fuzzyjson::ParsingResult::ok);
}

void test_rapidjson(randomjson::RandomJson& random_json) {
    fuzzyjson::RapidjsonParser rapidjson_parser;
    fuzzyjson::ParsingResult result = rapidjson_parser.parse(random_json.get_json(), random_json.get_size());
    assert(result == fuzzyjson::ParsingResult::ok);
}

void test_fuzz() {
    fuzzyjson::FuzzyJson fuzzy(std::string(TESTS_DIR)+"/test.json");
    auto mock1_parser = std::make_unique<fuzzyjson::MockParser>("Mock1", fuzzyjson::ParsingResult::ok);
    auto mock2_parser = std::make_unique<fuzzyjson::MockParser>("Mock2", fuzzyjson::ParsingResult::other_error);
    fuzzy.add_parser(std::move(mock1_parser));
    fuzzy.add_parser(std::move(mock2_parser));
    fuzzy.fuzz();
}


int main() {
    randomjson::RandomJson random_json("outbounds.json");
    test_rapidjson(random_json);
    test_simdjson(random_json);
    test_fuzz();
    return 0;
}