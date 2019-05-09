using System;

namespace CSharpConsoleApp
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            var messageHolder = new WinRTComponent.MessageHolder();
            Console.WriteLine(messageHolder.Message);
        }
    }
}
