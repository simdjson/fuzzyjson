#ifndef RAPIDJSONPARSER_H
#define RAPIDJSONPARSER_H

#include <string>

#include "parser.h"
#include "rapidjson/document.h"

namespace fuzzyjson {

ValueType get_value_type(const rapidjson::Value& value) {
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
            if (value.IsInt()) {
                return ValueType::integer;
            }
            else { // we assume value.IsDouble() is true
                return ValueType::floating;
            }
        default: // something went wrong
            return ValueType::error;
    }
}

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
        {}

        ~ObjectTraverser() = default;

        ValueType next() override {
            if (begin) {
                begin = false;
                is_key = true;
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
            return get_value_type(iterator->value);
        }

        std::string get_string() override { 
            if (is_key) {
                return iterator->name.GetString();
            }
            return iterator->value.GetString();
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

        std::string get_string() override { return iterator->GetString(); }
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
        SingleValueTraverser(rapidjson::Value* value) : value(value) {}
        ~SingleValueTraverser() = default;
        ValueType next() override { return ValueType::end_of_document; }
        ValueType get_type() override { return get_value_type(*value); }

        std::string get_string() override { return value->GetString(); }
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
    };

    public:
    RapidjsonTraverser(std::string parser_name, ParsingResult parsing_result, rapidjson::Document document)
    : Traverser(parser_name, parsing_result)
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
    rapidjson::Document document; // the variable is not used, but it owns pointers that are used everywhere
    ValueType current_type;
    std::stack<std::unique_ptr<TraverserHelper>> container_stack;
};

class RapidjsonParser : public Parser {
    public:
    RapidjsonParser()
    : Parser("rapidjson")
    {
    };

    ~RapidjsonParser() override = default;

    std::unique_ptr<Traverser> parse(const char* json, int size) override
    {
        rapidjson::Document document;
        document.Parse(json, size);

        auto rapidjson_result = static_cast<rapidjson::ParseErrorCode>(document.GetParseError());
        ParsingResult parsing_result = result_correspondances.at(rapidjson_result);

        return std::make_unique<RapidjsonTraverser>(get_name(), parsing_result, std::move(document));
    }

    private:
    rapidjson::ParseErrorCode last_result;

    std::unordered_map<rapidjson::ParseErrorCode, ParsingResult> result_correspondances {
        { rapidjson::ParseErrorCode::kParseErrorNone, ParsingResult::ok },
        { rapidjson::ParseErrorCode::kParseErrorDocumentEmpty, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorDocumentRootNotSingular, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorValueInvalid, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissName, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissColon, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissCommaOrCurlyBracket, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorArrayMissCommaOrSquareBracket, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorStringUnicodeEscapeInvalidHex, ParsingResult::encoding_error },
        { rapidjson::ParseErrorCode::kParseErrorStringUnicodeSurrogateInvalid, ParsingResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringEscapeInvalid, ParsingResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringMissQuotationMark, ParsingResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringInvalidEncoding, ParsingResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberTooBig, ParsingResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberMissFraction, ParsingResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberMissExponent, ParsingResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorTermination, ParsingResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorUnspecificSyntaxError, ParsingResult::other_error },
    };
};
}

#endif