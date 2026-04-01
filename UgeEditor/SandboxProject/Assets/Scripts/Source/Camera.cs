
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Source.Uge;
using Uge;

namespace Sandbox
{
    public class Camera : Entity
    {

        public float DistFromPlayer = 10.0f;
        private Entity Player;

        void OnCreate()
        {

            Player = FindEntityByName("Player");

        }

        void OnUpdate(float ts)
        {

            if (Player != null)
            {

                Translation = new Vector3(Player.Translation.XY, DistFromPlayer);
            }
            else
            {
                Console.WriteLine("Player is NULL!");
            }


                float speed = 1.0f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.Up))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.Down))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.Left))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.Right))
                velocity.X = 1.0f;

            velocity *= speed;

            Vector3 translation = Translation;
            translation += velocity * ts;
            Translation = translation;
        }

    }
}
