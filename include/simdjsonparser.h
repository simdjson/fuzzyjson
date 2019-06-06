#ifndef SIMDJSONPARSER_H
#define SIMDJSONPARSER_H

#include <stack>
#include <string>
#include <unordered_map>

#include "parser.h"
#include "simdjson.h"
#include "simdjson.cpp"

namespace fuzzyjson {

class SimdjsonTraverser : public Traverser {
    public:
    // The SimdjsonTraverser will own the ParsedJson
    SimdjsonTraverser(std::string parser_name, ParsingResult result, ParsedJson* parsed_json)
    : Traverser(parser_name, result)
    , parsed_json(parsed_json)
    , iterator(*parsed_json)
    {
        current_type = simdjsontype_to_fuzzytype(iterator.get_type());
        if (current_type == ValueType::object || current_type == ValueType::array) {
            container_stack.push(current_type);
        }
    }

    ~SimdjsonTraverser() override { delete parsed_json; }
 
    ValueType next() override {
        if (current_type == ValueType::end_of_document) {
            return ValueType::end_of_document;
        }

        if (current_type == ValueType::object) {
            container_stack.push(current_type);
            iterator.down();
            current_type = ValueType::key;
            return current_type;
        }

        if (current_type == ValueType::array) {
            container_stack.push(current_type);
            iterator.down();
            current_type = simdjsontype_to_fuzzytype(iterator.get_type());
            return current_type;
        }

        bool end_of_container = !iterator.next();
        if (end_of_container) {
            iterator.up();
            container_stack.pop();

            if (container_stack.size() != 0) {
                current_type = ValueType::end_of_container;
            }
            else {
                current_type = ValueType::end_of_document;
            }
            return current_type;
        }

        if (container_stack.top() == ValueType::object && current_type != ValueType::key) {
            current_type = ValueType::key;
        }
        else {
            current_type = simdjsontype_to_fuzzytype(iterator.get_type());
        }
        return current_type;
    }

    ValueType get_current_type() override { return current_type; }
    std::string get_string() override { return iterator.get_string(); }
    int64_t get_integer() override { return iterator.get_integer(); }
    double get_floating() override { return iterator.get_double(); }
    bool get_boolean() override { return iterator.is_true(); }

    private:
    ValueType current_type;
    std::stack<ValueType> container_stack;
    ParsedJson::iterator iterator;
    ParsedJson* parsed_json;

    ValueType simdjsontype_to_fuzzytype(char simdjson_type) {
        switch (simdjson_type) {
            case '{':
                return ValueType::object;
            case '[':
                return ValueType::array;
            case '"':
                return ValueType::string;
            case 'l':
                return ValueType::integer;
            case 'd':
                return ValueType::floating;
            case 't':
            case 'f':
                return ValueType::boolean;
            case 'n':
                return ValueType::null;
            default :
                return ValueType::error;
        }
    }
};


class SimdjsonParser : public Parser {
    public:
    SimdjsonParser()
    : Parser("simdjson")
    {}

    ~SimdjsonParser() = default;
    
    std::unique_ptr<Traverser> parse(const char* json, int size) override
    {
        ParsedJson* parsed_json = new ParsedJson();
        bool allocation_is_successful = parsed_json->allocateCapacity(size);
        if (!allocation_is_successful) {
            delete parsed_json;
            return std::make_unique<SimdjsonTraverser>(get_name(), ParsingResult::other_error, nullptr);
        }
        auto simdjson_result = static_cast<simdjson::errorValues>(json_parse(json, size, *parsed_json));
        ParsingResult result = result_correspondances.at(simdjson_result);
        return std::make_unique<SimdjsonTraverser>(get_name(), result, parsed_json);
    }

    private:
    simdjson::errorValues last_result;

    std::unordered_map<simdjson::errorValues, ParsingResult> result_correspondances {
        { simdjson::SUCCESS, ParsingResult::ok },
        { simdjson::CAPACITY, ParsingResult::other_error },
        { simdjson::TAPE_ERROR, ParsingResult::other_error },
        { simdjson::DEPTH_ERROR, ParsingResult::other_error },
    };
};
}

#endif