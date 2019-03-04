///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018-2019, Lucas Lazare                                                                                                ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  		files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,  ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  		is furnished to do so, subject to the following conditions:                                                                 ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <spdlog/spdlog.h>

#include "imterm/terminal.hpp"
#include "terminal_commands.hpp"

int main()
{
	sf::Clock deltaClock;
	sf::Clock musicOffsetClock;

	sf::RenderWindow window(sf::VideoMode(600, 400), "Terminal Demo");

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr;


	custom_command_struct cmd_struct; // terminal commands can interact with this structure
	ImTerm::terminal<terminal_commands> terminal_log(cmd_struct);
	terminal_log.set_min_log_level(ImTerm::message::severity::info);

	bool showing_term = true;

	unsigned long frame_count = 0ul;
	unsigned short frame_mult = 5u;
	spdlog::set_level(spdlog::level::trace);
	spdlog::default_logger()->sinks().push_back(terminal_log.get_terminal_helper());

	while(window.isOpen())
	{
		sf::Event event{};
		while(window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if(event.type == sf::Event::Closed)
			{
				window.close();
			} else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F11) {
					showing_term = true;
				}
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		if (showing_term) {
			if (frame_count % (30 * frame_mult) == 0) {
				spdlog::trace("Logging a trace message every {} frames", 30 * frame_mult);
			}
			if (frame_count % (60 * frame_mult) == 0) {
				spdlog::debug("Logging a debug message every {} frames", 60 * frame_mult);
			}
			if (frame_count % (120 * frame_mult) == 0) {
				spdlog::warn("Logging a warn message every {} frames", 120 * frame_mult);
			}
			if (frame_count % (90 * frame_mult) == 0) {
				spdlog::info("Logging an info message every {} frames", 90 * frame_mult);
			}
			if (frame_count % (180 * frame_mult) == 0) {
				spdlog::critical("Logging a critical message every {} frames", 180 * frame_mult);
			}
			if (frame_count % (150 * frame_mult) == 0) {
				spdlog::error("Logging an error message every {} frames", 150 * frame_mult);
			}
			showing_term = terminal_log.show();
			if (cmd_struct.should_close) {
				window.close();
			}
		}

		window.clear();
		ImGui::SFML::Render(window);
		window.display();

		++frame_count;
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}
