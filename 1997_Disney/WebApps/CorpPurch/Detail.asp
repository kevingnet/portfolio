<%
On Error Resume Next
szSQL = "SELECT * FROM qryPurchReqDetail WHERE PONo='" & Request.QueryString("PONo") & "'"
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
	Session("Err_Description") = "The query you submitted did not return any records.<BR>Please check your query parameters and retry."
	Response.Redirect "Errors.asp" 
End If

%>

<HTML>

<!-- #include file="Header.inc" -->

<BODY BACKGROUND='<% =Application("BackGroundImage") %>' BGPROPERTIES=FIXED onLoad="document.frmQuery.QUERY.focus();">

</HEAD>

<CENTER>
<IMG SRC='<% =Application("MainImage") %>' ALT='<% =Application("Credits") %>'>
</CENTER>

<CENTER>
<TABLE WIDTH=60% COLS=2 <% =Application("TableProperties") %>>
<TR>
<%
	If Trim(RS(0)) = "" Or IsNull(RS(0)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Buyer's name</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Buyer's name</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(0) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(1)) = "" Or IsNull(RS(1)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Entry date</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Entry date</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(1) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(2)) = "" Or IsNull(RS(2)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Requisition #</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Requisition #</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(2) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(3)) = "" Or IsNull(RS(3)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Client's name</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Client's name</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(3) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(4)) = "" Or IsNull(RS(4)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Deliver to</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Deliver to</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(4) %></TD><%
	End If
%>
</TR>
<TR>
	<TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Orig No(s)</TT></TD>
	<TD>
		<TABLE COLS=3 BORDER=1 CELLPADDING=3>
			<TR ALIGN=CENTER>
<%
			If Trim(RS(5)) = "" Or IsNull(RS(5)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(5) %></TD><%
			End If
%>
<%
			If Trim(RS(7)) = "" Or IsNull(RS(7)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(7) %></TD><%
			End If
%>
<%
			If Trim(RS(9)) = "" Or IsNull(RS(9)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(9) %></TD><%
			End If
%>
			</TR>
		</TABLE>
	</TD>
</TR>
<TR>
	<TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Dept No(s)</TT></TD>
	<TD>
		<TABLE COLS=3 BORDER=1 CELLPADDING=3>
			<TR ALIGN=CENTER>
<%
			If Trim(RS(6)) = "" Or IsNull(RS(6)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(6) %></TD><%
			End If
%>
<%
			If Trim(RS(8)) = "" Or IsNull(RS(8)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(8) %></TD><%
			End If
%>
<%
			If Trim(RS(10)) = "" Or IsNull(RS(10)) Then
%>
				<TD WIDTH=33%><% = Chr(160) %></TD><%
			Else
%>
				<TD WIDTH=33%><% = RS(10) %></TD><%
			End If
%>
			</TR>
		</TABLE>
	</TD>
</TR>
<TR>
<%
	If Trim(RS(11)) = "" Or IsNull(RS(11)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Vendor's name</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Vendor's name</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(11) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(12)) = "" Or IsNull(RS(12)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>PO #</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>PO #</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(12) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(13)) = "" Or IsNull(RS(13)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Purchase order type</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Purchase order type</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(13) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(14)) = "" Or IsNull(RS(14)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Expiration date</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Expiration date</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(14) %></TD><%
	End If
%>
</TR>
<TR>
<%
	If Trim(RS(15)) = "" Or IsNull(RS(15)) Then
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><TT>Due date</TT></TD>
		<TD><% = Chr(160) %></TD><%
	Else	
		%><TD NOWRAP=NOWRAP ALIGN=RIGHT><B><TT>Due date</TT></B></TD>
		<TD NOWRAP=NOWRAP><% = RS(15) %></TD><%
	End If
%>
</TR>
</TABLE>
</CENTER>

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

<!-- #include file="Footer.inc" -->

</BODY>
</HTML>
