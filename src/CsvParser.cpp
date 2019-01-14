/*
 * Created by Anton Zhigaylo <antoooon@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the MIT License
 */

#include <atomic>
#include <thread>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <parser/CsvParserImpl.hpp>

namespace
{
volatile std::sig_atomic_t is_interrupt;

void signalHandler(int sig)
{
    is_interrupt = sig;

    switch (sig)
    {
    case SIGTERM:
    case SIGINT:
    {
        std::cout << "CsvParser: Application termination requested [Signal:" << sig << "]" << std::endl;
        break;
    }
    default:
    {
        std::cerr << "CsvParser: Fatal signal obtained: [Signal:" << sig << "], exit..." << std::endl;
        _exit(EXIT_FAILURE);
    }
    }
}

void configureSignalHandlers()
{
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

} // namespace

int main(int argc, const char** argv)
{
    boost::filesystem::path cvs_file;
    boost::filesystem::path gtw_table_file;

    boost::program_options::options_description desc("CSV file parser");
    desc.add_options()
        ("help,h",    "produce help message")
        ("csv file,i", boost::program_options::value<boost::filesystem::path>(&cvs_file)->default_value(""),
                       std::string("Specify *.csv file with project information").c_str())
        ("gtw file,o", boost::program_options::value<boost::filesystem::path>(&gtw_table_file)->default_value(""),
                       std::string("Specify gatway table path.").c_str());

    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);
    }
    catch (const boost::program_options::error& e)
    {
        std::cerr << "CsvParser: Error while parsing command options:" <<  e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    if (true == cvs_file.empty() || true == gtw_table_file.empty())
    {
        std::cerr << "CsvParser: incoming csv and output gatway file must be specified !" << std::endl;
        return EXIT_FAILURE;
    }

    // Setup signal handlers
    configureSignalHandlers();

    try
    {
        std::cout << "CvsParser: CSV file parser starting..." << std::endl;

        std::unique_ptr<Parsers::CCsvPrserImpl> csv_parser;
        csv_parser.reset(new Parsers::CCsvPrserImpl(cvs_file.string(), gtw_table_file.string()));

        csv_parser->parseCsvProject();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "CsvParser: convertation done.." << std::endl;
        std::cout << "CsvParser: CSV file parser Stopping..." << std::endl;


        std::cout << "CsvParser: CSV file parser Stopped." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "CsvParser: Error while parsing gtw file: " <<  e.what() << std::endl;
        return 1;
    }

    return EXIT_SUCCESS;
}
