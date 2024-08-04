/*
 *  parser.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements the program options Parser object, which is used
 *      to process program options given to a program to influence execution
 *      behavior.
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#include <sstream>
#include <algorithm>
#include <cctype>
#include <terra/program_options/program_options.h>

namespace Terra::ProgramOptions
{

/*
 *  Parser::Parser()
 *
 *  Description:
 *      Parser constructor having no default program options.  It is assumed
 *      the caller will later call SetOptions() to provide the program
 *      options if something other than the defaults are desired.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
Parser::Parser() : Parser(Options{})
{
    // Nothing more to do
}

/*
 *  Parser::Parser()
 *
 *  Description:
 *      Parser constructor taking an Options and various optional
 *      configuration parameters.
 *
 *  Parameters:
 *      options [in]
 *          The valid program options.
 *
 *      short_flags [in]
 *          Vector of strings used to indicate program option flags (or
 *          "switches"), such as "-" or "/".  Defaults is "-".
 *
 *      long_flags [in]
 *          Vector of strings used to indicate program option flags (or
 *          "switches"), such as "-" or "/".  Defaults is "--".
 *
 *      option_value_separator [in]
 *          A string used to separate an option name from a value (e.g., "=" in
 *          an argument like "--param=value").  Default is "=".
 *
 *      case_insensitive [in]
 *          Indicates that options are matched case insensitively.  Default
 *          is false.
 *
 *  Returns:
 *      Nothing, though this constructor may throw an exception if the program
 *      options are invalid or if there is an error with the options flags.
 *
 *  Comments:
 *      None.
 */
Parser::Parser(Options options,
               std::vector<std::string> short_flags,
               std::vector<std::string> long_flags,
               std::string option_value_separator,
               bool case_insensitive) :
    options{std::move(options)},
    short_flags{std::move(short_flags)},
    long_flags{std::move(long_flags)},
    option_value_separator{std::move(option_value_separator)},
    case_insensitive{case_insensitive},
    option_map{}
{
}

/*
 *  Parser::SetOptions()
 *
 *  Description:
 *      This function will set the program options to use when ParseArguments()
 *      is subsequently called, along with flags, separator character, and a
 *      boolean indicating whether matches are case insensitive or not.  It
 *      will also clear any previously processed options.
 *
 *  Parameters:
 *      options [in]
 *          The valid program options.
 *
 *      short_flags [in]
 *          Vector of strings used to indicate program option flags (or
 *          "switches"), such as "-" or "/".  Defaults is "-".
 *
 *      long_flags [in]
 *          Vector of strings used to indicate program option flags (or
 *          "switches"), such as "-" or "/".  Defaults is "--".
 *
 *      option_value_separator [in]
 *          A string used to separate an option name from a value (e.g., "=" in
 *          an argument like "--param=value").  Default is "=".
 *
 *      case_insensitive [in]
 *          Indicates that options are matched case insensitively.  Default
 *          is false.
 *
 *  Returns:
 *      Nothing, though this function may throw an exception if the program
 *      options are invalid or if there is an error with the options flags.
 *
 *  Comments:
 *      None.
 */
void Parser::SetOptions(const Options &options,
                        const std::vector<std::string> &short_flags,
                        const std::vector<std::string> &long_flags,
                        const std::string &option_value_separator,
                        const bool case_insensitive)
{
    // Assign parameters to member variables
    this->options = options;
    this->short_flags = short_flags;
    this->long_flags = long_flags;
    this->option_value_separator = option_value_separator;
    this->case_insensitive = case_insensitive;

    // Clear any previously processed options
    ClearOptions();

    // Ensure the option flags are sane
    CheckOptionFlags();

    // Validate the options
    CheckOptions();
}

/*
 *  Parser::ClearOptions()
 *
 *  Description:
 *      Clear the internal data structures holding any previously parsed
 *      command-line options.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void Parser::ClearOptions()
{
    option_map.clear();
}

/*
 *  Parser::ParseArguments()
 *
 *  Description:
 *      This function will parse command-line arguments.
 *
 *  Parameters:
 *      argc [in]
 *          A count of the number of strings in the arv[] vector.  Note that
 *          this is precisely the same argument as used in C/C++ code in the
 *          main() function.  Thus, this must always be >= 1, since the first
 *          element in argv[] is the name of the program.  If argc is < 1,
 *          this function has no effect.
 *
 *      argv [in]
 *          A vector of null-terminated strings that are parsed by this object.
 *          The first element of the array is assumed to be the executable name
 *          and will be ignored.
 *
 *  Returns:
 *      Nothing, but if the user provides invalid input an exception will be
 *      thrown.
 *
 *  Comments:
 *      None.
 */
void Parser::ParseArguments(const int argc, const char *const argv[])
{
    if (argc > 0)
    {
        ParseArguments(std::vector<std::string_view>(argv, argv + argc));
    }
}

/*
 *  Parser::ParseArguments()
 *
 *  Description:
 *      This function will parse command-line arguments.
 *
 *  Parameters:
 *      arguments [in]
 *          A vector of user-provided program arguments.  This vector must
 *          include the command name (or some string, even if empty) as the
 *          first element of the vector to parallel the argv[] array.  The
 *          first element of the vector is skipped over when parsing.
 *
 *  Returns:
 *      Nothing, but if the user provides invalid input an exception will be
 *      thrown.
 *
 *  Comments:
 *      None.
 */
void Parser::ParseArguments(const std::vector<std::string> &arguments)
{
    ParseArguments(std::vector<std::string_view>(arguments.begin(),
                                                 arguments.end()));
}

/*
 *  Parser::ParseArguments()
 *
 *  Description:
 *      This function will parse command-line arguments.
 *
 *  Parameters:
 *      arguments [in]
 *          A vector of user-provided program arguments.  This vector must
 *          include the command name (or some string, even if empty) as the
 *          first element of the vector to parallel the argv[] array.  The
 *          first element of the vector is skipped over when parsing.
 *
 *  Returns:
 *      Nothing, but if the user provides invalid input an exception will be
 *      thrown.
 *
 *  Comments:
 *      None.
 */
void Parser::ParseArguments(const std::vector<std::string_view> &arguments)
{
    // There is nothing to do if the number arguments is <= 1
    if (arguments.size() <= 1) return;

    // Iterate over each of the arguments presented, ignoring the first element
    // since it is the name of the executed command
    for (std::size_t i = 1; i < arguments.size(); i++)
    {
        // Is there a parameter to pass?
        std::optional<std::string_view> parameter;

        if (i < (arguments.size() - 1)) parameter = arguments[i + 1];

        // Process the current argument, returning true if the next argument
        // (labeled parameter here) was also consumed
        if (ProcessArgument(arguments[i], parameter)) i++;
    }
}

/*
 *  Parser::OptionGiven()
 *
 *  Description:
 *      This function will return true if the specified option name was given
 *      by the user or false it was not.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name to use to see if an option was given.
 *
 *  Returns:
 *      True if the specified option was given by the user, false it it was not.
 *
 *  Comments:
 *      None.
 */
bool Parser::OptionGiven(const std::string &option_name)
{
    return option_map.contains(option_name);
}

/*
 *  Parser::GetOptionCount()
 *
 *  Description:
 *      This function will return a count of the number of times the specified
 *      option was given.  The string represents the option name.  An empty
 *      string refers to any command-line arguments that are not part
 *      of the command-line options (e.g., a list of filenames or other
 *      arguments given on the command-line).
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a count is desired.
 *
 *  Returns:
 *      A count of the number of options having the specified option name
 *      was parsed.  In the case of an empty string, it is a count of
 *      the unaffiliated command-line option strings observed.
 *
 *  Comments:
 *      None.
 */
std::size_t Parser::GetOptionCount(const std::string &option_name)
{
    auto it = option_map.find(option_name);

    if (it == option_map.end()) return 0;

    return (*it).second.size();
}

/*
 *  Parser::GetOptionString()
 *
 *  Description:
 *      This function will return the option string associated with a program
 *      option for which an argument is required.  In the case of arguments not
 *      associated with an option flag (e.g., a list of filenames or similar),
 *      those arguments are stored under the option name "" (empty string).
 *
 *      For options that do not have arguments, one should use OptionGiven()
 *      or GetOptionCount() to determine presence of the specified option.
 *
 *      If an option may be specified multiple times, one should call
 *      GetOptionStrings() instead of this function.  This function will return
 *      the argument string of only the first option string given if multiple
 *      instances are allowed.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a count is desired.
 *
 *  Returns:
 *      The string argument provided by the user for the given option name.
 *
 *  Comments:
 *      This function will throw an exception if the requested option was not
 *      given by the user.  One should first check for existence of an option
 *      by calling OptionGiven() or GetOptionCount().
 */
std::string Parser::GetOptionString(const std::string &option_name)
{
    return FindOptionStrings(option_name).front();
}

/*
 *  Parser::GetOptionStrings()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which values should be retrieved.
 *
 *  Returns:
 *      A vector of string arguments given by the user for the specified option
 *      name.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().
 */
std::vector<std::string> Parser::GetOptionStrings(
                                                const std::string &option_name)
{
    return FindOptionStrings(option_name);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<short>(
                                            const std::string &option_name,
                                            std::vector<short> &option_values,
                                            short min,
                                            short max)
{
    // Define the converter function
    auto converter = [min, max](const std::string &value) -> short
                     {
                         int i = std::stoi(value);

                         if ((i < min) || (i > max))
                         {
                             throw std::out_of_range("value out of range");
                         }

                         return static_cast<short>(i);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<unsigned short>(
                                    const std::string &option_name,
                                    std::vector<unsigned short> &option_values,
                                    unsigned short min,
                                    unsigned short max)
{
    // Define the converter function
    auto converter = [min, max](const std::string &value) -> unsigned short
                     {
                         int i = std::stoi(value);

                         if ((i < min) || (i > max))
                         {
                             throw std::out_of_range("value out of range");
                         }

                         return static_cast<unsigned short>(i);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<int>(
                                            const std::string &option_name,
                                            std::vector<int> &option_values,
                                            int min,
                                            int max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> int
                     {
                         return std::stoi(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<unsigned>(
                                        const std::string &option_name,
                                        std::vector<unsigned> &option_values,
                                        unsigned min,
                                        unsigned max)
{
    // Define the converter function
    auto converter = [min, max](const std::string &value) -> unsigned
                     {
                         unsigned long i = std::stoul(value);

                         if ((i < min) || (i > max))
                         {
                             throw std::out_of_range("value out of range");
                         }

                         return static_cast<unsigned>(i);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<long>(
                                            const std::string &option_name,
                                            std::vector<long> &option_values,
                                            long min,
                                            long max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> long
                     {
                         return std::stol(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<unsigned long>(
                                    const std::string &option_name,
                                    std::vector<unsigned long> &option_values,
                                    unsigned long min,
                                    unsigned long max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> unsigned long
                     {
                         return std::stoul(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<long long>(
                                        const std::string &option_name,
                                        std::vector<long long> &option_values,
                                        long long min,
                                        long long max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> long long
                     {
                         return std::stoll(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<unsigned long long>(
                                const std::string &option_name,
                                std::vector<unsigned long long> &option_values,
                                unsigned long long min,
                                unsigned long long max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> unsigned long long
                     {
                         return std::stoull(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<float>(
                                        const std::string &option_name,
                                        std::vector<float> &option_values,
                                        float min,
                                        float max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> float
                     {
                         return std::stof(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::GetOptionValues()
 *
 *  Description:
 *      This function will return the option values associated with a program
 *      option for which an argument is required.  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which a value should be retrieved.
 *
 *      option_values [out]
 *          The option values given for the requested option.
 *
 *      min [in]
 *          The minimum allowed value for the option.  It defaults to the
 *          minumum value for the type.
 *
 *      max [in]
 *          The maximum allowed value for the option.  It defaults to the
 *          maximum value for the type.
 *
 *  Returns:
 *      Nothing, though the requested option values are placed in the
 *      option_values parameter.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence
 *      by calling OptionGiven() or GetOptionCount().  This function will also
 *      throw an exception if there is an error converting the option string
 *      value to numeric value.  Lastly, an exception will be thrown if the
 *      value is not within the range min <= option_value <= max.
 */
template<> void Parser::GetOptionValues<double>(
                                        const std::string &option_name,
                                        std::vector<double> &option_values,
                                        double min,
                                        double max)
{
    // Define the converter function
    auto converter = [](const std::string &value) -> double
                     {
                         return std::stod(value);
                     };

    // Get each of the option values, applying the conversion function to each
    GetOptionValues(option_name, converter, option_values, min, max);
}

/*
 *  Parser::FindOptionStrings()
 *
 *  Description:
 *      This function will return a reference to the vector of option values
 *      associated with a program option for which an argument is required.
 *      In the case of arguments not associated with an option flag (e.g.,
 *      a list of filenames or similar), those arguments are stored under
 *      the option name "" (empty string).  For options that do not have
 *      arguments, one should use OptionGiven() or GetOptionCount() to
 *      determine presence of the specified option.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which values should be retrieved.
 *
 *  Returns:
 *      A reference to the vector of string arguments given by the user for
 *      the specified option name.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      was not given by the user.  One should first check for existence of
 *      an option by calling OptionGiven() or GetOptionCount().
 */
const std::vector<std::string> &Parser::FindOptionStrings(
                                            const std::string &option_name)
{
    auto it = option_map.find(option_name);

    if (it == option_map.end())
    {
        throw OptionsException(std::string("The option (\"") +
                                   option_name +
                                   std::string("\") was not given"),
                               OptionsError::OptionNotGiven);
    }

    return (*it).second;
}

/*
 *  Parser::CovertOptionValues()
 *
 *  Description:
 *      This function will convert the given option string to an option
 *      value of type T, converted via the given converter function.
 *
 *  Parameters:
 *      option_name [in]
 *          The option name for which values should be retrieved.
 *
 *      converter [in]
 *          The function that will convert the string "option value" to
 *          the specified type T.
 *
 *      option_values [out]
 *          The value of the option once the converter function is applied.
 *
 *      min [in]
 *          The minimum value for the option.
 *
 *      max [in]
 *          The maximum value for the option.
 *
 *  Returns:
 *      Nothing, but the converted value will be stored in the option_value
 *      argument.
 *
 *  Comments:
 *      This function will throw an exception if the requested option
 *      cannot be converted properly.
 */
template<NumericType T, typename Func>
void Parser::GetOptionValues(const std::string &option_name,
                             const Func &converter,
                             std::vector<T> &option_values,
                             T min,
                             T max)
{
    const std::string unknown = "<unknown>";
    const std::string *context = nullptr;
    T option_value{};

    // Get the original option string values
    const std::vector<std::string> &options_strings =
                                                FindOptionStrings(option_name);

    // Ensure the output vector is empty
    option_values.clear();

    try
    {
        // Now convert each string to a numeric value using the specified
        // converter function and place it in the output vector
        for (const auto &option_string : options_strings)
        {
            // Get a pointer to the option string for context
            context = &option_string;

            // Convert the option value
            option_value = converter(option_string);

            // Check the value to ensure is within range
            if ((option_value < min) || (option_value > max))
            {
                // Clear the option value for security reasons
                option_value = 0;

                throw std::out_of_range("value out of range");
            }

            // Store the converted value
            option_values.emplace_back(option_value);

            // Clear the option value for security reasons
            option_value = 0;
        }
    }
    catch (const std::invalid_argument &)
    {
        std::ostringstream oss;
        oss << "Invalid argument value for \""
            << option_name
            << "\": "
            << ((context == nullptr) ? unknown : *context);
        throw OptionsException(oss.str(), OptionsError::OptionValueError);
    }
    catch (const std::out_of_range &)
    {
        std::ostringstream oss;
        oss << "Argument value for \""
            << option_name
            << "\" is out-of-range: "
            << ((context == nullptr) ? unknown : *context)
            << " [valid range is "
            << min
            << " .. "
            << max
            << "]";
        throw OptionsException(oss.str(), OptionsError::OptionValueError);
    }
    catch (...)
    {
        std::ostringstream oss;
        oss << "Unknown error converting argument \""
            << option_name
            << "\": "
            << ((context == nullptr) ? unknown : *context);
        throw OptionsException(oss.str(), OptionsError::OptionValueError);
    }
}

/*
 *  Parser::CheckOptionFlags()
 *
 *  Description:
 *      This function will look through the long and short flag vectors to
 *      ensure there are no conflicts (e.g., both vectors having the same
 *      string values).
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing, but will throw an exception if a flag conflict is found.
 *
 *  Comments:
 *      None.
 */
void Parser::CheckOptionFlags()
{
    // Ensure the same flag was not specified for both long and short flags
    for (const auto &long_flag : long_flags)
    {
        for (const auto &short_flag : short_flags)
        {
            if (long_flag == short_flag)
            {
                throw SpecificationException("Conflicting option flag symbols",
                                             OptionsError::FlagConflict);
            }
        }
    }
}

/*
 *  Parser::CheckOptions()
 *
 *  Description:
 *      This function will look through the Options given via the constructor
 *      or SetOptions to check for errors.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void Parser::CheckOptions()
{
    std::unordered_map<std::string, bool> identifiers;
    std::unordered_map<std::string, bool> short_options;
    std::unordered_map<std::string, bool> long_options;

    // Note option identifiers found
    for (const auto &option : options)
    {
        if (option.name.empty())
        {
            throw SpecificationException("Empty option identifier found",
                                         OptionsError::EmptyIdentifierName);
        }

        // Ensure we have not seen this name before
        if (identifiers.find(option.name) != identifiers.end())
        {
            std::string error = "Duplicate option identifier found: ";
            throw SpecificationException(error + option.name,
                                         OptionsError::DuplicateIdentifier);
        }
        identifiers[option.name] = true;

        // Ensure we have not seen this short option before (if specified)
        if (!option.short_option.empty())
        {
            if (short_options.find(option.short_option) != short_options.end())
            {
                std::string error = "Duplicate short option observed: ";
                throw SpecificationException(
                                        error + option.short_option,
                                        OptionsError::DuplicateShortOption);
            }
            if (option.short_option.length() > 1)
            {
                std::string error = "A short option contains more than one "
                                    "character: ";
                throw SpecificationException(error + option.short_option,
                                             OptionsError::InvalidShortOption);
            }
            short_options[option.short_option] = true;
        }

        // Ensure we have not seen this long option before (if specified)
        if (!option.long_option.empty())
        {
            if (long_options.find(option.long_option) != long_options.end())
            {
                std::string error = "Duplicate long option observed: ";
                throw SpecificationException(error + option.long_option,
                                             OptionsError::DuplicateLongOption);
            }
            long_options[option.long_option] = true;
        }
    }
}

/*
 *  Parser::ProcessArgument()
 *
 *  Description:
 *      This function will consider the provided argument and determine if it
 *      is a long option argument, a short option argument, or just a string on
 *      the command-line.  If it is either type of option and the option
 *      consumes the provided additional parameter, that will be noted in the
 *      return value.  If it is neither a long option or a short option, but
 *      does not match any value in the option_spec, an exception will be
 *      thrown to indicate that there was a processing error.  However, if the
 *      string is merely a string (not something that looks like an option
 *      flag), then it will just append the string to the vector of strings to
 *      be considered by the application.  This means that "-" or "--" alone
 *      would just be considered strings.  That is useful to allow such strings
 *      to indicate input from stdin, for example.
 *
 *  Parameters:
 *      argument [in]
 *          The argument to process.
 *
 *      parameter [in]
 *          A possible parameter for an argument that expect a parameter to
 *          follow.
 *
 *  Returns:
 *      True if the parameter was consumed when processing the argument, false
 *      otherwise.
 *
 *  Comments:
 *      None.
 */
bool Parser::ProcessArgument(const std::string_view argument,
                             const std::optional<std::string_view> &parameter)
{
    // If the argument is zero-length, no point checking for options
    if (!argument.empty())
    {
        // Try to process the argument by considering long option formats first
        auto [long_option_matched, long_parameter_consumed] =
                                        ProcessLongOption(argument, parameter);

        if (long_option_matched) return long_parameter_consumed;

        // Try to process the argument by considering short option formats
        auto [short_option_matched, short_parameter_consumed] =
                                        ProcessShortOption(argument, parameter);

        if (short_option_matched) return short_parameter_consumed;
    }

    // Since neither the long or short option was matched, the argument will
    // be added to the list of strings
    option_map[""].emplace_back(argument);

    return false;
}

/*
 *  Parser::ProcessLongOption()
 *
 *  Description:
 *      This function consider the provided argument and determine if it is a
 *      long option argument.  If it is and the option consumes the provided
 *      additional parameter, that will be noted in the return value.  If it
 *      appears to be a long option based on flags, but it does not match any
 *      option, an exception will be thrown.  If it appears to match a long
 *      option flag and only the flags (e.g., "--"), it will be treated as a
 *      string and put into the option_map in the vector of strings under the
 *      key "".
 *
 *  Parameters:
 *      argument [in]
 *          The argument to process.
 *
 *      parameter [in]
 *          A possible parameter for an argument that expect a parameter to
 *          follow.
 *
 *  Returns:
 *      A pair of bool values, the first being true if the argument was
 *      processed as a long option argument and false if it was not processed
 *      as a long option argument.  If processed, the second bool indicates
 *      whether the optional parameter was also consumed as a parameter to the
 *      option or if it should be separately considered.
 *
 *  Comments:
 *      None.
 */
std::pair<bool, bool> Parser::ProcessLongOption(
                            const std::string_view argument,
                            const std::optional<std::string_view> &parameter)
{
    // Iterator pointing to start of argument string
    std::string_view::const_iterator argument_start_iterator{};

    // Iterator pointing to end of argument string
    std::string_view::const_iterator argument_end_iterator{};

    // Iterator pointing to start of a value string
    std::string_view::const_iterator value_start_iterator{};

    // Was an option matched?
    bool matched_option = false;

    // Indicate whether parameter was consumed
    bool parameter_consumed = false;

    // If the argument is an empty string, there is no long option
    if (argument.empty()) return {false, false};

    // Point to the start of the argument
    argument_start_iterator = argument.cbegin();

    // Find the start of the option (beyond the options strings (e.g., "--")),
    // returning if no flags found
    if (!FindOptionStart(long_flags, argument_start_iterator, argument.cend()))
    {
        return {false, false};
    }

    // If there are no other characters in the argument, then we matched only
    // flag characters (e.g., "--"); accept that as string by storing it in the
    // option map and returning
    if (argument_start_iterator == argument.cend())
    {
        option_map[""].emplace_back(argument);
        return {true, false};
    }

    // Iterate over the options specification trying to match long option values
    for (const auto &option : options)
    {
        // Count of option characters matched
        unsigned option_characters_matched = 0;

        // If no long option name was given, look at the next entry
        if (option.long_option.empty()) continue;

        // Try to match the option name
        argument_end_iterator = argument_start_iterator;
        for (const auto c : option.long_option)
        {
            if ((c == *argument_end_iterator) ||
                (case_insensitive &&
                 ((std::toupper(c) == std::toupper(*argument_end_iterator)))))
            {
                option_characters_matched++;
                if (++argument_end_iterator == argument.cend()) break;
            }
            else
            {
                break;
            }
        }

        // If we did not match all of the characters in the option name,
        // continue
        if (option_characters_matched < option.long_option.length()) continue;

        // Did we precisely match the name?
        if (argument_end_iterator == argument.cend())
        {
            // Store the command line option, noting if parameter is consumed
            parameter_consumed = StoreOption(option, parameter);

            // Note the option was matched
            matched_option = true;

            break;
        }

        // See if we can find a value parameter (e.g., "foo=bar")
        value_start_iterator = argument_end_iterator;
        if (FindStringStart(option_value_separator,
                            value_start_iterator,
                            argument.cend()))
        {
            // This option appears to have an argument, produce an error
            // if it is not supposed to have one
            if (!option.parameter_expected)
            {
                std::ostringstream oss;
                oss << "Option \""
                    << option.name
                    << "\" should not have a parameter: "
                    << std::string(value_start_iterator, argument.cend());
                throw OptionsException(oss.str(),
                                       OptionsError::MissingOptionArgument);
            }

            // If the command-line was "--foo=" (nothing following in the
            // argument string), treat it as an error
            if (value_start_iterator == argument.cend())
            {
                std::ostringstream oss;
                oss << "Option \""
                    << option.name
                    << "\" appears to have been given an empty parameter: "
                    << argument;
                throw OptionsException(oss.str(),
                                       OptionsError::MissingOptionArgument);
            }

            // This is a command-line option like "--foo=bar", so the
            // parameter is the rest of the string
            StoreOption(option,
                        std::string(value_start_iterator, argument.cend()));

            // Note the option was matched
            matched_option = true;

            break;
        }

        // If we get to this point, we must have matched something like
        // "--foo", but there are more characters in the argument and they are
        // not an option value (e.g., not "--foo=bar").  So this must be an
        // incorrect match.  Perhaps the user provided "--foobar" as an option
        // argument.  Continue looking for a better match.
    }

    // If we could not match an option, raise an exception
    if (!matched_option)
    {
        std::string error = "Invalid option specified: ";
        throw OptionsException(error + std::string(argument),
                               OptionsError::InvalidLongOption);
    }

    return {true, parameter_consumed};
}

/*
 *  Parser::ProcessShortOption()
 *
 *  Description:
 *      This function consider the provided argument and determine if it is a
 *      short option argument.  If it is and the option consumes the provided
 *      additional parameter, that will be noted in the return value.  If it
 *      appears to be a short option based on flags, but it does not match any
 *      option, an exception will be thrown.  If it appears to match a short
 *      option flag and only the flags (e.g., "-"), it will be treated as a
 *      string and put into the option_map in the vector of strings under the
 *      key "".
 *
 *  Parameters:
 *      argument [in]
 *          The argument to process.
 *
 *      parameter [in]
 *          A possible parameter for an argument that expect a parameter to
 *          follow.
 *
 *  Returns:
 *      A pair of bool values, the first being true if the argument was
 *      processed as a short option argument and false if it was not processed
 *      as a short option argument.  If processed, the second bool indicates
 *      whether the optional parameter was also consumed as a parameter to the
 *      option or if it should be separately considered.
 *
 *  Comments:
 *      None.
 */
std::pair<bool, bool> Parser::ProcessShortOption(
                            const std::string_view argument,
                            const std::optional<std::string_view> &parameter)
{
    // Iterator over the argument string
    std::string_view::const_iterator argument_iterator{};

    // Indicate whether parameter was consumed
    bool parameter_consumed = false;

    // If the argument is an empty string, there is no short option
    if (argument.empty()) return {false, false};

    // Point to the start of the argument
    argument_iterator = argument.cbegin();

    // Find the start of the option (beyond the options strings (e.g., "-")),
    // returning if no flags found
    if (!FindOptionStart(short_flags, argument_iterator, argument.cend()))
    {
        return {false, false};
    }

    // If there are no other characters in the argument, then we matched only
    // flag characters (e.g., "-"); accept that as string by storing it in the
    // option map and returning
    if (argument_iterator == argument.cend())
    {
        option_map[""].emplace_back(argument);
        return {true, false};
    }

    // Iterate over the characters in the option string
    while (argument_iterator != argument.cend())
    {
        // Was an option matched?
        bool matched_option = false;

        // Get character pointed to by the iterator
        std::string user_option = std::string() + *argument_iterator;
        argument_iterator++;

        // Iterate over the options to find a match
        for (const auto &option : options)
        {
            // If no short option name was given, look at the next entry
            if (option.short_option.empty()) continue;

            // If we matched the name, process it
            if ((option.short_option == user_option) ||
                (case_insensitive &&
                 ((Uppercase(option.short_option) == Uppercase(user_option)))))
            {
                // Store the value found
                if (argument_iterator == argument.cend())
                {
                    // Store the command-line option, noting if parameter is
                    // consumed
                    parameter_consumed = StoreOption(option, parameter);
                }
                else
                {
                    // Store the command-line option, passing an empty optional
                    // parameter
                    StoreOption(option, {});
                }

                // We matched an option
                matched_option = true;

                break;
            }
        }

        // If we could not match an option, raise an exception
        if (!matched_option)
        {
            std::string error = "Invalid option specified: ";
            throw OptionsException(error + std::string(argument),
                                   OptionsError::InvalidShortOption);
        }
    }

    return {true, parameter_consumed};
}

/*
 *  Parser::StoreOption()
 *
 *  Description:
 *      This function will store the given option in the option map.  The
 *      optional parameter is also stored if this option expects a parameter to
 *      be provided.
 *
 *  Parameters:
 *      option [in]
 *          The option to be stored in the option map.
 *
 *      parameter [in]
 *          A possible parameter for an argument that expect a parameter to
 *          follow.  This is an optional type since there may not actually be a
 *          parameter to provide and the caller just provides something if it
 *          has it.  This function determines whether the parameter is required
 *          or not and will throw an exception if a required parameter is
 *          missing.
 *
 *  Returns:
 *      Returns true if the parameter value was consumed (stored) or false if
 *      it was not.
 *
 *  Comments:
 *      None.
 */
bool Parser::StoreOption(const Option &option,
                         const std::optional<std::string_view> &parameter)
{
    // Indicates if the parameter was consumed
    bool parameter_consumed = false;

    // Throw an exception if this option is already in the option_map, but
    // multiple instances are not allowed
    if ((option_map.find(option.name) != option_map.end()) &&
        !option.multiple_allowed)
    {
        std::ostringstream oss;
        oss << "Option \""
            << option.name
            << "\" given multiple times, but only allowed once";
        throw OptionsException(oss.str(), OptionsError::MultipleInstances);
    }

    // Does the option have an expected parameter?
    if (!option.parameter_expected)
    {
        option_map[option.name].emplace_back("");
    }
    else
    {
        // Does the parameter have a value?
        if (!parameter)
        {
            std::ostringstream oss;
            oss << "Option \""
                << option.name
                << "\" is missing a required argument";
            throw OptionsException(oss.str(),
                                   OptionsError::MissingOptionArgument);
        }

        // Store the parameter with this option
        option_map[option.name].emplace_back(*parameter);

        parameter_consumed = true;
    }

    return parameter_consumed;
}

/*
 *  Parser::FindOptionStart()
 *
 *  Description:
 *      This function will accept a vector of strings containing option flags
 *      (e.g., "--") and try to find the start of the argument beyond those
 *      flags.  It will accept a start an end iterator for a string that is
 *      used to compare with
 *      the vector of flag strings.
 *
 *  Parameters:
 *      flags [in]
 *          Vector of option flags to evaluate.
 *
 *      argument_start_iterator [in/out]
 *          A string iterator pointing to the start of a command-line argument.
 *
 *      argument_end [in]
 *          A iterator pointing to the end of the string.
 *
 *  Returns:
 *      True if flags were found and false if no flags were found.  If the
 *      flags are true, the argument_start_iterator will be updated to point to
 *      the first character after the option flags (i.e., the actual argument
 *      name).
 *
 *  Comments:
 *      None.
 */
bool Parser::FindOptionStart(
                const std::vector<std::string> &flags,
                std::string_view::const_iterator &argument_start_iterator,
                const std::string_view::const_iterator &argument_end_iterator)
{
    // Saved copy of the start iterator
    std::string_view::const_iterator argument_start_iterator_saved{};

    // If the start and end iterators equate, there is nothing to find
    if (argument_start_iterator == argument_end_iterator) return false;

    // Let's point to the start of the argument
    argument_start_iterator_saved = argument_start_iterator;

    // Consider each possible flag string (e.g., "--")
    for (const auto &flag : flags)
    {
        // Let's point to the start of the argument
        argument_start_iterator = argument_start_iterator_saved;

        // See if we can find the flags at the start of the string
        if (FindStringStart(flag,
                            argument_start_iterator,
                            argument_end_iterator))
        {
            return true;
        }
    }

    return false;
}

/*
 *  Parser::FindStringStart()
 *
 *  Description:
 *      This function will find the start of a string following a specified
 *      string prefix.  The string prefix might be option flags (e.g., "--") or
 *      an assignment character (e.g., "=").
 *
 *  Parameters:
 *      prefix [in]
 *          A string prefix to skip over.
 *
 *      start_iterator [in/out]
 *          A string iterator pointing to the starting point to evaluate.
 *
 *      end_iterator [in]
 *          An iterator pointing to the end of the string.
 *
 *  Returns:
 *      Returns true if the prefix was located, in which case the
 *      start_iterator will be set to the first character after the prefix
 *      characters.  If the prefix could not be found, false is returned.  Note
 *      that if no characters follow the prefix, the start_iterator will be
 *      equal to end_iterator on return of true.
 *
 *  Comments:
 *      None.
 */
bool Parser::FindStringStart(
                        const std::string &prefix,
                        std::string_view::const_iterator &start_iterator,
                        const std::string_view::const_iterator &end_iterator)
{
    // Count of prefix characters matched
    unsigned prefix_characters_matched = 0;

    // If the start and end iterators equate, there is nothing to find
    if (start_iterator == end_iterator) return false;

    // Iterate over each character in the prefix
    for (const auto c : prefix)
    {
        if (c == *start_iterator)
        {
            prefix_characters_matched++;
            if (++start_iterator == end_iterator) break;
        }
        else
        {
            break;
        }
    }

    // Did we match the entire prefix?
    return (prefix.length() == prefix_characters_matched);
}

/*
 *  Parser::Uppercase()
 *
 *  Description:
 *      This function will return the uppercase version of a given string.
 *
 *  Parameters:
 *      some_sting [in]
 *          Some string to convert to uppercase.
 *
 *  Returns:
 *      Returns the uppercase transformation of a given string.
 *
 *  Comments:
 *      None.
 */
std::string Parser::Uppercase(std::string some_string)
{
    std::transform(some_string.begin(),
                   some_string.end(),
                   some_string.begin(),
                   [](char c) -> char
                   {
                       return static_cast<char>(std::toupper(c));
                   });

    return some_string;
}

} // namespace Terra::ProgramOptions
