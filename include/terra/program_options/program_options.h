/*
 *  program_options.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines the program options types and Parser object, which is
 *      used to process options given to a program to influence execution
 *      behavior.
 *
 *      The library uses std::string and requires characters to be in UTF-8.
 *      If using the Library on a system like Windows where command-line
 *      arguments are Unicode (UTF-16LE, in the case of Windows), command-line
 *      arguments need to be converted to UTF-8 before calling into this
 *      library.
 *
 *      If one need to access this object from multiple threads, it should
 *      be protected to ensure serial access.
 *
 *      Program options may be specified using either one or two option flags
 *      followed by an option name.  An option that does not have an associated
 *      value will have an empty string stored internally to represent presence
 *      of the option.
 *
 *      All of the raw strings provided to the program that are not prefaced by
 *      an option flag (e.g., "-" or "/") or appears to be a raw option flag
 *      with no option name will be stored under the option name "" (empty
 *      string).  Generally, such strings are a list of filenames or similar.
 *
 *      Option flags for short options and/or long options are provided as
 *      parameters during object construction.
 *
 *      The following is an example for specifying what the program options
 *      might be for a program that lists files on a disk.  While both the long
 *      and short forms are present for each program option in this example,
 *      either may be absent (i.e., an empty string) in practice.  However, at
 *      least one form must be present.
 *
 *      The Options includes the following:
 *          * Name of the option (used in calls to get values or counts)
 *          * Short option character (actually provided as a string)
 *          * Long option string
 *          * Indication that the argument may be given multiple times
 *          * Indication of whether an argument is expected
 *
 *      Consider the following example options:
 *
 *      Terra::ProgramOptions::Options options =
 *      {
 *      //    Name       Short    Long        Multi  Argument
 *          { "all",       "a",   "all",      false, false },
 *          { "pattern",   "p",   "pattern",  true,  true  },
 *          { "color",     "c",   "color",    false, false },
 *          { "size",      "s",   "min-size", false, true  },
 *          { "verbose",   "v",   "verbose",  true,  false }
 *      };
 *
 *      The above would allow a program to be executed with options like this:
 *
 *          filelist --all -p A* -p B* --color red /some/directory/to/list
 *
 *      Since short options may have other options following, the following
 *      is valid given the above program options:
 *
 *          filelist -ap A* -p B* --color red /some/directory/to/list
 *
 *      It is possible to have option flag characters as part of long option
 *      string name.  For example, if "color" above were "color-name", the user
 *      would be able to specify this:
 *
 *          filelist -ap A* -p B* --color-name red /some/directory/to/list
 *
 *      If the program is given input like this:
 *
 *          filelist -a -a -a -a /some/directory/to/list
 *
 *      It would be treated as an error since the "-a" option cannot be provided
 *      more than once, per the options specified above.  Some programs do use
 *      the presence of multiple instances of options that do not have option
 *      values.  For example, some programs that produce output might use
 *      "-v" multiple times to indicate a higher level of verbosity.  That can
 *      be accomplished by setting the "multiple" parameter to true.
 *
 *      Some programs use a single "-" or "--" to indicate standard input or
 *      some other processing logic.  While these might match initial option
 *      flag characters, since no alphanumeric characters follow they are just
 *      inserted as-is into the list of strings returned when getting the
 *      option values for the option "" (empty string).  If either of these
 *      appear where an option value is expected, it will be treated as an
 *      option value.
 *
 *      Once an option is found for which a value is expected to follow, the
 *      next string provided to the program is treated as the parameter,
 *      regardless of whether it looks like an option or not.  For example:
 *
 *          multiply -a -3.14 -b -1
 *
 *      Here, this fictional program would multiply -3.14 times -1.
 *
 *      If an option flag is matched, but the parser cannot match an option
 *      character or string required by the option, the input will be considered
 *      invalid and the ParseArguments() call will throw a
 *      OptionsException exception.
 *
 *      If the contents of the Options passed to the constructor or to
 *      SetOptions() is invalid, an exception of type
 *      SpecificationException will be thrown.  Validation is pretty trivial,
 *      substantially just looking for conflicting / duplicate option strings.
 *
 *      The default short and long option flags are "-" and "--", respectively.
 *      While these can be set to any valid character(s), it is best to follow
 *      standard practices on the target platform and avoid confusion with
 *      option string names.
 *
 *      A common practice on Windows has been to have options and values
 *      specified like:
 *
 *          dir /A:D
 *
 *      On Linux, this is fairly common:
 *
 *          ls --time=atime
 *
 *      Option value separators like these are supported, but only for long
 *      option names.
 *
 *      Long option names may be a single character in length, which is useful
 *      in implementing the DOS-style "/A:D" syntax.
 *
 *      Short options may have multiple characters together to as to allow
 *      commands like "tar -czvf -".  This example indicates four options,
 *      which the final "f" option taking a filename argument (which follows as
 *      "-").
 *
 *      The reasons for offering both long and short option names is to allow
 *      multiple short options to be prefaced with a single option flag
 *      (e.g., "-cvf" to indicate 3 options), to allow shorter aliases for the
 *      longer names, and to implement commonly supported program execution
 *      option syntax.
 *
 *      It is possible to specify that program option names are case
 *      insensitive (e.g., "--foo" is the same as "--FOO"), but option flags
 *      and option value separator characters are always case sensitive.
 *
 *      Note that the set of strings passed to ParseArguments() is expected
 *      to include the command invoked as the first string.  This may be
 *      an empty string, but the parser expects position zero in the vector
 *      to align with argv[0].  It is skipped over when parsing.
 *
 *      One may query how many values exist for a given option by calling
 *      GetOptionCount().  This is useful for checking option presence and for
 *      things like the -v option in the above example that is used for
 *      indicating a level of verbosity.  For example:
 *
 *          verbosity_level = program_options.GetOptionCount("verbose");
 *
 *      Once options are parsed, one can get the strings or numeric values
 *      for option strings by calling GetOptionString(), GetOptionStrings(),
 *      GetOptionValue(), or GetOptionValues().  The plural form of calls
 *      are for those options that allow multiple instances.  In call cases,
 *      if a requested option was not given by the user, an
 *      OptionsError::OptionNotGiven exception will be thrown.  One should call
 *      OptionGiven() or GetOptionCount() first to ensure that the option was
 *      given by the user before attempting to retrieve the value.  However, one
 *      may put the entire option processing block in a single try/catch block
 *      to simplify processing, which is why all of these functions behave
 *      uniformly.
 *
 *  Portability Issues:
 *      Requires C++20 or later.
 */

#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <limits>
#include <type_traits>
#include <cstdint>

namespace Terra::ProgramOptions
{

// Define the various error types for the following exceptions
enum class OptionsError
{
    // Errors related to OptionsSpec
    FlagConflict,
    EmptyIdentifierName,
    DuplicateIdentifier,
    DuplicateShortOption,
    DuplicateLongOption,

    // Errors related to both options spec and parsing
    InvalidShortOption,
    InvalidLongOption,

    // Errors relating to parsing options
    MultipleInstances,
    MissingOptionArgument,
    OptionNotGiven,
    OptionValueError
};

// Define an exception class for program options
class OptionsException : public std::runtime_error
{
    public:
        explicit OptionsException(const std::string &what_arg,
                                  OptionsError options_error) :
            std::runtime_error(what_arg),
            options_error(options_error)
        {
        }

        explicit OptionsException(const char *what_arg,
                                  OptionsError options_error) :
            std::runtime_error(what_arg),
            options_error(options_error)
        {
        }

        const OptionsError options_error;
};

// Define an exception that just for issues related to the options specification
class SpecificationException : public OptionsException
{
    using OptionsException::OptionsException;
};

// Define a structure containing a single program option
struct Option
{
    std::string name;                           // Option name
    std::string short_option;                   // Short option name
    std::string long_option;                    // Long option name
    bool multiple_allowed;                      // Multiple options allowed?
    bool parameter_expected;                    // Parameter expected?
};

// Define a type used to specify the set of valid options
using Options = std::vector<Option>;

// Define the class to parse program options
class Parser
{
    public:
        Parser();
        Parser(const Options &options,
               const std::vector<std::string> &short_flags = {"-"},
               const std::vector<std::string> &long_flags = {"--"},
               const std::string &option_value_separator = "=",
               const bool case_insensitive = false);
        Parser(const Parser &parser);
        Parser(Parser &&parser) noexcept;
        virtual ~Parser() = default;

        Parser &operator=(const Parser &program_options);

        void SetOptions(const Options &options,
                        const std::vector<std::string> &short_flags = {"-"},
                        const std::vector<std::string> &long_flags = {"--"},
                        const std::string &option_value_separator = "=",
                        const bool case_insensitive = false);

        virtual void ClearOptions();

        void ParseArguments(const int argc, const char *const argv[]);
        void ParseArguments(const std::vector<std::string> &arguments);
        void ParseArguments(const std::vector<std::string_view> &arguments);

        bool OptionGiven(const std::string &option_name);
        std::size_t GetOptionCount(const std::string &option_name);

        std::string GetOptionString(const std::string &option_name);
        std::vector<std::string> GetOptionStrings(
                                            const std::string &option_name);

        template<typename T,
            std::enable_if_t<std::is_integral<T>::value ||
                             std::is_floating_point<T>::value, bool> = true>
        void GetOptionValue(const std::string &option_name,
                            T &option_value,
                            T min = std::numeric_limits<T>::min(),
                            T max = std::numeric_limits<T>::max())
        {
            std::vector<T> option_values;
            GetOptionValues(option_name, option_values, min, max);
            option_value = option_values.front();
        }
        template<typename T,
            std::enable_if_t<std::is_integral<T>::value ||
                             std::is_floating_point<T>::value, bool> = true>
        void GetOptionValues(const std::string &option_name,
                             std::vector<T> &option_values,
                             T min = std::numeric_limits<T>::min(),
                             T max = std::numeric_limits<T>::max());

    protected:
        const std::vector<std::string> &FindOptionStrings(
                                            const std::string &option_name);
        template<typename T, typename Func,
            std::enable_if_t<std::is_integral<T>::value ||
                             std::is_floating_point<T>::value, bool> = true>
        void GetOptionValues(const std::string &option_name,
                             const Func &converter,
                             std::vector<T> &option_values,
                             T min,
                             T max);
        void CheckOptionFlags();
        void CheckOptions();
        bool ProcessArgument(const std::string_view argument,
                             const std::optional<std::string_view> &parameter);
        std::pair<bool, bool> ProcessLongOption(
                            const std::string_view argument,
                            const std::optional<std::string_view> &parameter);
        std::pair<bool, bool> ProcessShortOption(
                            const std::string_view argument,
                            const std::optional<std::string_view> &parameter);
        bool StoreOption(const Option &option,
                         const std::optional<std::string_view> &parameter);
        static bool FindOptionStart(
                const std::vector<std::string> &flags,
                std::string_view::const_iterator &argument_start_iterator,
                const std::string_view::const_iterator &argument_end_iterator);
        static bool FindStringStart(
                        const std::string &prefix,
                        std::string_view::const_iterator &start_iterator,
                        const std::string_view::const_iterator &end_iterator);
        static std::string Uppercase(std::string some_string);

        // Program options
        Options options;

        // Strings for short options
        std::vector<std::string> short_flags;

        // Strings for long flags
        std::vector<std::string> long_flags;

        // Flag/value separator
        std::string option_value_separator;

        // Options case insensitive?
        bool case_insensitive;

        // A map to hold the parsed program options
        // NOTE: The key "" (i.e., empty string) is used to hold all strings
        //       provided on the command-line  that are not associated with a
        //       named option (e.g., list of files or other non-option
        //       arguments on the command-line)
        std::unordered_map<std::string, std::vector<std::string>> option_map;
};

} // namespace Terra::ProgramOptions
