#ifndef IMTERM_TERMINAL_HPP
#define IMTERM_TERMINAL_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <array>
#include <imgui.h>

#include "misc.hpp"

#if __has_include("fmt/format.h")
#include "fmt/format.h"
#define IMTERM_FMT_INCLUDED
#endif

namespace ImTerm {

	// argument passed to commands
	template<typename Terminal>
	struct argument_t {
		using value_type = misc::non_void_t<typename Terminal::value_type>;

		value_type& val; // misc::details::structured_void if Terminal::value_type is void, reference to Terminal::value_type otherwise
		Terminal& term; // reference to the ImTerm::terminal that called the command

		std::vector<std::string> command_line; // list of arguments the user specified in the command line. command_line[0] is the command name
	};

	// structure used to represent a command
	template<typename Terminal>
	struct command_t {
		using command_function = void (*)(argument_t<Terminal>&);
		using further_completion_function = std::vector<std::string> (*)(argument_t<Terminal>& argument_line);

		std::string_view name{}; // name of the command
		std::string_view description{}; // short description
		command_function call{}; // function doing whatever you want

		further_completion_function complete{}; // function called when users starts typing in arguments for your command
												// return a vector of strings containing possible completions.

		friend constexpr bool operator<(const command_t& lhs, const command_t& rhs) {
			return lhs.name < rhs.name;
		}

		friend constexpr bool operator<(const command_t& lhs, const std::string_view& rhs) {
			return lhs.name < rhs;
		}

		friend constexpr bool operator<(const std::string_view& lhs, const command_t& rhs) {
			return lhs < rhs.name;
		}
	};

	struct message {
		enum class type {
			user_input,             // terminal wants to log user input
			error,                  // terminal wants to log an error in user input
			cmd_history_completion, // terminal wants to log that it replaced "!:*" family input in the appropriate string
		};
		struct severity {
			enum severity_t { // done this way to be used as array index without a cast
				trace,
				debug,
				info,
				warn,
				err,
				critical
			};
		};

		severity::severity_t severity; // severity of the message
		std::string value; // text to be displayed

		// the actual color used depends on the message's severity
		size_t color_beg; // color begins at value.data() + color_beg
		size_t color_end; // color ends at value.data() + color_end - 1
		// if color_beg == color_end, nothing is colorized.

		bool is_term_message; // if set to true, current msg is considered to be originated from the terminal,
		                      // never filtered out by severity filter, and applying for different rules regarding colors.
		                      // severity is also ignored for such messages
	};

	// Various settable colors for the terminal
	// if an optional is empty, current ImGui color will be used
	// theme examples can be found at the end of this file
	struct theme {
		struct constexpr_color {
			float r,g,b,a;

			ImVec4 imv4() const {
				return {r,g,b,a};
			}
		};

		std::string_view name; // if you want to give a name to the theme

		std::optional<constexpr_color> text;                        // global text color
		std::optional<constexpr_color> window_bg;                   // ImGuiCol_WindowBg & ImGuiCol_ChildBg
		std::optional<constexpr_color> border;                      // ImGuiCol_Border
		std::optional<constexpr_color> border_shadow;               // ImGuiCol_BorderShadow
		std::optional<constexpr_color> button;                      // ImGuiCol_Button
		std::optional<constexpr_color> button_hovered;              // ImGuiCol_ButtonHovered
		std::optional<constexpr_color> button_active;               // ImGuiCol_ButtonActive
		std::optional<constexpr_color> frame_bg;                    // ImGuiCol_FrameBg
		std::optional<constexpr_color> frame_bg_hovered;            // ImGuiCol_FrameBgHovered
		std::optional<constexpr_color> frame_bg_active;             // ImGuiCol_FrameBgActive
		std::optional<constexpr_color> text_selected_bg;            // ImGuiCol_TextSelectedBg, for text input field
		std::optional<constexpr_color> check_mark;                  // ImGuiCol_CheckMark
		std::optional<constexpr_color> title_bg;                    // ImGuiCol_TitleBg
		std::optional<constexpr_color> title_bg_active;             // ImGuiCol_TitleBgActive
		std::optional<constexpr_color> title_bg_collapsed;          // ImGuiCol_TitleBgCollapsed
		std::optional<constexpr_color> message_panel;               // logging panel
		std::optional<constexpr_color> auto_complete_selected;      // left-most text in the autocompletion OSD
		std::optional<constexpr_color> auto_complete_non_selected;  // every text but the left most in the autocompletion OSD
		std::optional<constexpr_color> auto_complete_separator;     // color for the separator in the autocompletion OSD
		std::optional<constexpr_color> cmd_backlog;                 // color for message type user_input
		std::optional<constexpr_color> cmd_history_completed;       // color for message type cmd_history_completion
		std::optional<constexpr_color> log_level_drop_down_list_bg; // ImGuiCol_PopupBg
		std::optional<constexpr_color> log_level_active;            // ImGuiCol_HeaderActive
		std::optional<constexpr_color> log_level_hovered;           // ImGuiCol_HeaderHovered
		std::optional<constexpr_color> log_level_selected;          // ImGuiCol_Header
		std::optional<constexpr_color> scrollbar_bg;                // ImGuiCol_ScrollbarBg
		std::optional<constexpr_color> scrollbar_grab;              // ImGuiCol_ScrollbarGrab
		std::optional<constexpr_color> scrollbar_grab_active;       // ImGuiCol_ScrollbarGrabActive
		std::optional<constexpr_color> scrollbar_grab_hovered;      // ImGuiCol_ScrollbarGrabHovered
		std::optional<constexpr_color> filter_hint;                 // ImGuiCol_TextDisabled
		std::optional<constexpr_color> filter_text;                 // user input in log filter
		std::optional<constexpr_color> matching_text;               // text matching the log filter

		std::array<std::optional<constexpr_color>, message::severity::critical + 1> log_level_colors{}; // colors by severity
	};

	enum class config_panels {
		autoscroll,
		autowrap,
		clearbutton,
		filter,
		long_filter, // like filter, but takes up space
		loglevel,
		blank, // invisible panel that takes up place, aligning items to the right. More than one can be used, splitting up the consumed space
				// ie: {clearbutton (C), blank, filter (F), blank, loglevel (L)} will result in the layout [C           F           L]
				// Shares space with long_filter
		none
	};

	// checking that you can use a given class as a TerminalHelper
	// giving human-friendlier error messages than just letting the compiler explode
	namespace details {
		template<typename TerminalHelper, typename CommandTypeCref>
		struct assert_wellformed {

			template<typename T>
			using find_commands_by_prefix_method_v1 =
			decltype(std::declval<T&>().find_commands_by_prefix(std::declval<std::string_view>()));

			template<typename T>
			using find_commands_by_prefix_method_v2 =
			decltype(std::declval<T&>().find_commands_by_prefix(std::declval<const char *>(),
			                                                    std::declval<const char *>()));

			template<typename T>
			using list_commands_method = decltype(std::declval<T&>().list_commands());

			template<typename T>
			using format_method = decltype(std::declval<T&>().format(std::declval<std::string>(), std::declval<message::type>()));

			static_assert(
					misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v1, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(std::string_view)'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<find_commands_by_prefix_method_v2, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> find_command_by_prefix(const char*, const char*)'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<list_commands_method, std::vector<CommandTypeCref>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::vector<command_type_cref> list_commands()'. "
					"See term::terminal_helper_example for reference");
			static_assert(
					misc::is_detected_with_return_type_v<format_method, std::optional<message>, TerminalHelper>,
					"TerminalHelper should implement the method 'std::optional<term::message> format(std::string, term::message::type)'. "
					"See term::terminal_helper_example for reference");
		};
	}

	template<typename TerminalHelper>
	class terminal {
	public:
		enum class position {
			up,
			down,
			nowhere // disabled
		};

		using buffer_type = std::array<char, 1024>;
		using small_buffer_type = std::array<char, 128>;
		using value_type = misc::non_void_t<typename TerminalHelper::value_type>;
		using command_type = command_t<terminal<TerminalHelper>>;
		using command_type_cref = std::reference_wrapper<const command_type>;
		using argument_type = argument_t<terminal>;

		using terminal_helper_is_valid = details::assert_wellformed<TerminalHelper, command_type_cref>;

		inline static const std::vector<config_panels> DEFAULT_ORDER = {config_panels::clearbutton,
				  config_panels::autoscroll, config_panels::autowrap, config_panels::long_filter, config_panels::loglevel};

		// You shall call this constructor you used a non void value_type
		template <typename T = value_type, typename = std::enable_if_t<!std::is_same_v<T, misc::details::structured_void>>>
		explicit terminal(value_type& arg_value, const char * window_name_ = "terminal", int base_width_ = 900,
		                  int base_height_ = 200, std::shared_ptr<TerminalHelper> th = std::make_shared<TerminalHelper>())
				: terminal(arg_value, window_name_, base_width_, base_height_, std::move(th), terminal_helper_is_valid{}) {}


		// You shall call this constructor you used a void value_type
		template <typename T = value_type, typename = std::enable_if_t<std::is_same_v<T, misc::details::structured_void>>>
		explicit terminal(const char * window_name_ = "terminal", int base_width_ = 900,
		                  int base_height_ = 200, std::shared_ptr<TerminalHelper> th = std::make_shared<TerminalHelper>())
		                  : terminal(misc::details::no_value, window_name_, base_width_, base_height_, std::move(th), terminal_helper_is_valid{}) {}

		// Returns the underlying terminal helper
		std::shared_ptr<TerminalHelper> get_terminal_helper() {
			return m_t_helper;
		}

		// shows the terminal. Call at each frame (in a more ImGui style, this would be something like ImGui::terminal(....);
		// returns true if the terminal thinks it should still be displayed next frame, false if it thinks it should be hidden
		// return value is true except if a command required a close, or if the "escape" key was pressed.
		bool show(const std::vector<config_panels>& panels_order = DEFAULT_ORDER) noexcept;

		// returns the command line history
		const std::vector<std::string>& get_history() const noexcept {
			return m_command_history;
		}

		// if invoked, the next call to "show" will return false
		void set_should_close() noexcept {
			m_close_request = true;
		}

		// clears all theme's optionals
		void reset_colors() noexcept;

		// returns current theme
		struct theme& theme() {
			return m_colors;
		}

		// set whether the autocompletion OSD should be above/under the text input, or if it should be disabled
		void set_autocomplete_pos(position p) {
			m_autocomplete_pos = p;
		}

		// returns current autocompletion position
		position get_autocomplete_pos() const {
			return m_autocomplete_pos;
		}

#ifdef IMTERM_FMT_INCLUDED
		// logs a colorless text to the message panel
		// added as terminal message with info severity
		template <typename... Args>
		void add_formatted(const char* fmt, Args&&... args) {
			add_text(fmt::format(fmt, std::forward<Args>(args)...));
		}

		// logs a colorless text to the message panel
		// added as terminal message with warn severity
		template <typename... Args>
		void add_formatted_err(const char* fmt, Args&&... args) {
			add_text_err(fmt::format(fmt, std::forward<Args>(args)...));
		}
#endif

		// logs a text to the message panel
		// added as terminal message with info severity
		void add_text(std::string str, unsigned int color_beg, unsigned int color_end);

		// logs a text to the message panel, color spans from color_beg to the end of the message
		// added as terminal message with info severity
		void add_text(std::string str, unsigned int color_beg) {
			add_text(str, color_beg, static_cast<unsigned>(str.size()));
		}

		// logs a colorless text to the message panel
		// added as a terminal message with info severity,
		void add_text(std::string str) {
			add_text(str, 0, 0);
		}

		// logs a text to the message panel
		// added as terminal message with warn severity
		void add_text_err(std::string str, unsigned int color_beg, unsigned int color_end);

		// logs a text to the message panel, color spans from color_beg to the end of the message
		// added as terminal message with warn severity
		void add_text_err(std::string str, unsigned int color_beg) {
			add_text_err(str, color_beg, static_cast<unsigned>(str.size()));
		}

		// logs a colorless text to the message panel
		// added as terminal message with warn severity
		void add_text_err(std::string str) {
			add_text_err(str, 0, 0);
		}

		// logs a message to the message panel
		void add_message(const message& msg) {
			add_message(message{msg});
		}
		void add_message(message&& msg);

		// clears the message panel
		void clear();

		message::severity::severity_t log_level() noexcept {
			return m_level + m_lowest_log_level_val;
		}

		void log_level(message::severity::severity_t new_level) noexcept {
			if (m_lowest_log_level_val > new_level) {
				set_min_log_level(new_level);
			}
			m_level = new_level - m_lowest_log_level_val;
		}

		// returns the text used to label the button that clears the message panel
		// set it to an empty optional if you don't want the button to be displayed
		std::optional<std::string>& clear_text() noexcept {
			return m_clear_text;
		}

		// returns the text used to label the checkbox enabling or disabling message panel auto scrolling
		// set it to an empty optional if you don't want the checkbox to be displayed
		std::optional<std::string>& autoscroll_text() noexcept {
			return m_autoscroll_text;
		}

		// returns the text used to label the checkbox enabling or disabling message panel text auto wrap
		// set it to an empty optional if you don't want the checkbox to be displayed
		std::optional<std::string>& autowrap_text() noexcept {
			return m_autowrap_text;
		}

		// returns the text used to label the drop down list used to select the minimum severity to be displayed
		// set it to an empty optional if you don't want the drop down list to be displayed
		std::optional<std::string>& log_level_text() noexcept {
			return m_log_level_text;
		}

		// returns the text used to label the text input used to filter out logs
		// set it to an empty optional if you don't want the filter to be displayed
		std::optional<std::string>& filter_hint() noexcept {
			return m_filter_hint;
		}

		// allows you to set the text in the log_level drop down list
		// the std::string_view/s are copied, so you don't need to manage their life-time
		// set log_level_text() to an empty optional if you want to disable the drop down list
		void set_level_list_text(std::string_view trace_str, std::string_view debug_str, std::string_view info_str,
					std::string_view warn_str, std::string_view err_str, std::string_view critical_str, std::string_view none_str);

		// sets the maximum verbosity a user can set in the terminal with the log_level drop down list
		// for instance, if you pass 'info', the user will be able to select 'info','warning','error', 'critical', and 'none',
		// but will never be able to view 'trace' and 'debug' messages
		void set_min_log_level(message::severity::severity_t level);

	private:
		explicit terminal(value_type& arg_value, const char * window_name_, int base_width_, int base_height_, std::shared_ptr<TerminalHelper> th, terminal_helper_is_valid&&);

		void try_log(std::string_view str, message::type type);

		void compute_text_size() noexcept;

		void display_settings_bar(const std::vector<config_panels>& panels_order) noexcept;

		void display_messages() noexcept;

		void display_command_line() noexcept;

		// displaying command_line itself
		void show_input_text() noexcept;

		void handle_unfocus() noexcept;

		void show_autocomplete() noexcept;

		void call_command() noexcept;

		std::optional<std::string> resolve_history_reference(std::string_view str, bool& modified) const noexcept;

		std::pair<bool, std::string> resolve_history_references(std::string_view str, bool& modified) const;


		static int command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept;

		int command_line_callback(ImGuiInputTextCallbackData * data) noexcept;

		static int try_push_style(ImGuiCol col, const std::optional<ImVec4>& color) {
			if (color) {
				ImGui::PushStyleColor(col, *color);
				return 1;
			}
			return 0;
		}

		static int try_push_style(ImGuiCol col, const std::optional<theme::constexpr_color>& color) {
			if (color) {
				ImGui::PushStyleColor(col, color->imv4());
				return 1;
			}
			return 0;
		}


		int is_space(std::string_view str) const;

		bool is_digit(char c) const;

		unsigned long get_length(std::string_view str) const;

		// Returns a vector containing each element that were space separated
		// Returns an empty optional if a '"' char was not matched with a closing '"',
		//                except if ignore_non_match was set to true
		std::optional<std::vector<std::string>> split_by_space(std::string_view in, bool ignore_non_match = false) const;

		////////////

		value_type& m_argument_value;
		mutable std::shared_ptr<TerminalHelper> m_t_helper;

		bool m_should_show_next_frame{true};
		bool m_close_request{false};

		const char * const m_window_name;

		const int m_base_width;
		const int m_base_height;

		struct theme m_colors{};

		// configuration
		bool m_autoscroll{true}; // TODO: accessors
		bool m_autowrap{true};  // TODO: accessors
		std::vector<std::string>::size_type m_last_size{0u};
		int m_level{message::severity::trace}; // TODO: accessors
#ifdef IMTERM_ENABLE_REGEX
		bool m_regex_search{true}; // TODO: accessors, button
#endif

		std::optional<std::string> m_autoscroll_text;
		std::optional<std::string> m_clear_text;
		std::optional<std::string> m_log_level_text;
		std::optional<std::string> m_autowrap_text;
		std::optional<std::string> m_filter_hint;
		std::string m_level_list_text{};
		const char* m_longest_log_level{nullptr}; // points to the longest log level, in m_level_list_text
		const char* m_lowest_log_level{nullptr}; // points to the lowest log level possible, in m_level_list_text
		message::severity::severity_t m_lowest_log_level_val{message::severity::trace};

		small_buffer_type m_log_text_filter_buffer{};
		small_buffer_type::size_type m_log_text_filter_buffer_usage{0u};


		// message panel variables
		unsigned long m_last_flush_at_history{0u}; // for the [-n] indicator on command line
		bool m_flush_bit{false};
		std::vector<message> m_logs{};


		// command line variables
		buffer_type m_command_buffer{};
		buffer_type::size_type m_buffer_usage{0u}; // max accessible: command_buffer[buffer_usage - 1]
		                                           // (buffer_usage might be 0 for empty string)
		buffer_type::size_type m_previous_buffer_usage{0u};
		bool m_should_take_focus{false};

		ImGuiID m_previously_active_id{0u};
		ImGuiID m_input_text_id{0u};

		// autocompletion
		std::vector<command_type_cref> m_current_autocomplete{};
		std::vector<std::string> m_current_autocomplete_strings{};
		std::string_view m_autocomlete_separator{" | "};
		position m_autocomplete_pos{position::down};
		bool m_command_entered{false};

		// command line: completion using history
		std::string m_command_line_backup{};
		std::string_view m_command_line_backup_prefix{};
		std::vector<std::string> m_command_history{};
		std::optional<std::vector<std::string>::iterator> m_current_history_selection{};

		bool m_ignore_next_textinput{false};
		bool m_has_focus{false};

	};


	namespace themes {

		constexpr theme light = {
				"Light Rainbow",
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.000f}, //text
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 1.000f}, //window_bg
				theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //border
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //button
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //button_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //button_active
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //frame_bg
				theme::constexpr_color{0.824f, 0.765f, 0.765f, 0.875f}, //frame_bg_hovered
				theme::constexpr_color{0.627f, 0.569f, 0.569f, 0.875f}, //frame_bg_active
				theme::constexpr_color{0.260f, 0.590f, 0.980f, 0.350f}, //text_selected_bg
				theme::constexpr_color{0.843f, 0.000f, 0.373f, 1.000f}, //check_mark
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_bg
				theme::constexpr_color{0.165f, 0.365f, 0.506f, 1.000f}, //title_bg_active
				theme::constexpr_color{0.243f, 0.443f, 0.624f, 0.850f}, //title_collapsed
				theme::constexpr_color{0.902f, 0.843f, 0.843f, 0.875f}, //message_panel
				theme::constexpr_color{0.196f, 1.000f, 0.196f, 1.000f}, //auto_complete_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 1.000f}, //auto_complete_non_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.392f}, //auto_complete_separator
				theme::constexpr_color{0.519f, 0.118f, 0.715f, 1.000f}, //cmd_backlog
				theme::constexpr_color{1.000f, 0.430f, 0.059f, 1.000f}, //cmd_history_completed
				theme::constexpr_color{0.901f, 0.843f, 0.843f, 0.784f}, //log_level_drop_down_list_bg
				theme::constexpr_color{0.443f, 0.705f, 1.000f, 1.000f}, //log_level_active
				theme::constexpr_color{0.443f, 0.705f, 0.784f, 0.705f}, //log_level_hovered
				theme::constexpr_color{0.443f, 0.623f, 0.949f, 1.000f}, //log_level_selected
				theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
				theme::constexpr_color{0.470f, 0.470f, 0.588f, 1.000f}, //scrollbar_grab
				theme::constexpr_color{0.392f, 0.392f, 0.509f, 1.000f}, //scrollbar_grab_active
				theme::constexpr_color{0.509f, 0.509f, 0.666f, 1.000f}, //scrollbar_grab_hovered
				theme::constexpr_color{0.470f, 0.470f, 0.470f, 1.000f}, //filter_hint
				theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.000f}, //filter_text
				theme::constexpr_color{0.549f, 0.196f, 0.039f, 1.000f}, //matching_text
				{
					theme::constexpr_color{0.078f, 0.117f, 0.764f, 1.f}, // log_level::trace
					theme::constexpr_color{0.100f, 0.100f, 0.100f, 1.f}, // log_level::debug
					theme::constexpr_color{0.301f, 0.529f, 0.000f, 1.f}, // log_level::info
					theme::constexpr_color{0.784f, 0.431f, 0.058f, 1.f}, // log_level::warning
					theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::error
					theme::constexpr_color{0.901f, 0.117f, 0.117f, 1.f}, // log_level::critical
				}
		};

		constexpr theme cherry {
			"Dark Cherry",
			theme::constexpr_color{0.649f, 0.661f, 0.669f, 1.000f}, //text
			theme::constexpr_color{0.130f, 0.140f, 0.170f, 1.000f}, //window_bg
			theme::constexpr_color{0.310f, 0.310f, 1.000f, 0.000f}, //border
			theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //border_shadow
			theme::constexpr_color{0.470f, 0.770f, 0.830f, 0.140f}, //button
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.860f}, //button_hovered
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //button_active
			theme::constexpr_color{0.200f, 0.220f, 0.270f, 1.000f}, //frame_bg
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.780f}, //frame_bg_hovered
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 1.000f}, //frame_bg_active
			theme::constexpr_color{0.455f, 0.198f, 0.301f, 0.430f}, //text_selected_bg
			theme::constexpr_color{0.710f, 0.202f, 0.207f, 1.000f}, //check_mark
			theme::constexpr_color{0.232f, 0.201f, 0.271f, 1.000f}, //title_bg
			theme::constexpr_color{0.502f, 0.075f, 0.256f, 1.000f}, //title_bg_active
			theme::constexpr_color{0.200f, 0.220f, 0.270f, 0.750f}, //title_bg_collapsed
			theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.500f}, //message_panel
			theme::constexpr_color{1.000f, 1.000f, 1.000f, 1.000f}, //auto_complete_selected
			theme::constexpr_color{0.500f, 0.450f, 0.450f, 1.000f}, //auto_complete_non_selected
			theme::constexpr_color{0.600f, 0.600f, 0.600f, 1.000f}, //auto_complete_separator
			theme::constexpr_color{0.860f, 0.930f, 0.890f, 1.000f}, //cmd_backlog
			theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.000f}, //cmd_history_completed
			theme::constexpr_color{0.100f, 0.100f, 0.100f, 0.860f}, //log_level_drop_down_list_bg
			theme::constexpr_color{0.730f, 0.130f, 0.370f, 1.000f}, //log_level_active
			theme::constexpr_color{0.450f, 0.190f, 0.300f, 0.430f}, //log_level_hovered
			theme::constexpr_color{0.730f, 0.130f, 0.370f, 0.580f}, //log_level_selected
			theme::constexpr_color{0.000f, 0.000f, 0.000f, 0.000f}, //scrollbar_bg
			theme::constexpr_color{0.690f, 0.690f, 0.690f, 0.800f}, //scrollbar_grab
			theme::constexpr_color{0.490f, 0.490f, 0.490f, 0.800f}, //scrollbar_grab_active
			theme::constexpr_color{0.490f, 0.490f, 0.490f, 1.000f}, //scrollbar_grab_hovered
			theme::constexpr_color{0.649f, 0.661f, 0.669f, 1.000f}, //filter_hint
			theme::constexpr_color{1.000f, 1.000f, 1.000f, 1.000f}, //filter_text
			theme::constexpr_color{0.490f, 0.240f, 1.000f, 1.000f}, //matching_text
			{
				theme::constexpr_color{0.549f, 0.561f, 0.569f, 1.f}, // log_level::trace
				theme::constexpr_color{0.153f, 0.596f, 0.498f, 1.f}, // log_level::debug
				theme::constexpr_color{0.459f, 0.686f, 0.129f, 1.f}, // log_level::info
				theme::constexpr_color{0.839f, 0.749f, 0.333f, 1.f}, // log_level::warning
				theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::error
				theme::constexpr_color{1.000f, 0.420f, 0.408f, 1.f}, // log_level::critical
			},
		};

		constexpr std::array list {
				cherry,
				light
		};
	}
}

#include "terminal.tpp"

#undef IMTERM_FMT_INCLUDED

#endif //IMTERM_TERMINAL_HPP
