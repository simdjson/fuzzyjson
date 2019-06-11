#include "fuzzyjson.h"
#include "randomjson.h"
#include "simdjsonparser.h"
#include "rapidjsonparser.h"
//#include "mockparser.h"

namespace fuzzyjson {

class Document {
    public:
    std::shared_ptr<char[]> data;
    int size;

    Document(std::string filepath)  {
        std::ifstream file (filepath, std::ios::in | std::ios::binary | std::ios::ate);
        size = file.tellg();
        data = std::make_unique<char[]>(size);
        file.seekg(0, std::ios::beg);
        file.read(data.get(), size);
        file.close();
    }
};

void test_one_number_document_parsing(std::shared_ptr<Parser> parser) {
    const Document json(std::string(TEST_DOCUMENTS)+"/one_number.json");

    std::unique_ptr<Traverser> traverser(parser->parse(json.data.get(), json.size));
    assert(traverser->get_parsing_state() == ParsingState::ok);
    assert(traverser->get_current_type() == ValueType::integer);
    assert(traverser->get_integer() == 1);
}

void test_simple_nested_document_parsing(std::shared_ptr<Parser> parser) {
    const Document json(std::string(TEST_DOCUMENTS)+"/simple_nested.json");
    std::unique_ptr<Traverser> traverser(parser->parse(json.data.get(), json.size));
    assert(traverser->get_parsing_state() == ParsingState::ok);
    assert(traverser->get_current_type() == ValueType::array);
    assert(traverser->next() == ValueType::object);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::array);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::integer);
    assert(traverser->get_integer() == 1);
    assert(traverser->next() == ValueType::floating);
    assert(floatings_are_equal(traverser->get_floating(), 1.2));
    assert(traverser->next() == ValueType::string);
    assert(traverser->get_string() == "string");
    assert(traverser->next() == ValueType::boolean);
    assert(traverser->get_boolean() == true);
    assert(traverser->next() == ValueType::boolean);
    assert(traverser->get_boolean() == false);
    assert(traverser->next() == ValueType::null);

    assert(traverser->next() == ValueType::object);
    assert(traverser->next() == ValueType::key);
    assert(traverser->get_string() == "object");
    assert(traverser->next() == ValueType::object);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::array);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::integer);
    assert(traverser->get_integer() == 1);
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::floating);
    assert(floatings_are_equal(traverser->get_floating(), 1.2));
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::string);
    assert(traverser->get_string() == "string");
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::boolean);
    assert(traverser->get_boolean() == true);
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::boolean);
    assert(traverser->get_boolean() == false);
    assert(traverser->next() == ValueType::key);
    assert(traverser->next() == ValueType::null);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::end_of_container);
    assert(traverser->next() == ValueType::end_of_document);
}

void test_rapidjson() {
    auto rapidjson_parser = std::make_shared<RapidjsonParser>();
    test_simple_nested_document_parsing(rapidjson_parser);
    test_one_number_document_parsing(rapidjson_parser);
}

void test_simdjson() {
    auto simdjson_parser = std::make_shared<SimdjsonParser>();
    test_simple_nested_document_parsing(simdjson_parser);
    test_one_number_document_parsing(simdjson_parser);
}

void test_fuzz() {
    /*FuzzyJson fuzzy(std::string(TESTS_DIR)+"/test.json");
    auto mock1_parser = std::make_unique<MockParser>("Mock1", ParsingResult::ok);
    auto mock2_parser = std::make_unique<MockParser>("Mock2", ParsingResult::other_error);
    fuzzy.add_parser(std::move(mock1_parser));
    fuzzy.add_parser(std::move(mock2_parser));*/

    FuzzyJson fuzzy(10000);

    fuzzy.add_parser(std::make_unique<SimdjsonParser>());
    fuzzy.add_parser(std::make_unique<RapidjsonParser>());

    fuzzy.fuzz();
}

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
void t() {
    rapidjson::Document d;
    d.SetObject();
    //rapidjson::Value contact(rapidjson::kObject);
    d.AddMember("name", "Milo", d.GetAllocator());
    d.AddMember("married", true, d.GetAllocator());


    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    const char* json = buffer.GetString();
    std::cout << json << std::endl;
}

void t2() {
    /*FuzzyJson fuzzy(std::string(TESTS_DIR)+"/test.json");
    auto mock1_parser = std::make_unique<MockParser>("Mock1", ParsingResult::ok);
    auto mock2_parser = std::make_unique<MockParser>("Mock2", ParsingResult::other_error);
    fuzzy.add_parser(std::move(mock1_parser));
    fuzzy.add_parser(std::move(mock2_parser));*/

    FuzzyJson fuzzy(1000);
    fuzzy.add_parser(std::make_unique<RapidjsonParser>());
    fuzzy.add_parser(std::make_unique<SimdjsonParser>());
    fuzzy.fuzz();
}
}

int main() {

    auto rapid = std::make_shared<fuzzyjson::RapidjsonParser>();
    auto simd = std::make_shared<fuzzyjson::SimdjsonParser>();
    //std::cout << "rapid" << std::endl;
    //fuzzyjson::t(rapid);
    //fuzzyjson::t(simd);
    //fuzzyjson::test_rapidjson();
    //fuzzyjson::test_simdjson();
    //fuzzyjson::t();
    for (int i = 0; i < 10000; i++)
    fuzzyjson::test_fuzz();
    return 0;
}
/*
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

int main ()
{
  using std::chrono::system_clock;
  std::time_t tt = system_clock::to_time_t (system_clock::now());

  struct std::tm* ptm = std::localtime(&tt);
  std::cout << std::put_time(ptm, "%F%T") << '\n';

  return 0;
}*/