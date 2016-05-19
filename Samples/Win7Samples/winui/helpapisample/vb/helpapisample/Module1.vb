Imports HelpPane

Module Module1
    Public objHelpPane As HelpPane.HxHelpPane

    Sub Main()
        Dim strIn As String
        Dim strSrch As String
        Dim strTopicDisp As String
        Dim strTopicToc As String

        Console.WriteLine("Please enter a value.")
        Console.WriteLine("1: DisplaySearchResults")
        Console.WriteLine("2: DisplayTask")
        Console.WriteLine("3: DisplayContents (TOC root)")
        Console.WriteLine("4: DisplayContents (specific task)")
        Console.Write(">")

        strIn = Console.ReadLine
        strSrch = ""
        strTopicDisp = ""
        strTopicToc = ""

        Select Case strIn
            Case "1"
                Try
                    objHelpPane = New HelpPane.HxHelpPane
                    Console.Write("Please enter a search keyword: ")
                    strSrch = Console.ReadLine
                    '(1) Function: Display search results.
                    'Parameter: any word or words that exist in registered help.
                    objHelpPane.DisplaySearchResults(strSrch)

                Catch ex1 As Exception
                    Console.WriteLine(ex1)
                End Try

            Case "2"
                Try
                    objHelpPane = New HelpPane.HxHelpPane
                    Console.Write("Please enter a topic ID: ")
                    '(2) Function: Display a registered topic under Windows namespace.
                    'Parameter: URL with valid help protocol and registered topic ID.
                    'For example: mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
                    strTopicDisp = Console.ReadLine
                    objHelpPane.DisplayTask(strTopicDisp)

                Catch ex2 As Exception
                    Console.WriteLine(ex2)
                End Try
            Case "3"
                Try
                    objHelpPane = New HelpPane.HxHelpPane
                    Console.Write("Displaying the TOC root.")
                    '(3) Function: Display the root TOC (table of contents).
                    'Parameter: NULL or empty string.
                    objHelpPane.DisplayContents("")

                Catch ex3 As Exception
                    Console.WriteLine(ex3)
                End Try
            Case "4"
                Try
                    objHelpPane = New HelpPane.HxHelpPane
                    Console.Write("Please enter a topic ID: ")
                    strTopicToc = Console.ReadLine
                    '(4)Function: Display a TOC (table of contents) page.
                    'Parameter: URL with valid help protocol and registered topic ID.
                    'For example: mshelp://Windows/?id=004630d0-9241-4842-9d3f-2a0c5825ef14
                    objHelpPane.DisplayContents(strTopicToc)

                Catch ex4 As Exception
                    Console.WriteLine(ex4)
                End Try

            Case Else
                Console.WriteLine("Please enter a valid value (1-4).")
        End Select
    End Sub

End Module
