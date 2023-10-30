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

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Necessary for Mac
#endif
    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Mac App", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |=
            ImGuiConfigFlags_DockingEnable;


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 3;


//    ImFont* mainFont = io.Fonts->AddFontFromFileTTF(
//            "Assets/Fonts/JetBrainsMono-2.304/fonts/ttf/JetBrainsMono-Regular.ttf",
//            17, &config);
//    io.FontDefault = mainFont;

//


    custom_command_struct cmd_struct; // terminal commands can interact with this structure
    ImTerm::terminal<terminal_commands> terminal_log(cmd_struct);
    terminal_log.set_min_log_level(ImTerm::message::severity::info);

    bool showing_term = true;

    unsigned long frame_count = 0ul;
    unsigned short frame_mult = 5u;
    spdlog::set_level(spdlog::level::trace);
    spdlog::default_logger()->sinks().push_back(terminal_log.get_terminal_helper());


    while(!glfwWindowShouldClose(window)) {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        // 1. Poll events and clear the frame
        glfwPollEvents();

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 windowPadding = style.WindowPadding; // Store original padding
        ImVec2 itemSpacing = style.ItemSpacing; // Store original item spacing
        ImVec2 framePadding = style.FramePadding; // Store original frame padding

        style.WindowPadding = ImVec2(0.0f, 0.0f);  // Set padding to zero for this window
        style.ItemSpacing = ImVec2(0.0f, 0.0f);    // Optional: reduce spacing between items
        style.FramePadding = ImVec2(0.0f, 0.0f);   // Optional: reduce frame padding
        //Render imgui
        {
            // Window flags for the fullscreen window
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoCollapse;
            window_flags |= ImGuiWindowFlags_NoResize;
            window_flags |= ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            window_flags |= ImGuiWindowFlags_NoNavFocus;
            window_flags |= ImGuiWindowFlags_NoBackground; ;  // Disables the close button


            // Make it fullscreen
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);

            // Begin the parent window
            ImGui::Begin("DockSpace Demo", nullptr, window_flags);

            // Restore original style after you're done with the fullscreen window
            style.FramePadding.x  = framePadding.x;
            style.WindowPadding.x =  windowPadding.x;
            style.ItemSpacing.x = itemSpacing.x;

            // Custom taskbar (menu bar)
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New")) {
                        // Handle "New" action here
                    }
                    if (ImGui::MenuItem("Open")) {
                        // Handle "Open" action here
                    }
                    if (ImGui::MenuItem("Save")) {
                        // Handle "Save" action here
                    }
                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(window, true); // Close the application
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo")) {
                        // Handle "Undo" action here
                    }
                    // ... add other Edit options as needed ...
                    ImGui::EndMenu();
                }
                // ... add other menus as needed ...
                ImGui::EndMenuBar();
            }

            // DockSpace
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

            // Example child window (You can create multiple windows like this)
            if (ImGui::Begin("Child Window", nullptr, ImGuiWindowFlags_NoCollapse)) {
                ImGui::Text("Hello from the Child Window!");
            }
            ImGui::End();

            ImGui::ShowDemoWindow();
            ImGui::ShowDebugLogWindow();
            ImGui::ShowMetricsWindow();
            ImGui::ShowStackToolWindow();

            if (showing_term) {
//                if (frame_count % (30 * frame_mult) == 0) {
//                    spdlog::trace("Logging a trace message every {} frames", 30 * frame_mult);
//                }
//                if (frame_count % (60 * frame_mult) == 0) {
//                    spdlog::debug("Logging a debug message every {} frames", 60 * frame_mult);
//                }
//                if (frame_count % (120 * frame_mult) == 0) {
//                    spdlog::warn("Logging a warn message every {} frames", 120 * frame_mult);
//                }
//                if (frame_count % (90 * frame_mult) == 0) {
//                    spdlog::info("Logging an info message every {} frames", 90 * frame_mult);
//                }
//                if (frame_count % (180 * frame_mult) == 0) {
//                    spdlog::critical("Logging a critical message every {} frames", 180 * frame_mult);
//                }
//                if (frame_count % (150 * frame_mult) == 0) {
//                    spdlog::error("Logging an error message every {} frames", 150 * frame_mult);
//                }
                showing_term = terminal_log.show();
                if (cmd_struct.should_close) {
                    glfwSetWindowShouldClose(window, true);
                }
            }
            ++frame_count;

            // End the parent window
            ImGui::End();
        }




        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  // <-- This is crucial

        // 4. Swap the buffers
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
};
