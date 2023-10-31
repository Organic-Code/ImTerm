#pragma once
#include <iostream>
#include <spdlog/spdlog.h>

#include <imgui.h>

#include <imgui_internal.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

//Terminal
#include "imterm/terminal.hpp"
#include "terminal_commands.hpp"

class Application {
public:
    Application();

    ~Application();

    void refresh();
    void refreshFromTerminal();
private:
    void init();
    void run();

    GLFWwindow *m_Window = nullptr;
    unsigned long m_FrameCount = 0ul;
    bool m_Initialized = false;
    bool m_ShowingTerm = true;

    static const int c_WindowWidth = 800;
    static const int c_WindowHeight = 600;
};
