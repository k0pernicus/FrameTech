//
//  transform.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _transform_hpp
#define _transform_hpp

#include "common.hpp"
#include <map>
#include <string>
#include "debug_tools.h"

// For the transformation / UBOs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES  // Force GLM to use the alignment requirements - does not work with nested structures !
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace frametech 
{
	namespace graphics 
	{
		/// @brief Hardcoded transformations for demonstration purposes
		enum Transformation {
			Constant,
			Rotate,
			RotateAndScale,
		};

		/// @brief Update data according to: the transformation and associated constant values
		/// WARNING: This function **does not** flip the Y-axis (for OpenGL compatibility)
		/// @param targeted_ubo The Transformation to get the data from
		/// @param delta_time The update time
		/// @param window_height Height of the swapchain (fixed to the window)
		/// @param window_width Width of the swapchain (fixed to the window)
		/// @return A ModelViewProjection structure that contains all the updated data
		ModelViewProjection computeTransform(
			const Transformation transformation_id,
			const float delta_time, 
			const uint32_t window_height, 
			const uint32_t window_width
		) noexcept;
	} // namespace graphics
} // namespace frametech

#endif // _transform_hpp
