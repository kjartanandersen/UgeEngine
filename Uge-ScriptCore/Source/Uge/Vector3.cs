/**
 * @file Vector3.cs
 * @brief Three-component vector used by scripts.
 * @ingroup group_scripting
 */


namespace Uge
{
    /**
     * @brief A three-component float vector.
     * @ingroup group_scripting
     *
     * Laid out to match `glm::vec3`, so positions cross the internal-call boundary without
     * conversion. This is the type entity transforms are read and written as.
     */
    public struct Vector3
    {
        public float X; ///< X component.
        public float Y; ///< Y component.
        public float Z; ///< Z component.

        

        /** @brief The X and Y components as a Uge.Vector2. */
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
         * @brief Constructs a vector from a 2D vector and a z component.
         * @param xy X and Y components.
         * @param z Z component.
         */
        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        /**
         * @brief Constructs a vector with every component set to the same value.
         * @param scalar Value for every component.
         */
        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        /**
         * @brief Constructs a vector from its components.
         * @param x X component.
         * @param y Y component.
         * @param z Z component.
         */
        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        /**
         * @brief Component-wise addition.
         * @param a Left operand.
         * @param b Right operand.
         * @return The sum.
         */
        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        /**
         * @brief Scalar multiplication.
         * @param vector Vector to scale.
         * @param scalar Factor to scale by.
         * @return The scaled vector.
         */
        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

    }
}
