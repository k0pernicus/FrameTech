//
//  inputs.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "inputs.hpp"
#include "debug_tools.h"
#include "engine.hpp"

const double CAMERA_ROTATION_STEP = 0.05;

void frametech::inputs::Handler::pollEvent(bool blank) noexcept {
	if (blank) return;
	std::scoped_lock<std::mutex> guard(m_lock);
	if (m_keys.empty()) return;
	switch (m_keys.front()) {
	case RIGHT: 
		{
			Log("RIGHT key has been hit");
			const glm::vec3 camera_direction = frametech::Engine::getInstance()->m_camera.getDirection();
			const glm::vec3 new_direction = glm::vec3(camera_direction.x + CAMERA_ROTATION_STEP, camera_direction.y, camera_direction.z);
			frametech::Engine::getInstance()->m_camera.setDirection(new_direction);
		} 
		break;
	case DOWN:
		{
			Log("DOWN key has been hit");
			const glm::vec3 camera_direction = frametech::Engine::getInstance()->m_camera.getDirection();
			const glm::vec3 new_direction = glm::vec3(camera_direction.x, camera_direction.y + CAMERA_ROTATION_STEP, camera_direction.z);
			frametech::Engine::getInstance()->m_camera.setDirection(new_direction);
		}
		break;
	case LEFT:
		{
			Log("LEFT key has been hit");
			const glm::vec3 camera_direction = frametech::Engine::getInstance()->m_camera.getDirection();
			const glm::vec3 new_direction = glm::vec3(camera_direction.x - CAMERA_ROTATION_STEP, camera_direction.y, camera_direction.z);
			frametech::Engine::getInstance()->m_camera.setDirection(new_direction);
		}
		break;
	case UP:
		{
			Log("UP key has been hit");
			const glm::vec3 camera_direction = frametech::Engine::getInstance()->m_camera.getDirection();
			const glm::vec3 new_direction = glm::vec3(camera_direction.x, camera_direction.y - CAMERA_ROTATION_STEP, camera_direction.z);
			frametech::Engine::getInstance()->m_camera.setDirection(new_direction);
		}
		break;
	default:    
		LogW("Unknown key with id %d"); 
		break;
	}
	m_keys.pop_front();
}

void frametech::inputs::Handler::addKeyEvent(const Key key) noexcept {
	std::scoped_lock<std::mutex> guard(m_lock);
	m_keys.push_back(key);
}
