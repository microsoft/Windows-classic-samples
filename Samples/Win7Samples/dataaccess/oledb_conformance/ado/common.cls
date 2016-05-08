VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Common"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Attribute VB_Ext_KEY = "SavedWithClassBuilder" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
'local variable(s) to hold property value(s)
Private mvarlogfile As String 'local copy
'local variable(s) to hold property value(s)
Private mvarLogNum As Variant 'local copy
'local variable(s) to hold property value(s)
Private mvarIgnoreErrors As Boolean 'local copy
Public Property Let IgnoreErrors(ByVal vData As Boolean)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.IgnoreErrors = 5
    mvarIgnoreErrors = vData
End Property

Public Property Get IgnoreErrors() As Boolean
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.IgnoreErrors
    IgnoreErrors = mvarIgnoreErrors
End Property

Public Property Let LogNum(ByVal vData As Variant)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.LogNum = 5
    mvarLogNum = vData
End Property

Public Property Set LogNum(ByVal vData As Object)
'used when assigning an Object to the property, on the left side of a Set statement.
'Syntax: Set x.LogNum = Form1
    Set mvarLogNum = vData
End Property

Public Property Get LogNum() As Variant
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.LogNum
    If IsObject(mvarLogNum) Then
        Set LogNum = mvarLogNum
    Else
        LogNum = mvarLogNum
    End If
End Property



Public Property Let logfile(ByVal vData As String)
'used when assigning a value to the property, on the left side of an assignment.
'Syntax: X.logfile = 5
    mvarlogfile = vData
End Property


Public Property Get logfile() As String
'used when retrieving value of a property, on the right side of an assignment.
'Syntax: Debug.Print X.logfile
    logfile = mvarlogfile
End Property

Public Function RSClose(myrs2 As Variant) As Variant
    'RSClose = myrs2.Close
End Function

Public Function RSOpen(myrs1 As Variant, g_rsstr As String, g_connstr As String, curtype As Variant, locktype As Variant) As Variant
    RSOpen = myrs1.Open(g_rsstr, g_connstr, curtype, locktype)
End Function

Public Function TestCheck(ByVal bTestPassed As Boolean, ByVal sInString As String, ByVal FileNum As Integer)

    Dim sLogString

    TestCheck = bTestPassed

    ' Output Test pass/fail
    If (bTestPassed = False) Then
        sLogString = sInString & "FAILED"
    Else
        sLogString = sInString & "PASSED"
    End If

    codelib.LogText sLogString

End Function
Public Function LogText(ByVal sInString As String)
    'Print #nFileNum, sInString
    LogNum = FreeFile   ' Get unused file number.
    Open logfile For Append Shared As #LogNum
    Print #LogNum, sInString
    Close #LogNum
End Function
Public Function ErrorHandler(lError As ModuleBase.IError, ByVal sInString As String, ByVal nExpError As Variant) As Boolean
ErrorHandler = True
If (Err.Number <> nExpError) Then
'    If IgnoreErrors Then
'        If Err.Number <> E_NOTIMPL And Err.Number <> adErrFeatureNotAvailable Then
'            sOutput = sLogString + " *** FAILED *** "
'            LogText sOutput
'            If nExpError = 0 Then
'                sOutput = " NO ERROR EXPECTED"
'            Else
'                sOutput = "    EXPECTED ERROR = " + CStr(nExpError)
'            End If
'            LogText sOutput
'            sOutput = "      ACTUAL ERROR = " + CStr(Err.Number)
'            LogText sOutput
'            sOutput = "             DESC  = " + Err.Description
'            LogText sOutput
'            ErrorHandler = False
'        End If
'    Else
        sOutput = sLogString + " *** FAILED *** "
        lError.Transmit sOutput + Chr(10)
        If nExpError = 0 Then
            sOutput = " NO ERROR EXPECTED"
        Else
            sOutput = "    EXPECTED ERROR = " + CStr(nExpError)
        End If
        lError.Transmit Chr(10) + sOutput
        sOutput = "      ACTUAL ERROR = " + CStr(Err.Number)
'MsgBox sOutput + chr(10) + "DESC = " + Err.Description
        lError.Transmit Chr(10) + sOutput
        sOutput = "             DESC = " + Err.Description
        lError.Transmit Chr(10) + sOutput
        sOutput = "             Source = " + Err.Source
        lError.Transmit Chr(10) + sOutput + Chr(10) + Chr(10)
        ErrorHandler = False
'    End If
End If
' Clear err regardless of whether expected or not
Err.Clear
End Function
Public Function genvalue(ByVal fld As ADODB.Field) As Variant
    
    Dim vData
    Randomize

    vData = Null

    'in case we can't have null
    If (fld.Attributes And adFldIsNullable) Then
        fNullBuff = 0
    Else
        fNullBuff = 1
    End If

    'Make the random variant here.
    Select Case fld.Type
        Case adBoolean
            vData = CBool(Int(2 * Rnd) - 1)
        Case adTinyInt, adSmallInt, adInteger, adBigInt
            vData = Fix(MaxVal(fld.Type) * (1 - (Rnd * 2)))
        Case adUnsignedTinyInt, adUnsignedSmallInt, adUnsignedInt, adUnsignedBigInt
            vData = Int(MaxVal(fld.Type) * Rnd)
        Case adDouble, adSingle, adCurrency
            vData = MaxVal(fld.Type) * (1 - (Rnd * 2))
        Case adDecimal, adNumeric
            vData = Fix(MaxVal(fld.Type) * (1 - (Rnd * 2)))
        Case adBinary, adVarBinary, adLongVarBinary
            'If the defined size > 255, it indicates this is a variable length blob field.
            'Since there are no text conversions with binary data, each char takes up 2 bytes (unicode)
            'Therefore, we need to restrict the string length to 1/2 the defined size.
            If (fld.DefinedSize > 255) Then
                vData = FetchRandString(Int((128 - fNullBuff) * Rnd) + fNullBuff)                           'FetchRandString(127)
            Else
                vData = FetchRandString(Int((Int(fld.DefinedSize \ 2) - fNullBuff + 1) * Rnd) + fNullBuff)  'FetchRandString(Int(fld.DefinedSize \ 2))
            End If
        Case adChar, adWChar, adVarChar, adVarWChar, adVariant, adLongVarChar, adLongVarWChar, adBSTR, adVariant
            'If the defined size > 255, it indicates this is a variable length text field.
            'Restrict the string length to 255 characters.
            If (fld.DefinedSize > 255) Then
                vData = FetchRandString(Int((256 - fNullBuff) * Rnd) + fNullBuff)                           'FetchRandString(255)
            Else
                vData = FetchRandString(Int((fld.DefinedSize - fNullBuff + 1) * Rnd) + fNullBuff)           'FetchRandString(fld.DefinedSize)
            End If
        Case adDate, adDBDate, adDBTimeStamp
            vData = 65536 * Rnd
        Case adDBTime
            vData = Time
        Case adGUID
            stAdd$ = "{"
            For i% = 0 To 31
                    If (Int(2 * Rnd) = 0) Then
                            stAdd$ = stAdd$ & Chr$(Int(5 * Rnd) + Asc("A"))
                    Else
                            stAdd$ = stAdd$ & Chr$(Int(9 * Rnd) + Asc("0"))
                    End If
                    If i% = 7 Or i% = 11 Or i% = 15 Or i% = 19 Then stAdd$ = stAdd$ & "-"
            Next i%
            stAdd$ = stAdd$ & "}"
            vData = stAdd$
        Case Else
            vData = Null
    End Select

    genvalue = vData
    
End Function
Public Function MaxVal(nType) As Variant

    Select Case nType
        Case adBigInt: MaxVal = 2147483647
        Case adCurrency: MaxVal = 214748.3647
        Case adDecimal: MaxVal = 922337203685477#
        Case adDouble: MaxVal = 922337203685477#
        Case adInteger: MaxVal = 2147483647
        Case adNumeric: MaxVal = 214748
        Case adSingle: MaxVal = 922337203685477#
        Case adSmallInt: MaxVal = 32767
        Case adTinyInt: MaxVal = 127
        Case adUnsignedBigInt: MaxVal = 2147483647
        Case adUnsignedInt: MaxVal = 2147483647
        Case adUnsignedSmallInt: MaxVal = 32767
        Case adUnsignedTinyInt: MaxVal = 255
        Case Else:  MaxVal = 0                  'Case not identified, assume 0
    End Select

End Function

Public Function FetchRandString(nSize) As String
Const daoc_stIllegalChars = ":;<=>?@^_'""{}[]|\~`×ÿ÷"

    Randomize
    stString = ""
    tChar = ""
    nCurrent = 0
    nCharLen = 1
    While nSize > nCurrent
        stChar = Chr(Int(((126 - 33 + 1) * Rnd) + 33))
        'check for illegal characters, null,  or string too long. If true, redo assignment, else
        If (InStr(daoc_stIllegalChars, stChar) = 0) And (Len(stChar) > 0) Then
           If (&HFF00 And CInt(Asc(stChar))) Then                'If true, double-byte, else single-byte
              nCharLen = 2
           Else
              nCharLen = 1
           End If
           If (nCharLen + nCurrent) <= nSize Then           'We haven't exceeded the size..
              nCurrent = nCurrent + nCharLen                'reset the size...
              stString = stString & stChar                  'and concatenate the char
           End If
        End If
    Wend
    FetchRandString = stString

End Function

Public Function GetColPosByType(oRS, nType)
    GetColPosByType = -1
    For i = 0 To oRS.Fields.Count - 1
        If oRS(i).Type = nType Then
            GetColPosByType = i
            Exit For
        End If
    Next
End Function

Public Function CanSetValue(ByVal fld As Field, ByVal nCursorType As Integer) As Boolean
    CanSetValue = True 'Assume true
    
    'Check to see if the field is updateable
    If ((fld.Attributes And adFldUpdatable) Or (fld.Attributes And adFldUnknownUpdatable)) = 0 Then
        CanSetValue = False
        Exit Function
    End If
    
    'If it is, see if ADO is capable of setting it.  All hacks for restricting datatypes
    'should go here
    Select Case fld.Type
        Case adArray: CanSetValue = False
        Case adBigInt: CanSetValue = True
        Case adBinary: CanSetValue = True
        Case adBoolean: CanSetValue = True
        Case adBSTR: CanSetValue = False
        Case adByRef: CanSetValue = False
        Case adChar: CanSetValue = True
        Case adCurrency: CanSetValue = True
        Case adDate: CanSetValue = True
        Case adDBDate: CanSetValue = True
        Case adDBTime: CanSetValue = True
        Case adDBTimeStamp: CanSetValue = True
        Case adDecimal: CanSetValue = True
        Case adDouble: CanSetValue = True
        Case adEmpty: CanSetValue = False
        Case adError: CanSetValue = False
        Case adGUID: CanSetValue = False
        Case adIDispatch: CanSetValue = False
        Case adInteger: CanSetValue = True
        Case adIUnknown: CanSetValue = False
        Case adLongVarBinary
            If nCursorType = adOpenForwardOnly Then
                CanSetValue = False
            Else
                CanSetValue = True
            End If
        Case adLongVarChar
            If nCursorType = adOpenForwardOnly Then
                CanSetValue = False
            Else
                CanSetValue = True
            End If
        Case adLongVarWChar
            If nCursorType = adOpenForwardOnly Then
                CanSetValue = False
            Else
                CanSetValue = True
            End If
        Case adNull: CanSetValue = False
        Case adNumeric: CanSetValue = True
        Case adSingle: CanSetValue = True
        Case adSmallInt: CanSetValue = True
        Case adTinyInt: CanSetValue = True
        Case adUnsignedBigInt: CanSetValue = True
        Case adUnsignedInt: CanSetValue = True
        Case adUnsignedSmallInt: CanSetValue = True
        Case adUnsignedTinyInt: CanSetValue = True
        Case adUserDefined: CanSetValue = False
        Case adVarBinary: CanSetValue = True
        Case adVarChar: CanSetValue = True
        Case adVariant: CanSetValue = False
        Case adVarWChar: CanSetValue = True
        Case adVector: CanSetValue = False
        Case adWChar: CanSetValue = True
        Case Else: CanSetValue = False
    End Select
End Function

Public Function StrType(ByVal nFieldType As Integer) As String
    Select Case nFieldType
        Case adArray: StrType = "adArray"
        Case adBigInt: StrType = "adBigInt"
        Case adBinary: StrType = "adBinary"
        Case adBoolean: StrType = "adBoolean"
        Case adBSTR: StrType = "adBSTR"
        Case adByRef: StrType = "adByRef"
        Case adChar: StrType = "adChar"
        Case adCurrency: StrType = "adCurrency"
        Case adDate: StrType = "adDate"
        Case adDBDate: StrType = "adDBDate"
        Case adDBTime: StrType = "adDBTime"
        Case adDBTimeStamp: StrType = "adDBTimeStamp"
        Case adDecimal: StrType = "adDecimal"
        Case adDouble: StrType = "adDouble"
        Case adEmpty: StrType = "adEmpty"
        Case adError: StrType = "adError"
        Case adGUID: StrType = "adGUID"
        Case adIDispatch: StrType = "adIDispatch"
        Case adInteger: StrType = "adInteger"
        Case adIUnknown: StrType = "adIUnknown"
        Case adLongVarBinary: StrType = "adLongVarBinary"
        Case adLongVarChar: StrType = "adLongVarChar"
        Case adLongVarWChar: StrType = "adLongVarWChar"
        Case adNull: StrType = "adNull"
        Case adNumeric: StrType = "adNumeric"
        Case adSingle: StrType = "adSingle"
        Case adSmallInt: StrType = "adSmallInt"
        Case adTinyInt: StrType = "adTinyInt"
        Case adUnsignedBigInt: StrType = "adUnsignedBigInt"
        Case adUnsignedInt: StrType = "adUnsignedInt"
        Case adUnsignedSmallInt: StrType = "adUnsignedSmallInt"
        Case adUnsignedTinyInt: StrType = "adUnsignedTinyInt"
        Case adUserDefined: StrType = "adUserDefined"
        Case adVarBinary: StrType = "adVarBinary"
        Case adVarChar: StrType = "adVarChar"
        Case adVariant: StrType = "adVariant"
        Case adVarWChar: StrType = "adVarWChar"
        Case adVector: StrType = "adVector"
        Case adWChar: StrType = "adWChar"
        Case Else: StrType = "Unknown data type"
    End Select
End Function

Public Function GetUpdatableCol(oRS)
    GetUpdatableCol = -1
    For i = 0 To oRS.Fields.Count - 1
        If ((oRS(i).Attributes And adFldUpdatable) Or (oRS(i).Attributes And adFldUnknownUpdatable)) Then
            If oRS(i).Type = adChar Then
                GetUpdatableCol = i
                Exit For
            End If
            'If oRS(i).Type <> adBoolean Then
            '    GetUpdatableCol = i
            '    Exit For
            'End If
        End If
    Next
    
End Function

Public Function RandRecNo(oRS) As Integer
    ' this routine generates a record number between the current position
    ' and the end of the recordset. The currency of the recordset is changed.
    oRS.MoveFirst
    RandRecNo = 0
    flipflag = True
    While Not oRS.EOF
        If flipflag Then
            flipflag = False
        Else
            RandRecNo = RandRecNo + 1
            flipflag = True
        End If
        oRS.MoveNext
    Wend
    oRS.MoveFirst
End Function
Public Function LogStatus(ByVal bstatus As Boolean, ByVal slogtext As String) As Boolean
    If (bstatus = False) Then
        LogStatus = False
        slogtext = slogtext & "FAILED"
    Else
        LogStatus = True
        slogtext = slogtext & "PASSED"
    End If

    ' write results to log file
    LogText slogtext
  
End Function
Public Function GetBestColumn(ByRef oRS As Recordset, Optional ByVal ndatatype1 As Integer, Optional ByVal ndatatype2 As Integer, Optional ByVal ndatatype3 As Integer, Optional ByVal ndatatype4 As Integer) As Integer
    ' GetBestColumn always returns a column.
    ' set default
    GetBestColumn = 0
    
    foundlevel = 5
    For i = 0 To oRS.Fields.Count - 1
        If oRS(i).Type = ndatatype1 Then
            GetBestColumn = i
            Exit For
        End If
        If (oRS(i).Type = ndatatype2 And foundlevel > 2) Then
            GetBestColumn = i
            foundlevel = 2
        End If
        If (oRS(i).Type = ndatatype3 And foundlevel > 3) Then
            GetBestColumn = i
            foundlevel = 3
        End If
        If (oRS(i).Type = ndatatype4 And foundlevel > 4) Then
            GetBestColumn = i
            foundlevel = 4
        End If
    Next i
    
End Function
Public Function CountRows(oRS) As Integer
    CountRows = 0
    oRS.MoveFirst
    While Not oRS.EOF
        CountRows = CountRows + 1
        oRS.MoveNext
    Wend
    oRS.MoveFirst
End Function
Public Function ValueCompare(CompareType As Integer, val1 As Variant, val2 As Variant) As Integer
    ValueCompare = -1
    'If both values are null, fall thru
    If IsNull(val1) And IsNull(val2) Then
        ValueCompare = 0
    'If one value is null, fail
    ElseIf IsNull(val1) Xor IsNull(val2) Then
        ValueCompare = 2
    Else
        If CompareType = adVariant Then
            CompareType = x
        End If
        'Now, we are ready to do some comparison
        Select Case CompareType
            Case adBoolean
                'hack to work around SS Driver boolean->unsignedtinyint problem
                'If CBool(vData(nField, nRow)) <> vValue2 Then GoTo lblErrTrap
                If val1 = val2 Then ValueCompare = 0
            Case adTinyInt, adSmallInt, adInteger, adBigInt, adUnsignedTinyInt, adUnsignedSmallInt, adUnsignedInt, adUnsignedBigInt
                'No problems here, straight comparison
                If val1 = val2 Then ValueCompare = 0
            Case adGUID
                'I think this is straight-forward as well
                If val1 = val2 Then ValueCompare = 0
            Case adDouble, adSingle, adCurrency
                'OK, here we only verify that the two numbers are accurate to five places
                If (1 - (val1 / val2)) <= 0.0001 Then ValueCompare = 0
            Case adDate, adDBDate, adDBTimeStamp, adDBTime
                'Verify that the two dates are accurate to four places
                If (1 - (val1 / val2)) <= 0.001 Then ValueCompare = 0
            Case adDecimal, adNumeric
                'If the variant holding the type is: string, then the values should match
                'exactly.  If it is: Float, then approxiamte
                'If VarType(val1) = vbSingle Or VarType(val1) = vbDouble Then
                '        If (1 - (val1 / val2)) <= (1 / 10 ^ fld.NumericScale) Then GoTo lblErrTrap
                'Else
                '        If CDec(vData(nField, nRow)) <> vValue2 Then GoTo lblErrTrap
                'End If
                If val1 = val2 Then ValueCompare = 0
            Case adBinary
                'If this is a fixed length field, then coerce the column value into
                'a BSTR, then TRIM off trailing zero's.
                If TypeName(val1) = "Byte()" Then
                    nLastPos = Int(InStr(val1, Chr(0)))
                    If nLastPos Then
                        vValue1 = Left(val1, nLastPos - 1)
                    Else
                        vValue1 = Trim(CStr(val1))
                    End If
                Else
                    vValue1 = Trim(CStr(val1))
                End If
                nLastPos = Int(InStr(val2, Chr(0)))
                If nLastPos Then
                    vValue2 = Left(val2, nLastPos - 1)
                Else
                    vValue2 = Trim(CStr(val2))
                End If
                If StrComp(vValue1, vValue2, vbBinaryCompare) = 0 Then ValueCompare = 0
            Case adChar, adWChar
                If StrComp(Trim(val1), Trim(val2), vbBinaryCompare) = 0 Then ValueCompare = 0
            Case adVarBinary, adVarChar, adVarWChar
                If StrComp(val1, val2, vbBinaryCompare) = 0 Then ValueCompare = 0
            Case adLongVarChar, adLongVarWChar, adLongVarBinary, adBSTR, adVariant
                If StrComp(val1, val2, vbBinaryCompare) = 0 Then ValueCompare = 0
            Case Else
                'perform a binary comparison
                If StrComp(val1, val2, vbBinaryCompare) = 0 Then ValueCompare = 0
        End Select
    End If
End Function

Public Function parse_init(ByVal instring As String, getwhat As Integer) As String
'    Dim value As String
'    f = xModInfo.GetInitStringValue("Hello", Value)
'DATASOURCE=datasource; USERID=userid; PASSWORD=password;
'LOCATION=server; FILE=filename.ini

    parse_init = ""
    Select Case getwhat
        Case 0 ' provider
            searchstr = "LTMPROVIDER:"
        Case 1 ' cursor location
            searchstr = "LTMCURSORLOC:"
        Case 2 ' rs string
            searchstr = "LTMRS:"
        Case 3 ' connection string
            searchstr = "LTMCONN:"
        Case 4 ' ini file string
            searchstr = "INIFILE:"
        Case 5 ' verbose string
            searchstr = "LTMVERBOSE:"
    End Select
    curchar = 1
    oneline = ""
    Do While curchar <= Len(instring)
        If Mid(instring, curchar, 1) = Chr(10) Then '"#" Then
            If InStr(oneline, searchstr) > 0 Then
                Exit Do
            Else
                oneline = ""
            End If
        Else
            oneline = oneline + Mid(instring, curchar, 1)
        End If
        curchar = curchar + 1
    Loop
    If InStr(oneline, searchstr) > 0 Then
        parse_init = Right(oneline, Len(oneline) - InStr(oneline, ":"))
    End If
    parse_init = Trim(parse_init)
End Function
 
Function GetConnStr(ByRef LocModInfo As ModInfo, ByRef Locprov As IProviderInfo) As String
Dim connds As String
Dim connloc As String
Dim connuid As String
Dim connpwd As String
Dim connps As String
Dim lprovstr As String
Dim ldb As String

       nocando = False
        ' we don't use the connection info from the ini file ?
        retcode = LocModInfo.GetInitStringValue("DATASOURCE", connds)
        retcode = LocModInfo.GetInitStringValue("LOCATION", connloc)
        retcode = LocModInfo.GetInitStringValue("DATABASE", ldb)
        connps = ""
        retcode = LocModInfo.GetInitStringValue("PROVIDERSTRING", connps)
'        If connds = "" And connloc = "" And connps = "" Then
'            nocando = True
'        End If
        retcode = LocModInfo.GetInitStringValue("USERID", connuid)
'        If connuid = "" Then
'            nocando = True
'        End If
        retcode = LocModInfo.GetInitStringValue("PASSWORD", connpwd)
'        If connpwd = "" Then
'            nocando = True
'        End If
        retcode = LocModInfo.GetInitStringValue("PROVIDER", lprovstr)
        If lprovstr = "" Then
            lprovstr = Locprov.GetName
        End If
'        If lprovstr = "" Then
'            nocando = True
'        End If
        If nocando Then
            GetConnStr = ""
        Else
            ' build the connection string
            fullconnstr = "Provider=" + lprovstr + ";"
            fullconnstr = fullconnstr + IIf(connds = "", "", "Data Source=" + connds + ";")
            fullconnstr = fullconnstr + IIf(ldb = "", "", "DataBase=" + ldb + ";")
            'fullconnstr = fullconnstr + IIf(connloc = "", "", "Location=" + connloc + ";")
            fullconnstr = fullconnstr + IIf(connps = "", "", connps + ";")
            fullconnstr = fullconnstr + IIf(connuid = "", "", "User Id=" + connuid + ";")
            fullconnstr = fullconnstr + IIf(connpwd = "", "", "Password=" + connpwd + ";")
            GetConnStr = fullconnstr
        End If
End Function

Function ConvToStr(ByRef LocFld As ADODB.Field) As String

If IsNull(LocFld.Value) Then
    ConvToStr = "NULL"
Else
    Select Case LocFld.Type
    
            Case adBoolean
                ConvToStr = IIf(LocFld.Value, "True", "False")
            Case adTinyInt, adSmallInt, adInteger, adBigInt
                ConvToStr = CStr(LocFld.Value)
            Case adUnsignedTinyInt, adUnsignedSmallInt, adUnsignedInt, adUnsignedBigInt
                ConvToStr = CStr(LocFld.Value)
            Case adDouble, adSingle, adCurrency
                ConvToStr = CStr(LocFld.Value)
            Case adDecimal, adNumeric
                ConvToStr = CStr(LocFld.Value)
            Case adBinary, adVarBinary
                ConvToStr = LocFld.Value
            Case adLongVarBinary
                ConvToStr = LocFld.Value
            Case adChar, adWChar, adVarChar, adVarWChar
                ConvToStr = LocFld.Value
            Case adLongVarChar, adLongVarWChar, adBSTR, adVariant
                ConvToStr = LocFld.Value
            Case adDate, adDBDate, adDBTimeStamp
                ConvToStr = CStr(LocFld.Value)
            Case adDBTime
                ConvToStr = CStr(LocFld.Value)
            Case adGUID
                ConvToStr = CStr(LocFld.Value)
            Case Else
                ConvToStr = "Unknown"
    
    End Select
End If

End Function
Function FindPropIndex(LocConn As ADODB.Connection, propstr As String) As Integer
FindPropIndex = -1
prpcnt = 0
For Each prp In LocConn.Properties
    If prp.Name = propstr Then
        FindPropIndex = prpcnt
        Exit For
    End If
    prpcnt = prpcnt + 1
Next prp

End Function

