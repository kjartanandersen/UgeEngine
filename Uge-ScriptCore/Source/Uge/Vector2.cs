/**
 * @file Vector2.cs
 * @brief A 2D vector for scripts.
 * @ingroup group_scripting
 */


namespace Uge
{
    /**
     * @brief A 2D float vector, laid out like `glm::vec2`.
     * @ingroup group_scripting
     */
    public struct Vector2
    {
        public float X; ///< X component.
        public float Y; ///< Y component.



        /** @brief The zero vector. */
        public static Vector2 Zero => new Vector2(0.0f);

        /**
         * @brief A vector with both components equal.
         * @param scalar The value.
         */
        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        /**
         * @brief A vector from its components.
         * @param x X.
         * @param y Y.
         */
        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        /**
         * @brief Adds two vectors.
         * @param a A vector.
         * @param b A vector.
         * @return The sum.
         */
        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X + b.X, a.Y + b.Y);
        }

        /**
         * @brief Scales a vector.
         * @param vector The vector.
         * @param scalar The factor.
         * @return The scaled vector.
         */
        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.X * scalar, vector.Y * scalar);
        }

    }
}
