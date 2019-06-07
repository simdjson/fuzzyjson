#ifndef PARSER_H
#define PARSER_H

#include <memory>
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
    error, // Not a value. Something went wrong.
};

std::string valuetype_to_string(ValueType valuetype) {
    switch (valuetype) {
        case ValueType::object:
            return "object";
        case ValueType::array:
            return "array";
        case ValueType::string:
            return "string";
        case ValueType::key:
            return "key";
        case ValueType::integer:
            return "integer";
        case ValueType::floating:
            return "floating";
        case ValueType::boolean:
            return "boolean";
        case ValueType::null:
            return "null";
        case ValueType::end_of_container:
            return "end_of_container";
        case ValueType::end_of_document:
            return "end_of_document";
        case ValueType::error:
        default:
            return "error";
    }
}

class Traverser
{
    public:
    Traverser(std::string parser_name, ParsingResult parsing_result)
    : parser_name(parser_name)
    , parsing_result(parsing_result)
    {}
    
    virtual ~Traverser() {}

    std::string get_parser_name() { return parser_name; }
    ParsingResult get_parsing_result() { return parsing_result; }

    std::string get_result_string() {
        return result_to_string.at(parsing_result);
    }

    virtual ValueType next() = 0;

    virtual ValueType get_current_type() = 0;
    virtual std::string get_string() = 0;
    virtual int64_t get_integer() = 0;
    virtual double get_floating() = 0;
    virtual bool get_boolean() = 0;

    private:
    std::string parser_name;
    ParsingResult parsing_result;
};

class Parser 
{
    public:
    Parser(std::string name) : name(name) {}
    virtual std::unique_ptr<Traverser> parse(const char* json, int size) = 0;
    virtual ~Parser() {}
    std::string get_name() { return name; }
    private:
    std::string name;
};
}
#endif