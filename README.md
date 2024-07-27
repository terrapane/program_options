# Program Options

This library defines the program options types and `Parser` object, which is
used to process options given to a program to influence execution behavior.

The library uses std::string and requires characters to be in UTF-8.  If using
the Library on a system like Windows where command-line arguments are Unicode
(UTF-16LE, in the case of Windows), command-line arguments need to be converted
to UTF-8 before calling into this library.

If one need to access this object from multiple threads, it should
be protected to ensure serial access.

## Usage

Program options may be specified using either one or two option flags
followed by an option name.  An option that does not have an associated
value will have an empty string stored internally to represent presence
of the option.

All of the raw strings provided to the program that are not prefaced by
an option flag (e.g., `-` or `/`) or appears to be a raw option flag
with no option name will be stored under the option name `""` (empty
string).  Generally, such strings are a list of filenames or similar.

Option flags for short options and/or long options are provided as
parameters during object construction.

The following is an example for specifying what the program options
might be for a program that lists files on a disk.  While both the long
and short forms are present for each program option in this example,
either may be absent (i.e., an empty string) in practice.  However, at
least one form must be present.

The Options includes the following:

* Name of the option (used in calls to get values or counts, as well as
  error messages when converting strings to numeric types)
* Short option character (actually provided as a string)
* Long option string
* Indication that the argument may be given multiple times
* Indication of whether an argument is expected

Consider the following example options:

```cpp
Terra::ProgramOptions::Options options =
{
//    Name      Short    Long        Multi  Argument
    { "all",      "a",   "all",      false, false },
    { "pattern",  "p",   "pattern",  true,  true  },
    { "color",    "c",   "color",    false, false },
    { "size",     "s",   "min-size", false, true  },
    { "verbose",  "v",   "verbose",  true,  false }
};
```

The above would allow a program to be executed with options like this:

```bash
filelist --all -p A* -p B* --color red /some/directory/to/list
```

Since short options may have other options following, the following
is valid given the above program options:

```bash
filelist -ap A* -p B* --color red /some/directory/to/list
```

It is possible to have option flag characters as part of long option
string name.  For example, if `"color"` above were `"color-name"`, the user
would be able to specify this:

```bash
filelist -ap A* -p B* --color-name red /some/directory/to/list
```

If the program is given input like this:

```bash
filelist -a -a -a -a /some/directory/to/list
```

It would be treated as an error since the `-a` option cannot be provided
more than once, per the option specified above.  Some programs do
use the presence of multiple instances of options that do not have
option values.  For example, some programs that produce output might use
`-v` multiple times to indicate a higher level of verbosity.  That can
be accomplished by setting the `multiple` parameter to true.

Some programs use a single `-` or `--` to indicate standard input or
some other processing logic.  While these might match initial option
flag characters, since no alphanumeric characters follow they are just
inserted as-is into the list of strings returned when getting the
option values for the option `""` (empty string).  If either of these
appear where an option value is expected, it will be treated as an
option value.

Once an option is found for which a value is expected to follow, the
next string provided to the program is treated as the parameter,
regardless of whether it looks like an option or not.  For example:

```bash
multiply -a -3.14 -b -1
```

Here, this fictional program would multiply -3.14 times -1.

If an option flag is matched, but the parser cannot match an option
character or string required by the option, the input will be considered
invalid and the `ParseArguments()` call will throw a
`OptionsException` exception.

If the contents of the `Options` passed to the constructor or to
`SetOptions()` is invalid, an exception of type
`SpecificationException` will be thrown.  Validation is pretty trivial,
substantially just looking for conflicting / duplicate option strings.

The default short and long option flags are `"-"` and `"--"`, respectively.
While these can be set to any valid character(s), it is best to follow
standard practices on the target platform and avoid confusion with
option string names.

A common practice on Windows has been to have options and values
specified like:

```dos
dir /A:D
```

On Linux, this is fairly common:

```bash
ls --time=atime
```

Option value separators like these are supported, but only for long
option names.

Long option names may be a single character in length, which is useful
in implementing the DOS-style `/A:D` syntax.

Short options may have multiple characters together to as to allow
commands like `tar -czvf -`.  This example indicates four options,
which the final `-f` option taking a filename argument (which follows as
`-`).

The reasons for offering both long and short option names is to allow
multiple short options to be prefaced with a single option flag
(e.g., `-cvf` to indicate 3 options), to allow shorter aliases for the
longer names, and to implement commonly supported program execution
option syntax.

It is possible to specify that program option names are case
insensitive (e.g., `--foo` is the same as `--FOO`), but option flags
and option value separator characters are always case sensitive.

Note that the set of strings passed to `ParseArguments()` is expected
to include the command invoked as the first string.  This may be
an empty string, but the parser expects position zero in the vector
to align with `argv[0]`.  It is skipped over when parsing.

One may query how many values exist for a given option by calling
`GetOptionCount()`.  This is useful for checking option presence and for
things like the `-v` option in the above example that is used for
indicating a level of verbosity.  For example:

```cpp
verbosity_level = parser.GetOptionCount("verbose");
```

Once options are parsed, one can get the strings or numeric values
for option strings by calling `GetOptionString()`, `GetOptionStrings()`,
`GetOptionValue()`, or `GetOptionValues()`.  The plural form of calls
are for those options that allow multiple instances.  In call cases,
if a requested option was not given by the user, an
`OptionsError::OptionNotGiven` exception will be thrown.  One
should call `OptionGiven()` or `GetOptionCount()` first to ensure that the
option was given by the user before attempting to retrieve the value.
However, one may put the entire option processing block in a single
`try`/`catch` block to simplify processing, which is why all of these
functions behave uniformly.

## Sample program

There is a sample `tar`-like program in the sample directory.  It is not
a complete representation of the `tar` command syntax, but it does
illustrate how to utilize the library using a program familiar to many.

The following is the output from the sample program given these
command-line arguments:

```text
$ ./program_options_sample --exclude c -vvvcf foo --level=3 -C /tmp \
                           --exclude a --exclude b alpha beta

Inspecting program options...

create flag was provided
filename flag was provided with value = foo
verbose flag was provided with 3 levels of verbosity
directory flag was provided with value = /tmp
level flag was provided with value = 3
exclude flag was provided with the following values:
    c
    a
    b
filenames specified:
    alpha
    beta
```
