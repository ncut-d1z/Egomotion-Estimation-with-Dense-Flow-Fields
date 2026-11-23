// Historical MAT dictionary command line. All MAT decoding and report
// generation live in src/io/; this file handles argv/exit/status only.
#include "egomotion/cli/mat_dictionary_command.hpp"

#include <iostream>

int main(int argc, char** argv) {
    return egomotion::cli::runMatDictionaryCommand(argc, argv, std::cerr);
}
