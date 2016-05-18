<%
	' This prints out the data that was POSTed 
	
  Dim vntPostedData, lngCount
  
  Response.Write "<p>"
  Response.Write("MessageType is ")
  vntPostedData = Request.Form("MessageType")
  Response.Write vntPostedData
  Response.Write "</p>"

  Response.Write "<p>"
  Response.Write("Subject is ")
  vntPostedData = Request.Form("Subject")
  Response.Write vntPostedData
  Response.Write "</p>"

  Response.Write "<p>"
  Response.Write("SubjectOther is ")
  vntPostedData = Request.Form("SubjectOther")
  Response.Write vntPostedData
  Response.Write "</p>"

  Response.Write "<p>"
  Response.Write("Comments is ")
  vntPostedData = Request.Form("Comments")
  Response.Write vntPostedData
  Response.Write "</p>"


  Response.Write "<p>"
  Response.Write("Username is ")
  vntPostedData = Request.Form("Username")
  Response.Write vntPostedData
  Response.Write "</p>"


  Response.Write "<p>"
  Response.Write("UserEmail is ")
  vntPostedData = Request.Form("UserEmail")
  Response.Write vntPostedData
  Response.Write "</p>"


  Response.Write "<p>"
  Response.Write("UserTel is ")
  vntPostedData = Request.Form("UserTel")
  Response.Write vntPostedData
  Response.Write "</p>"


  Response.Write "<p>"
  Response.Write("UserFAX is ")
  vntPostedData = Request.Form("UserFAX")
  Response.Write vntPostedData
  Response.Write "</p>"
%> 
