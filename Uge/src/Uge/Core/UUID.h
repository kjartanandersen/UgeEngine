/**
 * @file UUID.h
 * @brief 64-bit unique identifier used for entities and asset handles.
 * @ingroup group_core
 */

#pragma once

#include <xhash>


namespace Uge
{

	/**
	 * @brief A randomly generated 64-bit identifier.
	 * @ingroup group_core
	 *
	 * Stable across serialization, unlike an `entt::entity`, which is only valid within a
	 * single registry. Two things depend on that stability: Uge::IDComponent, which gives
	 * every entity an identity that survives a save/load round-trip, and Uge::AssetHandle,
	 * which is an alias for this type.
	 *
	 * Default construction draws from a uniformly distributed 64-bit random engine.
	 * Collisions are not checked for; at this width they are not a practical concern.
	 *
	 * Specializes `std::hash`, so a UUID can key an unordered container directly.
	 */
	class UUID
	{
	public:
		/** @brief Generates a new random identifier. */
		UUID();
		/**
		 * @brief Wraps an existing identifier value.
		 * @param uuid Raw value, typically read back from a serialized file.
		 */
		UUID(uint64_t uuid);
		/** @brief Copy constructor. */
		UUID(const UUID&) = default;

		/**
		 * @brief Implicit conversion to the raw 64-bit value.
		 * @return The underlying identifier.
		 */
		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;

	};

}

namespace std
{
	template<typename T> struct hash;

	/**
	 * @brief `std::hash` specialization letting Uge::UUID key unordered containers.
	 * @ingroup group_core
	 */
	template<>
	struct hash<Uge::UUID>
	{

		/**
		 * @brief Hashes an identifier.
		 * @param uuid Identifier to hash.
		 * @return The identifier value itself, which is already uniformly distributed.
		 */
		std::size_t operator()(const Uge::UUID& uuid) const
		{

			return (uint64_t)uuid;

		}

	};

}