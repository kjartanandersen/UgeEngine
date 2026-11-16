/**
 * @file UUID.h
 * @brief 64-bit unique identifiers.
 * @ingroup group_core
 */

#pragma once

#include <cstdint>
#include <functional>


namespace Uge
{

	/**
	 * @brief A random 64-bit identifier. `0` means none.
	 * @ingroup group_core
	 */
	class UUID
	{
	public:
		/** @brief The null identifier, `0`. */
		UUID() = default;

		/**
		 * @brief Creates a random identifier.
		 * @return A new identifier, never `0`.
		 */
		static UUID Generate();

		/**
		 * @brief Wraps an existing value.
		 * @param uuid The value.
		 */
		UUID(uint64_t uuid);
		/** @brief Copy constructor. */
		UUID(const UUID&) = default;

		/**
		 * @brief The raw value.
		 * @return The value.
		 */
		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID = 0;

	};

}

namespace std
{
	template<typename T> struct hash;

	/**
	 * @brief Lets Uge::UUID key unordered containers.
	 * @ingroup group_core
	 */
	template<>
	struct hash<Uge::UUID>
	{

		/**
		 * @brief Hashes an identifier.
		 * @param uuid Identifier to hash.
		 * @return Its value, which is already random.
		 */
		std::size_t operator()(const Uge::UUID& uuid) const
		{

			return (uint64_t)uuid;

		}

	};

}