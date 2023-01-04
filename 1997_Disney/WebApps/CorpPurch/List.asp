<% On Error Resume Next %>
<%
szSQL = Request.Form("txtSQL")
szConnectionString = Application("Conn")
Err.Clear
set RS = CreateObject("ADODB.Recordset")
RS.Open szSQL, szConnectionString

If Err.Number <> 0 Then
	Session("Err_Number") = Err.Number
	Session("Err_Source") = Err.Source
	Session("Err_Description") = Err.Description
	Response.Redirect "Errors.asp" 
End If

If RS.EOF = True Then
	Session("Err_Number") = -1
	Session("Err_Source") = "Query did not return any records"
	'Session("Err_Source") = szSQL
	Session("Err_Description") = "The query you submitted did not return any records.<BR>Please check your query parameters and retry."
	Response.Redirect "Errors.asp" 
End If

iStep = Request.Form("STEP")
iMaxRecords = Application("MaxRecords")

If iStep > 0 Then
	For i = 1 To iMaxRecords * iStep
	    RS.MoveNext
	Next
End If

iStep = iStep + 1
%>

<HTML>

<!-- #include file="Header.inc" -->

<BODY BACKGROUND='<% =Application("BackGroundImage") %>' BGPROPERTIES=FIXED>

</HEAD>

<CENTER>
<IMG SRC='<% =Application("MainImage") %>' ALT='<% =Application("Credits") %>'>
</CENTER>

<CENTER>
<TABLE WIDTH=50% COLS=5 <% =Application("TableProperties") %>>
<TR ALIGN=CENTER>
	<TH WIDTH=5%><TT><B>Requisition<BR>number</TT></B></TH>
	<TH WIDTH=5%><TT><B>PO<BR>number</TT></B></TH>
	<TH><TT><B>Vendor's<BR>name</TT></B></TH>
	<TH><TT><B>Client's<BR>name</TT></B></TH>
	<TH WIDTH=3%><TT><B>Due<BR>date</TT></B></TH>
</TR>
<%
i = 0
Dim szPONo
While Not RS.EOF And i < ( iMaxRecords )
%><TR><%
	If Trim(RS(0)) = "" Or IsNull(RS(0)) Then
		 %><TD>view details</TD><%
	Else	
		 %><TD NOWRAP=NOWRAP><% = RS(0) %></TD><%
	End If
	If Trim(RS(1)) = "" Or IsNull(RS(1)) Then
		 %><TD><% = Chr(160) %></TD><%
	Else	
		szPONo = Server.URLEncode( RS(1) )
		%><TD NOWRAP=NOWRAP><A HREF='Detail.asp?PONo=<% = szPONo  %>'><% = RS(1) %></A></TD><%
	End If
	If Trim(RS(2)) = "" Or IsNull(RS(2)) Then
		%><TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP><% = RS(2) %></TD><%
	End If
	If Trim(RS(3)) = "" Or IsNull(RS(3)) Then
		%><TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP><% = RS(3) %></TD><%
	End If
	If Trim(RS(4)) = "" Or IsNull(RS(4)) Then
		%><TD><% = Chr(160) %></TD><%
	Else	
		%><TD ALIGN=RIGHT NOWRAP=NOWRAP><% = RS(4) %></TD><%
	End If
	%></TR><%
	RS.MoveNext
	i = i + 1
Wend
%>
</TABLE>

<% 
If RS.EOF = False Then 
%>
<CENTER>
<TABLE COLS=3 ALIGN=CENTER WIDTH=60% CELLPADDING=5 CELLSPACING=0>
<TR>
	<FORM NAME=frmMore METHOD=POST ACTION='List.asp'>
		<TD ALIGN=CENTER>
			<INPUT TYPE=SUBMIT NAME=MORE VALUE='   More...  '>
			<INPUT TYPE=HIDDEN NAME=STEP VALUE=<% = iStep %>>
			<INPUT TYPE=HIDDEN NAME=txtSQL VALUE="<% = szSQL %>">
		</TD>
	</FORM>
	<FORM NAME=frmBack METHOD=GET>
		<TD ALIGN=CENTER>
			<INPUT TYPE="button" NAME=BACK VALUE='    Back    ' onClick="history.back();">
		</TD>
	</FORM>
	<FORM NAME=frmQuery METHOD=GET ACTION='Query.asp'>
		<TD ALIGN=CENTER>
			<INPUT TYPE=SUBMIT NAME=QUERY VALUE=' New query '>
		</TD>
	</FORM>
</TR>
</TABLE>
</CENTER>
<%
Else
%>
<CENTER>
<TABLE COLS=2 ALIGN=CENTER WIDTH=40% CELLPADDING=5 CELLSPACING=0>
<TR>
	<FORM NAME=frmBack METHOD=GET>
		<TD ALIGN=CENTER>
			<INPUT TYPE="button" NAME=BACK VALUE='    Back    ' onClick="history.back();">
		</TD>
	</FORM>
	<FORM NAME=frmQuery METHOD=GET ACTION='Query.asp'>
		<TD ALIGN=CENTER>
			<INPUT TYPE=SUBMIT NAME=QUERY VALUE=' New query '>
		</TD>
	</FORM>
</TR>
</TABLE>
</CENTER>
<%
End If 
RS.close
%>

<!-- #include file="Footer.inc" -->

</BODY>
</HTML>
