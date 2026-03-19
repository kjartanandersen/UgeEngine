using System;
using System.Runtime.CompilerServices;

namespace Uge
{

    public static class InternalCalls_Old
    {


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CppFunction();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string text, int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLogVector3(ref Vector3 parameter, out Vector3 outResult);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NativeLogDot(ref Vector3 vector1, ref Vector3 vector2);
    }
    public class Main_Old
    {

        

        public float FloatVar { get; set; }

        public Main_Old()
        {

            Console.WriteLine("Main Constructor!");

            InternalCalls_Old.CppFunction();
            Log("Native Logging", 1);

            Vector3 pos = new Vector3(5, 2, 1);
            Vector3 result = Log(pos);
            Console.WriteLine($"Calling NativeLogVec3 from C#: {result.X}, {result.Y}, {result.Z}");


            Vector3 vec1 = new Vector3(2, 3.35f, 7);
            Vector3 vec2 = new Vector3(1, 0, 0.33f);
            Console.WriteLine($"Calling NativeLogDot from C#: {Dot(vec1, vec2)}");



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

        private void Log(string text, int parameter)
        {
            InternalCalls_Old.NativeLog(text, parameter);
        }

        private Vector3 Log(Vector3 parameter)
        {
            InternalCalls_Old.NativeLogVector3(ref parameter, out Vector3 result);

            return result;
        }

        private float Dot(Vector3 vector1, Vector3 vector2)
        {
           
            return InternalCalls_Old.NativeLogDot(ref vector1, ref vector2);
        }

        

    }

}