//
//  inputs.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _inputs_hpp
#define _inputs_hpp

#include <deque>
#include <mutex>

namespace frametech {
	namespace inputs {
		enum Key {
			ALT_UP_COMBINED,
			ALT_DOWN_COMBINED,
			ALT_LEFT_COMBINED,
			ALT_RIGHT_COMBINED,
			UP,
			DOWN,
			LEFT,
			RIGHT,
		};

		class Handler {
		public:
			void pollEvent(bool blank = false) noexcept;
			void addKeyEvent(const Key key) noexcept;

		private:
			std::deque<Key> m_keys;
			std::mutex m_lock;
		};
	}
}

#endif // _inputs_hpp
