/*******************************************************************************
*  This file is part of the MapShot plugin
*  Copyright (C) 2009,2012 Roman Tsisyk <roman@tsisyk.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*******************************************************************************/

#include "StdAfx.h"
#include "CmdMakeShot.h"

GrymCore::ICommandActionPtr CCmdMakeShot::CreateInstance(
	CMapShotData* data)
{
    ATL::CComObject<CCmdMakeShot> *obj;
    ATLVERIFY(S_OK == ATL::CComObject<CCmdMakeShot>::CreateInstance(&obj));
	GrymCore::ICommandActionPtr rv = obj;
	ATLASSERT(NULL != rv);
	obj->m_data = data;
	ATLASSERT(NULL != obj->m_data);

	return rv;
}

CCmdMakeShot::CCmdMakeShot() 
	: 
	m_pCmdMakeShotPlacementCode(_T("MapShot:0130")),
	m_pCmdMakeShotTag(_T("2GIS.MapShot.CmdMakeShot")),
	m_data(NULL),
	m_bChecked(false),
	m_bToolAdded(false),
	m_pStartMapPoint(NULL), m_pEndMapPoint(NULL), m_pCurrentDownMapPoint(NULL),
	m_pPen(NULL),
	m_pBrush(NULL),
	m_nSelectionResizeDelta(6),
	m_selectionMode(StartSelection),
	m_dlg(NULL),
	m_hCurrentCursor(NULL),
	m_nCurrentCursorID(0),
	m_pIcon(NULL)
{

}

CCmdMakeShot::~CCmdMakeShot()
{
	m_pStartMapPoint = NULL;
	m_pEndMapPoint = NULL;
	m_pCurrentDownMapPoint = NULL;
	if(m_hCurrentCursor)
		::DeleteObject(m_hCurrentCursor);
}

/*
	Methods of GrymCore::IControlAccelerator
*/
STDMETHODIMP CCmdMakeShot::get_Accelerator(LONG *pVal)
{
	if (!pVal) return E_POINTER;
	*pVal = 0;
	return S_FALSE;
}


/*
	Methods of GrymCore::IControlPlacement
*/
STDMETHODIMP CCmdMakeShot::get_PlacementCode(BSTR *pVal)
{
	if (!pVal)
		return E_POINTER;

	*pVal = m_pCmdMakeShotPlacementCode.copy();
	return S_OK;
}



/*
	Methods of GrymCore::IControlAppearance
*/
STDMETHODIMP CCmdMakeShot::get_Tag(BSTR *pVal)
{
	if(!pVal)
		return E_POINTER;

	*pVal = m_pCmdMakeShotTag.copy();
	return S_OK;
}

STDMETHODIMP CCmdMakeShot::get_Caption(BSTR *pVal)
{
	if(!pVal) return E_POINTER;
	
	*pVal = m_data->LoadString(IDS_CMDMAKESHOT);
	return S_OK;
}

STDMETHODIMP CCmdMakeShot::get_Description(BSTR *pVal)
{
	if(!pVal)
		return E_POINTER;
	
	*pVal = m_data->LoadString(IDS_CMDMAKESHOT_DESC);
	return S_OK;
}

STDMETHODIMP CCmdMakeShot::get_Icon(IUnknown ** pVal)
{
	if(!pVal)
		return E_POINTER;

	if(NULL == m_pIcon)
	{
		m_pIcon = m_data->LoadRaster(IDB_CMDMAKESHOT);
		ATLASSERT(NULL != m_pIcon);
	}

	if(NULL == m_pIcon)
	{
		*pVal = NULL;
		return S_FALSE;
	}
	else
	{
		*pVal = IUnknownPtr(m_pIcon).Detach();
		return S_OK;
	}
}


/*
	Methods of GrymCore::ICommandState
*/
STDMETHODIMP CCmdMakeShot::get_Enabled(VARIANT_BOOL *pVal)
{
	if (!pVal)
		return E_POINTER;

	*pVal = VARIANT_TRUE;
	return S_OK;
}

STDMETHODIMP CCmdMakeShot::get_Checked(VARIANT_BOOL *pVal)
{
	if (!pVal)
		return E_POINTER;

	*pVal = (m_bChecked) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


/*
	Methods of GrymCore::IControlState
*/
STDMETHODIMP CCmdMakeShot::get_Available(VARIANT_BOOL *pVal)
{
	if (!pVal)
		return E_POINTER;

	*pVal = VARIANT_TRUE;
	return S_OK;
}


/*
	Methods of GrymCore::ITool
*/
STDMETHODIMP CCmdMakeShot::get_Cursor(OLE_HANDLE *pCursor)
{
	if ( !pCursor )
		return E_POINTER;

	UINT nNewCursorID = IDC_MAPHAND;

	switch(m_selectionMode)
	{
		case SelectionCanMove:
		case SelectionMove: nNewCursorID = IDC_SELECTIONMOVE; break;
		case SelectedCanResizeTop:
		case SelectedCanResizeBottom:
		case ResizeTop:
		case ResizeBottom: nNewCursorID = IDC_RESIZEV; break;
		case SelectedCanResizeLeft:
		case SelectedCanResizeRight:
		case ResizeLeft:
		case ResizeRight: nNewCursorID = IDC_RESIZEH; break;
		case StartSelection:
		case ResizeCorner: nNewCursorID = IDC_RESIZECORNER; break;
	}

	if(nNewCursorID != m_nCurrentCursorID)
	{
		if(NULL != m_hCurrentCursor)
			::DeleteObject(m_hCurrentCursor);

		m_hCurrentCursor = m_data->LoadCursor(nNewCursorID);
		m_nCurrentCursorID = nNewCursorID;
	}

	*pCursor = reinterpret_cast<OLE_HANDLE>(m_hCurrentCursor);

	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_Activate(VARIANT_BOOL bActivate, VARIANT_BOOL *bpRet)
{
	if ( !bpRet )
		return E_POINTER;

	*bpRet = VARIANT_TRUE;

	if(bActivate)
	{
		m_bChecked = true;
		// Dialog initialization
		m_dlg = new CCmdMakeShotDlg(this);
		ATLASSERT(NULL != m_dlg);

		// GDI+ objects initialization
		m_pPen = new Gdiplus::Pen(Gdiplus::Color(240,23,0,0), 2.0f); 
		ATLASSERT(NULL != m_pPen);
		m_pPen->SetDashStyle(Gdiplus::DashStyleDash);
		
		m_pBrush = new Gdiplus::SolidBrush(Gdiplus::Color(100, 20, 20, 10));
		ATLASSERT(NULL != m_pBrush);
	}
	else
	{
		m_bChecked = false;

		m_selectionMode = StartSelection;
		m_data->GetMap()->Invalidate( VARIANT_TRUE );

		// Dialog destroying
		if(m_dlg->IsWindow())
		{
			m_dlg->DestroyWindow();
		}

		delete m_dlg;
		m_dlg = NULL;

		// GDI+ objects destroying
		delete m_pPen;
		m_pPen = NULL;
		
		delete m_pBrush;
		m_pBrush = NULL;
	}

	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_OnKeyDown( LONG /*nChar*/, LONG /*nShift*/ , VARIANT_BOOL * /*pVal*/)
{
	return S_FALSE;
}

STDMETHODIMP CCmdMakeShot::raw_OnKeyUp( LONG /*nChar*/, LONG /*nShift*/ , VARIANT_BOOL * /*pVal*/)
{
	return S_FALSE;
}

STDMETHODIMP CCmdMakeShot::raw_OnMouseDown( enum GrymCore::MouseButton mButton, LONG /*nShift*/, LONG  x, LONG  y, VARIANT_BOOL *pVal)
{
	*pVal = VARIANT_FALSE;

	m_pCurrentDownMapPoint = m_data->DeviceToMap(x, y);;

	if(GrymCore::MouseButtonLeft == mButton)
	{
		// end selection
		if(m_selectionMode == ResizeCorner)
		{
			*pVal = VARIANT_TRUE;	

			m_pEndMapPoint = m_pCurrentDownMapPoint;
			if(std::fabs(m_pStartMapPoint->X - m_pEndMapPoint->X) > m_data->MinMapWidth &&
			   std::fabs(m_pStartMapPoint->Y - m_pEndMapPoint->Y) > m_data->MinMapHeight )
			{
				GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(m_pStartMapPoint, m_pEndMapPoint);
				m_pStartMapPoint = m_data->GetFactory()->CreateMapPoint(pStartEndMapRect->MinX, pStartEndMapRect->MinY);
				m_pEndMapPoint = m_data->GetFactory()->CreateMapPoint(pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

				m_selectionMode = Selected;

				UpdateDialog();
			}
		}

		if(m_selectionMode >= ResizeLeft && m_selectionMode <= ResizeBottom)
		{
			*pVal = VARIANT_TRUE;
			if(std::fabs(m_pStartMapPoint->X - m_pEndMapPoint->X) > m_data->MinMapWidth &&
				std::fabs(m_pStartMapPoint->Y - m_pEndMapPoint->Y) > m_data->MinMapHeight )
			{
				GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(
					m_pStartMapPoint, m_pEndMapPoint);
				m_pStartMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MinX, pStartEndMapRect->MinY);
				m_pEndMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

				m_selectionMode = Selected;

				UpdateDialog();
			}
		}

		if(m_selectionMode == SelectionMove)
		{
			m_selectionMode = Selected;
		}

		if(m_selectionMode == SelectionCanMove)
		{
			m_selectionMode = SelectionMove;
			*pVal = VARIANT_TRUE;
		}
		
		if(m_selectionMode >= SelectedCanResizeLeft && 
			m_selectionMode <= SelectedCanResizeBottom)
		{
			m_selectionMode = SelectionModeEnum(ResizeLeft + 
			(m_selectionMode - SelectedCanResizeLeft));
			*pVal = VARIANT_TRUE;
		}

		// start selection
		if(m_selectionMode == StartSelection)
		{
			m_pStartMapPoint = m_data->DeviceToMap(x, y);

			m_selectionMode = ResizeCorner;

			*pVal = VARIANT_TRUE;
		}
	}

	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_OnMouseUp( enum GrymCore::MouseButton mButton, LONG /*nShift*/, LONG  x, LONG  y,  VARIANT_BOOL *pVal)
{
	*pVal = VARIANT_FALSE;

	if(GrymCore::MouseButtonLeft == mButton)
	{
		if(m_selectionMode == ResizeCorner)
		{
			*pVal = VARIANT_TRUE;

			m_pEndMapPoint = m_data->DeviceToMap(x, y);

			if(std::fabs(m_pStartMapPoint->X - m_pEndMapPoint->X) > m_data->MinMapWidth &&
			   std::fabs(m_pStartMapPoint->Y - m_pEndMapPoint->Y) > m_data->MinMapHeight )
			{
				GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(
					m_pStartMapPoint, m_pEndMapPoint);
				m_pStartMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MinX, pStartEndMapRect->MinY);
				m_pEndMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

				m_selectionMode = Selected;
				
				UpdateDialog();
			}
		}

		if(m_selectionMode >= ResizeLeft && m_selectionMode <= ResizeBottom)
		{
			*pVal = VARIANT_TRUE;
			if(std::fabs(m_pStartMapPoint->X - m_pEndMapPoint->X) > m_data->MinMapWidth &&
				std::fabs(m_pStartMapPoint->Y - m_pEndMapPoint->Y) > m_data->MinMapHeight )
			{
				GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(
					m_pStartMapPoint, m_pEndMapPoint);
				m_pStartMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MinX, pStartEndMapRect->MinY);
				m_pEndMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

				m_selectionMode = Selected;
				
				UpdateDialog();
			}
		}

		if(m_selectionMode == SelectionMove)
		{
			m_selectionMode = Selected;
			*pVal = VARIANT_TRUE;
		}
	}

	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_OnMouseDblClk( enum GrymCore::MouseButton mButton,
											 LONG /*nShift*/, LONG  /*x*/, LONG  /*y*/,  VARIANT_BOOL *pVal)
{
	*pVal = VARIANT_FALSE;

	if(GrymCore::MouseButtonLeft==mButton && m_selectionMode == Selected)
	{
		m_dlg->Save();
	}

	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_OnMouseMove(LONG /*nShift*/, LONG x, LONG y, 
										   VARIANT_BOOL *pVal)
{
	*pVal = VARIANT_FALSE;

	if(m_selectionMode == StartSelection)
		return S_OK;

	GrymCore::IGrymObjectFactoryPtr pFactory = m_data->GetFactory();

	GrymCore::IMapPointPtr pCurrentMoveMapPoint = m_data->DeviceToMap(x, y);
	if(m_selectionMode == ResizeCorner)
	{
		m_pEndMapPoint = pCurrentMoveMapPoint;
		m_data->GetMap()->Invalidate( VARIANT_FALSE );
	}

	GrymCore::IDevPointPtr m_pStartDevPoint = m_data->MapToDevice(m_pStartMapPoint);
	GrymCore::IDevPointPtr m_pEndDevPoint = m_data->MapToDevice(m_pEndMapPoint);

	if(m_selectionMode >= ResizeLeft && m_selectionMode <= ResizeBottom)
	{
		switch(m_selectionMode)
		{
		case ResizeLeft:
			m_pStartMapPoint = pFactory->CreateMapPoint(pCurrentMoveMapPoint->X, m_pStartMapPoint->Y);
			break;
		case ResizeRight:
			m_pEndMapPoint = pFactory->CreateMapPoint(pCurrentMoveMapPoint->X, m_pEndMapPoint->Y);
			break;
		case ResizeTop:
			m_pStartMapPoint = pFactory->CreateMapPoint(m_pStartMapPoint->X, pCurrentMoveMapPoint->Y);
			break;
		case ResizeBottom:
			m_pEndMapPoint = pFactory->CreateMapPoint(m_pEndMapPoint->X, pCurrentMoveMapPoint->Y);
			break;
		}

		m_data->GetMap()->Invalidate( VARIANT_FALSE );
		UpdateDialog();

	}
	else if(m_selectionMode >= SelectionCanMove && m_selectionMode <= SelectedCanResizeBottom)
	{
		m_selectionMode = Selected;

		// Left
		if( (m_pStartDevPoint->X - m_nSelectionResizeDelta) < x && 
			(m_pStartDevPoint->X + m_nSelectionResizeDelta) > x &&
			m_pStartDevPoint->Y <= y && m_pEndDevPoint->Y >= y) 
			m_selectionMode = SelectedCanResizeLeft;

		// Right
		if( (m_pEndDevPoint->X - m_nSelectionResizeDelta) < x && 
			(m_pEndDevPoint->X + m_nSelectionResizeDelta) > x &&
			m_pStartDevPoint->Y <= y && m_pEndDevPoint->Y >= y) 
			m_selectionMode = SelectedCanResizeRight;

		// Top
		if( (m_pStartDevPoint->Y - m_nSelectionResizeDelta) < y && 
			(m_pStartDevPoint->Y + m_nSelectionResizeDelta) > y &&
			m_pStartDevPoint->X <= x && m_pEndDevPoint->X >= x) 
			m_selectionMode = SelectedCanResizeTop;

		// Bottom
		if( (m_pEndDevPoint->Y - m_nSelectionResizeDelta) < y && 
			(m_pEndDevPoint->Y + m_nSelectionResizeDelta) > y &&
			m_pStartDevPoint->X <= x && m_pEndDevPoint->X >= x) 
			m_selectionMode = SelectedCanResizeBottom;		
	}
	else if(m_selectionMode == SelectionMove)
	{
		DOUBLE deltaX = pCurrentMoveMapPoint->X - m_pCurrentDownMapPoint->X;
		DOUBLE deltaY = pCurrentMoveMapPoint->Y - m_pCurrentDownMapPoint->Y;
		m_pStartMapPoint->X+=deltaX;
		m_pEndMapPoint->X+=deltaX;	
		m_pStartMapPoint->Y+=deltaY;
		m_pEndMapPoint->Y+=deltaY;
		m_pCurrentDownMapPoint = pCurrentMoveMapPoint;
		m_data->GetMap()->Invalidate( VARIANT_FALSE );
		*pVal = VARIANT_TRUE;

		return S_OK;
	}

	if(m_selectionMode == Selected || m_selectionMode == SelectionCanMove)
	{
		if(m_pStartDevPoint->X < x &&  m_pEndDevPoint->X > x &&
			m_pStartDevPoint->Y < y &&  m_pEndDevPoint->Y > y)
			m_selectionMode = SelectionCanMove;
		else m_selectionMode = Selected;
	}

	*pVal = VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CCmdMakeShot::raw_OnClick(long, long)
{
	return S_FALSE;
}

STDMETHODIMP CCmdMakeShot::raw_OnDraw(GrymCore::IMapDevice *pDevice)
{
	if(m_selectionMode != StartSelection)
	{
		HDC hDC = reinterpret_cast<HDC>(pDevice->GetSafeDC());

		Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromHDC(hDC);
		ATLASSERT(NULL != pGraphics);
		pGraphics->SetPageUnit(Gdiplus::UnitPixel);

		GrymCore::IGrymObjectFactoryPtr pFactory = m_data->GetFactory();

		GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(
				m_pStartMapPoint, m_pEndMapPoint);
		GrymCore::IMapPointPtr pTmpStartMapPoint = m_data->GetFactory()->CreateMapPoint(
				pStartEndMapRect->MinX, pStartEndMapRect->MinY);
		GrymCore::IMapPointPtr pTmpEndMapPoint = m_data->GetFactory()->CreateMapPoint(
					pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

		GrymCore::IDevPointPtr pStartDevPoint = m_data->MapToDevice(pTmpStartMapPoint);
		GrymCore::IDevPointPtr pEndDevPoint = m_data->MapToDevice(pTmpEndMapPoint);

		pGraphics->DrawRectangle(m_pPen, pStartDevPoint->X, pStartDevPoint->Y,
			pEndDevPoint->X - pStartDevPoint->X, pEndDevPoint->Y - pStartDevPoint->Y);
		
		pGraphics->FillRectangle(m_pBrush, pStartDevPoint->X, pStartDevPoint->Y,
			pEndDevPoint->X - pStartDevPoint->X, pEndDevPoint->Y - pStartDevPoint->Y);

		delete pGraphics;
		pGraphics = NULL;

		return S_OK;
	}

	return S_FALSE;
}


/*
	Methods of GrymCore::ICommandAction
*/
STDMETHODIMP CCmdMakeShot::raw_OnCommand()
{
	GrymCore::IMapToolsPtr pMapTools = m_data->GetMap()->Tools;
	ATLASSERT(NULL != pMapTools);
	
	if (!m_bChecked)
	{
		if (!m_bToolAdded)
		{
			if (S_OK == pMapTools->AddTool(GrymCore::IToolBasePtr(this)))
				m_bToolAdded = true;
		}

		if(m_bToolAdded)
		{
			pMapTools->CurrentTool = GrymCore::IToolBasePtr(this);
			m_data->GetMap()->Invalidate(VARIANT_TRUE);
		}
	}
	else
	{
		pMapTools->SetDefaultTool();
		m_data->GetMap()->Invalidate(VARIANT_TRUE);
	}

	return S_OK;
}

void CCmdMakeShot::UpdateDialog()
{
	GrymCore::IGrymObjectFactoryPtr pFactory = m_data->GetFactory();

	GrymCore::IMapRectPtr pStartEndMapRect = m_data->SwapMapPoints(
			m_pStartMapPoint, m_pEndMapPoint);
	GrymCore::IMapPointPtr pTmpStartMapPoint = m_data->GetFactory()->CreateMapPoint(
			pStartEndMapRect->MinX, pStartEndMapRect->MinY);
	GrymCore::IMapPointPtr pTmpEndMapPoint = m_data->GetFactory()->CreateMapPoint(
			pStartEndMapRect->MaxX, pStartEndMapRect->MaxY);

	ATLASSERT(m_dlg != NULL);
	if(!m_dlg->IsWindow())
	{
		GrymCore::IDevPointPtr pStartDevPoint = m_data->MapToDevice(pTmpStartMapPoint);
		GrymCore::IDevPointPtr pEndDevPoint = m_data->MapToDevice(pTmpEndMapPoint);

		GrymCore::IDevSizePtr pImageSize =  pFactory->CreateDevSize(
			pEndDevPoint->X - pStartDevPoint->X, 
			pEndDevPoint->Y - pStartDevPoint->Y);

		m_dlg->put_InitialParams(pStartEndMapRect, pImageSize);
		m_dlg->Create(m_data->GetParentHWND());
		m_dlg->ShowWindow(SW_NORMAL);
	}
	else m_dlg->MapRect = pStartEndMapRect;
}
