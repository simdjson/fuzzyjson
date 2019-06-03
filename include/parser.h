#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

namespace fuzzyjson
{

enum class ParsingResult
{
    ok,
    number_error,
    string_error,
    encoding_error,
    other_error,
};

std::unordered_map<ParsingResult, std::string> result_to_string {
    { ParsingResult::ok , "ok"},
    { ParsingResult::number_error, "number_error" },
    { ParsingResult::string_error, "string_error" },
    { ParsingResult::encoding_error, "encoding_error" },
    { ParsingResult::other_error, "other_error" },
};

enum class ValueType
{
    object,
    array,
    string,
    key,
    integer,
    floating,
    boolean,
    null, // json null value
    end_of_container, // end of object or array
    end_of_document,
};


class Traverser
{
    public:
    Traverser(std::string parser_name, ParsingResult parsing_result)
    : parser_name(parser_name)
    , parsing_result(parsing_result)
    {};

    std::string get_parser_name() { return parser_name; }
    ParsingResult get_parsing_result() { return parsing_result; }

    std::string get_result_string() {
        return result_to_string.at(parsing_result);
    }

    void next() {
        ValueType value_type;
        switch (value_type) {
            case ValueType::object:
            case ValueType::array:
                down();
                break;
            case ValueType::key:
            case ValueType::integer:
            case ValueType::floating:
            case ValueType::boolean:
            case ValueType::null:
                advance_container();
                break;
            case ValueType::end_of_container:
                up();
                break;
            case ValueType::end_of_document:
            break;
        }

        index++;

        value_type;
    }
    int get_index() { return index; }

    ValueType get_type();
    std::string get_string();
    int get_integer();
    double get_floating();
    bool get_boolean();

    private:
    void up();
    void down();
    void advance_container();

    std::string parser_name;
    ParsingResult parsing_result;
    int index; // represents the numbers of next() required to arrive to the current value
};

class Parser 
{
    public:
    Parser(std::string name) : name(name) {};
    virtual Traverser parse(char* const json, int size) = 0;
    virtual ~Parser() {};
    std::string get_name() { return name; }
    private:
    std::string name;
};

}

#endif