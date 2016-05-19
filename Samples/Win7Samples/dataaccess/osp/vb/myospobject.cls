VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0   'False
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
END
Attribute VB_Name = "MyOSPObject"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Implements OLEDBSimpleProvider
Dim MyOSPArray()
Dim RowCount As Integer
Dim ColCount As Integer
Dim colListeners As New Collection
Dim ospl As OLEDBSimpleProviderListener
Public FilePath As String

Public Sub LoadData()
    Dim GetLine As Variant
    Dim Spot As Integer
    
    On Error GoTo ErrorTrap
    Open FilePath For Input Lock Read Write As #1
    Position = 1
    Row = 0
    Line Input #1, GetLine
    Spot = InStr(1, GetLine, ";")
    RowCount = val(Left$(GetLine, Spot))
    ColCount = val(Right$(GetLine, Len(GetLine) - Spot))
    ReDim MyOSPArray(RowCount + 1, ColCount + 1)
    While Not EOF(1)
        Line Input #1, GetLine
        Col = 1
        Spot = InStr(1, GetLine, ";")
        While Spot <> 0
            MyOSPArray(Row, Col) = Left$(GetLine, Spot - 1)
            Col = Col + 1
            GetLine = Right$(GetLine, Len(GetLine) - Spot)
            Spot = InStr(1, GetLine, ";")
        Wend
        If Len(GetLine) <> 0 Then
               MyOSPArray(Row, Col) = GetLine
        End If
        Row = Row + 1
    Wend
    Close #1
    Exit Sub
ErrorTrap:
    Err.Raise (E_FAIL)
End Sub

Public Sub SaveData()
    Dim GetLine As Variant
    Dim Spot As Integer
    
    On Error GoTo ErrorTrap
    Open FilePath For Output Lock Read Write As #1
    Print #1, RowCount & ";" & ColCount
    
    For iRow = 0 To RowCount
        For iCol = 1 To ColCount
            PutLine = PutLine & MyOSPArray(iRow, iCol) & ";"
        Next iCol
        Print #1, PutLine
        PutLine = ""
    Next iRow
    Close #1
    Exit Sub
ErrorTrap:
    Err.Raise (E_FAIL)
End Sub


Private Sub Class_Terminate()
    SaveData
End Sub

Private Sub OLEDBSimpleProvider_addOLEDBSimpleProviderListener(ByVal pospIListener As OLEDBSimpleProviderListener)
    'Add a listener:
    If Not (pospIListener Is Nothing) Then
        Set ospl = pospIListener
        colListeners.Add ospl
    End If
    'Debug code to see listener count
    'MyOSPArray(1, 1) = "LCnt = " & colListeners.Count
End Sub

Private Function OLEDBSimpleProvider_deleteRows(ByVal iRow As Long, ByVal cRows As Long) As Long
    Dim TempArray()
    Dim listener As OLEDBSimpleProviderListener
    Dim v As Variant
    
    'Make sure iRow is in the correct range:
    If iRow < 1 Or iRow > RowCount Then
        Err.Raise (E_FAIL)
    End If
        
    'Set cRows to the actual number which can be deleted
    If iRow + cRows > RowCount + 1 Then cRows = RowCount - iRow + 1
        
    'Establish a Temporary Array
    cNewRows = RowCount - cRows
    ReDim TempArray(cNewRows + 1, ColCount + 1)
    
    'Notify each listener:
    For Each v In colListeners
        Set listener = v
        listener.aboutToDeleteRows iRow, cRows
    Next
        
    'Copy over the first rows which are not being deleted
    For Row = 0 To iRow - 1
        For Col = 0 To ColCount
            TempArray(Row, Col) = MyOSPArray(Row, Col)
        Next Col
    Next Row
        
    'Copy the last rows which are not being deleted
    For Row = iRow + cRows To RowCount
        For Col = 0 To ColCount
            TempArray(Row - cRows, Col) = MyOSPArray(Row, Col)
        Next Col
    Next Row
       
    'Re-allocate the array to copy into it
    ReDim MyOSPArray(cNewRows + 1, ColCount + 1)
        
    'Set the real row count back in
    RowCount = cNewRows
        
    'Copy over the rows
    For Row = 0 To cNewRows
        For Col = 0 To ColCount
            MyOSPArray(Row, Col) = TempArray(Row, Col)
        Next Col
    Next Row
        
    'Clear the temporary memory
    ReDim TempArray(0)
        
    'Notify each listener
    For Each v In colListeners
        Set listener = v
        listener.deletedRows iRow, cRows
    Next
        
    'Return number of deleted rows
    OLEDBSimpleProvider_deleteRows = cRows
End Function

Private Function OLEDBSimpleProvider_find(ByVal iRowStart As Long, ByVal iColumn _
As Long, ByVal val As Variant, ByVal findFlags As OSPFIND, _
ByVal compType As OSPCOMP) As Long

    Dim RowStart, RowStop
    If (findFlags And (OSPFIND_UP Or OSPFIND_UPCASESENSITIVE)) <> 0 Then
        RowStart = RowCount + 1
        RowStop = 0
        StepValue = -1
    Else
        RowStart = 0
        RowStop = RowCount + 1
        StepValue = 1
    End If
    
    If (findFlags And (OSPFIND_CASESENSITIVE Or OSPFIND_UPCASESENSITIVE)) <> 0 Then
        CaseSens = 1   'Use a Text Compare not Case Sensensitve
    Else
        CaseSens = 0   'Not Case Sensensitve use Binary Compare
    End If
            
    If VarType(val) = vbString Then
        StringComp = True
    Else
        StringComp = Flase
    End If
    
    iAnswerRow = -1
    For iRow = RowStart To RowStop Step StepValue
        If StringComp Then
            CompResult = StrComp(MyOSPArray(iRow, iColumn), val, CaseSens)
            Select Case (compType)
                Case OSPCOMP_DEFAULT, OSPCOMP_EQ:
                    If CompResult = 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_GE
                    If CompResult >= 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_GT
                    If CompResult > 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_LE
                    If CompResult <= 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_LT
                    If CompResult < 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_NE
                    If CompResult <> 0 Then
                        iAnswerRow = iRow
                        Exit For
                    End If
            End Select
        Else
            Select Case (compType)
                Case OSPCOMP_DEFAULT, OSPCOMP_EQ:
                    If MyOSPArray(iRow, iColumn) = val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_GE
                    If MyOSPArray(iRow, iColumn) >= val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_GT
                     If MyOSPArray(iRow, iColumn) > val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_LE
                    If MyOSPArray(iRow, iColumn) <= val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_LT
                    If MyOSPArray(iRow, iColumn) < val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
                Case OSPCOMP_NE
                    If MyOSPArray(iRow, iColumn) <> val Then
                        iAnswerRow = iRow
                        Exit For
                    End If
            End Select
        End If
    Next iRow
    OLEDBSimpleProvider_find = iAnswerRow
    
End Function

Private Function OLEDBSimpleProvider_getColumnCount() As Long
    OLEDBSimpleProvider_getColumnCount = ColCount
End Function

Private Function OLEDBSimpleProvider_getEstimatedRows() As Long
    OLEDBSimpleProvider_getEstimatedRows = RowCount
End Function

Private Function OLEDBSimpleProvider_getLocale() As String
    OLEDBSimpleProvider_getLocale = ""
End Function

Private Function OLEDBSimpleProvider_getRowCount() As Long
    OLEDBSimpleProvider_getRowCount = RowCount
End Function

Private Function OLEDBSimpleProvider_getRWStatus(ByVal iRow As Long, ByVal iColumn As Long) As OSPRW
    If iColumn = 1 Then
        OLEDBSimpleProvider_getRWStatus = OSPRW_READONLY
    Else
        OLEDBSimpleProvider_getRWStatus = OSPRW_READWRITE
    End If
End Function

Private Function OLEDBSimpleProvider_getVariant(ByVal iRow As Long, ByVal iColumn As Long, ByVal format As OSPFORMAT) As Variant
     OLEDBSimpleProvider_getVariant = MyOSPArray(iRow, iColumn)
End Function

Private Function OLEDBSimpleProvider_insertRows(ByVal iRow As Long, ByVal cRows As Long) As Long
    Dim TempArray()
    Dim listener As OLEDBSimpleProviderListener
    Dim v As Variant
    
    'Establish a Temporary Array
    cNewRows = RowCount + cRows
    ReDim TempArray(cNewRows + 1, ColCount + 1)
    
    'If inserting past the end of the array, insert at the end of the array
    If iRow > RowCount Then iRow = RowCount + 1
    
    'Notify listener
    For Each v In colListeners
        Set listener = v
        listener.aboutToInsertRows iRow, cRows
    Next
        
    'Copy over the existing rows
    For Row = 0 To iRow
        For Col = 0 To ColCount
            TempArray(Row, Col) = MyOSPArray(Row, Col)
        Next Col
    Next Row
    
    'Copy the last rows which follow the inserted rows
    For Row = iRow + 1 + cRows To cNewRows
        For Col = 0 To ColCount
            TempArray(Row, Col) = MyOSPArray(Row - cRows, Col)
        Next Col
    Next Row
        
    'Re-allocate the array to copy into it
    ReDim MyOSPArray(cNewRows + 1, ColCount + 1)
    
    'Copy over the rows
    For Row = 0 To cNewRows
        For Col = 0 To ColCount
            MyOSPArray(Row, Col) = TempArray(Row, Col)
        Next Col
    Next Row
        
    'Clear the temporary memory
    ReDim TempArray(0)
    
    'Set the real row count back in
    RowCount = cNewRows
    
    'Notify listener
    For Each v In colListeners
        Set listener = v
        listener.insertedRows iRow, cRows
    Next
    
    'Return number of inserted rows
    OLEDBSimpleProvider_insertRows = cRows
End Function

Private Function OLEDBSimpleProvider_isAsync() As Long
    OLEDBSimpleProvider_isAsync = False
End Function

Private Sub OLEDBSimpleProvider_removeOLEDBSimpleProviderListener(ByVal pospIListener As OLEDBSimpleProviderListener)
    'Remove the listener:
    For i = 1 To colListeners.Count
        If colListeners(i) Is pospIListener Then
            colListeners.Remove i
        End If
    Next
End Sub

Private Sub OLEDBSimpleProvider_setVariant(ByVal iRow As Long, ByVal iColumn As Long, ByVal format As OSPFORMAT, ByVal Var As Variant)
    Dim listener As OLEDBSimpleProviderListener
    Dim v As Variant
    For Each v In colListeners
        Set listener = v
        listener.aboutToChangeCell iRow, iColumn    'Pre-notification
    Next
    MyOSPArray(iRow, iColumn) = Var
    For Each v In colListeners
        Set listener = v
        listener.cellChanged iRow, iColumn          'Post-notification
    Next
End Sub

Private Sub OLEDBSimpleProvider_stopTransfer()
    'Do nothing because we are already populated
End Sub
