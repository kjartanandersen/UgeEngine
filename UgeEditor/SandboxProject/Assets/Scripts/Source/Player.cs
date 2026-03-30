
using Source.Uge;
using System;
using Uge;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        public float Speed1 = 1.0f;
        public float Time;
        

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");

            m_Transform = GetComponent<TransformComponent>();
        }

        void OnUpdate(float ts)
        {
            Time += ts;
            // Console.WriteLine($"Player.OnUpdate: {ts}");

            float speed = Speed1;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 10.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -10.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -10.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 10.0f;

            velocity *= speed;

            Vector3 translation = m_Transform.Translation;
            translation += velocity * ts;
            m_Transform.Translation = translation;
        }

    }
}
