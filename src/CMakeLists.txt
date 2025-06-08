cmake_minimum_required(VERSION 3.15)
project(NoitaWispCalculator LANGUAGES CXX)


function(PRETTY_PRINT_FILES title file_list)
    string(LENGTH "${title}" title_length)
    math(EXPR line_length "${title_length} + 2")
    string(REPEAT "-" ${line_length} separator)
    
    message(STATUS "${separator}")
    message(STATUS " ${title}:")
    message(STATUS "${separator}")
    
    foreach(file ${file_list})
        get_filename_component(filename ${file} NAME)
        message(STATUS "  - ${filename}")
    endforeach()
    
    message(STATUS "Total: ${ARGC} files\n")
endfunction()



# Configuration des répertoires
set(EXTERNAL_DIR ../external)
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(SOURCE_DIR .)
set(RESOURCES_DIR ../assets)

# Détection automatique des fichiers sources (en excluant les exemples ImGui)
file(GLOB_RECURSE APP_SOURCES 
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.hpp"
    "${SOURCE_DIR}/*.h"
)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Détection automatique des ressources
file(GLOB_RECURSE APP_RESOURCES
    "${RESOURCES_DIR}/*.png"
    "${RESOURCES_DIR}/*.jpg"
    "${RESOURCES_DIR}/*.jpeg"
)

# Afficher les fichiers détectés de manière organisée
pretty_print_files("Project sources detected" "${APP_SOURCES}")
pretty_print_files("Project resources detected" "${APP_RESOURCES}")

# Création de la bibliothèque principale
add_library(main_lib STATIC
    ${IMGUI_SOURCES}
    ${APP_SOURCES}
)

# Configuration des include directories
target_include_directories(main_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXTERNAL_DIR}/imgui
    ${EXTERNAL_DIR}/imgui/backends
    ${EXTERNAL_DIR}/stb_image
    ${SOURCE_DIR}
)

# Find GLFW package
find_package(glfw3 3.3 REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Configuration spécifique à la plateforme
if(UNIX AND NOT APPLE)
    target_link_libraries(main_lib PRIVATE glfw OpenGL::OpenGL OpenGL::GLX)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(main_lib PRIVATE 
        glfw 
        OpenGL::OpenGL 
        ${COCOA_LIBRARY} 
        ${IOKIT_LIBRARY}
    )
elseif(WIN32)
    target_link_libraries(main_lib PRIVATE glfw opengl32)
endif()

# Options de compilation
target_compile_options(main_lib PRIVATE
    -Wall
    -Wextra
    -Werror
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Création de l'exécutable principal
add_executable(noitawisp ${SOURCE_DIR}/main.cpp)
target_link_libraries(noitawisp PRIVATE main_lib)

set_target_properties(noitawisp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)