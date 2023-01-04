

// @filename:   MachParamsDlg.cpp
// @fileauthor: Kevin Guerra

#include <windows.h>
#include "MachParamsDlg.h"
#include "MachParamsDlgMacros.h"
#include "SelectTool.h"
#include "math.h"
#include "CommentsDlg.h"
#include "RapidAreaDlg.h"
#include "SelectTool.h"

using namespace New5Axis;

#ifdef MessageBox
#undef MessageBox
#endif

void MachParamsDlg::InitializeTabTool()
{
	int fail = FALSE;
	OperationMode opMode = m_scMachParams->GetOperationMode();
	bool isSavedParams = m_scMachParams->IsSaved();
	
	//select a tool (without UI) if it is create mode and it
	//isn't saved parameters (these don't need to re-selected)
	if (opMode == OperationMode::CREATE && m_SelectToolMaterial)
	{
		fail = SelectTool(m_scMachParams, false);
	}
	
	//if (fail == FALSE) //by IG: fixes a nasty bug. When you changed system units after a tool path calc, it would fail, :(
		UpdateToolInfo();

	fail = FALSE;
	if (opMode == OperationMode::CREATE && !isSavedParams && m_SelectToolMaterial)
	{
		//select a material (without UI)
		fail = SelectMaterial(m_scMachParams, false);

		//set the surface speed the same as default surf speed
		m_scToolParams->SurfaceSpeed = m_scToolParams->DefaultSurfaceSpeed;
		CalcControlsSPF();
	}
	else //EDIT, REGEN, UPDATE and saved params
		InitializeControlsSPF();

	//update the material info
	if (fail == FALSE)
		UpdateMaterialInfo();

	if( m_scMachParams->IsMillTurn() )
	{
		m_cbSpindle->Enabled = true;
		m_cbTurret->Enabled = true;
	}else{
		m_cbSpindle->SelectedIndex = static_cast<int>(UITypes::DB2UISpindleTypes(SpindleTypes::SPINDLET_MAIN));
		m_cbTurret->SelectedIndex = static_cast<int>(UITypes::DB2UITurretTypes(TurretTypes::TURRETT_FRONT));
		m_cbSpindle->Enabled = false;
		m_cbTurret->Enabled = false;
	}

}

void MachParamsDlg::UpdateMaterialInfo()
{
	double surfSpeed = 0.0;

	m_lblSelectedMaterial->Text = m_scToolParams->MaterialDescription;

	//use the default surface speed for displaying in the dialog label
	surfSpeed = m_scToolParams->DefaultSurfaceSpeed;
	
	m_lblSurfaceSpeedVal->Text = (surfSpeed.ToString()->Format(m_formatDblStr, (surfSpeed)));
}

void MachParamsDlg::UpdateToolInfo()
{
	GET_COMBOBOX_RESOURCES7( Coolant )
	GET_COMBOBOX_RESOURCES2( SpindleDirection )
	GET_COMBOBOX_RESOURCES2( Spindle )
	GET_COMBOBOX_RESOURCES5( Turret )
	GET_COMBOBOX_RESOURCES8( ToolMaterial )
	if (scApp_GetUnitType() == TOOLLIB_UNITS_US){
		GET_COMBOBOX_wUNITS_RESOURCES2( FeedRate, FeedRateIN )
	}else{
		GET_COMBOBOX_wUNITS_RESOURCES2( FeedRate, FeedRateMM )
	}

	PARAM2CHECKBOX( CalcSpeedAuto, ToolParams )
	PARAM2COMBOBOX( FeedRate, ToolParams )
	PARAM2COMBOBOX2( SpindleDirection, ToolParams )
	PARAM2COMBOBOX2( Spindle, ToolParams )
	PARAM2COMBOBOX2( Coolant, ToolParams )
	PARAM2COMBOBOX2( Turret, ToolParams )

	m_lblSelectedTool->Text = m_scToolParams->ToolDescription;

	m_cbToolMaterial->SelectedIndex = m_scToolParams->ToolMaterial;

	PARAM2TEXT_INT( ToolNumber, ToolParams )
	PARAM2TEXT_INT( LengthOffset, ToolParams )
	PARAM2TEXT_INT( WorkOffset, ToolParams )
	PARAM2TEXT_INT( DiameterOffset, ToolParams )
	PARAM2TEXT_INT( NumberOfFlutes, ToolParams )
	PARAM2TEXT_INT( ProgramNumber, ToolParams )

	PARAM2TEXT( CornerRadius, ToolParams )
	PARAM2TEXT( ToolDiameter, ToolParams )
	PARAM2TEXT( HighFeedRate, ToolParams )
	PARAM2TEXT( GaugeLengthX, ToolParams )
	PARAM2TEXT( GaugeLengthZ, ToolParams )
	PARAM2LABEL( ChipLoadPerFlute, ToolParams )

	if( m_scToolParams->ToolClass == TL_MILL_CLASS_BALLNOSE )
	{
		m_lblProgramToTool->Enabled = true;
		m_rbProgramToToolTip->Enabled = true;
		m_rbProgramToToolCenter->Enabled = true;
		m_rbProgramToToolTip->Checked = (m_scToolParams->ProgramTool == TL_PROGRAM_TO_TIP);
		m_rbProgramToToolCenter->Checked = !m_rbProgramToToolTip->Checked;
	}
	else
	{
		m_lblProgramToTool->Enabled = false;
		m_rbProgramToToolTip->Enabled = false;
		m_rbProgramToToolCenter->Enabled = false;
		m_scToolParams->ProgramTool = TL_PROGRAM_TO_TIP;
		m_rbProgramToToolTip->Checked = true;
		m_rbProgramToToolCenter->Checked = false;
	}

	//pass the comment string to tool parameters
	String ^comments = m_scToolParams->UserComments;
	comments->Trim();
	if (String::IsNullOrEmpty(comments))
		m_btnComments->Text = m_ResourceManager->GetString("IDS_COMMENTS_NONE");
	else
		m_btnComments->Text = comments;

	if (m_scMachParams->GetOperationMode() == OperationMode::EDIT_PARAMETERS)
	{
		m_btnSelectTool->Enabled = false;
		m_btnSelectMaterial->Enabled = false;
		m_txtCornerRadius->Enabled = false;
		m_txtToolDiameter->Enabled = false;
		m_rbProgramToToolTip->Enabled = false;
		m_rbProgramToToolCenter->Enabled = false;
		m_lblChipLoadPerFlute->Enabled = false;
		m_txtHighFeedRate->Enabled = false;
		m_btnInsertPostProcessorCommand->Enabled = false;
	}
}

DEFINE_CONTROL_HANDLER( m_btnSelectTool_Click )
{
	//put up the tool selection dialog
	int fail = SelectTool(m_scMachParams, true);
	if (!fail)
	{
		//set the surface speed the same as default surf speed
		//for the given material
		m_scToolParams->SurfaceSpeed = m_scToolParams->DefaultSurfaceSpeed;
		UpdateToolInfo();
		if (m_chkCalcSpeedAuto->Checked)
			CalcControlsSPF();

		if( m_scToolParams->ToolClass == TL_MILL_CLASS_BALLNOSE )
		{
			m_lblProgramToTool->Enabled = true;
			m_rbProgramToToolTip->Enabled = true;
			m_rbProgramToToolCenter->Enabled = true;
			m_rbProgramToToolTip->Checked = (m_scToolParams->ProgramTool == TL_PROGRAM_TO_TIP);
			m_rbProgramToToolCenter->Checked = !m_rbProgramToToolTip->Checked;
		}
		else
		{
			m_lblProgramToTool->Enabled = false;
			m_rbProgramToToolTip->Enabled = false;
			m_rbProgramToToolCenter->Enabled = false;
			m_scToolParams->ProgramTool = TL_PROGRAM_TO_TIP;
			m_rbProgramToToolTip->Checked = true;
			m_rbProgramToToolCenter->Checked = false;
		}
	}
}

DEFINE_CONTROL_HANDLER( m_btnSelectMaterial_Click )
{
	//put up the material selection dialog
	int fail = SelectMaterial(m_scMachParams, true);
	if (!fail)
	{
		//update the material info
		UpdateMaterialInfo();

		//set the surface speed the same as default surf speed
		//for the gcnew material (if the auto calc speed is ON)
		if (m_chkCalcSpeedAuto->Checked)
		{
			m_scToolParams->SurfaceSpeed = m_scToolParams->DefaultSurfaceSpeed;
			CalcControlsSPF();
		}
	}
}

DEFINE_CONTROL_HANDLER( m_btnCalcSpeed_Click )
{
	//set the surface speed the same as default surf speed
	//for the given material
	m_scToolParams->SurfaceSpeed = m_scToolParams->DefaultSurfaceSpeed;
	CalcControlsSPF();
}

DEFINE_CONTROL_HANDLER( m_btnInsertPostProcessorCommand_Click )
{
	//call the post-processor insertion function
	TCHAR sIText[256];
	long nLength =255;
	long nWhere;

	if (scNCPost_GetIncIText(sIText, nLength, &nWhere) == SUCCESS)
	{ // if IText -Put it into PacketDB
		m_scToolParams->SetInsertPostProcessorCommand(nWhere, sIText);
		m_scMachParams->SetToolParams(m_scToolParams);
	} // if IText
}

DEFINE_CONTROL_HANDLER( m_btnComments_Click )
{
	String ^comments = m_scToolParams->UserComments;

	//open a rich textbox for obtaining the user comments
	CommentsDlg ^comDlg = gcnew CommentsDlg(comments);

	System::Windows::Forms::DialogResult dlgRes = comDlg->ShowDialog();
	if (dlgRes == ::DialogResult::OK)
	{
		String ^ commentsLabel = m_ResourceManager->GetString("IDS_COMMENTS_NONE");
		String ^ commentsNew = comDlg->GetCommentText();
		commentsNew->Trim();
		if( commentsLabel->Length != commentsNew->Length ||
			(String::Compare( commentsNew, commentsLabel, true /*ignore case*/ ) == 0 ) )
		{
			m_scToolParams->UserComments = commentsNew;
			m_btnComments->Text = commentsNew;
		}
		if( String::IsNullOrEmpty(commentsNew) )
			m_btnComments->Text = commentsLabel;
	}
}


DEFINE_TXTCONTROL_HANDLERS( ToolNumber, ToolParams, Int )}
DEFINE_TXTCONTROL_HANDLERS( WorkOffset, ToolParams, Int )}
DEFINE_TXTCONTROL_HANDLERS( LengthOffset, ToolParams, Int )}
DEFINE_TXTCONTROL_HANDLERS( DiameterOffset, ToolParams, Int )}
DEFINE_TXTCONTROL_HANDLERS( GaugeLengthX, ToolParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( GaugeLengthZ, ToolParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( HighFeedRate, ToolParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( ProgramNumber, ToolParams, Int )}

DEFINE_TXTCONTROL_HANDLERS( ToolDiameter, ToolParams, Double )
	double toolDia = GetDoubleFromTextBox(m_txtToolDiameter);
	double prevToolDia = m_scToolParams->ToolDiameter;
	double toolRadius = m_scToolParams->ToolDiameter/2;

	if (m_txtToolDiameter->Focused)
	{
		m_scCuttingParams->StartMargin = toolRadius;
		m_scCuttingParams->EndMargin = toolRadius;

		if (fabs(prevToolDia-toolDia) > 0.0)
		{
			double tipRad = m_scToolParams->CornerRadius;
			String ^sToolDesc = m_lblCustom;
			if (tipRad > 0.0)
				sToolDesc = String::Concat(sToolDesc, ToString()->Format("{0:f2}, {1:f2}", (toolDia), (tipRad)));
			else
				sToolDesc = String::Concat(sToolDesc, ToString()->Format("{0:f2}", (toolDia)));
			m_scToolParams->ToolDiameter = toolDia;
			m_scToolParams->ToolDescription = sToolDesc;
			m_lblSelectedTool->Text = m_scToolParams->ToolDescription;
			//CalcControlsSPF();
		}
	}
}

DEFINE_TXTCONTROL_HANDLERS( CornerRadius, ToolParams, Double )
	double tipRad = GetDoubleFromTextBox(m_txtCornerRadius);
	double prevTipRad = m_scToolParams->CornerRadius;

	if (m_txtCornerRadius->Focused && fabs(prevTipRad-tipRad) > 0.0)
	{
		double toolDia = m_scToolParams->ToolDiameter;
		String ^sToolDesc = m_lblCustom;
		if (tipRad > 0.0)
			sToolDesc = String::Concat(sToolDesc, ToString()->Format("{0:f2}, {1:f2}", (toolDia), (tipRad)));
		else
			sToolDesc = String::Concat(sToolDesc, ToString()->Format("{0:f2}", (toolDia)));
		m_scToolParams->CornerRadius = tipRad;
		m_scToolParams->ToolDescription = sToolDesc;
		m_lblSelectedTool->Text = m_scToolParams->ToolDescription;
		//CalcControlsSPF();
	}
}

DEFINE_TXTCONTROL_HANDLERS( NumberOfFlutes, ToolParams, Int )
	int numFlutes = GetIntFromTextBox(m_txtNumberOfFlutes);
	int prevNumFlutes = m_scToolParams->NumberOfFlutes;

	if (m_txtNumberOfFlutes->Focused && prevNumFlutes != numFlutes)
	{
		m_scToolParams->NumberOfFlutes = numFlutes;
		//CalcControlsSPF();
	}
}

DEFINE_COMBOBOX_CONTROL_HANDLER( SpindleDirection, ToolParams )}
DEFINE_COMBOBOX_CONTROL_HANDLER( Spindle, ToolParams )}
DEFINE_COMBOBOX_CONTROL_HANDLER( Turret, ToolParams )}
DEFINE_COMBOBOX_CONTROL_HANDLER( Coolant, ToolParams )}

DEFINE_CONTROL_HANDLER( m_cbToolMaterial_SelectedIndexChanged )
{
	int toolMatl = m_cbToolMaterial->SelectedIndex;
	int prevToolMatl = m_scToolParams->ToolMaterial;

	if (m_cbToolMaterial->Focused && prevToolMatl != toolMatl)
	{
		m_scToolParams->ToolMaterial = toolMatl;

		//get the gcnew tool material info
		SelectMaterial(m_scMachParams, false);

		//update the material info
		UpdateMaterialInfo();

		//now update the speed, feed and plunge rates (before that set the 
		//surface speed the same as default surf speed for this material
		//if the auto calc speed is ON)
		if (m_chkCalcSpeedAuto->Checked)
		{
			m_scToolParams->SurfaceSpeed = m_scToolParams->DefaultSurfaceSpeed;
			CalcControlsSPF();
		}
	}
}

DEFINE_CONTROL_HANDLER( m_chkStartPoint_CheckedChanged )
{
	m_btnStartPoint->Enabled = m_chkStartPoint->Checked;
	m_scStartPointParams->Use = m_chkStartPoint->Checked;
}

DEFINE_CONTROL_HANDLER( m_chkCalcSpeedAuto_CheckedChanged )
{
	bool calcSpd = m_chkCalcSpeedAuto->Checked;
	m_scToolParams->CalcSpeedAuto = calcSpd;
}

DEFINE_CONTROL_HANDLER( m_rbProgramToToolTip_CheckedChanged )
{
	if( m_rbProgramToToolTip->Focused )
		m_scToolParams->ProgramTool = TL_PROGRAM_TO_TIP;
}

DEFINE_CONTROL_HANDLER( m_rbProgramToToolCenter_CheckedChanged )
{
	if( m_rbProgramToToolCenter->Focused )
		m_scToolParams->ProgramTool = TL_PROGRAM_TO_CENTER;
}

//--------------------------------------------------------
DEFINE_CONTROL_HANDLER( m_cbFeedRate_SelectedIndexChanged )
{
	FeedRateTypes curFeedRate = UITypes::UI2DBFeedRateTypes(static_cast<UIFeedRateTypes>(m_cbFeedRate->SelectedIndex));
	if (curFeedRate != m_scToolParams->FeedRateType)
	{
		m_scToolParams->FeedRateType = curFeedRate;
		CalcControlFeedRate();
		CalcControlPlungeRate();
		//check here
		//CalcControlFeedChipLoad();
		//CalcControlPlungeChipLoad();
	}
}

//DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( SpindleSpeed, ToolParams, Int )
DEFINE_CONTROL_HANDLER( m_txtSpindleSpeed_TextChanged ) 
{
	int spinSpd = GetIntFromTextBox(m_txtSpindleSpeed);
	int prevSpinSpd = m_scToolParams->SpindleSpeed;

	m_scToolParams->SpindleSpeed = spinSpd;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtSpindleSpeed->Focused && prevSpinSpd != spinSpd)
	{
		CalcControlSurfaceSpeed();
		CalcControlFeedChipLoad();
		CalcControlPlungeChipLoad();
		//check here
		//CalcControlFeedRate();
		//CalcControlPlungeRate();
	}
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( SpindleSpeed, Int )
	EvalAsInt(m_txtSpindleSpeed);
		CalcControlSurfaceSpeed();
		CalcControlFeedChipLoad();
		CalcControlPlungeChipLoad();
		//check here
		//CalcControlFeedRate();
		//CalcControlPlungeRate();
}
DEFINE_TXTCONTROL_HANDLERS3( SurfaceSpeed, ToolParams, Double )
	double surfSpd = GetDoubleFromTextBox(m_txtSurfaceSpeed);
	double prevSurfSpd = m_scToolParams->SurfaceSpeed;

	m_scToolParams->SurfaceSpeed = surfSpd;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtSurfaceSpeed->Focused && fabs(prevSurfSpd-surfSpd) > 0.0)
	{
		CalcControlSpeedFeed();
		CalcControlFeedRate();
		CalcControlPlungeRate();
		//check here
		//CalcControlFeedChipLoad();
		//CalcControlPlungeChipLoad();
	}
}

//DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( FeedRate, ToolParams, Double )
DEFINE_CONTROL_HANDLER( m_txtFeedRate_TextChanged ) 
{
	double feedRate = GetDoubleFromTextBox(m_txtFeedRate);
	double prevFeedRate = m_scToolParams->FeedRate;

	m_scToolParams->FeedRate = feedRate;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtFeedRate->Focused && fabs(prevFeedRate-feedRate) > 0.0)
	{
		CalcControlFeedChipLoad();
	}
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( FeedRate, Double )
	EvalAsDouble(m_txtFeedRate);
	CalcControlFeedChipLoad();
}
DEFINE_TXTCONTROL_HANDLERS3( FeedChipLoad, ToolParams, Double )
	double feedChipLoad = GetDoubleFromTextBox(m_txtFeedChipLoad);
	double prevFeedChipLoad = m_scToolParams->FeedChipLoad;

	m_scToolParams->FeedChipLoad = feedChipLoad;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtFeedChipLoad->Focused && fabs(prevFeedChipLoad-feedChipLoad) > 0.0)
	{
		CalcControlFeedRate();
	}
}

//DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( PlungeRate, ToolParams, Double )
DEFINE_CONTROL_HANDLER( m_txtPlungeRate_TextChanged ) 
{
	double plungeRate = GetDoubleFromTextBox(m_txtPlungeRate);
	double prevPlungeRate = m_scToolParams->PlungeRate;

	m_scToolParams->PlungeRate = plungeRate;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtPlungeRate->Focused && fabs(prevPlungeRate-plungeRate) > 0.0)
	{
		CalcControlPlungeChipLoad();
	}
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( PlungeRate, Double )
	EvalAsDouble(m_txtPlungeRate);
	CalcControlPlungeChipLoad();
}
DEFINE_TXTCONTROL_HANDLERS3( PlungeChipLoad, ToolParams, Double )
	double plungeChipLoad = GetDoubleFromTextBox(m_txtPlungeChipLoad);
	double prevPlungeChipLoad = m_scToolParams->PlungeChipLoad;

	m_scToolParams->PlungeChipLoad = plungeChipLoad;
	//check to make sure it is a keyboard event and the value has changed
	if (m_txtPlungeChipLoad->Focused &&	fabs(prevPlungeChipLoad-plungeChipLoad) > 0.0)
	{
		CalcControlPlungeRate();
	}
}

//....................................................
void MachParamsDlg::CalcControlSurfaceSpeed()
{
	if (m_scMachParams->GetUnits() == TOOLLIB_UNITS_METRIC)
		m_scToolParams->SurfaceSpeed = (Math::PI*(m_scToolParams->ToolDiameter/2.0)*m_scToolParams->SpindleSpeed)/500.0;
	else
		m_scToolParams->SurfaceSpeed = (Math::PI*(m_scToolParams->ToolDiameter/2.0)*m_scToolParams->SpindleSpeed)/6.0;
	m_txtSurfaceSpeed->Text = (m_scToolParams->SurfaceSpeed.ToString()->Format(m_formatDblStr, (m_scToolParams->SurfaceSpeed)));
}

void MachParamsDlg::CalcControlFeedRate()
{
	if (m_scToolParams->FeedRateType == FeedRateTypes::FEED_RATE_IPR)
		m_scToolParams->FeedRate = m_scToolParams->FeedChipLoad * m_scToolParams->NumberOfFlutes;
	else
		m_scToolParams->FeedRate = m_scToolParams->FeedChipLoad * m_scToolParams->NumberOfFlutes * m_scToolParams->SpindleSpeed;
	m_txtFeedRate->Text = (m_scToolParams->FeedRate.ToString()->Format(m_formatDblStr, (m_scToolParams->FeedRate)));
}

void MachParamsDlg::CalcControlPlungeRate()
{
	if (m_scToolParams->FeedRateType == FeedRateTypes::FEED_RATE_IPR)
		m_scToolParams->PlungeRate = m_scToolParams->PlungeChipLoad * m_scToolParams->NumberOfFlutes;
	else
		m_scToolParams->PlungeRate = m_scToolParams->PlungeChipLoad * m_scToolParams->NumberOfFlutes * m_scToolParams->SpindleSpeed;
	m_txtPlungeRate->Text = (m_scToolParams->PlungeRate.ToString()->Format(m_formatDblStr, (m_scToolParams->PlungeRate)));
}

void MachParamsDlg::CalcControlFeedChipLoad()
{
	if (m_scToolParams->NumberOfFlutes == 0)
		return;

	if (m_scToolParams->FeedRateType == FeedRateTypes::FEED_RATE_IPR)
		m_scToolParams->FeedChipLoad = m_scToolParams->FeedRate;
	else
		m_scToolParams->FeedChipLoad = (m_scToolParams->FeedRate/m_scToolParams->SpindleSpeed) / m_scToolParams->NumberOfFlutes;

	m_txtFeedChipLoad->Text = (m_scToolParams->FeedChipLoad.ToString()->Format(m_formatDblStr, (m_scToolParams->FeedChipLoad)));
}

void MachParamsDlg::CalcControlPlungeChipLoad()
{
	if (m_scToolParams->NumberOfFlutes == 0)
		return;
	if (m_scToolParams->FeedRateType == FeedRateTypes::FEED_RATE_IPR)
		m_scToolParams->PlungeChipLoad = m_scToolParams->PlungeRate;
	else
		m_scToolParams->PlungeChipLoad = (m_scToolParams->PlungeRate/m_scToolParams->SpindleSpeed) / m_scToolParams->NumberOfFlutes;

	m_txtPlungeChipLoad->Text = (m_scToolParams->PlungeChipLoad.ToString()->Format(m_formatDblStr, (m_scToolParams->PlungeChipLoad)));
}

void MachParamsDlg::CalcControlSpeedFeed()
{
	int nSpinSpeed = 0, numFlutes = 0;
	//bool bCalcSpeeds = m_chkCalcSpeedAuto->Checked;
	double rpm = 0.0, ipr = 0.0, minFeed, maxFeed;
	double chipLoad = 0.0, chipLoadFac = 1.0, surfSpeed = 0.0, lfSpinSpeed = 0.0;
	double totalRadius = 0.0, feedRate = 0.0, plungeRate = 0.0;

	totalRadius = m_scToolParams->ToolDiameter/2.0;
	numFlutes = m_scToolParams->NumberOfFlutes;
	surfSpeed = m_scToolParams->SurfaceSpeed;
	chipLoad = m_scToolParams->ChipLoadPerFlute;

	//calculate the speed/feed 
	if (/*(bCalcSpeeds) &&*/ (totalRadius > 0.0))
	{ // if	totalRadius
		if (m_scMachParams->GetUnits() == TOOLLIB_UNITS_METRIC)
		{
			rpm = (1000.0*surfSpeed)/(2.0*Math::PI*totalRadius);
			maxFeed = MAX_FEEDRATE_INCH * MILS_PER_INCH;
			minFeed = MIN_FEEDRATE_INCH * MILS_PER_INCH;
		}
		else
		{
			rpm = (12.0*surfSpeed)/(2.0*Math::PI*totalRadius);
			maxFeed = MAX_FEEDRATE_INCH;
			minFeed = MIN_FEEDRATE_INCH;
		}

		if (rpm > MAX_RPM) rpm = MAX_RPM;
		else if (rpm < MIN_RPM)	rpm = MIN_RPM;

		ipr = chipLoad * numFlutes * chipLoadFac;
		feedRate = ipr * rpm;

		if (feedRate > maxFeed) feedRate = maxFeed;
		else if (feedRate < minFeed) feedRate = minFeed;

		lfSpinSpeed = floor(rpm + 0.5); // rpm (rounded)
		nSpinSpeed = (UINT) lfSpinSpeed; // rpm
		if (m_scToolParams->FeedRateType == FeedRateTypes::FEED_RATE_IPR) 
		{
			feedRate = ipr;
			plungeRate = ipr/2.0;
		}
		else 
		{
			plungeRate = feedRate/2.0;
		}

		//now update the dialog using the values calculated above
		m_txtSpindleSpeed->Text = (nSpinSpeed.ToString()->Format("{0}", (nSpinSpeed)));
		m_txtFeedRate->Text = (feedRate.ToString()->Format(m_formatDblStr, (feedRate)));
		m_txtPlungeRate->Text = (plungeRate.ToString()->Format(m_formatDblStr, (plungeRate)));
	} // if	totalRadius
}

//Update the speed, feed and plunge values on the toolpage (lower part)
void MachParamsDlg::InitializeControlsSPF()
{
	PARAM2TEXT_INT( SpindleSpeed, ToolParams )
	PARAM2TEXT( FeedRate, ToolParams )
	PARAM2TEXT( PlungeRate, ToolParams )
	PARAM2TEXT( SurfaceSpeed, ToolParams )
	PARAM2TEXT( FeedChipLoad, ToolParams )
	PARAM2TEXT( PlungeChipLoad, ToolParams )
}

//Calculate the speed, feed and plunge values on the toolpage (lower part).
//Also update the associated dialog textboxes with the gcnew values.
void MachParamsDlg::CalcControlsSPF()
{
	CalcControlSpeedFeed();
	CalcControlSurfaceSpeed();
	CalcControlFeedChipLoad();
	CalcControlPlungeChipLoad();
}

