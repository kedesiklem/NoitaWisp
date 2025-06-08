#include "Modifier.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/gl.h>
#include <algorithm>
#include <map>
#include <numeric>
#include <iostream>

#define ASSETS "assets"

// Liste des modificateurs disponibles
std::vector<Modifier> Modifier::modifiers = {
    {std::string(ASSETS) + "/Spell_zero_damage.png", "null_shot", 280},
    {std::string(ASSETS) + "/Spell_true_orbit.png", "true_orbit / phasing_arc", 80},
    {std::string(ASSETS) + "/Spell_lifetime.png", "increase_lifetime", 75},
    {std::string(ASSETS) + "/Spell_spiraling_shot.png", "spiral_arc", 50},
    {std::string(ASSETS) + "/Spell_pingpong_path.png", "pingpong_path / orbiting_arc", 25},
    {std::string(ASSETS) + "/Spell_chain_shot.png", "chain_spell", -30},
    {std::string(ASSETS) + "/Spell_lifetime_down.png", "reduce_lifetime", -42}
};

Modifier::Modifier(const std::string& f, const std::string& n, int l) 
    : file(f), name(n), lifetime(l), selected(false) {}

bool Modifier::operator==(const Modifier& other) const {
    return file == other.file && name == other.name && lifetime == other.lifetime;
}

bool Modifier::operator<(const Modifier& other) const {
    return file < other.file;
}

size_t Modifier::hash() const {
    return std::hash<std::string>{}(name);
}

bool Modifier::InitializeTextures() {
    for (auto& modifier : modifiers) {
        // Chargement de l'image avec STB Image
        int width, height, channels;
        unsigned char* data = stbi_load(modifier.file.c_str(), &width, &height, &channels, 4);
        
        if (!data) {
            printf("Failed to load image: %s\n", modifier.file.c_str());
            continue;
        }
        
        // Création de la texture OpenGL
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Configuration de la texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Upload des données de l'image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        
        // Stockage de la texture pour ImGui
        modifier.texture = static_cast<intptr_t>(textureID);
    }
    return true;
}

void Modifier::CleanupTextures() {
    for (auto& modifier : modifiers) {
        if (modifier.texture) {
            GLuint textureID = static_cast<GLuint>(modifier.texture);
            glDeleteTextures(1, &textureID);
            modifier.texture = ImTextureID(0);
        }
    }
}

// Fonction de comparaison pour trier les solutions par taille
static bool compare_by_size(const std::vector<Modifier>& a, const std::vector<Modifier>& b) {
    return a.size() < b.size();
}

static void step_wisp(int current_min,
                      int current_max,
                      int depth, 
                      const std::vector<Modifier>& mod_pos, 
                      const std::vector<Modifier>& mod_neg,
                      std::vector<Modifier> current_solution,
                      int target_min,
                      int target_max,
                      std::vector<std::vector<Modifier>>& solutions) {

    // Vérification de la solution
    bool isSolution = (current_min <= target_min && current_max >= target_min) || 
                     (current_min <= target_max && current_max >= target_max);
    
    if (isSolution) {
        solutions.push_back(current_solution);
    } 
    else if (depth > 0) {
        // Cas où on est en dessous de la cible
        if (current_max < target_min && !mod_pos.empty()) {
            
            auto new_solution = current_solution;
            new_solution.push_back(mod_pos[0]);
            step_wisp(current_min + mod_pos[0].lifetime,
                     current_max + mod_pos[0].lifetime, 
                     depth - 1, 
                     mod_pos, mod_neg, 
                     new_solution, 
                     target_min, target_max, 
                     solutions);
            
            // Explorer la branche sans ce modificateur
            std::vector<Modifier> remaining_pos(mod_pos.begin() + 1, mod_pos.end());
            step_wisp(current_min, current_max, depth, 
                     remaining_pos, mod_neg, 
                     current_solution, 
                     target_min, target_max, 
                     solutions);
        } 
        // Cas où on est au dessus de la cible
        else if (current_min > target_max && !mod_neg.empty()) {
            
            auto new_solution = current_solution;
            new_solution.push_back(mod_neg[0]);
            step_wisp(current_min + mod_neg[0].lifetime,
                     current_max + mod_neg[0].lifetime,
                     depth - 1,
                     mod_pos, mod_neg,
                     new_solution,
                     target_min, target_max,
                     solutions);
            
            // Explorer la branche sans ce modificateur
            std::vector<Modifier> remaining_neg(mod_neg.begin() + 1, mod_neg.end());
            step_wisp(current_min, current_max, depth,
                     mod_pos, remaining_neg,
                     current_solution,
                     target_min, target_max,
                     solutions);
        }
    }
}

std::vector<std::vector<Modifier>> Modifier::CalculateCombinations(int spell_value_min, int spell_value_max, int depth, int target_min, int target_max) {
    // Récupérer les modificateurs sélectionnés
    std::vector<Modifier> selected_modifiers;
    for (auto& m : modifiers) {
        if (m.selected) {
            selected_modifiers.push_back(m);
        }
    }

    // Séparation des modificateurs en positifs et négatifs
    std::vector<Modifier> mod_pos, mod_neg;
    for (const auto& m : selected_modifiers) {
        if (m.lifetime > 0) {
            mod_pos.push_back(m);
        } else {
            mod_neg.push_back(m);
        }
    }
    
    std::vector<std::vector<Modifier>> solutions;
    
    step_wisp(spell_value_min, spell_value_max, depth, mod_pos, mod_neg, {}, target_min, target_max, solutions);
    
    // Tri des solutions par taille croissante
    std::sort(solutions.begin(), solutions.end(), compare_by_size);
    
    return solutions;
}