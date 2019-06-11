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
    struct ContainerInfos {
        ValueType type;
        bool is_empty;
    };

    public:
    // The SimdjsonTraverser will own the ParsedJson
    SimdjsonTraverser(std::string parser_name, ParsingState parsing_state, ParsedJson* parsed_json)
    : Traverser(parser_name, parsing_state)
    , parsed_json(parsed_json)
    , iterator(*parsed_json)
    {
        current_type = simdjsontype_to_fuzzytype(iterator.get_type());
    }

    ~SimdjsonTraverser() override { delete parsed_json; }
 
    ValueType next() override {
        ValueType previous_type = current_type;
        if (previous_type == ValueType::array || previous_type == ValueType::object) {
            bool container_is_empty = false;
            bool end_of_container = !iterator.down();
            if (end_of_container) {
                current_type = ValueType::end_of_container;
                container_is_empty = true;
            }
            else {
                if (previous_type == ValueType::object) {
                    current_type = ValueType::key;
                }
                else {
                    current_type = simdjsontype_to_fuzzytype(iterator.get_type());
                }
            }
            ContainerInfos container_infos {previous_type, container_is_empty};
            container_stack.push(container_infos);

            return current_type;
        }

        if (previous_type == ValueType::end_of_container) {
            // if the container is empty then we're not actually into it.
            if (!container_stack.top().is_empty) {
                iterator.up();
            }
            container_stack.pop();
        }

        if (previous_type == ValueType::end_of_document || container_stack.size() == 0) {
            current_type = ValueType::end_of_document;
            return current_type;
        }

        bool end_of_container = !iterator.next();
        if (end_of_container) {
            current_type = ValueType::end_of_container;
            return current_type;
        }
        
        if (container_stack.top().type == ValueType::object && previous_type != ValueType::key) {
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
    std::stack<ContainerInfos> container_stack;
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
        ParsedJson* parsed_json = new ParsedJson(); // The traverser will take onership of it
        bool allocation_is_successful = parsed_json->allocateCapacity(size);
        if (!allocation_is_successful) {
            return std::make_unique<SimdjsonTraverser>(get_name(), ParsingState::error, parsed_json);
        }
        auto simdjson_result = static_cast<simdjson::errorValues>(json_parse(json, size, *parsed_json));
        ParsingState state = simdjson_result == simdjson::errorValues::SUCCESS ? ParsingState::ok : ParsingState::error;
        return std::make_unique<SimdjsonTraverser>(get_name(), state, parsed_json);
    }
};
}

#endif