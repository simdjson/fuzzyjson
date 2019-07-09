#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include <unordered_map>

namespace fuzzyjson
{

enum class ParsingState
{
    ok,
    error,
};

std::unordered_map<ParsingState, std::string> parsing_state_to_string {
    { ParsingState::ok , "ok"},
    { ParsingState::error, "error" },
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
    Traverser(std::string parser_name, ParsingState parsing_state)
    : parser_name(parser_name)
    , parsing_state(parsing_state)
    {}
    
    virtual ~Traverser() {}

    // checks if the detected error is an error we already know about
    virtual bool is_known_problem(const char* json, int size) { return false; };

    std::string get_parser_name() { return parser_name; }
    ParsingState get_parsing_state() { return parsing_state; }

    std::string get_parsing_state_as_string() {
        return parsing_state_to_string.at(parsing_state);
    }

    virtual ValueType next() = 0;

    virtual ValueType get_current_type() = 0;
    virtual std::string get_string() = 0;
    virtual int64_t get_integer() = 0;
    virtual double get_floating() = 0;
    virtual bool get_boolean() = 0;

    private:
    std::string parser_name;
    ParsingState parsing_state;
};

class InvalidTraverser : public Traverser
{
    public:
    InvalidTraverser(std::string parser_name)
    : Traverser(parser_name, ParsingState::error)
    {}

    ~InvalidTraverser() = default;
    // getters return arbitratry values. There is no valid reason to call them.
    ValueType next() override { return ValueType::error; }
    ValueType get_current_type() override { return ValueType::error; }
    std::string get_string() override { return ""; }
    int64_t get_integer() override { return 0; }
    double get_floating() override { return 0.0; }
    bool get_boolean() override { return false; }
};

class Parser 
{
    public:
    Parser(std::string name) : name(name) {}
    virtual ~Parser() {}

    virtual std::unique_ptr<Traverser> parse(const char* json, int size) = 0;

    // Name used to identify the parser in the reports
    std::string get_name() { return name; }

    private:
    std::string name;
};
}
#endif