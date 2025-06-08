#pragma once
#include <vector>
#include "Modifier.hpp"

namespace UI {
    void RenderMainWindow(bool* showSolutions, std::vector<std::vector<Modifier>>& solutions);
    void RenderSolutionsWindow(bool* p_open, std::vector<std::vector<Modifier>>& solutions);
    void SetupImGuiStyle();
    void SetupDefaultLayout();
}