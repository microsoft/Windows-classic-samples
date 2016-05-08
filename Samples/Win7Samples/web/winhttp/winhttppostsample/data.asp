<%
        ' Copyright (c) Microsoft Corporation. All rights reserved. 
	' This prints out the data that was POSTed and also sends the header "Request-Method"
	' whose value is the request method used to access this page.
	
  Dim vntPostedData, lngCount
  
  Response.AddHeader "Request-Method", Request.ServerVariables("REQUEST_METHOD")

  lngCount = Request.TotalBytes
  if (lngCount <> 0) then
	  vntPostedData = Request.BinaryRead(lngCount)
	  Response.BinaryWrite vntPostedData
  end if
%> 
