VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   2430
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   2430
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "&Start Omni"
      Height          =   495
      Left            =   1080
      TabIndex        =   0
      Top             =   1200
      Width           =   2415
   End
   Begin VB.Label Label1 
      Caption         =   "ADO Omni Client"
      BeginProperty Font 
         Name            =   "Comic Sans MS"
         Size            =   15.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   960
      TabIndex        =   1
      Top             =   240
      Width           =   2655
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub Command1_Click()
    Dim conn As New ADODB.Connection
    Dim cmd As New ADODB.Command
    Dim rs As New ADODB.Recordset
    Dim str As String
    Dim rowNum As Long
   
'1. Specify a valid path to the data file and name
    str = "Provider=TheProvider.MSOmniProv.1;Data Source=..\..\Data\data.sxt"
'2. Open the connection
    conn.Open str
    Set cmd.ActiveConnection = conn

'3. Specify a valid table name
    cmd.CommandText = "Select * from OmniTable"
    rs.CursorLocation = adUseServer
'4. Open the Recordset
    rs.Open cmd, , adOpenStatic, adLockBatchOptimistic
  
'5. Test for scrolling MoveFirst, MoveNext, MoveLast, MovePrevious
    'MoveFirst, MoveNext,
    rs.MoveFirst
    While Not rs.EOF
        For i = 0 To rs.Fields.Count - 1
           Debug.Print rs.Fields(i).Name & ": " & rs.Fields(i).Value & ": "; rs.Fields(i).DefinedSize
        Next i
        rs.MoveNext
    Wend
    
    'Test for MovePrevious, and MoveLast
    rs.MoveLast
    While Not rs.BOF
        For i = 0 To rs.Fields.Count - 1
            Debug.Print rs.Fields(i).Name & ": " & rs.Fields(i).Value
        Next i
    rs.MovePrevious
    Wend
    
'6. Test for Immediate Update, Insert and Delete
    ' Deleting the Last Record
    Debug.Print "Deleting a Record\n"
    rs.MoveLast
    rs.Delete
    ' ADO Docs: If you delete the last remaining record in the Recordset object,
    '           the BOF and EOF properties may remain False until you attempt to
    '           reposition the current record.
    rs.MoveFirst 'MoveFirst sets the EOF and BOF value
    ' If there are no more records all Move operations are invalid
    ' Only AddNew can now be done
    'Append a new Record
    Debug.Print "Appending a Record\n"
    rs.AddNew
    rs.Fields(0).Value = "001"
    rs.Fields(1).Value = "Name"
    rs.Fields(2).Value = "Title"
    rs.Fields(3).Value = 8999
    rs.Update
    'View the appended record
    For i = 0 To rs.Fields.Count - 1
        Debug.Print rs.Fields(i).Name & ": " & rs.Fields(i).Value & ": "; rs.Fields(i).DefinedSize
    Next i
    
    'Update a record
    Debug.Print "Updating a Record\n"
    rs.MoveFirst
    rs.Fields(0).Value = "002"
    rs.Fields(1).Value = "Name"
    rs.Fields(2).Value = "Title"
    rs.Fields(3).Value = 898
    rs.Update
    'View the changed record
    For i = 0 To rs.Fields.Count - 1
        Debug.Print rs.Fields(i).Name & ": " & rs.Fields(i).Value & ": "; rs.Fields(i).DefinedSize
    Next i
    
   'Clean-up
    rs.Close
    conn.Close
End Sub

