#include "fuzzyjson.h"
#include "randomjson.h"
#include "simdjsonparser.h"
//#include "rapidjsonparser.h"
//#include "mockparser.h"

void test_simdjson(randomjson::RandomJson& random_json) {
    fuzzyjson::SimdjsonParser simdjson_parser;
    std::unique_ptr<fuzzyjson::Traverser> traverser(simdjson_parser.parse(random_json.get_json(), random_json.get_size()));
    assert(traverser->get_parsing_result() == fuzzyjson::ParsingResult::ok);
    fuzzyjson::ValueType valuetype = traverser->next();
    std::cout << fuzzyjson::valuetype_to_string(valuetype);
}
/*
void test_rapidjson(randomjson::RandomJson& random_json) {
    fuzzyjson::RapidjsonParser rapidjson_parser;
    fuzzyjson::Traverser traverser = rapidjson_parser.parse(random_json.get_json(), random_json.get_size());
    assert(traverser.get_parsing_result() == fuzzyjson::ParsingResult::ok);
}
*/

/*
void test_fuzz() {
    fuzzyjson::FuzzyJson fuzzy(std::string(TESTS_DIR)+"/test.json");
    auto mock1_parser = std::make_unique<fuzzyjson::MockParser>("Mock1", fuzzyjson::ParsingResult::ok);
    auto mock2_parser = std::make_unique<fuzzyjson::MockParser>("Mock2", fuzzyjson::ParsingResult::other_error);
    fuzzy.add_parser(std::move(mock1_parser));
    fuzzy.add_parser(std::move(mock2_parser));
    fuzzy.fuzz();
}*/


int main() {
    randomjson::RandomJson random_json(std::string(TESTS_DIR)+"/test.json");
    //test_rapidjson(random_json);
    test_simdjson(random_json);
    //test_fuzz();
    return 0;
}