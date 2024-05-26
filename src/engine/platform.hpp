//
//  platform.hpp
//  engine
//
//  Created by Antonin on 03/10/2023.
//

#ifndef platform_h
#define platform_h

#if defined(__x86_64__) || defined(_WIN64) || defined(__aarch64__)
// Built-in types
#include <cstdint>
typedef std::uint8_t        u8;
typedef std::uint16_t       u16;
typedef std::uint32_t       u32;
typedef std::uint64_t       u64;
typedef unsigned long long  u64;
typedef std::int8_t         i8;
typedef std::int16_t        i16;
typedef std::int32_t        i32;
typedef std::int64_t        i64;
typedef signed long long    i64;
typedef float               f32;
typedef double              f64;
#else
#warning ("Warning: unsupported platform, does not know how to manage built-in types!")
#warning ("FrameTech does not support 32 bits architecture")
#endif

#endif /* platform_h */
