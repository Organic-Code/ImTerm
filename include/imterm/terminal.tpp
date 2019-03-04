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

#include <imgui.h>
#include <imgui_internal.h>
#include <array>
#include <cctype>
#include <charconv>

#include "misc.hpp"

namespace ImTerm {
namespace details {
	template <typename T>
	using is_space_method = decltype(std::declval<T&>().is_space(std::declval<std::string_view>()));

	template <typename TerminalHelper>
	std::enable_if_t<misc::is_detected_v<is_space_method, TerminalHelper>, int>
	constexpr is_space(std::shared_ptr<TerminalHelper>& t_h, std::string_view str) {
		static_assert(std::is_same_v<decltype(t_h->is_space(str)), int>, "TerminalHelper::is_space(std::string_view) should return an int");
		return t_h->is_space(str);
	}

	template <typename TerminalHelper>
	std::enable_if_t<!misc::is_detected_v<is_space_method, TerminalHelper>, int>
	constexpr is_space(std::shared_ptr<TerminalHelper>&, std::string_view str) {
		return str[0] == ' ' ? 1 : 0;
	}

	template <typename T>
	using get_length_method = decltype(std::declval<T&>().get_length(std::declval<std::string_view>()));

	template <typename TerminalHelper>
	std::enable_if_t<misc::is_detected_v<get_length_method, TerminalHelper>, unsigned long>
	constexpr get_length(std::shared_ptr<TerminalHelper>& t_h, std::string_view str) {
		static_assert(std::is_same_v<decltype(t_h->get_length(str)), int>, "TerminalHelper::get_length(std::string_view) should return an int");
		return t_h->get_length(str);
	}

	template <typename TerminalHelper>
	std::enable_if_t<!misc::is_detected_v<get_length_method, TerminalHelper>, unsigned long>
	constexpr get_length(std::shared_ptr<TerminalHelper>&, std::string_view str) {
		return str.size();
	}

	template <typename T>
	using set_terminal_method = decltype(std::declval<T&>().set_terminal(std::declval<terminal<T>&>()));

	template <typename TerminalHelper>
	std::enable_if_t<misc::is_detected_v<set_terminal_method, TerminalHelper>>
	assign_terminal(TerminalHelper& helper, terminal <TerminalHelper>& terminal) {
		helper.set_terminal(terminal);
	}

	template <typename TerminalHelper>
	std::enable_if_t<!misc::is_detected_v<set_terminal_method, TerminalHelper>>
	assign_terminal(TerminalHelper& helper, terminal <TerminalHelper>& terminal) {}
}

template <typename TerminalHelper>
terminal<TerminalHelper>::terminal(value_type& arg_value, const char* window_name_, int base_width_, int base_height_
		, std::shared_ptr<TerminalHelper> th, terminal_helper_is_valid&& /*unused*/)
		: m_argument_value{arg_value}
		, m_t_helper{std::move(th)}
		, m_window_name(window_name_)
		, m_base_width(base_width_)
		, m_base_height(base_height_)
		, m_autoscroll_text{"autoscroll"}
		, m_clear_text{"clear"}
		, m_log_level_text{"log level"}
		, m_autowrap_text{"autowrap"}
{
	assert(m_t_helper != nullptr);
	details::assign_terminal(*m_t_helper, *this);

	std::fill(m_command_buffer.begin(), m_command_buffer.end(), '\0');
	set_level_list_text("trace", "debug", "info", "warning", "error", "critical", "none");
}

template <typename TerminalHelper>
bool terminal<TerminalHelper>::show() noexcept {
	if (m_flush_bit) {
		m_last_flush_at_history = m_command_history.size();
		m_flush_bit = false;
	}

	m_should_show_next_frame = !m_close_request;
	m_close_request = false;

	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_base_width), static_cast<float>(m_base_height)), ImGuiCond_Once);

	int pop_count = 0;
	pop_count += try_push_style(ImGuiCol_Text, m_colors.text);
	pop_count += try_push_style(ImGuiCol_WindowBg, m_colors.window_bg);
	pop_count += try_push_style(ImGuiCol_ChildBg, m_colors.window_bg);
	pop_count += try_push_style(ImGuiCol_Border, m_colors.border);
	pop_count += try_push_style(ImGuiCol_BorderShadow, m_colors.border_shadow);
	pop_count += try_push_style(ImGuiCol_Button, m_colors.button);
	pop_count += try_push_style(ImGuiCol_ButtonHovered, m_colors.button_hovered);
	pop_count += try_push_style(ImGuiCol_ButtonActive, m_colors.button_active);
	pop_count += try_push_style(ImGuiCol_FrameBg, m_colors.frame_bg);
	pop_count += try_push_style(ImGuiCol_FrameBgHovered, m_colors.frame_bg_hovered);
	pop_count += try_push_style(ImGuiCol_FrameBgActive, m_colors.frame_bg_active);
	pop_count += try_push_style(ImGuiCol_TextSelectedBg, m_colors.text_selected_bg);
	pop_count += try_push_style(ImGuiCol_CheckMark, m_colors.check_mark);
	pop_count += try_push_style(ImGuiCol_TitleBg, m_colors.title_bg);
	pop_count += try_push_style(ImGuiCol_TitleBgActive, m_colors.title_bg_active);
	pop_count += try_push_style(ImGuiCol_TitleBgCollapsed, m_colors.title_bg_collapsed);
	pop_count += try_push_style(ImGuiCol_Header, m_colors.log_level_selected);
	pop_count += try_push_style(ImGuiCol_HeaderActive, m_colors.log_level_active);
	pop_count += try_push_style(ImGuiCol_HeaderHovered, m_colors.log_level_hovered);
	pop_count += try_push_style(ImGuiCol_PopupBg, m_colors.log_level_drop_down_list_bg);
	pop_count += try_push_style(ImGuiCol_ScrollbarBg, m_colors.scrollbar_bg);
	pop_count += try_push_style(ImGuiCol_ScrollbarGrab, m_colors.scrollbar_grab);
	pop_count += try_push_style(ImGuiCol_ScrollbarGrabActive, m_colors.scrollbar_grab_active);
	pop_count += try_push_style(ImGuiCol_ScrollbarGrabHovered, m_colors.scrollbar_grab_hovered);


	if (m_has_focus) {
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
		++pop_count;
		m_has_focus = false;
	}

	if (!ImGui::Begin(m_window_name, nullptr, ImGuiWindowFlags_NoScrollbar)) {
		ImGui::End();
		ImGui::PopStyleColor(pop_count);
		return true;
	}

	compute_text_size();
	display_settings_bar();
	display_messages();
	display_command_line();

	ImGui::End();
	ImGui::PopStyleColor(pop_count);

	return m_should_show_next_frame;
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::reset_colors() noexcept {
	for (std::optional<theme::constexpr_color>& color : m_colors.log_level_colors) {
		color.reset();
	}
	m_colors.text.reset();
	m_colors.window_bg.reset();
	m_colors.border.reset();
	m_colors.border_shadow.reset();
	m_colors.button.reset();
	m_colors.button_hovered.reset();
	m_colors.button_active.reset();
	m_colors.frame_bg.reset();
	m_colors.frame_bg_hovered.reset();
	m_colors.frame_bg_active.reset();
	m_colors.text_selected_bg.reset();
	m_colors.check_mark.reset();
	m_colors.title_bg.reset();
	m_colors.title_bg_active.reset();
	m_colors.title_bg_collapsed.reset();
	m_colors.message_panel.reset();
	m_colors.auto_complete_selected.reset();
	m_colors.auto_complete_non_selected.reset();
	m_colors.auto_complete_separator.reset();
	m_colors.auto_complete_selected.reset();
	m_colors.auto_complete_non_selected.reset();
	m_colors.auto_complete_separator.reset();
	m_colors.cmd_backlog.reset();
	m_colors.cmd_history_completed.reset();
	m_colors.log_level_drop_down_list_bg.reset();
	m_colors.log_level_active.reset();
	m_colors.log_level_hovered.reset();
	m_colors.log_level_selected.reset();
	m_colors.scrollbar_bg.reset();
	m_colors.scrollbar_grab.reset();
	m_colors.scrollbar_grab_active.reset();
	m_colors.scrollbar_grab_hovered.reset();
}

template<typename TerminalHelper>
void terminal<TerminalHelper>::add_text(std::string str, unsigned int color_beg, unsigned int color_end) {
	message msg;
	msg.is_term_message = true;
	msg.severity = message::severity::info;
	msg.color_beg = color_beg;
	msg.color_end = color_end;
	msg.value = std::move(str);
	m_logs.emplace_back(std::move(msg));
}

template<typename TerminalHelper>
void terminal<TerminalHelper>::add_text_err(std::string str, unsigned int color_beg, unsigned int color_end) {
	message msg;
	msg.is_term_message = true;
	msg.severity = message::severity::warn;
	msg.color_beg = color_beg;
	msg.color_end = color_end;
	msg.value = std::move(str);
	m_logs.emplace_back(std::move(msg));
}

template<typename TerminalHelper>
void terminal<TerminalHelper>::add_message(message&& msg) {
	if (msg.is_term_message && msg.severity != message::severity::warn) {
		msg.severity = message::severity::info;
	}
	m_logs.emplace_back(std::move(msg));
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::clear() {
	m_flush_bit = true;
	m_logs.clear();
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::set_level_list_text(std::string_view trace_str, std::string_view debug_str
		, std::string_view info_str, std::string_view warn_str, std::string_view err_str, std::string_view critical_str
		, std::string_view none_str) {

	m_level_list_text.clear();
	m_level_list_text.shrink_to_fit();
	m_level_list_text.resize(
			trace_str.size() + 1
		+ debug_str.size() + 1
		+ info_str.size() + 1
		+ warn_str.size() + 1
		+ err_str.size() + 1
		+ critical_str.size() + 1
		+ 1, '\0');

	const std::string_view* const levels[] = {&trace_str, &debug_str, &info_str, &warn_str, &err_str, &critical_str, &none_str};

	unsigned int current_shift = 0;
	for (const std::string_view* const lvl : levels) {
		std::copy(lvl->begin(), lvl->end(), m_level_list_text.data() + current_shift);
		current_shift += static_cast<unsigned int>(lvl->size()) + 1u;
	}

	set_min_log_level(m_lowest_log_level_val);
}


template <typename TerminalHelper>
void terminal<TerminalHelper>::set_min_log_level(message::severity::severity_t level) {
	m_lowest_log_level_val = level;
	m_lowest_log_level = m_level_list_text.data();
	for (int i = level ; i > 0 ; --i) {
		m_lowest_log_level += std::strlen(m_lowest_log_level) + 1;
	}
	m_selector_size_global.reset();

	m_longest_log_level = "";
	std::size_t longest_len = 0u;
	const char* current_str = m_lowest_log_level;

	for (int i = level ; i < message::severity::critical + 2 ; ++i) {
		auto length = std::strlen(current_str);
		auto regular_len = get_length({current_str, length});
		if (regular_len > longest_len) {
			longest_len = regular_len;
			m_longest_log_level = current_str;
		}
		current_str += length + 1;
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::try_log(std::string_view str, message::type type) {
	message::severity::severity_t severity;
	switch (type) {
		case message::type::user_input:
			severity = message::severity::trace;
			break;
		case message::type::error:
			severity = message::severity::err;
			break;
		case message::type::cmd_history_completion:
			severity = message::severity::debug;
			break;
	}
	std::optional<message> msg = m_t_helper->format({str.data(), str.size()}, type);
	if (msg) {
		msg->is_term_message = true;
		msg->severity = severity;
		m_logs.emplace_back(std::move(*msg));
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::compute_text_size() noexcept {
	if (m_log_level_text) {
		if (!m_selector_size_global) {
			m_selector_label_size = ImGui::CalcTextSize(m_log_level_text->data());
			m_selector_label_size.x += ImGui::GetStyle().ItemSpacing.x;
			m_selector_size_global = ImGui::CalcTextSize(m_longest_log_level);
			m_selector_size_global->x +=
					m_selector_label_size.x + ImGui::GetStyle().ItemInnerSpacing.x * 3 + ImGui::GetFrameHeight();
			m_selector_size_global->y += m_selector_label_size.y;
		}
	} else {
		m_selector_size_global.reset();
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::display_settings_bar() noexcept {

	bool same_line_req{false};
	auto same_line = [&same_line_req]() {
		if (same_line_req) {
			ImGui::SameLine();
		}
		same_line_req = true;
	};

	if (m_clear_text) {
		if (ImGui::Button(m_clear_text->data())) {
			clear();
		}
		same_line_req = true;
	}

	if (m_autowrap_text) {
		same_line();
		ImGui::Checkbox(m_autowrap_text->data(), &m_autowrap);
	}

	if (m_autoscroll_text) {
		same_line();
		ImGui::Checkbox(m_autoscroll_text->data(), &m_autoscroll);
	}

	if (m_log_level_text && m_selector_size_global) {
		same_line();
		ImGui::Dummy(ImVec2(std::max(ImGui::GetContentRegionAvailWidth() - m_selector_size_global->x, 0.f), 1));

		ImGui::SameLine();
		ImGui::TextUnformatted(m_log_level_text->data(), m_log_level_text->data() + m_log_level_text->size());

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::Combo("##terminal:log_level_selector:combo", &m_level, m_lowest_log_level);
		ImGui::PopItemWidth();
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::display_messages() noexcept {
	ImVec2 avail_space = ImGui::GetContentRegionAvail();
	if (avail_space.y > m_selector_size_global->y) {

		int style_push_count = try_push_style(ImGuiCol_ChildBg, m_colors.message_panel);
		if (ImGui::BeginChild("terminal:logs_window", ImVec2(avail_space.x, avail_space.y - m_selector_size_global->y), false,
		                      ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar)) {

			unsigned traced_count = 0;
			void (*text_formatted) (const char*, ...);
			if (m_autowrap) {
				text_formatted = ImGui::TextWrapped;
			} else {
				text_formatted = ImGui::Text;
			}

			for (const message& msg : m_logs) {
				if (msg.severity < (m_level + m_lowest_log_level_val) && !msg.is_term_message) {
					continue;
				}

				if (msg.color_beg < msg.color_end) {
					text_formatted("%.*s", msg.color_beg, msg.value.data());
					ImGui::SameLine(0.f, 0.f);

					int pop = 0;
					if (msg.is_term_message) {
						if (msg.severity == message::severity::trace) {
							pop += try_push_style(ImGuiCol_Text, m_colors.cmd_backlog);
							// todo: print even when there is no color in the message
							text_formatted("[%d] ", static_cast<int>(traced_count + m_last_flush_at_history - m_command_history.size()));
							++traced_count;
							ImGui::SameLine(0.f, 0.f);
						} else if (msg.severity == message::severity::debug) {
							pop += try_push_style(ImGuiCol_Text, m_colors.cmd_history_completed);
						} else if (msg.severity == message::severity::err) {
							pop += try_push_style(ImGuiCol_Text, m_colors.log_level_colors[msg.severity]);
						}
					} else {
						pop += try_push_style(ImGuiCol_Text, m_colors.log_level_colors[msg.severity]);
					}

					text_formatted("%.*s", msg.color_end - msg.color_beg, msg.value.data() + msg.color_beg);
					ImGui::PopStyleColor(pop);

					ImGui::SameLine(0.f, 0.f);
					text_formatted("%.*s", msg.value.size() - msg.color_end, msg.value.data() + msg.color_end);
				} else {
					text_formatted("%.*s", msg.value.size(), msg.value.data());
				}
			}
		}
		if (m_autoscroll) {
			if (m_last_size != m_logs.size()) {
				ImGui::SetScrollHereY(1.f);
				m_last_size = m_logs.size();
			}
		} else {
			m_last_size = 0u;
		}
		ImGui::PopStyleColor(style_push_count);
		ImGui::EndChild();
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::display_command_line() noexcept {
	if (!m_command_entered && ImGui::GetActiveID() == m_input_text_id && m_input_text_id != 0 && m_current_autocomplete.empty()) {
		if (m_autocomplete_pos != position::nowhere && m_buffer_usage == 0u && m_current_autocomplete_strings.empty()) {
			m_current_autocomplete = m_t_helper->list_commands();
		}
	}

	ImGui::Separator();
	show_input_text();
	handle_unfocus();
	show_autocomplete();
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::show_input_text() noexcept {
	ImGui::PushItemWidth(-1.f);
	if (m_should_take_focus) {
		ImGui::SetKeyboardFocusHere();
		m_should_take_focus = false;
	}
	m_previous_buffer_usage = m_buffer_usage;

	if (ImGui::InputText("##terminal:input_text", m_command_buffer.data(), m_command_buffer.size(),
	                     ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
	                     terminal::command_line_callback_st, this) && !m_ignore_next_textinput) {
		m_current_history_selection = {};
		if (m_buffer_usage > 0u && m_command_buffer[m_buffer_usage - 1] == '\0') {
			--m_buffer_usage;
		} else if (m_buffer_usage + 1 < m_command_buffer.size() && m_command_buffer[m_buffer_usage + 1] == '\0' && m_command_buffer[m_buffer_usage] != '\0'){
			++m_buffer_usage;
		} else {
			m_buffer_usage = misc::strnlen(m_command_buffer.data(), m_command_buffer.size());
		}

		if (m_autocomplete_pos != position::nowhere) {

			int sp_count = 0;
			auto is_space_lbd = [this, &sp_count](char c) {
				if (sp_count > 0) {
					--sp_count;
					return true;
				} else {
					sp_count = is_space({&c, static_cast<unsigned>(m_command_buffer.data() + m_buffer_usage - &c)});
					if (sp_count > 0) {
						--sp_count;
						return true;
					}
					return false;
				}
			};
			char* beg = std::find_if_not(m_command_buffer.data(), m_command_buffer.data() + m_buffer_usage,
			                            is_space_lbd);
			sp_count = 0;
			const char* ed = std::find_if(beg, m_command_buffer.data() + m_buffer_usage, is_space_lbd);

			if (ed == m_command_buffer.data() + m_buffer_usage) {
				m_current_autocomplete = m_t_helper->find_commands_by_prefix(beg, ed);
				m_current_autocomplete_strings.clear();
				m_command_entered = true;
			} else {
				m_command_entered = false;
				m_current_autocomplete.clear();
				std::vector<command_type_cref> cmds = m_t_helper->find_commands_by_prefix(beg, ed);

				if (!cmds.empty()) {
					std::string_view sv{m_command_buffer.data(), m_buffer_usage};
					std::optional<std::vector<std::string>> splitted = split_by_space(sv, true);
					assert(splitted);
					argument_type arg{m_argument_value, *this, *splitted};
					m_current_autocomplete_strings = cmds[0].get().complete(arg);
				}
			}
		} else {
			m_command_entered = false;
		}
	}
	m_ignore_next_textinput = false;
	ImGui::PopItemWidth();

	if (m_input_text_id == 0u) {
		m_input_text_id = ImGui::GetItemID();
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::handle_unfocus() noexcept {
	auto clear_frame = [this]() {
		m_command_buffer[0] = '\0';
		m_buffer_usage = 0u;
		m_command_line_backup_prefix.remove_prefix(m_command_line_backup_prefix.size());
		m_command_line_backup.clear();
		m_current_history_selection = {};
		m_current_autocomplete.clear();
	};

	if (m_previously_active_id == m_input_text_id && ImGui::GetActiveID() != m_input_text_id) {
		if (ImGui::IsKeyPressedMap(ImGuiKey_Enter)) {
			call_command();
			m_should_take_focus = true;
			clear_frame();

		} else if (ImGui::IsKeyPressedMap(ImGuiKey_Escape)) {
			if (m_buffer_usage == 0u && m_previous_buffer_usage == 0u) {
				m_should_show_next_frame = false; // should hide on next frames
			} else {
				m_should_take_focus = true;
			}
			clear_frame();
		}
	}
	m_previously_active_id = ImGui::GetActiveID();
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::show_autocomplete() noexcept {
	constexpr ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
	                                           | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
	                                           | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (m_autocomplete_pos == position::nowhere) {
		return;
	}

	if ((m_input_text_id == ImGui::GetActiveID() || m_should_take_focus) && (!m_current_autocomplete.empty() || !m_current_autocomplete_strings.empty())) {
		m_has_focus = true;

		ImGui::SetNextWindowBgAlpha(0.9f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::SetNextWindowFocus();

		ImVec2 auto_complete_pos = ImGui::GetItemRectMin();

		if (m_autocomplete_pos == position::up) {
			auto_complete_pos.y -= (m_selector_size_global->y + 3);
		} else {
			auto_complete_pos.y = ImGui::GetItemRectMax().y;
		}

		ImVec2 auto_complete_max_size = ImGui::GetItemRectSize();
		auto_complete_max_size.y = -1.f;
		ImGui::SetNextWindowPos(auto_complete_pos);
		ImGui::SetNextWindowSizeConstraints({0.f, 0.f}, auto_complete_max_size);
		if (ImGui::Begin("##terminal:auto_complete", nullptr, overlay_flags)) {

			auto print_separator = [this]() {
				ImGui::SameLine(0.f, 0.f);
				int pop = try_push_style(ImGuiCol_Text, m_colors.auto_complete_separator);
				ImGui::TextUnformatted(m_autocomlete_separator.data(),
				                       m_autocomlete_separator.data() + m_autocomlete_separator.size());
				ImGui::PopStyleColor(pop);
				ImGui::SameLine(0.f, 0.f);
			};

			int max_displayable_sv = 0;
			float separator_length = ImGui::CalcTextSize(m_autocomlete_separator.data(),
			                                             m_autocomlete_separator.data() + m_autocomlete_separator.size()).x;
			float total_text_length = ImGui::CalcTextSize("...").x;

			std::vector<std::string_view> autocomplete_text;
			if (m_current_autocomplete_strings.empty()) {
				autocomplete_text.reserve(m_current_autocomplete.size());
				for (const command_type& cmd : m_current_autocomplete) {
					autocomplete_text.emplace_back(cmd.name);
				}
			} else {
				autocomplete_text.reserve(m_current_autocomplete_strings.size());
				for (const std::string& str : m_current_autocomplete_strings) {
					autocomplete_text.emplace_back(str);
				}
			}

			for (const std::string_view& sv : autocomplete_text) {
				float t_len = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x + separator_length;
				if (t_len + total_text_length < auto_complete_max_size.x) {
					total_text_length += t_len;
					++max_displayable_sv;
				} else {
					break;
				}
			}

			std::string_view last;
			int pop_count = 0;

			if (max_displayable_sv != 0) {
				const std::string_view& first = autocomplete_text[0];
				pop_count += try_push_style(ImGuiCol_Text, m_colors.auto_complete_selected);
				ImGui::TextUnformatted(first.data(), first.data() + first.size());
				pop_count += try_push_style(ImGuiCol_Text, m_colors.auto_complete_non_selected);
				for (int i = 1 ; i < max_displayable_sv ; ++i) {
					const std::string_view vs = autocomplete_text[i];
					print_separator();
					ImGui::TextUnformatted(vs.data(), vs.data() + vs.size());
				}
				ImGui::PopStyleColor(pop_count);
				if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) {
					last = autocomplete_text[max_displayable_sv];
				}
			}

			pop_count = 0;
			if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) {

				if (max_displayable_sv == 0) {
					last = autocomplete_text.front();
					pop_count += try_push_style(ImGuiCol_Text, m_colors.auto_complete_selected);
					total_text_length -= separator_length;
				} else {
					pop_count += try_push_style(ImGuiCol_Text, m_colors.auto_complete_non_selected);
					print_separator();
				}

				std::vector<char> buf;
				buf.resize(last.size() + 4);
				std::copy(last.begin(), last.end(), buf.begin());
				std::fill(buf.begin() + last.size(), buf.end(), '.');
				auto size = static_cast<unsigned>(last.size() + 3);
				while (size >= 4 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
					buf[size - 4] = '.';
					--size;
				}
				while (size != 0 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
					--size;
				}
				ImGui::TextUnformatted(buf.data(), buf.data() + size);
				ImGui::PopStyleColor(pop_count);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

template <typename TerminalHelper>
void terminal<TerminalHelper>::call_command() noexcept {
	m_current_autocomplete_strings.clear();
	m_current_autocomplete.clear();

	bool modified{};
	std::pair<bool, std::string> resolved = resolve_history_references({m_command_buffer.data(), m_buffer_usage}, modified);

	if (!resolved.first) {
		try_log(R"(No such event: )" + resolved.second, message::type::error);
		return;
	}

	std::optional<std::vector<std::string>> splitted = split_by_space(resolved.second);
	if (!splitted) {
		try_log({m_command_buffer.data(), m_buffer_usage}, message::type::user_input);
		try_log("Unmatched \"", message::type::error);
		return;
	}

	try_log({m_command_buffer.data(), m_buffer_usage}, message::type::user_input);
	if (splitted->empty()) {
		return;
	}
	if (modified) {
		try_log("> " + resolved.second, message::type::cmd_history_completion);
	}

	std::vector<command_type_cref> matching_command_list = m_t_helper->find_commands_by_prefix(splitted->front());
	if (matching_command_list.empty()) {
		splitted->front() += ": command not found";
		try_log(splitted->front(), message::type::error);
		m_command_history.emplace_back(std::move(resolved.second));
		return;
	}

	argument_type arg{m_argument_value, *this, *splitted};
	m_flush_bit = false;
	matching_command_list[0].get().call(arg);
	m_command_history.emplace_back(std::move(resolved.second)); // resolved.second has ownership over *splitted
	if (m_flush_bit) {
		m_last_flush_at_history = m_command_history.size();
	}
}

template <typename TerminalHelper>
std::pair<bool, std::string> terminal<TerminalHelper>::resolve_history_references(std::string_view str, bool& modified) const {
	enum class state {
		nothing, // matched nothing
		part_1,  // matched one char: '!'
		part_2,  // matched !-
		part_3,  // matched !-[n]
		part_4,  // matched !-[n]:
		finalize // matched !-[n]:[n]
	};

	modified = false;
	if (str.empty()) {
		return {true, {}};
	}

	if (str.size() == 1) {
		return {(str[0] != '!'), {str.data(), str.size()}};
	}

	std::string ans;
	ans.reserve(str.size());

	auto substr_beg = str.data();
	auto it = std::next(substr_beg);

	state current_state = (*substr_beg == '!') ? state::part_1 : state::nothing;
	bool escaped = (*substr_beg == '\\');

	auto resolve = [&](std::string_view history_request, bool add_escaping = true) -> bool {
		bool local_modified{};
		std::optional<std::string> solved = resolve_history_reference(history_request, local_modified);
		if (!solved) {
			return false;
		}

		auto is_space_lbd = [&solved, this](char c) {
			return is_space({&c, static_cast<unsigned>(&solved.value()[solved->size() - 1] + 1 - &c)}) > 0;
		};

		modified |= local_modified;
		if (add_escaping) {
			if (solved->empty()) {
				ans += R"("")";
			} else if (std::find_if(solved->begin(), solved->end(), is_space_lbd) != solved->end()) {
				ans += '"';
				ans += *solved;
				ans += '"';
			} else {
				ans += *solved;
			}
		} else {
			ans += *solved;
		}
		substr_beg = std::next(it);
		current_state = state::nothing;
		return true;
	};

	while (it != str.data() + str.size()) {
		switch (current_state) {
			case state::nothing:
				if (*it == '!' && !escaped) {
					current_state = state::part_1;
					ans += std::string_view{substr_beg, static_cast<unsigned>(it - substr_beg)};
					substr_beg = it;
				}
				break;

			case state::part_1:
				if (*it == '-') {
					current_state = state::part_2;
				} else if (*it == ':') {
					current_state = state::part_4;
				} else if (*it == '!') {
					if (!resolve("!!", false)) {
						return {false, "!!"};
					}
				} else {
					current_state = state::nothing;
				}
				break;
			case state::part_2:
				if (is_digit(*it)) {
					current_state = state::part_3;
				} else {
					return {false, {substr_beg, it}};
				}
				break;
			case state::part_3:
				if (*it == ':') {
					current_state = state::part_4;
				} else if (!is_digit(*it)) {
					if (!resolve({substr_beg, static_cast<unsigned>(it - substr_beg)}, false)) {
						return {false, {substr_beg, it}};
					}
				}
				break;
			case state::part_4:
				if (is_digit(*it)) {
					current_state = state::finalize;
				} else if (*it == '*') {
					if (!resolve({substr_beg, static_cast<unsigned>(it + 1 - substr_beg)}, false)) {
						return {false, {substr_beg, it}};
					}
				} else {
					return {false, {substr_beg, it}};
				}
				break;
			case state::finalize:
				if (!is_digit(*it)) {
					if (!resolve({substr_beg, static_cast<unsigned>(it - substr_beg)})) {
						return {false, {substr_beg, it}};
					}
				}
				break;
		}

		escaped = (*it == '\\');
		++it;
	}

	bool escape = true;
	if (substr_beg != it) {
		switch (current_state) {
			case state::nothing:
				[[fallthrough]];
			case state::part_1:
				ans += std::string_view{substr_beg, static_cast<unsigned>(it - substr_beg)};
				break;
			case state::part_2:
				[[fallthrough]];
			case state::part_4:
				return {false, {substr_beg, it}};
			case state::part_3:
				escape = false;
				[[fallthrough]];
			case state::finalize:
				if (!resolve({substr_beg, static_cast<unsigned>(it - substr_beg)}, escape)) {
					return {false, {substr_beg, it}};
				}
				break;
		}
	}

	return {true,std::move(ans)};
}

template <typename TerminalHelper>
std::optional<std::string> terminal<TerminalHelper>::resolve_history_reference(std::string_view str, bool& modified) const noexcept {
	modified = false;

	if (str.empty() || str[0] != '!') {
		return std::string{str.begin(), str.end()};
	}

	if (str.size() < 2) {
		return {};
	}

	if (str[1] == '!') {
		if (m_command_history.empty() || str.size() != 2) {
			return {};
		} else {
			modified = true;
			return {m_command_history.back()};
		}
	}

	// ![stuff]
	unsigned int backward_jump = 1;
	unsigned int char_idx = 1;
	if (str[1] == '-') {
		if (str.size() <= 2 || !is_digit(str[2])) {
			return {};
		}

		unsigned int val{0};
		std::from_chars_result res = std::from_chars(str.data() + 2, str.data() + str.size(), val, 10);
		if (val == 0) {
			return {}; // val == 0  <=> (garbage input || user inputted 0)
		}

		backward_jump = val;
		char_idx = static_cast<unsigned int>(res.ptr - str.data());
	}

	if (m_command_history.size() < backward_jump) {
		return {};
	}

	if (char_idx >= str.size()) {
		modified = true;
		return m_command_history[m_command_history.size() - backward_jump];
	}

	if (str[char_idx] != ':') {
		return {};
	}


	++char_idx;
	if (str.size() <= char_idx) {
		return {};
	}

	if (str[char_idx] == '*') {
		modified = true;
		const std::string& cmd = m_command_history[m_command_history.size() - backward_jump];

		int sp_count = 0;
		auto is_space_lbd = [&sp_count, &cmd, this] (char c) {
			if (sp_count > 0) {
				--sp_count;
				return true;
			}
			sp_count = is_space({&c, static_cast<unsigned>(&*cmd.end() - &c)});
			if (sp_count > 0) {
				--sp_count;
				return true;
			}
			return false;
		};

		auto first_non_space = std::find_if_not(cmd.begin(), cmd.end(), is_space_lbd);
		sp_count = 0;
		auto first_space = std::find_if(first_non_space, cmd.end(), is_space_lbd);
		sp_count = 0;
		first_non_space = std::find_if_not(first_space, cmd.end(), is_space_lbd);

		if (first_non_space == cmd.end()) {
			return std::string{""};
		}
		return std::string{first_non_space, cmd.end()};
	}

	if (!is_digit(str[char_idx])) {
		return {};
	}

	unsigned int val1{};
	std::from_chars_result res1 = std::from_chars(str.data() + char_idx, str.data() + str.size(), val1, 10);
	if (!misc::success(res1.ec) || res1.ptr != str.data() + str.size()) { // either unsuccessful or we didn't reach the end of the string
		return {};
	}

	const std::string& cmd = m_command_history[m_command_history.size() - backward_jump]; // 1 <= backward_jump <= command_history.size()
	std::optional<std::vector<std::string>> args = split_by_space(cmd);

	if (!args || args->size() <= val1) {
		return {};
	}

	modified = true;
	return (*args)[val1];

}

template <typename TerminalHelper>
int terminal<TerminalHelper>::command_line_callback_st(ImGuiInputTextCallbackData * data) noexcept {
	return reinterpret_cast<terminal *>(data->UserData)->command_line_callback(data);
}

template <typename TerminalHelper>
int terminal<TerminalHelper>::command_line_callback(ImGuiInputTextCallbackData* data) noexcept {

	auto paste_buffer = [data](auto begin, auto end, auto buffer_shift) {
		misc::copy(begin, end, data->Buf + buffer_shift, data->Buf + data->BufSize - 1);
		data->BufTextLen = std::min(static_cast<int>(std::distance(begin, end) + buffer_shift), data->BufSize - 1);
		data->Buf[data->BufTextLen] = '\0';
		data->BufDirty = true;
		data->SelectionStart = data->SelectionEnd;
		data->CursorPos = data->BufTextLen;
	};

	auto auto_complete_buffer = [data, this](std::string&& str, auto reference_size) {
		auto buff_end = misc::erase_insert(str.begin(), str.end(), data->Buf + data->CursorPos - reference_size
				, data->Buf + m_buffer_usage, data->Buf + data->BufSize, reference_size);

		data->BufTextLen = static_cast<unsigned>(std::distance(data->Buf, buff_end));
		data->Buf[data->BufTextLen] = '\0';
		data->BufDirty = true;
		data->SelectionStart = data->SelectionEnd;
		data->CursorPos = std::min(static_cast<int>(data->CursorPos + str.size() - reference_size), data->BufTextLen);
		m_buffer_usage = static_cast<unsigned>(data->BufTextLen);
	};

	if (data->EventKey == ImGuiKey_Tab) {
		std::vector<std::string_view> autocomplete_text;
		if (m_current_autocomplete_strings.empty()) {
			autocomplete_text.reserve(m_current_autocomplete.size());
			for (const command_type& cmd : m_current_autocomplete) {
				autocomplete_text.emplace_back(cmd.name);
			}
		} else {
			autocomplete_text.reserve(m_current_autocomplete_strings.size());
			for (const std::string& str : m_current_autocomplete_strings) {
				autocomplete_text.emplace_back(str);
			}
		}


		if (autocomplete_text.empty()) {
			if (m_buffer_usage == 0 || data->CursorPos < 2) {
				return 0;
			}

			auto excl = misc::find_last(m_command_buffer.data(), m_command_buffer.data() + data->CursorPos, '!');
			if (excl == m_command_buffer.data() + data->CursorPos) {
				return 0;
			}
			if (excl == m_command_buffer.data() + data->CursorPos - 1 && m_command_buffer[data->CursorPos - 2] == '!') {
				--excl;
			}
			bool modified{};
			std::string_view reference{excl, static_cast<unsigned>(m_command_buffer.data() + data->CursorPos - excl)};
			std::optional<std::string> val = resolve_history_reference(reference, modified);
			if (!modified) {
				return 0;
			}

			if (reference.substr(reference.size() - 2) != ":*" && reference.find(':') != std::string_view::npos) {
				auto is_space_lbd = [&val, this] (char c) {
					return is_space({&c, static_cast<unsigned>(&val.value()[val->size()] + 1 - &c)}) > 0;
				};

				if (std::find_if(val->begin(), val->end(), is_space_lbd) != val->end()) {
					val = '"' + std::move(*val) + '"';
				}
			}
			auto_complete_buffer(std::move(*val), static_cast<unsigned>(reference.size()));

			return 0;
		}

		std::string_view complete_sv = autocomplete_text[0];

		auto quote_count = std::count(m_command_buffer.data(), m_command_buffer.data() + m_buffer_usage, '"');
		const char* command_beg = nullptr;
		if (quote_count % 2) {
			command_beg = misc::find_last(m_command_buffer.data(), m_command_buffer.data() + m_buffer_usage, '"');
		} else {
			command_beg = misc::find_terminating_word(m_command_buffer.data(), m_command_buffer.data() + m_buffer_usage
					, [this](std::string_view sv) { return is_space(sv); });;
		}


		bool space_found = std::find_if(complete_sv.begin(), complete_sv.end(), [this,&complete_sv](char c)
				{ return is_space({&c, static_cast<unsigned>(&complete_sv[complete_sv.size() - 1] + 1 - &c)}) > 0; }) != complete_sv.end();

		if (space_found) {
			std::string complete;
			complete.reserve(complete_sv.size() + 2);
			complete = '"';
			complete += complete_sv;
			complete += '"';
			paste_buffer(complete.data(), complete.data() + complete.size(), command_beg - m_command_buffer.data());
		} else {
			paste_buffer(complete_sv.data(), complete_sv.data() + complete_sv.size(), command_beg - m_command_buffer.data());
		}

		m_buffer_usage = static_cast<unsigned>(data->BufTextLen);
		m_current_autocomplete.clear();
		m_current_autocomplete_strings.clear();

	} else if (data->EventKey == ImGuiKey_UpArrow) {
		if (m_command_history.empty()) {
			return 0;
		}
		m_ignore_next_textinput = true;

		if (!m_current_history_selection) {

			m_current_history_selection = m_command_history.end();
			m_command_line_backup = std::string(m_command_buffer.data(), m_command_buffer.data() + m_buffer_usage);
			m_command_line_backup_prefix = m_command_line_backup;

			auto is_space_lbd = [this](unsigned int idx) {
				const char* ptr = &m_command_line_backup_prefix[idx];
				return is_space({ptr, static_cast<unsigned>(m_command_line_backup_prefix.size() - idx)});
			};
			unsigned int idx = 0;
			int space_count = 0;
			while (idx < m_command_line_backup_prefix.size() && (space_count = is_space_lbd(idx)) > 0) {
				idx += space_count;
			}
			if (idx > m_command_line_backup_prefix.size()) {
				idx = static_cast<unsigned>(m_command_line_backup_prefix.size());
			}

			m_command_line_backup_prefix.remove_prefix(idx);
			m_current_autocomplete.clear();
			m_current_autocomplete_strings.clear();
		}

		auto it = misc::find_first_prefixed(
				m_command_line_backup_prefix
				, std::reverse_iterator(*m_current_history_selection)
				, m_command_history.rend()
				, [this](std::string_view str) { return is_space(str); }
		);

		if (it != m_command_history.rend()) {
			m_current_history_selection = std::prev(it.base());
			paste_buffer((*m_current_history_selection)->begin() + m_command_line_backup_prefix.size()
					, (*m_current_history_selection)->end(), m_command_line_backup.size());
			m_buffer_usage = static_cast<unsigned>(data->BufTextLen);
		} else {
			if (m_current_history_selection == m_command_history.end()) {
				// no auto completion occured
				m_ignore_next_textinput = false;
				m_current_history_selection = {};
				m_command_line_backup_prefix = {};
				m_command_line_backup.clear();
			}
		}

	} else if (data->EventKey == ImGuiKey_DownArrow) {

		if (!m_current_history_selection) {
			return 0;
		}
		m_ignore_next_textinput = true;

		m_current_history_selection = misc::find_first_prefixed(
				m_command_line_backup_prefix
				, std::next(*m_current_history_selection)
				, m_command_history.end()
				, [this](std::string_view str) { return is_space(str); }
		);

		if (m_current_history_selection != m_command_history.end()) {
			paste_buffer((*m_current_history_selection)->begin() + m_command_line_backup_prefix.size()
					, (*m_current_history_selection)->end(), m_command_line_backup.size());
			m_buffer_usage = static_cast<unsigned>(data->BufTextLen);

		} else {
			data->BufTextLen = static_cast<int>(m_command_line_backup.size());
			data->Buf[data->BufTextLen] = '\0';
			data->BufDirty = true;
			data->SelectionStart = data->SelectionEnd;
			data->CursorPos = data->BufTextLen;
			m_buffer_usage = static_cast<unsigned>(data->BufTextLen);

			m_current_history_selection = {};
			m_command_line_backup_prefix = {};
			m_command_line_backup.clear();
		}

	} else {
		// todo: log in some meaningfull way ?
	}

	return 0;
}

template<typename TerminalHelper>
int terminal<TerminalHelper>::is_space(std::string_view str) const {
	return details::is_space(m_t_helper, str);
}

template<typename TerminalHelper>
bool terminal<TerminalHelper>::is_digit(char c) const {
	return c >= '0' && c <= '9';
}

template<typename TerminalHelper>
unsigned long terminal<TerminalHelper>::get_length(std::string_view str) const {
	return details::get_length(m_t_helper, str);
}

template <typename TerminalHelper>
std::optional<std::vector<std::string>> terminal<TerminalHelper>::split_by_space(std::string_view in, bool ignore_non_match) const {
	std::vector<std::string> out;

	const char* it = &in[0];
	const char* const in_end = &in[in.size() - 1] + 1;

	auto skip_spaces = [&]() {
		int space_count;
		do {
			space_count = is_space({it, static_cast<unsigned>(in_end - it)});
			it += space_count;
		} while (it != in_end && space_count > 0);
	};

	if (it != in_end) {
		skip_spaces();
	}

	if (it == in_end) {
		return out;
	}

	bool matched_quote{};
	bool matched_space{};
	std::string current_string{};
	do {
		if (*it == '"') {
			bool escaped;
			do {
				escaped = (*it == '\\');
				++it;

				if (it != in_end && (*it != '"' || escaped)) {
					if (*it == '\\') {
						if (escaped) {
							current_string += *it;
						}
					} else {
						current_string += *it;
					}
				} else {
					break;
				}

			} while (true);

			if (it == in_end) {
				if (!ignore_non_match) {
					return {};
				}
			} else {
				++it;
			}
			matched_quote = true;
			matched_space = false;

		} else if (is_space({it, static_cast<unsigned>(in_end - it)}) > 0) {
			out.emplace_back(std::move(current_string));
			current_string = {};
			skip_spaces();
			matched_space = true;
			matched_quote = false;

		} else if (*it == '\\') {
			matched_quote = false;
			matched_space = false;
			++it;
			if (it != in_end) {
				current_string += *it;
				++it;
			}
		} else {
			matched_space = false;
			matched_quote = false;
			current_string += *it;
			++it;
		}

	} while (it != in_end);

	if (!current_string.empty()) {
		out.emplace_back(std::move(current_string));
	} else if (matched_quote || matched_space) {
		out.emplace_back();
	}

	return out;
}

} // namespace term
