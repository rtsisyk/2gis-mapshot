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

#include <atlhost.h>
#include <cmath>
#include "CmdMakeShot.h"

class CCmdMakeShot;
class CCmdMakeShotDlg : 
	public ATL::CDialogImpl<CCmdMakeShotDlg>
{
public:

	CCmdMakeShotDlg(CCmdMakeShot *cmd);
	~CCmdMakeShotDlg() {}
	
	enum { IDD = IDD_MAPSHOT };

	HRESULT Save();

	void put_InitialParams(GrymCore::IMapRectPtr pMapRect, 
										GrymCore::IDevSizePtr pImageSize);
    /*
	__declspec(property(get=get_Scale, put=put_Scale)) DOUBLE Scale;
	DOUBLE get_Scale();
	void put_Scale(DOUBLE scale);

	__declspec(property(get=get_ImageSize)) GrymCore::IDevSizePtr ImageSize;
	GrymCore::IDevSizePtr CCmdMakeShotDlg::get_ImageSize();
	*/
	__declspec(property(put=put_MapRect)) GrymCore::IMapRectPtr MapRect;
	void put_MapRect(GrymCore::IMapRectPtr pMapRect);

protected:
	BEGIN_MSG_MAP(CCmdMakeShotDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_SAVEBUTTON, BN_CLICKED, OnBnClickedSavebutton)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_CANCELBUTTON, BN_CLICKED, OnBnClickedCancelbutton)
		COMMAND_HANDLER(IDC_WIDTHEDIT, EN_UPDATE, OnEnChangeValue)
		COMMAND_HANDLER(IDC_HEIGHTEDIT, EN_UPDATE, OnEnChangeValue)
		COMMAND_HANDLER(IDC_SCALEEDIT, EN_UPDATE, OnEnChangeValue)
		NOTIFY_HANDLER(IDC_SCALESPIN, UDN_DELTAPOS, OnDeltaposScalespin)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedSavebutton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedCancelbutton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEnChangeValue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDeltaposScalespin(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);

private:
	CCmdMakeShot *m_cmd;

	DOUBLE m_scale;
	GrymCore::IMapRectPtr m_pMapRect;
	GrymCore::IDevSizePtr m_pImageSize;

	TCHAR szMapSizeFormat[255];

	void UpdateEdits(bool bUpdateWidth, bool bUpdateHeight, bool bUpdateScale);

	bool m_bUpdateLock;
};
