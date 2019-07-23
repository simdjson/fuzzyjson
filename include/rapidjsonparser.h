#ifndef RAPIDJSONPARSER_H
#define RAPIDJSONPARSER_H

#include <memory>
#include <string>

#include "parser.h"
#include "rapidjson/document.h"

namespace fuzzyjson {

class RapidjsonTraverser : public Traverser
{
    // forward declarations
    class ObjectTraverser;
    class ArrayTraverser;
    class SingleValueTraverser;

    class TraverserHelper {
        public:
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
        ObjectTraverser(rapidjson::Value::MemberIterator iterator, rapidjson::Value::MemberIterator end)
        : iterator(iterator)
        , end(end)
        , begin(true)
        , is_key(true)
        {}

        ~ObjectTraverser() = default;

        ValueType next() override {
            // The program won't crash if next() is called after the end.
            // Also used when the object is empty.
            if (iterator == end) {
                return ValueType::end_of_container;
            }

            if (begin) {
                begin = false;
                return ValueType::key;
            }

            // We have not advanced the iterator yet. If is_key is true now, it only means the last element was a key.
            // In that case, we now want the value corresponding to that key.
            if (is_key) {
                is_key = false;
                return get_value_type(iterator->value);
            }
            else {
                iterator++;
                if (iterator == end) {
                    return ValueType::end_of_container;
                }
                else {
                    // We fetch the key. Next time we'll fetch the value.
                    is_key = true;
                    return ValueType::key;
                }
            }
        }

        ValueType get_type() override {
            if (begin) {
                return ValueType::object;
            }
            if (is_key) {
                return ValueType::key;
            }
            return get_value_type(iterator->value);
        }

        std::string get_string() override { 
            if (is_key) {
                return std::string(iterator->name.GetString(), iterator->name.GetStringLength());
            }
            return std::string(iterator->value.GetString(), iterator->value.GetStringLength());
        }
        int64_t get_integer() override { return iterator->value.GetInt64(); }
        double get_floating() override { return iterator->value.GetDouble(); }
        bool get_boolean() override { return iterator->value.GetBool(); }
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            const auto& object = iterator->value.GetObject();
            return std::make_unique<ObjectTraverser>(object.MemberBegin(), object.MemberEnd());
        };
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            const auto& array = iterator->value.GetArray();
            return std::make_unique<ArrayTraverser>(array.Begin(), array.End());
        };

        private:
        rapidjson::Value::MemberIterator iterator;
        rapidjson::Value::MemberIterator end;
        bool begin;
        bool is_key;
    };

    class ArrayTraverser : public TraverserHelper {
        public:
        ArrayTraverser(rapidjson::Value::ValueIterator iterator, rapidjson::Value::ValueIterator end)
        : iterator(iterator)
        , end(end)
        , begin(true)
        {}

        ~ArrayTraverser() = default;

        ValueType next() override {
            if (begin) {
                begin = false;
            } 
            else {
                iterator++;
            }

            if (iterator == end) {
                return ValueType::end_of_container;
            }
            else {
                return get_value_type(*iterator);
            }
        }

        ValueType get_type() override {
            if (begin) {
                return ValueType::array;
            }
            return get_value_type(*iterator);
        }

        std::string get_string() override { return std::string(iterator->GetString(), iterator->GetStringLength()); }
        int64_t get_integer() override { return iterator->GetInt64(); }
        double get_floating() override { return iterator->GetDouble(); }
        bool get_boolean() override { return iterator->GetBool(); }
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            const auto& object = iterator->GetObject();
            return std::make_unique<ObjectTraverser>(object.MemberBegin(), object.MemberEnd());
        };
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            const auto& array = iterator->GetArray();
            return std::make_unique<ArrayTraverser>(array.Begin(), array.End());
        };

        private:
        rapidjson::Value::ValueIterator iterator;
        rapidjson::Value::ValueIterator end;
        bool begin;
    };

    // To use when the document has one single value
    class SingleValueTraverser : public TraverserHelper {
        public:
        SingleValueTraverser(rapidjson::Value* value)
        : value(value)
        , current_type(get_value_type(*value))
        {}
        ~SingleValueTraverser() = default;
        ValueType next() override { current_type = ValueType::end_of_document; return current_type; }
        ValueType get_type() override { return current_type; }

        std::string get_string() override { return std::string(value->GetString(), value->GetStringLength()); }
        int64_t get_integer() override { return value->GetInt64(); }
        double get_floating() override { return value->GetDouble(); }
        bool get_boolean() override { return value->GetBool(); }

        // There should not be any valid reason to call these.
        std::unique_ptr<ObjectTraverser> get_object_traverser() override {
            const auto& object = value->GetObject();
            return std::make_unique<ObjectTraverser>(value->MemberBegin(), value->MemberEnd());
        };
        std::unique_ptr<ArrayTraverser> get_array_traverser() override {
            const auto& array = value->GetArray();
            return std::make_unique<ArrayTraverser>(value->Begin(), value->End());
        };

        private:
        rapidjson::Value* value;
        ValueType current_type;
    };

    static ValueType get_value_type(const rapidjson::Value& value) {
        rapidjson::Type rapidjson_type = value.GetType();
        switch (rapidjson_type) {
            case rapidjson::Type::kNullType :
                return ValueType::null;
            case rapidjson::Type::kFalseType :
            case rapidjson::Type::kTrueType :
                return ValueType::boolean;
            case rapidjson::Type::kObjectType :
                return ValueType::object;
            case rapidjson::Type::kArrayType :
                return ValueType::array;
            case rapidjson::Type::kStringType :
                return ValueType::string;
            case rapidjson::Type::kNumberType :
                if (value.IsInt64()) {
                    return ValueType::integer;
                }
                else { // we assume value.IsDouble() is true
                    return ValueType::floating;
                }
            default: // something went wrong
                return ValueType::error;
        }
    }

    public:
    RapidjsonTraverser(std::string parser_name, ParsingState parsing_state, rapidjson::Document document)
    : Traverser(parser_name, parsing_state)
    , document(std::move(document))
    {
        current_type = get_value_type(this->document);

        if (current_type == ValueType::object) {
            const auto& object = this->document.GetObject();
            container_stack.push(std::make_unique<ObjectTraverser>(object.MemberBegin(), object.MemberEnd()));
        }
        else if (current_type == ValueType::array) {
            const auto& array = this->document.GetArray();
            container_stack.push(std::make_unique<ArrayTraverser>(array.Begin(), array.End()));
        }
        else { // single value
            container_stack.push(std::make_unique<SingleValueTraverser>(&(this->document)));
        }
    }
    
    ~RapidjsonTraverser() {}

    bool is_known_problem(const char* json, int size) override
    {
        if (get_parsing_state() == ParsingState::ok) {
            // rapidjson does not detect any problem when one of these values is on the first byte.
            switch (static_cast<unsigned char>(json[0])) {
                case 0xbb:
                case 0xbf:
                case 0xef:
                    return true;
                default:
                    break;
            }

            // rapidjson does not detect any problem when there is a null character after the json data
            int i = size-1;
            while (json[i] != '}' || json[i] != ']' || json[i] != '"') {
                if (json[i] == 0) {
                    return true;
                }
                i--;
            }
        }

        return false;
    }

    ValueType next() {
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
    rapidjson::Document document; // the variable itself is not used, but it owns pointers that are used everywhere
    ValueType current_type;
    std::stack<std::unique_ptr<TraverserHelper>> container_stack;
};

class RapidjsonParser : public Parser {
    public:
    RapidjsonParser()
    : Parser("rapidjson")
    {}

    ~RapidjsonParser() override = default;

    std::unique_ptr<Traverser> parse(const char* json, int size) override
    {
        rapidjson::Document document;
        document.Parse<rapidjson::kParseValidateEncodingFlag>(json, size);

        auto rapidjson_result = static_cast<rapidjson::ParseErrorCode>(document.GetParseError());

        if (rapidjson_result == rapidjson::ParseErrorCode::kParseErrorNone) {
            return std::make_unique<RapidjsonTraverser>(get_name(), ParsingState::ok, std::move(document));
        }
        else {
            return std::make_unique<InvalidTraverser>(get_name());
        }
    }
};
}

#endif