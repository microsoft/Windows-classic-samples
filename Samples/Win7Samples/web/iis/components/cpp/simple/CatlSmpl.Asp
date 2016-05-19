<HTML>

<HEAD>
<TITLE>Testing CATLSmpl</TITLE> 
</HEAD> 

<H1>Testing CATLSmpl</H1>

<% Set myobj = Server.CreateObject("IISSample.C++ATLSimple") %>
myProperty Value = <%= myobj.myProperty %><BR>

<% newvalue = "Simple" %>
Set myProperty to <%= newvalue %>
<% myobj.myProperty = newvalue %><BR>
myProperty Value is now: <%= myobj.myProperty %><BR>

<% mystring = "My String" %>
Call myMethod with <%= mystring %>: <%= myobj.myMethod(mystring) %><BR>

</HTML>
