#pragma once
#include <string>
#include <vector>
#include <imgui.h>

struct Modifier {
    std::string file;
    std::string name;
    int lifetime;
    bool selected;
    ImTextureID texture;
    
    Modifier(const std::string& f, const std::string& n, int l);
    
    bool operator==(const Modifier& other) const;
    bool operator<(const Modifier& other) const;
    size_t hash() const;

    static bool InitializeTextures();
    static void CleanupTextures();
    static std::vector<std::vector<Modifier>> CalculateCombinations(int spell_value_min, int spell_value_max, int depth, int target_min, int target_max);
    static std::vector<Modifier> modifiers;
};