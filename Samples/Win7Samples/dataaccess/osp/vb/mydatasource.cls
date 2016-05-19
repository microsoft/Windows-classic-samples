VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0   'False
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 1  'vbDataSource
END
Attribute VB_Name = "MyDataSource"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Private Sub Class_GetDataMember(DataMember As String, Data As Object)

    If DataMember = "" Then Err.Raise (E_FAIL)
    
    Dim MyOSP As New MyOSPObject
    MyOSP.FilePath = DataMember
    MyOSP.LoadData
    
    Set Data = MyOSP
    
End Sub

