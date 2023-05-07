//
//  common.hpp
//  FrameTech
//
//  Created by Antonin on 28/04/2023.
//

#pragma once
#ifndef common_hpp
#define common_hpp

#include <glm/glm.hpp>

// Compatibility layer with shaders
// No need any namespace here

typedef struct ModelViewProjection
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
} ModelViewProjection;

#endif // common_hpp
