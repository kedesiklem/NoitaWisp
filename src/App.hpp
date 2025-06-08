#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include "backend/Modifier.hpp"

class App {
public:
    App();
    ~App();
    
    void Run();
    bool Initialize();
    void Shutdown();
    
private:
    GLFWwindow* window;
    bool showSolutions = false;
    std::vector<std::vector<Modifier>> solutions;
    
    void Render();
    void CalculateWisp();
};