/*
 *  sample.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This is a simple sample program that demonstrates how to use the
 *      ProgramOptions object.
 *
 *  Portability Issues:
 *      None.
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <wchar.h>
#endif
#include <terra/program_options/program_options.h>

void usage()
{
    const std::string Indent_String = "                ";
    std::cout << "usage: tar_like [-c|--create] [-z|--compress] "
              << "[-v] [{-f|--filename} <filename>]"
              << std::endl << Indent_String
              << "[-?|--help] [{-C | --directory} <directory>]"
              << std::endl << Indent_String
              << "[--exclude <exclude expression>...] [{-l|--level} <level>]"
              << std::endl << Indent_String
              << "<FILE> ..."
              << std::endl;
}

#ifdef _WIN32
// This function is used on Windows to convert UTF-16 to UTF-8
std::vector<std::string> convert_arguments(const int argc,
                                           const wchar_t *const argv[])
{
    std::vector<std::string> arguments;

    for (std::size_t i = 0; i < argc; i++)
    {
        // How many characters are in the string?
        auto length = wcslen(argv[i]);

        // If the length is zero, just push an empty string onto the vector
        if (length == 0)
        {
            arguments.emplace_back(std::string());
            continue;
        }

        // Compute space required to convert UTF-16LE to UTF-8
        std::size_t octets = WideCharToMultiByte(CP_UTF8,
                                                 0,
                                                 argv[i],
                                                 length,
                                                 nullptr,
                                                 0,
                                                 nullptr,
                                                 nullptr);

        // A zero indicates an error
        if (octets == 0)
        {
            throw std::runtime_error("Failed to convert command arguments");
        }

        // Allocate string with space for converted argument
        std::string argument(octets, '\0');

        // Convert the argument to UTF-8, ensuring length is as expected
        if (WideCharToMultiByte(CP_UTF8,
                                0,
                                argv[i],
                                length,
                                argument.data(),
                                argument.size(),
                                nullptr,
                                nullptr) != octets)
        {
            throw std::runtime_error("Error converting command arguments");
        };

        // Put the string on the arguments vector
        arguments.emplace_back(argument);
    }

    return arguments;
}
#endif

template<typename T>
bool parse_options(Terra::ProgramOptions::Parser &parser,
                   const int argc,
                   const T * const argv[])
{
    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short  Long         Multi   Argument
        { "create",    "c", "create",    false,  false },
        { "compress",  "z", "compress",  false,  false },
        { "help",      "?", "help",      false,  false },
        { "level",     "l", "level",     false,  true  },
        { "filename",  "f", "filename",  false,  true  },
        { "verbose",   "v", "",          true,   false },
        { "directory", "C", "directory", false,  true  },
        { "exclude",   "",  "exclude",   true,   true  }
    };
    // clang-format on

    // Configure the programs option object with the above options specification

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        std::cout << "Program options specification error: "
                  << e.what()
                  << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cout << "Unknown error parsing program options specification: "
                  << e.what()
                  << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "Unknown error parsing program options specification"
                  << std::endl;
        return false;
    }

    // Now parse the program options

    try
    {
#ifdef _WIN32
        // Windows provides arguments as wchar_t; convert them to UTF-8
        std::vector<std::string> arguments = convert_arguments(argc, argv);
        parser.ParseArguments(arguments);
#else
        parser.ParseArguments(argc, argv);
#endif
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << e.what() << std::endl << std::endl;
        usage();
        return false;
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error parsing program options: "
                  << e.what()
                  << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "Unexpected error parsing program options"
                  << std::endl;
        return false;
    }

    return true;
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    Terra::ProgramOptions::Parser parser;       // Program options parser
    std::size_t count;                          // Option count value

#ifdef _WIN32
    // On Windows, ensure output uses UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // If no options were given, that is an error
    if (argc == 1)
    {
        usage();
        return EXIT_FAILURE;
    }

    // Parse the program options using the program_options object
    if (!parse_options(parser, argc, argv)) return EXIT_FAILURE;

    std::cout << std::endl
              << "Inspecting program options..."
              << std::endl
              << std::endl;

    try
    {
        // Produce output based on the program options presented
        if (parser.GetOptionCount("help") > 0)
        {
            usage();
            return EXIT_SUCCESS;
        }

        if (parser.GetOptionCount("create") > 0)
        {
            std::cout << "create flag was provided" << std::endl;
        }

        if (parser.GetOptionCount("compress") > 0)
        {
            std::cout << "compress flag was provided" << std::endl;
        }

        if (parser.GetOptionCount("filename") > 0)
        {
            std::string filename = parser.GetOptionString("filename");
            std::cout << "filename flag was provided with value = "
                      << filename
                      << std::endl;
        }

        count = parser.GetOptionCount("verbose");
        if (count > 0)
        {
            std::cout << "verbose flag was provided with "
                      << count
                      << " levels of verbosity"
                      << std::endl;
        }

        if (parser.GetOptionCount("directory") > 0)
        {
            std::string directory = parser.GetOptionString("directory");
            std::cout << "directory flag was provided with value = "
                      << directory
                      << std::endl;
        }

        if (parser.GetOptionCount("level") > 0)
        {
            unsigned level;
            parser.GetOptionValue("level", level, unsigned(0), unsigned(99));
            std::cout << "level flag was provided with value = "
                      << level
                      << std::endl;
        }

        if (parser.GetOptionCount("exclude") > 0)
        {
            std::vector<std::string> excludes =
                                            parser.GetOptionStrings("exclude");

            std::cout << "exclude flag was provided with the following values:"
                      << std::endl;
            for (const auto &exclude : excludes)
            {
                std::cout << "    " << exclude << std::endl;
            }
        }

        // Any program parameters not associated with an option wind up here
        if (parser.GetOptionCount("") > 0)
        {
            std::vector<std::string> filenames = parser.GetOptionStrings("");

            std::cout << "filenames specified:" << std::endl;
            for (const auto &filename : filenames)
            {
                std::cout << "    " << filename << std::endl;
            }
        }
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "Unknown error processing arguments" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
