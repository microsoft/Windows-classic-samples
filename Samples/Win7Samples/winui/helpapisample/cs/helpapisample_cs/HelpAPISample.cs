using System;
using HelpPane;

namespace HelpAPISample
{
    class HelpAPISample
    {
        

        static void Main(string[] args)
        {
            string strIn;
            string strSrch;
            string strTopicDisp;
            string strTopicToc;

            Console.WriteLine("Please enter a value");
            Console.WriteLine("1: DisplaySearchResults");
            Console.WriteLine("2: DisplayTask");
            Console.WriteLine("3: DisplayContents (TOC root)");
            Console.WriteLine("4: DisplayContents (specific task)");
            Console.Write(">");
            strIn = Console.ReadLine();
            if (strIn == "1")
                try
                {
                    HelpPane.HxHelpPane pane = new HelpPane.HxHelpPaneClass();
                    Console.Write("Please enter a search keyword: ");
                    strSrch = Console.ReadLine();
                    // (1) Function: Display search results
                    // Parameter: any word or words that exist in registered help contents
                    pane.DisplaySearchResults(strSrch);
                }
                catch (Exception ex1)
                {
                    Console.Write(ex1);
                }

            else if (strIn == "2")
                try
                {
                    HelpPane.HxHelpPane pane = new HelpPane.HxHelpPaneClass();
                    Console.Write("Please enter a topic ID: ");
                    strTopicDisp = Console.ReadLine();
                    // (2) Function: Display a registered topic under Windows namespace
                    // Parameter: url with valid help protocol and registered topic id
                    // such as: mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
                    pane.DisplayTask(strTopicDisp);
                }
                catch (Exception ex2)
                {
                    Console.Write(ex2);
                }

            else if (strIn == "3")
                try
                {
                    HelpPane.HxHelpPane pane = new HelpPane.HxHelpPaneClass();
                    Console.WriteLine("Displaying the TOC root.");
                    // (3) Function: Display the root TOC (Table of content)
                    // Parameter: NULL or empty string
                    pane.DisplayContents(null);
                }
                catch (Exception ex3)
                {
                    Console.Write(ex3);
                }

            else if (strIn == "4")
                try
                {
                    HelpPane.HxHelpPane pane = new HelpPane.HxHelpPaneClass();
                    Console.Write("Please enter a topic ID: ");
                    strTopicToc = Console.ReadLine();
                    // (3-1) Function: Display a TOC (Table of content) page
                    // Parameter: url with valid help protocol and authoried toc id
                    // such as mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
                    pane.DisplayContents(strTopicToc);
                }
                catch (Exception ex4)
                {
                    Console.Write(ex4);
                }
            else
            {
                Console.WriteLine("Please enter a valid value (1-4).");
            }
        }
    }
}
