/*******************************************************************************
*  This file is part of the MapShot plugin
*  Copyright (C) 2009 Roman O Tsisyck <inbox@art1x.com>
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

#pragma once

#include "MapShotData.h"
#include "CmdMakeShotDlg.h"

class CCmdMakeShotDlg;
class ATL_NO_VTABLE CCmdMakeShot:
	public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
	public GrymCore::ICommandAccelerator,
	public GrymCore::IControlPlacement,
	public GrymCore::IControlAppearance,
	public GrymCore::ICommandAction,
	public GrymCore::ICommandState,
	public GrymCore::ITool
{
public:
	static GrymCore::ICommandActionPtr CreateInstance(
		CMapShotData* data);

protected:
	CCmdMakeShot();
	~CCmdMakeShot();

	BEGIN_COM_MAP(CCmdMakeShot)
		COM_INTERFACE_ENTRY(GrymCore::ICommandAction)
		// COM_INTERFACE_ENTRY_FUNC(__uuidof(GrymCore::ICommandAccelerator), 0, ICommandAcceleratorQIFunc)
		COM_INTERFACE_ENTRY(GrymCore::IControlPlacement)
		COM_INTERFACE_ENTRY(GrymCore::IControlAppearance)
		COM_INTERFACE_ENTRY(GrymCore::IControlState)
		COM_INTERFACE_ENTRY(GrymCore::ICommandState)
		COM_INTERFACE_ENTRY(GrymCore::ITool)
		COM_INTERFACE_ENTRY(GrymCore::IToolBase)
	END_COM_MAP() 

	// GrymCore::ICommandAccelerator
	STDMETHOD(get_Accelerator)(LONG *pVal);

	// GrymCore::IControlPlacement 
	STDMETHOD(get_PlacementCode)(BSTR *pVal);

	// GrymCore::IControlAppearance
	STDMETHOD(get_Tag)(BSTR *pVal); 
	STDMETHOD(get_Caption)(BSTR *pVal);
	STDMETHOD(get_Description)(BSTR *pVal);
	STDMETHOD(get_Icon)(IUnknown ** pVal);

	// GrymCore::ICommandState
	STDMETHOD(get_Enabled)(VARIANT_BOOL *pVal);
	STDMETHOD(get_Checked)(VARIANT_BOOL *pVal);

	// GrymCore::IControlState
	STDMETHOD(get_Available)(VARIANT_BOOL *pVal);

	// GrymCore::IToolBase
	//STDMETHOD(get_Tag)(BSTR *pVal);

	// GrymCore::ITool
	STDMETHOD(get_Cursor)(OLE_HANDLE *pCursor);
	STDMETHOD(raw_Activate)(VARIANT_BOOL bActivate,VARIANT_BOOL *bpRet);
	STDMETHOD(raw_OnKeyDown)( LONG nChar, LONG nShift, VARIANT_BOOL *pVal);
	STDMETHOD(raw_OnKeyUp)( LONG nChar, LONG nShift , VARIANT_BOOL *pVal);  
	STDMETHOD(raw_OnMouseDown)( enum GrymCore::MouseButton mButton, LONG nShift, LONG  X, LONG  Y,  VARIANT_BOOL *pVal);
	STDMETHOD(raw_OnMouseDblClk)( enum GrymCore::MouseButton mButton, LONG nShift, LONG  X, LONG  Y,  VARIANT_BOOL *pVal);
	STDMETHOD(raw_OnMouseUp)( enum GrymCore::MouseButton mButton, LONG nShift, LONG  X, LONG  Y,  VARIANT_BOOL *pVal);
	STDMETHOD(raw_OnMouseMove)(LONG nShift,LONG  X, LONG  Y, VARIANT_BOOL *pVal);
	STDMETHOD(raw_OnClick)(long, long);
	STDMETHOD(raw_OnDraw)(GrymCore::IMapDevice *pDevice);

	// GrymCore::ICommandAction
	STDMETHOD(raw_OnCommand)();

private:	
	// static HRESULT WINAPI ICommandAcceleratorQIFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD dw);

	void UpdateDialog();

	const _bstr_t m_pCmdMakeShotPlacementCode;
	const _bstr_t m_pCmdMakeShotTag;

	bool m_bToolAdded;
	bool m_bChecked;
	CMapShotData* m_data;	

	GrymCore::IMapPointPtr m_pStartMapPoint, m_pEndMapPoint, m_pCurrentDownMapPoint;

	Gdiplus::Pen* m_pPen;
	Gdiplus::Brush* m_pBrush;

	enum SelectionModeEnum { StartSelection, ResizeCorner, ResizeLeft, ResizeRight,
		ResizeTop, ResizeBottom, SelectionMove, SelectionCanMove, Selected, 
		SelectedCanResizeLeft, SelectedCanResizeRight,
		SelectedCanResizeTop, SelectedCanResizeBottom } m_selectionMode;
	
	const LONG m_nSelectionResizeDelta;
	CCmdMakeShotDlg *m_dlg;

	UINT m_nCurrentCursorID;
	HCURSOR m_hCurrentCursor;

	GrymCore::IRasterPtr m_pIcon;
	friend class CCmdMakeShotDlg;
};
