
//
// @filename:   MachParamsDlg.cpp
// @fileauthor: Kevin Guerra

#include "MachParamsDlg.h"
#include "MachParamsDlgMacros.h"
#include "RapidAreaDlg.h"
#include "SelectUtils.h"
#include "StartPointParamsDlg.h"
#include "CuttingParamsMarginsDlg.h"
#include "CuttingParamsPointsDlg.h"

using namespace New5Axis;

#ifdef MessageBox
#undef MessageBox
#endif

enum CutAreaButton { None, Margins, SetPoints };

#define PROCESS_CUTTING_AREA_VALUES \
	m_numNumberOfCuts->Value = 1; \
	m_numNumberOfCuts->Enabled = false; \
	m_btnCuttingAreaSelect->Enabled = true; \
	switch( m_scCuttingParams->CuttingArea ) \
	{ \
	case CuttingAreaTypes::CUT_AREA_AVOID_CUTS: \
		m_btnCuttingAreaSelect->Enabled = false; \
		m_btnCuttingAreaSelect->Text = m_lblBtnCuttingAreaSelect; \
		m_btnCuttingAreaSelect->Tag = (int)CutAreaButton::None; \
		break; \
	case CuttingAreaTypes::CUT_AREA_EXACT_SURFACE: \
		m_btnCuttingAreaSelect->Text = m_lblBtnCuttingAreaMargins; \
		m_btnCuttingAreaSelect->Tag = (int)CutAreaButton::Margins; \
		break; \
	case CuttingAreaTypes::CUT_AREA_NUMBER_OF_CUTS: \
		m_btnCuttingAreaSelect->Text = m_lblBtnCuttingAreaMargins; \
		if( m_scCuttingParams->NumberOfCuts < 1 ) m_scCuttingParams->NumberOfCuts = 1; \
		m_numNumberOfCuts->Value = m_scCuttingParams->NumberOfCuts; \
		m_numNumberOfCuts->Enabled = true; \
		m_btnCuttingAreaSelect->Tag = (int)CutAreaButton::Margins; \
		break; \
	case CuttingAreaTypes::CUT_AREA_LIMIT_CUTS: \
		m_btnCuttingAreaSelect->Text = m_lblBtnCuttingAreaSetPoints; \
		m_btnCuttingAreaSelect->Tag = (int)CutAreaButton::SetPoints; \
		break; \
	}

void MachParamsDlg::InitializeTabCutControl()
{
	GET_COMBOBOX_RESOURCES2( RetraceType )
	GET_COMBOBOX_RESOURCES6( CuttingMode )
	GET_COMBOBOX_RESOURCES2( StepType )
	GET_COMBOBOX_RESOURCES2( CutDirection )
	GET_COMBOBOX_RESOURCES3( CutOrder )

	PARAM2COMBOBOX3( StepType, CuttingParams )
	PARAM2COMBOBOX2( CutDirection, CuttingParams )
	PARAM2COMBOBOX2( CutOrder, CuttingParams )
	PARAM2COMBOBOX3( RetraceType, CuttingParams )

	PARAM2TEXT( RapidDistance, CuttingParams )
	PARAM2TEXT( RapidClearance, CuttingParams )
	PARAM2TEXT( PlungeClearance, CuttingParams )
	PARAM2TEXT( AirMoveSafetyClearance, CuttingParams )
	PARAM2TEXT( PlanarAngle, CuttingParams )
	PARAM2TEXT( SurfaceEdgeMergeDistance, CuttingParams )
	PARAM2TEXT( Increment, CuttingParams )
	PARAM2TEXT( DriveSurfacesStockToLeave, CuttingParams )
	PARAM2TEXT( SurfaceTolerance, CuttingParams )
	PARAM2TEXT( AxialShift, CuttingParams ) 

	PARAM2CHECKBOX( ReverseToolPath, CuttingParams )
	PARAM2CHECKBOX( SpiralFlag, CuttingParams )
	PARAM2CHECKBOX( UseBoundary, CuttingParams )
	PARAM2CHECKBOX( EnforceCuttingDirection, CuttingParams )

	if (m_scCuttingParams->StepType == StepTypes::STEP_TYPE_INCREMENT)
		m_lblIncrement->Text = (m_ResourceManager->GetString("IDS_LABEL_INCR"));
	else 
		m_lblIncrement->Text = (m_ResourceManager->GetString("IDS_LABEL_SCALLOP"));

	m_lblBtnCuttingAreaSelect = m_ResourceManager->GetString("IDS_LABEL_BTN_CUTTING_AREA_SELECT");
	m_lblBtnCuttingAreaMargins = m_ResourceManager->GetString("IDS_LABEL_BTN_CUTTING_AREA_MARGINS");
	m_lblBtnCuttingAreaSetPoints = m_ResourceManager->GetString("IDS_LABEL_BTN_CUTTING_AREA_SET_POINTS");

	m_cbCutDirection->Enabled = m_scCuttingParams->RetraceType == RetraceTypes::RETRACE_TYPE_ZIG;
	m_chkSpiralFlag->Enabled = m_scCuttingParams->RetraceType == RetraceTypes::RETRACE_TYPE_ZIG;
	m_txtPlanarAngle->Enabled = m_scCuttingParams->CuttingMode == CuttingModeTypes::CUT_MODE_PARALLEL;

	if (m_scMachParams->IsSwarfMilling())
	{
		ComboBox::ObjectCollection^ cutModeItems = m_cbCuttingMode->Items;
		//remove the top three options and the last one
		cutModeItems->RemoveAt(0);
		cutModeItems->RemoveAt(0);
		cutModeItems->RemoveAt(0);
		cutModeItems->RemoveAt(2);
		//m_cbCuttingMode_SelectedIndexChanged(this, gcnew System::EventArgs());
		RepositionControlsForSwarfOperation();
		if( m_scCuttingParams->CuttingMode == CuttingModeTypes::CUT_MODE_BETWEEN_CURVES )
			m_cbCuttingMode->SelectedIndex = 1;
		else
			m_cbCuttingMode->SelectedIndex = 0;
	}
	else
	{
		PARAM2COMBOBOX2( CuttingMode, CuttingParams )
	}

	switch( m_scCuttingParams->CuttingMode )
	{
	case CuttingModeTypes::CUT_MODE_PARALLEL: 
	case CuttingModeTypes::CUT_MODE_ALONG_CURVE:
	case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
		GET_COMBOBOX_RESOURCES3( CuttingArea ) //disable the 4th option
		break;
	case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
	case CuttingModeTypes::CUT_MODE_PARALLEL_CURVE:
	case CuttingModeTypes::CUT_MODE_PARALLEL_SURFACE:
		GET_COMBOBOX_RESOURCES4( CuttingArea )
		break;
	}
	PARAM2COMBOBOX2( CuttingArea, CuttingParams )
	PROCESS_CUTTING_AREA_VALUES

	if (m_scMachParams->GetOperationMode() == OperationMode::EDIT_PARAMETERS)
	{
		m_tabCutControlParams->Enabled = false;
	}

	RapidAreaParams ^ m_scRapidAreaParams = m_scMachParams->GetRapidAreaParams();
	RapidAreaDlg raDlg;
	raDlg.m_pParentDlg = this;
	raDlg.m_ResourceManager = m_ResourceManager;
	raDlg.SetParams(m_scRapidAreaParams);
	raDlg.InitControls();
	m_btnRapidDistance->Text = raDlg.GetRapidAreaTypeTextString(m_scRapidAreaParams);

	StartPointParams ^ spParams = m_scMachParams->GetStartPointParams();
	m_chkStartPoint->Checked = spParams->Use;
	m_btnStartPoint->Enabled = m_chkStartPoint->Checked;

	switch( m_scCuttingParams->CuttingMode )
	{
	case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
	case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
		m_scCuttingParams->AddToolRadiusToMarginPossiblyChangedByUser = false;
		break;
	default:
		m_scCuttingParams->AddToolRadiusToMarginPossiblyChangedByUser = m_scCuttingParams->AddToolRadiusToMargin;
		break;
	}
}

DEFINE_TXTCONTROL_HANDLERS( Increment, CuttingParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( DriveSurfacesStockToLeave, CuttingParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( SurfaceTolerance, CuttingParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( AxialShift, CuttingParams, Double )}

DEFINE_TXTCONTROL_HANDLERS( AirMoveSafetyClearance, CuttingParams, Double )}

DEFINE_TXTCONTROL_HANDLERS( PlanarAngle, CuttingParams, Double )}
DEFINE_TXTCONTROL_HANDLERS( SurfaceEdgeMergeDistance, CuttingParams, Double )}

DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( RapidClearance, CuttingParams, Double )
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( RapidClearance, Double )
}

DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( RapidDistance, CuttingParams, Double )
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( RapidDistance, Double )
	if (m_scCuttingParams->RapidDistance < 0) 
	{
		MessageBox::Show(m_ResourceManager->GetString("IDS_VALIDATION_GTE_ZERO"), "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		m_scCuttingParams->RapidDistance = 0;
		m_txtRapidDistance->Text = (m_scCuttingParams->RapidDistance.ToString()->Format(m_formatDblStr, (m_scCuttingParams->RapidDistance)));
	}

	if (m_scCuttingParams->RapidDistance < m_scCuttingParams->PlungeClearance) 
	{
		MessageBox::Show(m_ResourceManager->GetString("IDS_RAPID_VALIDATION"), "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		m_scCuttingParams->RapidDistance = m_scCuttingParams->PlungeClearance;
		m_txtRapidDistance->Text = (m_scCuttingParams->RapidDistance.ToString()->Format(m_formatDblStr, (m_scCuttingParams->RapidDistance)));
	}
}

DEFINE_TXTCONTROL_HANDLER_TEXTCHANGED( PlungeClearance, CuttingParams, Double )
}
DEFINE_TXTCONTROL_HANDLERS_KEYPRESS_LEAVE( PlungeClearance, Double )
	if (m_scCuttingParams->PlungeClearance < 0) 
	{
		MessageBox::Show(m_ResourceManager->GetString("IDS_VALIDATION_GTE_ZERO"), "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		m_scCuttingParams->PlungeClearance = 0;
		m_txtPlungeClearance->Text = (m_scCuttingParams->PlungeClearance.ToString()->Format(m_formatDblStr, (m_scCuttingParams->PlungeClearance)));
	}

	if (m_scCuttingParams->RapidDistance < m_scCuttingParams->PlungeClearance) 
	{
		MessageBox::Show(m_ResourceManager->GetString("IDS_RAPID_VALIDATION"), "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		m_scCuttingParams->PlungeClearance = m_scCuttingParams->RapidDistance;
		m_txtPlungeClearance->Text = (m_scCuttingParams->PlungeClearance.ToString()->Format(m_formatDblStr, (m_scCuttingParams->PlungeClearance)));
	}
}


DEFINE_COMBOBOX_CONTROL_HANDLER( CutDirection, CuttingParams )}
DEFINE_COMBOBOX_CONTROL_HANDLER( CutOrder, CuttingParams )}

DEFINE_CHECKBOX_CONTROL_HANDLER( ReverseToolPath, CuttingParams )}
DEFINE_CHECKBOX_CONTROL_HANDLER( UseBoundary, CuttingParams )}

static CuttingAreaTypes stCurrentCuttingArea = CuttingAreaTypes::CUT_AREA_AVOID_CUTS;
//DEFINE_COMBOBOX_CONTROL_HANDLER( CuttingMode, CuttingParams )
DEFINE_CONTROL_HANDLER( m_cbCuttingMode_SelectedIndexChanged )
{
	UICuttingModeTypes uiLastSelection = UITypes::DB2UICuttingModeTypes(m_scCuttingParams->CuttingMode);
	int idx = m_cbCuttingMode->SelectedIndex;
	if (m_scMachParams->IsSwarfMilling())
	{
		idx = idx==0?3:4;
		m_scCuttingParams->CuttingMode = UITypes::UI2DBCuttingModeTypes(static_cast<UICuttingModeTypes>(idx));
	}else{
		m_scCuttingParams->CuttingMode = UITypes::UI2DBCuttingModeTypes(static_cast<UICuttingModeTypes>(idx));
	}

	//UICuttingModeTypes cutMode;
	bool enabled = true;
	if (m_scMachParams->IsSwarfMilling())
	{
		//cutMode = UITypes::UI2DBCuttingModeTypes(static_cast<UICuttingModeTypes>(uiCuttingMode));
		//cutMode = uiCuttingMode;
		m_cbToolVectorCut->SelectedIndex = static_cast<int>(UITypes::DB2UISwarfToolVectorCutTypes(m_scVectorParams->ToolVectorCut));
	}
	else
	{
		UICuttingModeTypes uiCuttingMode = static_cast<UICuttingModeTypes>(idx);
		UICutToolVectorCutTypes vecCtrlType = UITypes::DB2UICutToolVectorCutTypes(m_scVectorParams->ToolVectorCut);
		if (uiCuttingMode == UICuttingModeTypes::UI_CUT_MODE_BETWEEN_SURFACES)
		{
			if( ReserveGougeCheckGroupForMorphOperation() )
			{
				enabled = false;
				vecCtrlType = UICutToolVectorCutTypes::UI_CUT_FOLLOW_ISO_SURF;
				System::Object^ __mcTemp = gcnew System::Object;
				__mcTemp = m_ResourceManager->GetString("m_cbToolVectorCut.Items6");
				m_cbToolVectorCut->Items->Add(__mcTemp);
				if(!m_UpdateOnly)
				{
					//m_chkBoxGouChkTool->Checked = true;
					//m_comBoxGouChkStgy->SelectedIndex = static_cast<int>(UIGougeStgy::UI_AVOID_MOTION);
				}
			}
			else
			{
				String^ msg = gcnew String("Unable to occupy a check group for morph surfaces. Please free a group and retry" );
				MessageBox::Show(msg, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
				m_cbCuttingMode->SelectedIndex = static_cast<int>(uiLastSelection);
				return;
			}
		}
		else
		{
			FreeGougeCheckGroupForMorphOperation();
			//need to handle the case where previously the above cut mode was
			//chosen and we need to delete it from the vector Ctrl types (ugh!)
			if (m_cbToolVectorCut->Items->Count > 4)
			{
				vecCtrlType = UICutToolVectorCutTypes::UI_CUT_NORMAL_TO_SURF;
				m_cbToolVectorCut->Items->RemoveAt(4);
			}
		}
		//cutMode = uiCuttingMode;
		m_cbToolVectorCut->SelectedIndex = static_cast<int>(vecCtrlType);
	}
	m_cbToolVectorCut->Enabled = enabled;

	m_txtPlanarAngle->Enabled = m_scCuttingParams->CuttingMode == CuttingModeTypes::CUT_MODE_PARALLEL;

	static CuttingAreaTypes CuttingAreaTemp;
	if( m_InitializingDialog == false )
	{
		m_scCuttingParams->CuttingArea = stCurrentCuttingArea;
		switch( m_scCuttingParams->CuttingMode )
		{
		case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
			GET_COMBOBOX_RESOURCES3( CuttingArea ) //disable the 4th option
			m_scCuttingParams->CuttingArea = CuttingAreaTypes::CUT_AREA_EXACT_SURFACE;
			break;
		case CuttingModeTypes::CUT_MODE_PARALLEL: 
		case CuttingModeTypes::CUT_MODE_ALONG_CURVE:
			GET_COMBOBOX_RESOURCES3( CuttingArea ) //disable the 4th option
			if(m_scCuttingParams->CuttingArea == CuttingAreaTypes::CUT_AREA_NUMBER_OF_CUTS)
			{
				CuttingAreaTemp = CuttingAreaTypes::CUT_AREA_NUMBER_OF_CUTS;
				m_scCuttingParams->CuttingArea = CuttingAreaTypes::CUT_AREA_AVOID_CUTS;
			}
			else
			{
				CuttingAreaTemp = CuttingAreaTypes::CUT_AREA_AVOID_CUTS;
			}
			break;
		case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
		case CuttingModeTypes::CUT_MODE_PARALLEL_CURVE:
		case CuttingModeTypes::CUT_MODE_PARALLEL_SURFACE:
			GET_COMBOBOX_RESOURCES4( CuttingArea )
			if( CuttingAreaTemp == CuttingAreaTypes::CUT_AREA_NUMBER_OF_CUTS )
				m_scCuttingParams->CuttingArea = CuttingAreaTypes::CUT_AREA_NUMBER_OF_CUTS;
			break;
		}
		PARAM2COMBOBOX2( CuttingArea, CuttingParams )
		PROCESS_CUTTING_AREA_VALUES
	}
	if( m_InitializingDialog == true )
		return;
	switch( m_scCuttingParams->CuttingMode )
	{
	case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
	case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
		m_scCuttingParams->AddToolRadiusToMargin = true;
		break;
	default:
		m_scCuttingParams->AddToolRadiusToMarginPossiblyChangedByUser = m_scCuttingParams->AddToolRadiusToMargin;
		break;
	}
}
DEFINE_CONTROL_HANDLER( m_cbCuttingArea_SelectedIndexChanged )
{
	if( m_InitializingDialog == true )
		return;
	//if( !m_cbCuttingArea->Focused )
	//	return;
	m_scCuttingParams->CuttingArea = UITypes::UI2DBCuttingAreaTypes(static_cast<UICuttingAreaTypes>(m_cbCuttingArea->SelectedIndex));
	if( m_cbCuttingArea->Focused )
		stCurrentCuttingArea = m_scCuttingParams->CuttingArea;
	PROCESS_CUTTING_AREA_VALUES
}

DEFINE_CONTROL_HANDLER( m_numNumberOfCuts_Changed )
{
	System::Decimal value = m_numNumberOfCuts->Value;
	m_scCuttingParams->NumberOfCuts = System::Decimal::ToInt32(value);
}

DEFINE_COMBOBOX_CONTROL_HANDLER4( StepType, CuttingParams, StepTypes )

	UIStepTypes uiStepType = static_cast<UIStepTypes>(m_cbStepType->SelectedIndex);

	double value = 0.0;

	if (uiStepType == UIStepTypes::UI_STEP_TYPE_INCREMENT)
	{
		value = m_scCuttingParams->Increment;
		m_lblIncrement->Text = (m_ResourceManager->GetString("IDS_LABEL_INCR"));
	}
	else
	{
		value = m_scCuttingParams->Increment;
		m_lblIncrement->Text = (m_ResourceManager->GetString("IDS_LABEL_SCALLOP"));
	}

	m_txtIncrement->Text = (value.ToString()->Format(m_formatDblStr, (value)));
}

DEFINE_COMBOBOX_CONTROL_HANDLER4( RetraceType, CuttingParams, RetraceTypes )

	UIRetraceTypes retraceType = static_cast<UIRetraceTypes>(m_cbRetraceType->SelectedIndex);

	m_cbCutDirection->Enabled = m_scCuttingParams->RetraceType == RetraceTypes::RETRACE_TYPE_ZIG;
	m_chkSpiralFlag->Enabled = m_scCuttingParams->RetraceType == RetraceTypes::RETRACE_TYPE_ZIG;
	m_chkEnforceCuttingDirection->Enabled = m_scCuttingParams->RetraceType == RetraceTypes::RETRACE_TYPE_ZIG;
	PARAM2CHECKBOX( SpiralFlag, CuttingParams )

}

DEFINE_CHECKBOX_CONTROL_HANDLER( SpiralFlag, CuttingParams )

	m_scCuttingParams->SpiralFlag = m_chkSpiralFlag->Checked;

}

DEFINE_CHECKBOX_CONTROL_HANDLER( EnforceCuttingDirection, CuttingParams )
	m_scCuttingParams->EnforceCuttingDirection = m_chkEnforceCuttingDirection->Checked;
}

DEFINE_CONTROL_HANDLER( m_btnRapidDistance_Click )
{
	double planeDist = 0.0;

	this->Hide();

	planeDist = SelectRapidPlaneDist();
	m_scRapidAreaParams->Distance = planeDist;
	m_scCuttingParams->RapidClearance = planeDist;

	m_txtRapidClearance->Text = (planeDist.ToString()->Format(m_formatDblStr, (planeDist)));

	this->Show();
}

DEFINE_CONTROL_HANDLER( m_btnSelectRapidArea_Click )
{
	m_scRapidAreaParams->Distance = m_scCuttingParams->RapidClearance;

	switch( m_scMachParams->GetOperationMode() )
	{
	case OperationMode::REGEN_WITH_SAVED_GEOM:
	case OperationMode::REGEN_WITH_RESEL_GEOM:
	case OperationMode::EDIT_PARAMETERS:
		break;
	case OperationMode::CREATE:
	case OperationMode::UPDATE:
		{
			int rotAxis = 0;
			double rotAngle = 0.0;
			scNCOp_GetRotaryAxis(&rotAxis, &rotAngle);
			//int axis1 = (int)m_TemplateWrapper->GetRotaryAxis();
			int axis2 = (int)m_scMachParams->GetRotaryAxis();
			//always use the latest axis selected by the user, in case the user changed the global rotary axis
			if( rotAxis != axis2 )
			{
				m_scMachParams->SetRotaryAxis((RotaryAxis)rotAxis);
				m_scRapidAreaParams->SetCylinderRotaryAxis(rotAxis);
			}
		}
		break;
	}

	RapidAreaParams ^ raParamsTemp = gcnew RapidAreaParams(m_scRapidAreaParams);

	//create new dialog and fill its clearance params
	RapidAreaDlg raDlg;
	raDlg.m_pParentDlg = this;
	raDlg.m_ResourceManager = m_ResourceManager;
	raDlg.SetDistanceString(m_txtRapidClearance->Text);
	raDlg.SetParams(raParamsTemp);

	System::Windows::Forms::DialogResult dlgRes = raDlg.ShowDialog(this);
	if (dlgRes == System::Windows::Forms::DialogResult::OK)
	{
		double value;
		raParamsTemp = raDlg.GetParams();
		if( !(*m_scRapidAreaParams == *raParamsTemp) )
		{
			m_scRapidAreaParams = raParamsTemp;
			m_scMachParams->SetRapidAreaParams(raParamsTemp);
			m_scCuttingParams->RapidClearance = raParamsTemp->Distance;
		}
		if( raDlg.DistanceStringChanged() )
		{
			value = raParamsTemp->GetRapidPlane();
			m_txtRapidClearance->Text = (value.ToString()->Format(m_formatDblStr, (value)));
		}
		m_btnRapidDistance->Text = raDlg.GetRapidAreaTypeText();
	}
}

DEFINE_CONTROL_HANDLER( m_btnStartPoint_Click )
{
	//create new dialog and fill its clearance params
	StartPointParamsDlg spDlg;
	spDlg.m_pParentDlg = this;

	switch( m_scStartPointParams->SelectionType )
	{
	case StartPointSelectionTypes::SP_USE_POSITION:
		spDlg.m_rbSetPointByPosition->Checked = true;
		break;
	case StartPointSelectionTypes::SP_USE_SURFACE_NORMAL:
		spDlg.m_rbSetPointBySurfaceNormalDirection->Checked = true;
		break;
	}
	switch( m_scStartPointParams->OffsetType )
	{
	case StartPointOffsetTypes::SP_SHIFT_BY_VAL:
		spDlg.m_rbSetPointApplicableShiftByValue->Checked = true;
		break;
	case StartPointOffsetTypes::SP_ROTATE_BY_DEG:
		spDlg.m_rbSetPointApplicableRotateByDegree->Checked = true;
		break;
	case StartPointOffsetTypes::SP_MINIMIZE_SURF_NORM_CHANGE:
		spDlg.m_rbSetPointApplicableMinimizeSurfaceNormalChange->Checked = true;
		break;
	}
	double value = 0.0;

	value = m_scStartPointParams->StartPositionX;
	spDlg.m_txtX->Text = (value.ToString()->Format(m_formatDblStr, (value)));
	value = m_scStartPointParams->StartPositionY;
	spDlg.m_txtY->Text = (value.ToString()->Format(m_formatDblStr, (value)));
	value = m_scStartPointParams->StartPositionZ;
	spDlg.m_txtZ->Text = (value.ToString()->Format(m_formatDblStr, (value)));
	value = m_scStartPointParams->RotationAngle;
	spDlg.m_txtSetPointApplicableRotateByDegree->Text = (value.ToString()->Format(m_formatDblStr, (value)));
	value = m_scStartPointParams->ShiftValue;
	spDlg.m_txtSetPointApplicableShiftByValue->Text = (value.ToString()->Format(m_formatDblStr, (value)));

	System::Windows::Forms::DialogResult dlgRes = spDlg.ShowDialog(this);
	if (dlgRes == System::Windows::Forms::DialogResult::OK)
	{
		if( spDlg.m_rbSetPointByPosition->Checked )
			m_scStartPointParams->SelectionType =  StartPointSelectionTypes::SP_USE_POSITION;
		else
			m_scStartPointParams->SelectionType =  StartPointSelectionTypes::SP_USE_SURFACE_NORMAL;

		if( spDlg.m_rbSetPointApplicableShiftByValue->Checked )
			m_scStartPointParams->OffsetType =  StartPointOffsetTypes::SP_SHIFT_BY_VAL;
		else if( spDlg.m_rbSetPointApplicableRotateByDegree->Checked )
			m_scStartPointParams->OffsetType =  StartPointOffsetTypes::SP_ROTATE_BY_DEG;
		else
			m_scStartPointParams->OffsetType =  StartPointOffsetTypes::SP_MINIMIZE_SURF_NORM_CHANGE;

		m_scStartPointParams->StartPositionX = GetDoubleFromTextBox(spDlg.m_txtX);
		m_scStartPointParams->StartPositionY = GetDoubleFromTextBox(spDlg.m_txtY);
		m_scStartPointParams->StartPositionZ = GetDoubleFromTextBox(spDlg.m_txtZ);
		m_scStartPointParams->RotationAngle = GetDoubleFromTextBox(spDlg.m_txtSetPointApplicableRotateByDegree);
		m_scStartPointParams->ShiftValue = GetDoubleFromTextBox(spDlg.m_txtSetPointApplicableShiftByValue);
	}
}

#define PARAM2TEXT( param, ctl ) \
	value = m_scCuttingParams->param; \
	dlg.ctl->Text = (value.ToString()->Format(m_formatDblStr, (value)));

#define TEXT2PARAM( param, ctl ) \
	m_scCuttingParams->param = GetDoubleFromTextBox(dlg.ctl);

DEFINE_CONTROL_HANDLER( m_btnCuttingAreaSelect_Click )
{
	double value = 0.0;
	System::Windows::Forms::DialogResult dlgRes = System::Windows::Forms::DialogResult::Cancel;
	switch( (CutAreaButton)(int)m_btnCuttingAreaSelect->Tag )
	{
	case CutAreaButton::Margins:
		{
			CuttingParamsMarginsDlg dlg;
			dlg.m_pParentDlg = this;

			switch( m_scCuttingParams->CuttingMode )
			{
			case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
			case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
				PARAM2TEXT( StartMargin, m_txtStartMargin )
				PARAM2TEXT( EndMargin, m_txtEndMargin )
				break;
			case CuttingModeTypes::CUT_MODE_PARALLEL_CURVE:
			case CuttingModeTypes::CUT_MODE_PARALLEL_SURFACE:
				PARAM2TEXT( StartMargin, m_txtStartMargin )
				dlg.m_txtEndMargin->Enabled = false;
				break;
			case CuttingModeTypes::CUT_MODE_PARALLEL: 
			case CuttingModeTypes::CUT_MODE_ALONG_CURVE:
				dlg.m_txtStartMargin->Enabled = false;
				dlg.m_txtEndMargin->Enabled = false;
				break;
			}

			PARAM2TEXT( AdditionalMargin, m_txtAdditionalMargin )
			dlg.m_chkAddToolRadius->Checked = m_scCuttingParams->AddToolRadiusToMargin;

			dlgRes = dlg.ShowDialog(this);
			if (dlgRes == System::Windows::Forms::DialogResult::OK)
			{
				TEXT2PARAM( StartMargin, m_txtStartMargin )
				TEXT2PARAM( EndMargin, m_txtEndMargin )
				TEXT2PARAM( AdditionalMargin, m_txtAdditionalMargin )
				m_scCuttingParams->AddToolRadiusToMargin = dlg.m_chkAddToolRadius->Checked;
				switch( m_scCuttingParams->CuttingMode )
				{
				case CuttingModeTypes::CUT_MODE_BETWEEN_SURFACES:
				case CuttingModeTypes::CUT_MODE_BETWEEN_CURVES:
					break;
				default:
					m_scCuttingParams->AddToolRadiusToMarginPossiblyChangedByUser = m_scCuttingParams->AddToolRadiusToMargin;
					break;
				}
			}
		}
		break;
	case CutAreaButton::SetPoints:
		{
			CuttingParamsPointsDlg dlg;
			dlg.m_pParentDlg = this;

			PARAM2TEXT( Point1X, m_txtX1 )
			PARAM2TEXT( Point1Y, m_txtY1 )
			PARAM2TEXT( Point1Z, m_txtZ1 )
			PARAM2TEXT( Point2X, m_txtX2 )
			PARAM2TEXT( Point2Y, m_txtY2 )
			PARAM2TEXT( Point2Z, m_txtZ2 )

			dlgRes = dlg.ShowDialog(this);
			if (dlgRes == System::Windows::Forms::DialogResult::OK)
			{
				TEXT2PARAM( Point1X, m_txtX1 )
				TEXT2PARAM( Point1Y, m_txtY1 )
				TEXT2PARAM( Point1Z, m_txtZ1 )
				TEXT2PARAM( Point2X, m_txtX2 )
				TEXT2PARAM( Point2Y, m_txtY2 )
				TEXT2PARAM( Point2Z, m_txtZ2 )
			}
		}
		break;
	}
}
