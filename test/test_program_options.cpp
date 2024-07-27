/*
 *  test_program_options.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file will test the Program Options library.
 *
 *  Portability Issues:
 *      None.
 */

#include <terra/program_options/program_options.h>
#include <terra/stf/stf.h>

// The following is used to test the move constructor
Terra::ProgramOptions::Options GetCommandParser()
{
    // clang-format off
    Terra::ProgramOptions::Options options(
        {
        //    Name       Short    Long        Multi  Argument
            { "all",       "a",   "all",      false, false },
            { "pattern",   "p",   "pattern",  true,  true  },
            { "color",     "c",   "color",    false, true  },
            { "size",      "s",   "min-size", false, true  }
        });

    return options;
}

// Test a simple example using the argc/argv interface
STF_TEST(ProgramOptions, TestCommand)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.SetOptions(options);
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        std::cout << "Error parsing program options spec: " << e.what()
                  << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << "Error parsing program options: " << e.what() << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error parsing program options: " << e.what()
                  << std::endl;
        std::cout << "Exception type: " << typeid(e).name() << std::endl;
        STF_ASSERT_TRUE(false);
    }

    // Ensure parameter counts are as expected
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("all"));
    STF_ASSERT_EQ(std::size_t(2), parser.GetOptionCount("pattern"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("color"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("size"));
    STF_ASSERT_EQ(std::size_t(3), parser.GetOptionCount(""));

    // The two pattern values should be "foo" and "bar"
    std::vector<std::string> patterns = parser.GetOptionStrings("pattern");
    STF_ASSERT_EQ(std::string("foo"), patterns[0]);
    STF_ASSERT_EQ(std::string("bar"), patterns[1]);

    // The size parameter should be 20
    unsigned size;
    parser.GetOptionValue("size", size);
    STF_ASSERT_EQ(unsigned(20), size);

    // Same size parameter via vector of values
    std::vector<std::size_t> sizes;
    parser.GetOptionValues("size", sizes);
    STF_ASSERT_EQ(std::size_t(1), sizes.size());
    STF_ASSERT_EQ(std::size_t(20), sizes.front());

    // There should be 3 files
    std::vector<std::string> filenames = parser.GetOptionStrings("");
    STF_ASSERT_EQ(std::size_t(3), filenames.size());
    STF_ASSERT_EQ(std::string("file1"), filenames[0]);
    STF_ASSERT_EQ(std::string("file2"), filenames[1]);
    STF_ASSERT_EQ(std::string("file3"), filenames[2]);
}

// Test like above, but exercise copy, assignment, and move operations
STF_TEST(ProgramOptions, TestCommandUsingMoveConstructor)
{
    // Copy elision should optimize this
    Terra::ProgramOptions::Parser parser_0{GetCommandParser()};

    // Force use of copy constructor constructor
    Terra::ProgramOptions::Parser parser_1{parser_0};

    // Force use of assignment operator
    Terra::ProgramOptions::Parser parser_2;
    parser_2 = parser_1;

    // Use std::move to explicitly invoke the move constructor
    Terra::ProgramOptions::Parser parser{std::move(parser_2)};

    // The rest replicates the above test logic

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        std::cout << "Error parsing program options spec: " << e.what()
                  << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << "Error parsing program options: " << e.what() << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error parsing program options: " << e.what()
                  << std::endl;
        std::cout << "Exception type: " << typeid(e).name() << std::endl;
        STF_ASSERT_TRUE(false);
    }

    // Ensure parameter counts are as expected
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("all"));
    STF_ASSERT_EQ(std::size_t(2), parser.GetOptionCount("pattern"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("color"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("size"));
    STF_ASSERT_EQ(std::size_t(3), parser.GetOptionCount(""));

    // The two pattern values should be "foo" and "bar"
    std::vector<std::string> patterns = parser.GetOptionStrings("pattern");
    STF_ASSERT_EQ(std::string("foo"), patterns[0]);
    STF_ASSERT_EQ(std::string("bar"), patterns[1]);

    // The size parameter should be 20
    unsigned size;
    parser.GetOptionValue("size", size);
    STF_ASSERT_EQ(unsigned(20), size);

    // Same size parameter via vector of values
    std::vector<std::size_t> sizes;
    parser.GetOptionValues("size", sizes);
    STF_ASSERT_EQ(std::size_t(1), sizes.size());
    STF_ASSERT_EQ(std::size_t(20), sizes.front());

    // There should be 3 files
    std::vector<std::string> filenames = parser.GetOptionStrings("");
    STF_ASSERT_EQ(std::size_t(3), filenames.size());
    STF_ASSERT_EQ(std::string("file1"), filenames[0]);
    STF_ASSERT_EQ(std::string("file2"), filenames[1]);
    STF_ASSERT_EQ(std::string("file3"), filenames[2]);
}

// Test a simple example using the string_view interface
STF_TEST(ProgramOptions, TestCommandStringVector)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    // Simulate command-line arguments
    std::vector<std::string> argv =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.SetOptions(options);
        parser.ParseArguments(argv);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        std::cout << "Error parsing program options spec: " << e.what()
                  << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << "Error parsing program options: " << e.what() << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error parsing program options: " << e.what()
                  << std::endl;
        std::cout << "Exception type: " << typeid(e).name() << std::endl;
        STF_ASSERT_TRUE(false);
    }

    // Ensure parameter counts are as expected
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("all"));
    STF_ASSERT_EQ(std::size_t(2), parser.GetOptionCount("pattern"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("color"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("size"));
    STF_ASSERT_EQ(std::size_t(3), parser.GetOptionCount(""));

    // The two pattern values should be "foo" and "bar"
    std::vector<std::string> patterns = parser.GetOptionStrings("pattern");
    STF_ASSERT_EQ(std::string("foo"), patterns[0]);
    STF_ASSERT_EQ(std::string("bar"), patterns[1]);

    // The size parameter should be 20
    unsigned size;
    parser.GetOptionValue("size", size);
    STF_ASSERT_EQ(unsigned(20), size);

    // Same size parameter via vector of values
    std::vector<std::size_t> sizes;
    parser.GetOptionValues("size", sizes);
    STF_ASSERT_EQ(std::size_t(1), sizes.size());
    STF_ASSERT_EQ(std::size_t(20), sizes.front());

    // There should be 3 files
    std::vector<std::string> filenames = parser.GetOptionStrings("");
    STF_ASSERT_EQ(std::size_t(3), filenames.size());
    STF_ASSERT_EQ(std::string("file1"), filenames[0]);
    STF_ASSERT_EQ(std::string("file2"), filenames[1]);
    STF_ASSERT_EQ(std::string("file3"), filenames[2]);
}

// Test that options spec errors: flag conflict
STF_TEST(ProgramOptions, TestOptionsSpecFlagConflict)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options_spec =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "create",   false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options_spec, {"-"}, {"-"});
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                            Terra::ProgramOptions::OptionsError::FlagConflict)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options spec errors: empty identifier name
STF_TEST(ProgramOptions, TestOptionsSpecEmptyIdentifier)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { ""       ,   "p",   "pattern",  true,  true  },
        { "color",     "c",   "create",   false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::EmptyIdentifierName)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options spec errors: duplicate identifier
STF_TEST(ProgramOptions, TestOptionsSpecDuplicateIdentifier)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "create",   false, true  },
        { "all",       "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::DuplicateIdentifier)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options spec errors: duplicate short option
STF_TEST(ProgramOptions, TestOptionsSpecDuplicateShortOption)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "create",   false, true  },
        { "size",      "a",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::DuplicateShortOption)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options spec errors: duplicate long option
STF_TEST(ProgramOptions, TestOptionsSpecDuplicateLongOption)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "all",      false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::DuplicateLongOption)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options spec errors: invalid short option
STF_TEST(ProgramOptions, TestOptionsSpecInvalidShortOption1)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "pp",  "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::InvalidShortOption)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: invalid short option
STF_TEST(ProgramOptions, InvalidShortOption2)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-q",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::InvalidShortOption)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: invalid long option
STF_TEST(ProgramOptions, InvalidLongOptionParsing)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--InvalidOption",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::InvalidLongOption)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: multiple instances
STF_TEST(ProgramOptions, MultipleInstances1)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "--color",
        "blue",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::MultipleInstances)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: multiple instances
STF_TEST(ProgramOptions, MultipleInstances2)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-s",
        "99",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::MultipleInstances)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: missing option argument
STF_TEST(ProgramOptions, MissingOptionArgument)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 10;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                    Terra::ProgramOptions::OptionsError::MissingOptionArgument)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: option not given
STF_TEST(ProgramOptions, OptionNotGiven)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 11;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);

        // Try to get the missing "-s" argument
        std::string value = parser.GetOptionString("size");
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::OptionNotGiven)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test that options parsing errors: option not given
STF_TEST(ProgramOptions, OptionGiven)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 11;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);
    }
    catch (...)
    {
        exception_caught = true;
    }

    // Ensure there were no parsing errors
    STF_ASSERT_FALSE(exception_caught);

    // Verify presence and absence of options
    STF_ASSERT_TRUE(parser.OptionGiven("color"));
    STF_ASSERT_FALSE(parser.OptionGiven("size"));
}

// Test that options parsing errors: option value error
STF_TEST(ProgramOptions, OptionValueError)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    bool exception_caught = false;

    try
    {
        parser.SetOptions(options);
    }
    catch (...)
    {
        // Should not land here
        exception_caught = true;
    }

    // There should be no exception so far
    STF_ASSERT_FALSE(exception_caught);

    // Simulate command-line arguments
    const int argc = 13;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "foo",
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "200",
        "file2",
        "file3"
    };

    try
    {
        parser.ParseArguments(argc, argv);

        // Try to get a constrained value where the given value is out of range
        unsigned value{};
        parser.GetOptionValue("size", value, unsigned(0), unsigned(99));
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        if (e.options_error ==
                        Terra::ProgramOptions::OptionsError::OptionValueError)
        {
            exception_caught = true;
        }
    }
    catch (...)
    {
        // Should not land here
    }

    STF_ASSERT_TRUE(exception_caught);
}

// Test zero-length arguments
STF_TEST(ProgramOptions, TestZeroLengthArgument)
{
    Terra::ProgramOptions::Parser parser;

    // clang-format off
    const Terra::ProgramOptions::Options options =
    {
    //    Name       Short    Long        Multi  Argument
        { "all",       "a",   "all",      false, false },
        { "pattern",   "p",   "pattern",  true,  true  },
        { "color",     "c",   "color",    false, true  },
        { "size",      "s",   "min-size", false, true  }
    };
    // clang-format on

    // Simulate command-line arguments
    const int argc = 14;
    const char *argv[] =
    {
        "ls_type_program",
        "-a",
        "-p",
        "", // Empty pattern
        "file1",
        "-p",
        "bar",
        "--color",
        "red",
        "-s",
        "20",
        "file2",
        "", // Empty file name
        "file3"
    };

    try
    {
        parser.SetOptions(options);
        parser.ParseArguments(argc, argv);
    }
    catch (const Terra::ProgramOptions::SpecificationException &e)
    {
        std::cout << "Error parsing program options spec: " << e.what()
                  << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const Terra::ProgramOptions::OptionsException &e)
    {
        std::cout << "Error parsing program options: " << e.what() << std::endl;
        STF_ASSERT_TRUE(false);
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error parsing program options: " << e.what()
                  << std::endl;
        std::cout << "Exception type: " << typeid(e).name() << std::endl;
        STF_ASSERT_TRUE(false);
    }

    // Ensure parameter counts are as expected
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("all"));
    STF_ASSERT_EQ(std::size_t(2), parser.GetOptionCount("pattern"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("color"));
    STF_ASSERT_EQ(std::size_t(1), parser.GetOptionCount("size"));
    STF_ASSERT_EQ(std::size_t(4), parser.GetOptionCount(""));

    // The two pattern values should be "" and "bar"
    std::vector<std::string> patterns = parser.GetOptionStrings("pattern");
    STF_ASSERT_EQ(std::string(""), patterns[0]);
    STF_ASSERT_EQ(std::string("bar"), patterns[1]);

    // The size parameter should be 20
    unsigned size;
    parser.GetOptionValue("size", size);
    STF_ASSERT_EQ(unsigned(20), size);

    // Same size parameter via vector of values
    std::vector<std::size_t> sizes;
    parser.GetOptionValues("size", sizes);
    STF_ASSERT_EQ(std::size_t(1), sizes.size());
    STF_ASSERT_EQ(std::size_t(20), sizes.front());

    // There should be 3 files
    std::vector<std::string> filenames = parser.GetOptionStrings("");
    STF_ASSERT_EQ(std::size_t(4), filenames.size());
    STF_ASSERT_EQ(std::string("file1"), filenames[0]);
    STF_ASSERT_EQ(std::string("file2"), filenames[1]);
    STF_ASSERT_EQ(std::string(""), filenames[2]);
    STF_ASSERT_EQ(std::string("file3"), filenames[3]);
}
