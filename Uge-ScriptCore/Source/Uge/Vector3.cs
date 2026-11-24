/**
 * @file Vector3.cs
 * @brief A 3D vector for scripts.
 * @ingroup group_scripting
 */


namespace Uge
{
    /**
     * @brief A 3D float vector, laid out like `glm::vec3`.
     * @ingroup group_scripting
     */
    public struct Vector3
    {
        public float X; ///< X component.
        public float Y; ///< Y component.
        public float Z; ///< Z component.

        

        /** @brief X and Y, as a Uge.Vector2. */
        public Vector2 XY
        {
            get
            {
                return new Vector2(X, Y);
            }

            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        /** @brief The zero vector. */
        public static Vector3 Zero => new Vector3(0.0f);

        /**
         * @brief A vector from a 2D vector and a z.
         * @param xy X and Y.
         * @param z Z.
         */
        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        /**
         * @brief A vector with all components equal.
         * @param scalar The value.
         */
        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        /**
         * @brief A vector from its components.
         * @param x X.
         * @param y Y.
         * @param z Z.
         */
        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        /**
         * @brief Adds two vectors.
         * @param a A vector.
         * @param b A vector.
         * @return The sum.
         */
        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        /**
         * @brief Scales a vector.
         * @param vector The vector.
         * @param scalar The factor.
         * @return The scaled vector.
         */
        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

    }
}
