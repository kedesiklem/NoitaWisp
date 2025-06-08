#include "UI.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>


void UI::SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    
    style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

}

void UI::SetupDefaultLayout() {
    // 1. Créer un dockspace principal
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNodeDockedWindows(viewport->ID, true); // Clear existing layout
    ImGui::DockBuilderAddNode(viewport->ID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(viewport->ID, viewport->Size);

    // 2. Diviser l'espace horizontalement
    ImGuiID dock_main_id = viewport->ID;
    ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.5f, nullptr, &dock_main_id);
    
    // 3. Assigner les fenêtres aux docks
    ImGui::DockBuilderDockWindow("Wisp Calculator", dock_left_id);
    ImGui::DockBuilderDockWindow("Solutions Found", dock_main_id);
    
    // 4. Configuration supplémentaire pour la fenêtre Debug
    ImGui::DockBuilderGetNode(dock_left_id)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowDockID(dock_left_id, ImGuiCond_FirstUseEver);
    
    // 5. Finaliser la configuration
    ImGui::DockBuilderFinish(dock_main_id);
    
    // 6. Configurer la fenêtre Debug (optionnelle)
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
}


void UI::RenderMainWindow(bool* showSolutions, std::vector<std::vector<Modifier>>& solutions) {
    static int spellLifetimeMin = 0;
    static int spellLifetimeMax = 100;
    static bool intervalLifetime = true;
    static int targetLifetimeMin = -1;
    static int targetLifetimeMax = -1;
    static bool wispMode = true;
    static int depthValue = 5;
    static bool selectAll = false;

    ImGui::Begin("Wisp Calculator");
    
    ImGui::Text("Spell Parameters:");

    ImGui::Checkbox("Spell lifetime interval", &intervalLifetime);
    if (intervalLifetime) {
        ImGui::InputInt("Spell Min Lifetime", &spellLifetimeMin);
        ImGui::InputInt("Spell Max Lifetime", &spellLifetimeMax);
    }else{
        ImGui::InputInt("Spell Lifetime", &spellLifetimeMin);
        spellLifetimeMax = spellLifetimeMin;
    }


    ImGui::Separator();
    
    ImGui::Checkbox("Wisp", &wispMode);
    if (wispMode) {
        targetLifetimeMax = targetLifetimeMin = -1;
    }else{
        ImGui::InputInt("Target Min Lifetime", &targetLifetimeMin);
        ImGui::InputInt("Target Max Lifetime", &targetLifetimeMax);
    }

    ImGui::Separator();
    ImGui::InputInt("Search Depth", &depthValue);
    ImGui::Separator();
    
    ImGui::Text("Available Modifiers:");
    
    if (ImGui::Button(selectAll ? "Unselect All" : "Select All")) {
        selectAll = !selectAll;
        for (auto& m : Modifier::modifiers) {
            m.selected = selectAll;
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Calculate Wisp")) {
        *showSolutions = true;
        solutions = Modifier::CalculateCombinations(spellLifetimeMin, spellLifetimeMax, depthValue, targetLifetimeMin, targetLifetimeMax);
    }
    
    // Afficher les modificateurs avec leurs icônes
    ImGui::BeginChild("ModifiersGrid", ImVec2(0, 200), true);
    const float button_size = 80.0f;
    const int columns = (int)(ImGui::GetContentRegionAvail().x / button_size);
    
    if (columns > 0) {
        ImGui::Columns(columns, "modifiersColumns", false);
        for (auto& m : Modifier::modifiers) {
            ImGui::BeginGroup();

        if (m.selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            if (ImGui::ImageButton(m.name.c_str(), m.texture, ImVec2(64, 64))) {
                m.selected = !m.selected;
            }
            ImGui::PopStyleColor();
        }else{
            if (ImGui::ImageButton(m.name.c_str(), m.texture, ImVec2(64, 64))) {
                m.selected = !m.selected;
            }
        }

        // Overing text
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", m.name.c_str());
            ImGui::EndTooltip();
        }


        ImGui::Text("%d", m.lifetime);

            ImGui::EndGroup();
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    ImGui::End();
}


int CalculateTotalEffect(const std::vector<Modifier>& solution) {
    int total = 0;
    for (const auto& mod : solution) {
        total += mod.lifetime;
    }
    return total;
}


void UI::RenderSolutionsWindow(bool* p_open, std::vector<std::vector<Modifier>>& solutions) {
    if (!*p_open) return;

    ImGui::SetNextWindowSize(ImVec2(1200, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Solutions Found", p_open)) {
        if (solutions.empty()) {
            ImGui::Text("No solutions found. Try adjusting parameters.");
        } else {
            // Collect all unique modifiers (with textures)
            std::set<Modifier> uniqueModifiers;
            for (const auto& solution : solutions) {
                for (const auto& mod : solution) {
                    uniqueModifiers.insert(mod);
                }
            }

            // Header with solutions count
            ImGui::Text("%zu solutions found:", solutions.size());


            // Main solutions table with dynamic columns
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            
            if (ImGui::BeginTable("SolutionsTable", 3 + uniqueModifiers.size(), 
                ImGuiTableFlags_Borders | 
                ImGuiTableFlags_ScrollY | 
                ImGuiTableFlags_ScrollX |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_SizingFixedFit))
            {
                // Configure fixed columns
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30);
                ImGui::TableSetupColumn("Steps", ImGuiTableColumnFlags_WidthFixed, 50);
                
                // Configure dynamic columns for each modifier type
                for (const auto& modifier : uniqueModifiers) {
                    ImGui::TableSetupColumn(modifier.name.c_str(), 0, 40);
                }

                // New columns
                ImGui::TableSetupColumn("Total Effect", ImGuiTableColumnFlags_WidthFixed, 80);

                // Custom header row with images
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                
                // Fixed headers
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("#");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Steps");

                // Modifier headers with images
                int col = 2;
                for (const auto& modifier : uniqueModifiers) {
                    ImGui::TableSetColumnIndex(col++);
                    ImGui::BeginGroup();
                    if (modifier.texture) {
                        ImGui::Image(modifier.texture, ImVec2(20, 20));
                        ImGui::SameLine();
                    }
                    ImGui::EndGroup();
                }

                // New column headers
                ImGui::TableSetColumnIndex(col++);
                ImGui::Text("Total Effect");

                // Populate table
                for (size_t i = 0; i < solutions.size(); i++) {
                    ImGui::TableNextRow();
                    
                    // Solution number
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%zu", i + 1);

                    // Step count
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%zu", solutions[i].size());

                    // Count modifiers for this solution
                    std::map<Modifier, int> modifierCounts;
                    int totalEffect = 0;
                    for (const auto& mod : solutions[i]) {
                        modifierCounts[mod]++;
                        totalEffect += mod.lifetime;
                    }

                    // Fill modifier columns
                    col = 2;
                    for (const auto& modifier : uniqueModifiers) {
                        ImGui::TableSetColumnIndex(col++);
                        auto it = modifierCounts.find(modifier);
                        if (it != modifierCounts.end()) {
                            ImGui::Text("%d", it->second);
                        } else {
                            ImGui::TextDisabled("-");
                        }
                    }
                    ImGui::TableSetColumnIndex(col++);
                    ImGui::Text("%+d", totalEffect);

                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
    }
    ImGui::End();
}