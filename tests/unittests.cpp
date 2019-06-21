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
    assert(floatings_are_equal(traverser->get_floating(), 1.2, std::numeric_limits<double>::epsilon()));
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
    assert(floatings_are_equal(traverser->get_floating(), 1.2, std::numeric_limits<double>::epsilon()));
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
    randomjson::RandomJsonSettings settings(50);
    FuzzyJson fuzzy(settings);

    fuzzy.add_parser(std::make_unique<RapidjsonParser>());
    fuzzy.add_parser(std::make_unique<SimdjsonParser>());

    fuzzy.fuzz();
}

}

int main() {
    fuzzyjson::test_rapidjson();
    fuzzyjson::test_simdjson();
    for (int i = 0; i < 5; i++)
        fuzzyjson::test_fuzz();
}
