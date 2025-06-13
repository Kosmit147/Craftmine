/**
 * @file pch.hpp
 * @brief Precompiled header file for commonly used includes, type aliases, and forward declarations.
 */

#pragma once

#include <zenith.hpp>
#include <mdspan>

/**
 * @brief Type alias for 8-bit unsigned integer.
 */
using u8 = std::uint8_t;
/**
 * @brief Type alias for 8-bit signed integer.
 */
using i8 = std::int8_t;
/**
 * @brief Type alias for 16-bit unsigned integer.
 */
using u16 = std::uint16_t;
/**
 * @brief Type alias for 16-bit signed integer.
 */
using i16 = std::int16_t;
/**
 * @brief Type alias for 32-bit unsigned integer.
 */
using u32 = std::uint32_t;
/**
 * @brief Type alias for 32-bit signed integer.
 */
using i32 = std::int32_t;
/**
 * @brief Type alias for 64-bit unsigned integer.
 */
using u64 = std::uint64_t;
/**
 * @brief Type alias for 64-bit signed integer.
 */
using i64 = std::int64_t;
/**
 * @brief Type alias for signed integer with pointer size.
 */
using isize = std::intmax_t;
/**
 * @brief Type alias for unsigned integer with pointer size.
 */
using usize = std::uintmax_t;

/**
 * @brief Type alias for zth::Error.
 */
using zth::Error;
/**
 * @brief Type alias for zth::nil (null/empty value).
 */
using zth::nil;
/**
 * @brief Type alias for zth::Optional.
 */
using zth::Optional;
/**
 * @brief Type alias for zth::Reference.
 */
using zth::Reference;
/**
 * @brief Type alias for zth::Result.
 */
using zth::Result;
