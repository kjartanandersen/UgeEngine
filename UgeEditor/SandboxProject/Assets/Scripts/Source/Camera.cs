
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

            float camDistSpeed = 1.0f;

            DistFromPlayer += camDistSpeed * ts;
            
        }

    }
}
