#include "fuzzyjson.h"
#include "args.hxx"
#include "rapidjsonparser.h"
#include "simdjsonparser.h"

int main(int argc, char **argv) {
    args::ArgumentParser argument_parser("", "");
    args::HelpFlag help(argument_parser, "help", "Display this help menu", {'h', "help"});

    // RandomJson settings
    args::ValueFlag<std::string> file(argument_parser, "string", "The json filename",  {'f', "file"});
    args::ValueFlag<int> size(argument_parser, "int", "The size of the json file to generate", {'s', "size"});
    args::ValueFlag<int> generation_seed(argument_parser, "int", "The seed used by the number generator to generate a json file", {'g', "generation_seed"});
    args::ValueFlag<int> mutation_seed(argument_parser, "int", "The seed used by the number generator to mutate the json file", {'m', "mutation_seed"});
    // FuzzyJson setting
    args::ValueFlag<int> id(argument_parser, "int", "An identifier to distinguish FuzzyJson processes.", {'i', "id"});
    args::ValueFlag<int> max_mutations(argument_parser, "int", "The max number of mutations", {'a', "max_mutations"});
    args::ValueFlag<double> precision(argument_parser, "double", "The precision used to compare floating points.", {'p', "precision"});
    args::ValueFlag<bool> verbose(argument_parser, "bool", "Whether fuzzyjson will print infos or not.", {'v', "verbose"});

    args::CompletionFlag completion(argument_parser, {"complete"});
    try {
        argument_parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e) {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&) {
        std::cout << argument_parser;
        return 0;
    }
    catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << argument_parser;
        return 1;
    }

    randomjson::Settings json_settings;
    fuzzyjson::Settings fuzzy_settings;

    if (file) {
        json_settings.filepath = file.Get();
    }
    if (size) {
       json_settings.size = size.Get();
    }
    if (generation_seed) {
        json_settings.generation_seed = generation_seed.Get();
    }
    if (mutation_seed) {
        json_settings.mutation_seed = mutation_seed.Get();
    }
    if (id) {
        std::cout << id.Get() << std::endl;
        fuzzy_settings.id = id.Get();
        std::cout << fuzzy_settings.id << std::endl;
    }
    if (max_mutations) {
        fuzzy_settings.max_mutations = max_mutations.Get();
    }
    if (precision) {
        fuzzy_settings.floating_precision = precision.Get();
    }
    if (verbose) {
        fuzzy_settings.verbose = verbose.Get();
    }

    fuzzyjson::FuzzyJson fuzzy(json_settings, fuzzy_settings);
    fuzzy.add_parser(std::make_unique<fuzzyjson::SimdjsonParser>());
    fuzzy.add_parser(std::make_unique<fuzzyjson::RapidjsonParser>());
    fuzzy.fuzz();
}