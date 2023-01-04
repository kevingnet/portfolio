<% On Error Resume Next

Application("TableProperties") = "CELLPADDING=5 CELLSPACING=1 FRAME=BOX ALIGN=CENTER BORDER=5 BORDERCOLORLIGHT=#C0A070 BORDERCOLORDARK=#A06030 BGCOLOR=#F0F0F0 "

Application("BackGroundImage") = "Images/Coolbar3.jpg"
Application("MainImage") = "Images/CorpPurchBanner.gif"

Application("MaxRecords") = 1000

Application("AppPath") = Left ( szFilePath, iPos )
Application("AppName") = "Corporate Purchasing Query System"

Application("Conn") = "DRIVER={SQL Server};SERVER=FIS-WEB;UID=PurchReq;PWD=disney;DATABASE=PurchReq;"
Application("Credits") = "Corporate Purchasing"

%>

<HTML>

<!-- #include file="Header.inc" -->

<!-- #include file="CheckSubmit.inc" -->

<SCRIPT LANGUAGE="JavaScript">
<!-- 

function SubmitForm() 
{
	if (document.frmQuery.txt1.value == '' &&
		document.frmQuery.txtReqNo.value == '' &&
		document.frmQuery.txtVendorName.value == '' &&
		document.frmQuery.txtClientName.value == '' )
	{   
		alert ('Enter a value to use as search criteria');
		document.frmQuery.txt1.focus(); return false ;
	}
	else 
	{ 
		var szSQL = 'SELECT * FROM qryPurchReqResult WHERE ((' ;
		var bAddedExpression = false ;

		if ( document.frmQuery.txt1.value != '' )
		{
			if ( document.frmQuery.cbPONo.selectedIndex == 0 ) 
			{
				szSQL = szSQL + 'PONo Like \'' + document.frmQuery.txt1.value + '%\'' ;
			}
			if ( document.frmQuery.cbPONo.selectedIndex == 1 ) 
			{
				szSQL = szSQL + 'PONo Like \'%' + document.frmQuery.txt1.value + '\'' ;
			}
			if ( document.frmQuery.cbPONo.selectedIndex == 2 ) 
			{
				szSQL = szSQL + 'PONo Like \'%' + document.frmQuery.txt1.value + '%\'' ;
			}
			if ( document.frmQuery.cbPONo.selectedIndex == 3 ) 
			{
				szSQL = szSQL + 'PONo Like \'' + document.frmQuery.txt1.value + '\'' ;
			}
			bAddedExpression = true ; 
		}
		if ( document.frmQuery.txtReqNo.value != '' )
		{
			if ( bAddedExpression == true )
			{
				szSQL = szSQL + ') AND (' ;
			}
			if ( document.frmQuery.cbReqNo.selectedIndex == 0 ) 
			{
				szSQL = szSQL + 'ReqNo Like \'' + document.frmQuery.txtReqNo.value + '%\'' ;
			}
			if ( document.frmQuery.cbReqNo.selectedIndex == 1 ) 
			{
				szSQL = szSQL + 'ReqNo Like \'%' + document.frmQuery.txtReqNo.value + '\'' ;
			}
			if ( document.frmQuery.cbReqNo.selectedIndex == 2 ) 
			{
				szSQL = szSQL + 'ReqNo Like \'%' + document.frmQuery.txtReqNo.value + '%\'' ;
			}
			if ( document.frmQuery.cbReqNo.selectedIndex == 3 ) 
			{
				szSQL = szSQL + 'ReqNo Like \'' + document.frmQuery.txtReqNo.value + '\'' ;
			}
			bAddedExpression = true ;
		}

		if ( document.frmQuery.txtVendorName.value != '' )
		{
			if ( bAddedExpression == true )
			{
				szSQL = szSQL + ') AND (' ;
			}
			if ( document.frmQuery.cbVendorName.selectedIndex == 0 ) 
			{
				szSQL = szSQL + 'VendorName Like \'' + document.frmQuery.txtVendorName.value + '%\'' ;
			}
			if ( document.frmQuery.cbVendorName.selectedIndex == 1 ) 
			{
				szSQL = szSQL + 'VendorName Like \'%' + document.frmQuery.txtVendorName.value + '\'' ;
			}
			if ( document.frmQuery.cbVendorName.selectedIndex == 2 ) 
			{
				szSQL = szSQL + 'VendorName Like \'%' + document.frmQuery.txtVendorName.value + '%\'' ;
			}
			if ( document.frmQuery.cbVendorName.selectedIndex == 3 ) 
			{
				szSQL = szSQL + 'VendorName Like \'' + document.frmQuery.txtVendorName.value + '\'' ;
			}
			bAddedExpression = true ;
		}

		if ( document.frmQuery.txtClientName.value != '' )
		{
			if ( bAddedExpression == true )
			{
				szSQL = szSQL + ') AND (' ;
			}
			if ( document.frmQuery.cbClientName.selectedIndex == 0 ) 
			{
				szSQL = szSQL + 'ClientName Like \'' + document.frmQuery.txtClientName.value + '%\'' ;
			}
			if ( document.frmQuery.cbClientName.selectedIndex == 1 ) 
			{
				szSQL = szSQL + 'ClientName Like \'%' + document.frmQuery.txtClientName.value + '\'' ;
			}
			if ( document.frmQuery.cbClientName.selectedIndex == 2 ) 
			{
				szSQL = szSQL + 'ClientName Like \'%' + document.frmQuery.txtClientName.value + '%\'' ;
			}
			if ( document.frmQuery.cbClientName.selectedIndex == 3 ) 
			{
				szSQL = szSQL + 'ClientName Like \'' + document.frmQuery.txtClientName.value + '\'' ;
			}
			bAddedExpression = true ;
		}
		szSQL = szSQL + '));' ;

		document.frmQuery.txtSQL.value = szSQL ;
		//alert (szSQL);
		document.frmQuery.submit(); 
		return true ;
	}
}

// -->
</SCRIPT> 

<BODY BACKGROUND='<% =Application("BackGroundImage") %>' BGPROPERTIES=FIXED onLoad="document.frmQuery.txt1.focus();">

</HEAD>

<CENTER>
<IMG ALIGN=CENTER SRC='<% =Application("MainImage") %>' ALT='<% =Application("Credits") %>'>
</CENTER>

<CENTER>
<FORM NAME=frmQuery METHOD=POST ACTION='List.asp' onSubmit="return SubmitForm();">
<TABLE WIDTH=30% COLS=3 <% =Application("TableProperties") %>>
<TR ALIGN=CENTER>
	<TD NOWRAP=NOWRAP COLSPAN=3 BGCOLOR=#000000><FONT COLOR=#FFFFFF><TT>Please make a selection</TT></TD>
</TR>
<TR><COL ALIGN=RIGHT>
	<TD WIDTH=45% ALIGN=RIGHT NOWRAP=NOWRAP><B><TT>PO #</TT></B></TD>
	<TD WIDTH=10% ><SELECT NAME=cbPONo SIZE=1>
		<OPTION VALUE='0'>begins with</OPTION>
		<OPTION VALUE='1'>ends with</OPTION>
		<OPTION VALUE='2'>contains</OPTION>
		<OPTION VALUE='3'>equals</OPTION></SELECT></TD>
	<TD WIDTH=45%><INPUT TYPE=TEXT NAME=txt1 SIZE=10 MAXLENGTH=40 onKeyPress="return checkSubmit(event)"></TD>
</TR>
<TR>
	<TD ALIGN=RIGHT NOWRAP=NOWRAP><B><TT>Requisition #</TT></B></TD>
	<TD><SELECT NAME=cbReqNo SIZE=1>
		<OPTION VALUE='0'>begins with</OPTION>
		<OPTION VALUE='1'>ends with</OPTION>
		<OPTION VALUE='2'>contains</OPTION>
		<OPTION VALUE='3'>equals</OPTION></SELECT></TD>
	<TD><INPUT TYPE=TEXT NAME=txtReqNo SIZE=10 MAXLENGTH=30 onKeyPress="return checkSubmit(event)"></TD>
</TR>
<TR>
	<TD ALIGN=RIGHT NOWRAP=NOWRAP><B><TT>Vendor's Name</TT></B></TD>
	<TD><SELECT NAME=cbVendorName SIZE=1>
		<OPTION VALUE='0'>begins with</OPTION>
		<OPTION VALUE='1'>ends with</OPTION>
		<OPTION VALUE='2' SELECTED>contains</OPTION>
		<OPTION VALUE='3'>equals</OPTION></SELECT></TD>
	<TD><INPUT TYPE=TEXT NAME=txtVendorName SIZE=20 MAXLENGTH=40 onKeyPress="return checkSubmit(event)"></TD>
</TR>
<TR>
	<TD ALIGN=RIGHT NOWRAP=NOWRAP><B><TT>Client's Name</TT></B></TD>
	<TD><SELECT NAME=cbClientName SIZE=1>
		<OPTION VALUE='0'>begins with</OPTION>
		<OPTION VALUE='1'>ends with</OPTION>
		<OPTION VALUE='2' SELECTED>contains</OPTION>
		<OPTION VALUE='3'>equals</OPTION></SELECT></TD>
	<TD><INPUT TYPE=TEXT NAME=txtClientName SIZE=20 MAXLENGTH=40 onKeyPress="return checkSubmit(event)"></TD>
</TR>
</TABLE>

<BR>
<INPUT NAME=cmdSubmit TYPE=SUBMIT VALUE='Submit Query' LANGUAGE=JavaScript>
<INPUT TYPE=RESET VALUE='Reset Defaults' LANGUAGE=JavaScript ONCLICK="document.frmQuery.txt1.focus();">
<INPUT TYPE=HIDDEN NAME=STEP VALUE=0>
<INPUT TYPE=HIDDEN NAME=txtSQL VALUE=0>
</FORM>
</CENTER>

<BR>
<CENTER>
<A TARGET='_parent' HREF='/asp/CorpPurchOld/Query.asp'>History, FYI 98 or prior
</A>
</CENTER>
<BR>

<!-- #include file="Footer.inc" -->

</BODY>
</HTML>
