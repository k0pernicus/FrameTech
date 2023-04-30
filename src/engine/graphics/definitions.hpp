//
//  definitions.hpp
//  FrameTech
//
//  Created by Antonin on 28/04/2023.
//

#pragma once
#ifndef definitions_h
#define definitions_h

#include <glm/glm.hpp>

// Compatibility layer with shaders
// No need any namespace here

typedef struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
} UniformBufferObject;

#endif // command_h
