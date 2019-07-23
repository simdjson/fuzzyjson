#ifndef SAJSONPARSER_H
#define SAJSONPARSER_H

#include <string>

#include "parser.h"
#define SAJSON_UNSORTED_OBJECT_KEYS
#include "sajson.h"

namespace fuzzyjson {
int n = 0;
class SajsonTraverser : public Traverser
{
    // forward declarations
    class ObjectTraverser;
    class ArrayTraverser;
    class SingleValueTraverser;

    class TraverserHelper {
        public:
        int id;
        virtual ~TraverserHelper() {}

        // seriously
        virtual ValueType next() = 0;
        virtual ValueType get_type() = 0;
        virtual std::string get_string() = 0;
        virtual int64_t get_integer() = 0;
        virtual double get_floating() = 0;
        virtual bool get_boolean() = 0;
        virtual std::unique_ptr<ObjectTraverser> get_object_traverser() = 0;
        virtual std::unique_ptr<ArrayTraverser> get_array_traverser() = 0;
    };

    class ObjectTraverser : public TraverserHelper {
        public:
        int id;
        ObjectTraverser(sajson::value object)
        : object(object)
        , current_type(ValueType::object)
        , index(-1)
        , id(n++)
        {
            if (object.get_type() == 5) {
            }
            assert(object.get_type() == sajson::TYPE_OBJECT);
        }

        ValueType next() override {
            ValueType previous_type = current_type;
            if (previous_type != ValueType::key) {
                index++;
            }

            if (index >= object.get_length()) {
                current_type = ValueType::end_of_container;
                return current_type;
            }

            if (previous_type == ValueType::key) {
                sajson::type sajsontype = object.get_object_value(index).get_type();
                current_type = sajsontype_to_fuzzytype(sajsontype);
            }
            else {
                current_type = ValueType::key;
            }
            return current_type;
        }
        ValueType get_type() override { return current_type; };
        std::string get_string() override {
            if (current_type == ValueType::key) {
                return  object.get_object_key(index).as_string();
            }
            else {
                return object.get_object_value(index).as_string();
            }
        }
        int64_t get_integer() override { return object.get_object_value(index).get_integer_value(); }
        double get_floating() override { return object.get_object_value(index).get_double_value(); }
        bool get_boolean() override { return object.get_object_value(index).get_type() == sajson::TYPE_TRUE ? true : false; }
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            return std::make_unique<ObjectTraverser>(object.get_object_value(index));
        }
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            return std::make_unique<ArrayTraverser>(object.get_object_value(index));
        }

        private:
        sajson::value object;
        ValueType current_type;
        int index;
    };

    class ArrayTraverser : public TraverserHelper {
        public:
        ArrayTraverser(sajson::value array)
        : array(array)
        , current_type(ValueType::array)
        , index(-1)
        {}

        ValueType next() override {
            index++;
            if (index >= array.get_length()) {
                current_type = ValueType::end_of_container;
                return current_type;
            }
            current_type = sajsontype_to_fuzzytype(array.get_array_element(index).get_type());
            return current_type;
        }
        ValueType get_type() override { return current_type; }
        std::string get_string() override { return array.get_array_element(index).as_string(); }
        int64_t get_integer() override { return array.get_array_element(index).get_integer_value(); }
        double get_floating() override { return array.get_array_element(index).get_double_value(); }
        bool get_boolean() override { return (array.get_array_element(index).get_type() == sajson::TYPE_TRUE ? true : false); }
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            return std::make_unique<ObjectTraverser>(array.get_array_element(index));
        }
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            return std::make_unique<ArrayTraverser>(array.get_array_element(index));
        }

        private:
        sajson::value array;
        ValueType current_type;
        int index;
    };

    // To use when the document has one single value
    class SingleValueTraverser : public TraverserHelper {
        public:
        SingleValueTraverser(sajson::value value)
        : value(value)
        , current_type(sajsontype_to_fuzzytype(value.get_type())) {}
        ~SingleValueTraverser() = default;
        ValueType next() override { current_type = ValueType::end_of_document; return current_type; }
        ValueType get_type() override { return current_type; }

        std::string get_string() override { return std::string(value.as_string(), value.get_string_length()); }
        int64_t get_integer() override { return value.get_integer_value(); }
        double get_floating() override { return value.get_double_value(); }
        bool get_boolean() override { return (value.get_type() == sajson::TYPE_TRUE ? true : false); }

        // There should not be any valid reason to call these.
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            return std::make_unique<ObjectTraverser>(value);
        }
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            return std::make_unique<ArrayTraverser>(value);
        }

        private:
        sajson::value value;
        ValueType current_type;
    };

    public:
    // takes ownership of the buffer
    SajsonTraverser(std::string parser_name, ParsingState parsing_state, sajson::document document, char* buffer)
    : Traverser(parser_name, parsing_state)
    , document(std::move(document))
    , buffer(buffer)
    {
        sajson::value root = document.get_root();
        switch (root.get_type()) {
            case sajson::TYPE_OBJECT: {
                auto helper = std::make_unique<ObjectTraverser>(root);
                current_type = ValueType::object;
                container_stack.push(std::move(helper));
                break;
            }
            case sajson::TYPE_ARRAY: {
                auto helper = std::make_unique<ArrayTraverser>(root);
                current_type = ValueType::array;
                container_stack.push(std::move(helper));
                break;
            }
            default: {
                auto helper = std::make_unique<SingleValueTraverser>(root);
                current_type = sajsontype_to_fuzzytype(root.get_type());
                container_stack.push(std::move(helper));
                break;
            }
        }
    }

    ~SajsonTraverser() { delete[] buffer; }

    ValueType next() override {
        if (container_stack.size() == 0) {
            current_type = ValueType::end_of_document;
            return current_type;
        }
        current_type = container_stack.top()->next();
        switch (current_type) {
            case ValueType::end_of_container :
                container_stack.pop();
                break;
            case ValueType::object :
                container_stack.push(container_stack.top()->get_object_traverser());
                break;
            case ValueType::array :
                container_stack.push(container_stack.top()->get_array_traverser());
                break;
        }
        
        return current_type;
    }
    ValueType get_current_type() { return current_type; }
    std::string get_string() { return container_stack.top()->get_string(); }
    int64_t get_integer() { return container_stack.top()->get_integer(); }
    double get_floating() { return container_stack.top()->get_floating(); }
    bool get_boolean() { return container_stack.top()->get_boolean(); }

    private:
    ValueType current_type;
    std::stack<std::unique_ptr<TraverserHelper>> container_stack;
    sajson::document document; // all values point to it
    char* buffer; // is used solely by document, but document won't delete it by itself.

    static ValueType sajsontype_to_fuzzytype(sajson::type type) {
        switch (type) {
            case sajson::TYPE_INTEGER :
                return ValueType::integer;
            case sajson::TYPE_DOUBLE :
                return ValueType::floating;
            case sajson::TYPE_NULL :
                return ValueType::null;
            case sajson::TYPE_FALSE :
            case sajson::TYPE_TRUE :
                return ValueType::boolean;
            case sajson::TYPE_STRING :
                return ValueType::string;
            case sajson::TYPE_ARRAY :
                return ValueType::array;
            case sajson::TYPE_OBJECT :
                return ValueType::object;
            default:
                return ValueType::error;
        }
    }
};

class SajsonParser : public Parser {
    public:
    SajsonParser()
    : Parser("sajson")
    {};

    ~SajsonParser() = default;

    std::unique_ptr<Traverser> parse(const char* json, int size) override
    {
        char* buffer = static_cast<char*>(malloc(size + 1));
        memcpy(buffer, json, size);
        sajson::document document = sajson::parse(sajson::dynamic_allocation(),
                                            sajson::mutable_string_view(size, buffer));
        if (document.is_valid()) {
            // takes ownership of the buffer
            return std::make_unique<SajsonTraverser>(get_name(), ParsingState::ok, std::move(document), buffer);
        }
        else {
            delete[] buffer;
            return std::make_unique<InvalidTraverser>(get_name());
        }
    }

};
}

#endif