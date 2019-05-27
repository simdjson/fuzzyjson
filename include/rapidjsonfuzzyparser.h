#ifndef RAPIDJSONFUZZYPARSER_H
#define RAPIDJSONFUZZYPARSER_H

#include <string>

#include "fuzzyjsonparser.h"
#include "rapidjson/document.h"

namespace fuzzyjson {

class RapidjsonFuzzyParser : public FuzzyJsonParser {
    public:
    RapidjsonFuzzyParser() : FuzzyJsonParser("rapidjson") {};
    FuzzyParserResult parse(char* json, int size) override
    {
        rapidjson::Document d;
        d.Parse(json, size);

        rapidjson::ParseErrorCode result = static_cast<rapidjson::ParseErrorCode>(d.GetParseError());

        return result_correspondances.at(result);
    }

    std::unordered_map<rapidjson::ParseErrorCode, FuzzyParserResult> result_correspondances {
        { rapidjson::ParseErrorCode::kParseErrorNone, FuzzyParserResult::ok },
        { rapidjson::ParseErrorCode::kParseErrorDocumentEmpty, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorDocumentRootNotSingular, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorValueInvalid, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissName, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissColon, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorObjectMissCommaOrCurlyBracket, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorArrayMissCommaOrSquareBracket, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorStringUnicodeEscapeInvalidHex, FuzzyParserResult::encoding_error },
        { rapidjson::ParseErrorCode::kParseErrorStringUnicodeSurrogateInvalid, FuzzyParserResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringEscapeInvalid, FuzzyParserResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringMissQuotationMark, FuzzyParserResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorStringInvalidEncoding, FuzzyParserResult::string_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberTooBig, FuzzyParserResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberMissFraction, FuzzyParserResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorNumberMissExponent, FuzzyParserResult::number_error },
        { rapidjson::ParseErrorCode::kParseErrorTermination, FuzzyParserResult::other_error },
        { rapidjson::ParseErrorCode::kParseErrorUnspecificSyntaxError, FuzzyParserResult::other_error },
    };
};
}

#endif