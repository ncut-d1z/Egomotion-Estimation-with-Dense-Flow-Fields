#include "application_main.hpp"
#include "process_record.hpp"
#include "stream_output.hpp"

#include <filesystem>
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        return egomotion::cli::recordProcess(argc, argv, egomotionApplication,
            std::filesystem::current_path(), std::cout, std::cerr);
    } catch (const std::exception& error) {
        return egomotion::cli::streams::outputFailure(std::cerr, error.what());
    }
}
