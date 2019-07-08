# FuzzyJson
FuzzyJson compares the parsing of different json parsers. Its goal is to find bugs.

It currently supports [simdjson](https://github.com/lemire/simdjson) and [rapidjson](https://github.com/Tencent/rapidjson/). Other parsers can be easily added.

## Simple usage
```
mkdir build
cd build
cmake ..
make
./fuzzytest --help
./fuzzytest --size 100 --max_mutations 1000
```
The argument --help displays the help menu to show all the options. Then we generate a 100 bytes random json document and compare the parsing with simdjson and rapidjson on 1000 mutations.

FuzzyJson is a lot more flexible when used as a library.

# Advanced usage
FuzzyJson uses [RandomJson](https://github.com/ioioioio/randomjson) to handle json documents. To create a fuzzyjson::FuzzyJson object, we must pass it a randomjson::Settings object.
```C
#include "randomjson.h"
#include "fuzzyjson.h"

int json_size = 100
randomjson::Settings json_settings(json_size);
fuzzyjson::FuzzyJson fuzzy(json_settings);
```

FuzzyJson can be configurated with a fuzzyjson::settings
```C
fuzzyjson::Settings fuzzy_settings;
fuzzy_settings.max_mutations = 5000;
fuzzyjson::FuzzyJson fuzzy2(json_settings, fuzzy_settings);
```

Once the fuzzyjson::FuzzyJson object is created, we must pass it the parsers we want to use.
```C
#include "simdjsonparser.h"
#include "rapidjsonparser.h"

fuzzy.add_parser(std::make_unique<RapidjsonParser>());
fuzzy.add_parser(std::make_unique<SimdjsonParser>());
```
Note that FuzzyJson will currently crash if there is only one parser. The desired behaviour for that case has not been decided yet.

The only thing left to do is to start the fuzz.
```C
fuzzy.fuzz();
```

## Reports
When a difference between parsings is detected, FuzzyJson generates a report and makes a copy of the json. The report gives information about how the json document has been generated and where the difference between parsings have been detected.

Instead of a copy of the json that caused a problem, it might sound preferable to regenerate the document from its settings (size, seeds, mutations, etc.). However, FuzzyJson revert all the mutations that generated an invalid documents. That means we would need a way to retrieve the mutations to skip. That would be easy to implement, the problem is that there are so many skipped mutations that the reports would likely become as big or bigger than the json document. 

FuzzyJson saves the current parsed json in a document called temp.json. Though it might sound inefficient, it is important to have a copy of the json that caused a problem in case of an unexpected crash, and no better solution has been found yet. If no crash occurred, the temp.json is deleted at the end of the execution.

## Add a new parser
To add a new parser, one must implements the class fuzzyjson::Parser in [include/parser.h](https://github.com/ioioioio/fuzzyjson/blob/master/include/parser.h). There are two examples in [include/simdjsonparser.h](https://github.com/ioioioio/fuzzyjson/blob/master/include/simdjsonparser.h) and [include/rapidjsonparser.h](https://github.com/ioioioio/fuzzyjson/blob/master/include/rapidjsonparser.h).

The Parser constructor requires a name to be given to the parser. It is used in the reports to identify the parser.

In order to make the parser work, one virtual function must be implemented: parse(). That function returns a fuzzyjson::Traverser. Though it is not mandatory, it has been found convenient to return a fuzzyjson::InvalidTraverser when the parsing fails.

The tricky part is to implement the fuzzyjson::Traverser for the parser. The Traverser must traverse the json data in a precise way. The functions test_simple_nested_document_parsing() and test_one_number_document_parsing() int [tests/unittests.cpp](https://github.com/ioioioio/fuzzyjson/blob/master/tests/unittests.cpp) show how it must be done. It is suggested to use them to assert the Traverser works properly.

## Multiprocessing
It is suggested to launch FuzzyJson in multiple processes. When FuzzyJson is launched in multiple processes, it is recommended to give each instance an id. Otherwise, each instances will overwrite the same temp.json (and possibly reports made on the same nanosecond), and useful information could be lost.

The "simple" way:
```
./fuzzytest --size 100 --max_mutations 1000 --id 1
```

The "advanced" way:
```C
fuzzyjson::Settings fuzzy_settings3;
fuzzy_settings3.id = 3;
fuzzyjson::FuzzyJson fuzzy3(json_settings, fuzzy_settings);
```

[run.py](https://github.com/ioioioio/fuzzyjson/blob/master/run.py) is already configured to launch a process on each processor on the machine. Though it is not showed in the Python script, to launch FuzzyJson like this could be particularly useful to handle crashes.
