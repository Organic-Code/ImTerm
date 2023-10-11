## Table of Contents

- [About ImTerm](#about-imterm)
    * [Features](#features)
    * [How to include in your project](#how-to-include-in-your-project)
- [How does it work](#how-does-it-work)
    * [Basic usage](#basic-usage)
        - [TerminalHeplers](#terminalhelpers)
        - [command_type](#command_type)
    * [Tuning](#tuning)
        - [colors and text](#colors-and-text)
        - [non-ascii characters](#non-ascii-characters)
        - [spdlog integration](#spdlog-integration)
        - [extra](#extra)
- [Author](#author)
- [License](#license)

# About ImTerm

ImTerm is a header only "terminal" library for ImGui, intended to help you build a debug console for your application.
ImTerm does not implement commands on its own, but tries to make it easier to have a nice feeling terminal where you can type in
your custom commands, and aims to be utf8 friendly. It also comes with an optional spdlog integration if you happen to use it.

![](demo.gif)

## Features

ImTerm features revolves mainly around completion:

- basic tab completion
- contextual tab completion
- OSD for completion
- history completion support, expanding on a ``tab`` press
    - !! refers to previous command
    - !-n refers to nth command, starting from the last one
    - !:m refers to mth argument of last command, (with m=0, you are referring to the last command name)
    - !-n:m refers to the mth argument of the nth command, starting from the last command
- prefixed history search (type your prefix, hit the arrow keys, and you're done!).

If you want to type in ``!:`` or ``!!`` if your command argument, you'll have to escape one of the exclamation marks with ``\``

Typing ``command_name Hello, World!`` will call ``command_name`` with two arguments, ``Hello,`` and ``World!``.
To merge the two arguments, simply add surrounding quotes: ``command_name "Hello, World!"``.

## How to include in your project

ImTerm only has [ImGui](https://github.com/ocornut/imgui) as mandatory dependency, and has spdlog as
an optional depency. If you use [spdlog](https://github.com/gabime/spdlog), then you'll get an easier integration,
otherwise that's not a problem, and you won't need to add spdlog to your project.

Since ImTerm is a header only library, appart from its ImGui dependency (that you probably already use if you came there)
you'll just need to get ImTerm headers, ``#include <imterm/terminal.hpp>``, and you should be done!

ImTerm requires C++17 or above.

# How does it work

## Basic usage

## command_type and argument_type

Adding commands to an ImTerm::terminal is done via the types ``command_type_cref``, ``command_type`` and ``argument_type``

The ``command_type`` type (which I should rather call  ``ImTerm::command_t<ImTerm::terminal<TerminalHelper>>``) is a structured used to represent a command, and contains the following attributes:

- the command name name (as an ``std::string_view``)
- a short description of the command(as an ``std::string_view``) 
- the command callback (as a function pointer)
- the completion callback (as a function pointer)

``command_type_cref`` is the type ``std::reference_wrapper<const command_type>`` (ie: a const reference that you can store in a vector).


The command callback function can do whatever it wants and takes as a parameter an ``argument_t<ImTerm::terminal<TerminalHelper>>``, containing:

- a reference to your custom argument (of type ``TerminalHelper::value_type``, can be void)
- a reference to the terminal instance that called the method
- the list of arguments (including the command name), as an ``std::vector<std::string>``

The completion callback function takes the same type of argument and should return an ``std::vector<std::string>`` containing a list of
possible contextual completion (you may return an empty vector if you don't want to autocomplete user's inputs).


## TerminalHelpers

To use ``ImTerm::terminal``'s class, you must supply it a TerminalHelper.
The TerminalHelper class is responsible for command management. Because It is expected that your commands will need some kind of argument
to interact with the rest of your application, your TerminalHelper must define the type ``TerminalHelper::value_type``.
That value will be passed to your commands as part of their arguments.

It must implement the following methods:

- ``std::vector<command_type_cref> find_command_by_prefix(std::string_view prefix)``, which shall return the list of commands starting by ``prefix`` (you might want to use ``ImTerm::misc::prefix_search``)
- ``std::vector<command_type_cref> find_command_by_prefix(const char* beg, const char* end)``, which shall return the list of commands starting by the string formed by ``[beg, end)`` (that might just disappear from the needed methods soon)
- ``std::vector<command_type_cref> list_commands()``, which shall return the list of all commands
- ``std::optional<ImTerm::message> format(std::string msg, message::type msg_type)``, which shall format a string to your liking, or return an empty optional so that it won't be logged (ImTerm::terminal will invoke this methods when it wants to emit feedback to the user).

Of course, it's a bit of a bummer to have to implement all those methods, so if you want you can also simply inherit from ``ImTerm::basic_terminal_helper``
(defined in ``imterm/terminal_helpers.hpp``), which does all that for you. Afterward, you just have to add your commands using ``basic_terminal_helper::add_command_(const command_type&)``

Here is a basic example of what a TerminalHelper can look like:
```cpp
	class terminal_helper_example : ImTerm::basic_terminal_helper<terminal_helper_example, void> {
	public:
		static std::vector<std::string> no_completion(std::string_view) { return {}; }
		
		// clears the logging screen. argument_type is aliased in ImTerm::basic_terminal_helper
		static void clear(argument_type& arg) {
			arg.term.clear();
		}

        // prints the text passed as parameter to the logging screen. argument_type is aliased in ImTerm::basic_terminal_helper
		static void echo(argument_type& arg) {
			if (arg.command_line.size() < 2) {
				return;
			}
			std::string str = std::move(arg.command_line[1]);
			for (auto it = std::next(arg.command_line.begin(), 2) ; it != arg.command_line.end() ; ++it) {
				str += " " + std::move(*it);
			}
			message msg;
			msg.value = std::move(str);
			msg.color_beg = msg.color_end = 0; // color is disabled when color_beg == color_end
			// other parameters are ignored
			arg.term.add_message(std::move(msg));
		}
		
		terminal_helper_example() {
			add_command_({"clear", "clear the screen", clear, no_completion});
			add_command_({"echo", "echoes your text", echo, no_completion});
		}
	};
```


## Tuning

## colors and text

``ImTerm::terminal`` manages almost every aforementioned feature. Pretty much every property
you could think of is configurable, either be it via an optional or via your TerminalHelper implementation (or will be in the (hopefully near) future).

Regarding optionals, if any of them is empty, it will be ignored.
For colors, that means ImTerm will use the default color instead of a custom one. For the top bar texts (used for the terminal user options
such as the ``clear`` button), that means the option will not be available for the end user.

## non-ascii characters

You can also tune space detection and string length calculation. Why would you want to do that? Well, that's if you happen
to use non-ascii characters:

- tab completion might behave in a peculiar way due to the size not being computed correctly
- you might use a non ascii character to represent a space, leading to unexpected tokenization.

To do this, you'll need to implement two extra methods in your TerminalHelper:
- ``int is_space(std::string_view)`` shall return the number of char participating in the representation of the beginning space (in the ascii world, that means that it returns 1 if the string starts by a space, and 0 otherwise). It shall not return a value greater than the size of the string.
- ``int get_length(std::string_view)`` shall return the number of glyphs represented in the given string (in the ascii world, this methods simply returns ``.size()``)

## spdlog integration

If spdlog can be included (that is ``__has_include("spdlog/spdlog.h")`` is resolved as ``true`` by the compiler), an extra class is defined in ``terminal_helpers.hpp``:
``basic_spdlog_terminal_helper``. It does the same thing as ``basic_terminal_helper`` from which it inherits, but it also inherits from ``spdlog::sinks::sink``, which
mean you can use it as a sink for any of your spdlog logger. Messages will be logged to the terminal if you use it this way.
It also furnishes spdlog style formatting facility for messages comming from the terminal intended to be logged to the terminal.

## extra

If you want to be able to interact with the terminal directly from you TerminalHelper, you may define the extra method ``void set_terminal(terminal<TerminalHelper>& term)``.
This method will be invoked right after the instantiation of ImTerm::terminal if it exists, and the passed reference will be valid throughout the whole lifetime of
the terminal.



# Author
Lucas Lazare, a computer engineering student.

# License

This project is under the MIT license.

