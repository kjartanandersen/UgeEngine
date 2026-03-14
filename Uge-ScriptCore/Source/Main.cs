using System;

namespace Uge
{
    public class Main
    {

        public float FloatVar { get; set; }

        public Main()
        {

            Console.WriteLine("Main Constructor!");


        }

        public void PrintMessage()
        {
            Console.WriteLine("Hello World from C#!");
        }

        public void PrintCustomMessage(string msg)
        {
            Console.WriteLine($"C# Says: {msg}");

        }

        public void PrintInt(int num)
        {
            Console.WriteLine($"C# Says: {num}");

        }

        public void PrintInts(int num1, int num2)
        {
            Console.WriteLine($"C# Says: {num1} and {num2}");

        }

    }

}