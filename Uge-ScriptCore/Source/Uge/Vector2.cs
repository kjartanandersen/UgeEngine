/**
 * @file Vector2.cs
 * @brief Two-component vector used by scripts.
 * @ingroup group_scripting
 */


namespace Uge
{
    /**
     * @brief A two-component float vector.
     * @ingroup group_scripting
     *
     * Laid out to match `glm::vec2` so it can be marshalled across the internal-call
     * boundary without conversion.
     */
    public struct Vector2
    {
        public float X; ///< X component.
        public float Y; ///< Y component.



        /** @brief The zero vector. */
        public static Vector2 Zero => new Vector2(0.0f);

        /**
         * @brief Constructs a vector with both components set to the same value.
         * @param scalar Value for every component.
         */
        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        /**
         * @brief Constructs a vector from its components.
         * @param x X component.
         * @param y Y component.
         */
        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        /**
         * @brief Component-wise addition.
         * @param a Left operand.
         * @param b Right operand.
         * @return The sum.
         */
        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X + b.X, a.Y + b.Y);
        }

        /**
         * @brief Scalar multiplication.
         * @param vector Vector to scale.
         * @param scalar Factor to scale by.
         * @return The scaled vector.
         */
        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.X * scalar, vector.Y * scalar);
        }

    }
}
