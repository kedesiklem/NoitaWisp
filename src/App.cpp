#include "App.hpp"
#include "backend/UI.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

App::App() : window(nullptr) {}

App::~App() {
    Shutdown();
}

bool App::Initialize() {
    if (!glfwInit()) return false;
    
    window = glfwCreateWindow(1280, 720, "Wisp Calculator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    UI::SetupImGuiStyle();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // Initialiser les modificateurs avec les textures
    if (!Modifier::InitializeTextures()) {
        return false;
    }
    
    return true;
}

void App::Run() {
    if (!Initialize()) return;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
            break;
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        Render();
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(window);
    }
}

void App::Shutdown() {
    Modifier::CleanupTextures();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void App::Render() {
    // Espace de docking principal
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
    
    // Afficher l'interface utilisateur
    UI::RenderMainWindow(&showSolutions, solutions);
    static bool first_run = true;
    if (first_run) {
        UI::SetupDefaultLayout();
        first_run = false;
    }
    if (showSolutions) {
        UI::RenderSolutionsWindow(&showSolutions, solutions);
    }
}