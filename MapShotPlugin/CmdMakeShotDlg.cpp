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

#include "stdafx.h"
#include "CmdMakeShotDlg.h"

#include <cmath>

CCmdMakeShotDlg::CCmdMakeShotDlg(CCmdMakeShot *cmd)
{
	m_bUpdateLock = true;
	_tcscpy_s(szMapSizeFormat, _T(""));

	m_cmd = cmd;
}

void CCmdMakeShotDlg::put_InitialParams(GrymCore::IMapRectPtr pMapRect, 
										GrymCore::IDevSizePtr pImageSize)
{
	m_pMapRect = pMapRect;
	m_pImageSize = pImageSize;

	m_scale = m_pMapRect->Width / (double) pImageSize->Width;
}

LRESULT CCmdMakeShotDlg::OnInitDialog(UINT /* uMsg */, WPARAM /* wParam */, 
									  LPARAM /* lParam */, BOOL& /* bHandled */)
{
	GetDlgItem(IDC_HEIGHTEDIT).SendMessage(EM_SETLIMITTEXT, 5, 0);
	GetDlgItem(IDC_SCALEEDIT).SendMessage(EM_SETLIMITTEXT, 10, 0);
	GetDlgItem(IDC_WIDTHEDIT).SendMessage(EM_SETLIMITTEXT, 5, 0);

	m_bUpdateLock = false;
	
	::LoadString( ATL::_AtlBaseModule.GetResourceInstance(), IDS_MAPSIZE, 
		szMapSizeFormat, sizeof(szMapSizeFormat)/ sizeof(TCHAR));

	UpdateEdits(true, true, true);

	return 0; 
}

LRESULT CCmdMakeShotDlg::OnBnClickedSavebutton(WORD /*wNotifyCode*/, 
											   WORD /*wID*/, HWND /*hWndCtl*/, 
											   BOOL& /*bHandled*/)
{
	Save();
	return 0;
}

LRESULT CCmdMakeShotDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, 
								 LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DestroyWindow();
	return 0;
}

LRESULT CCmdMakeShotDlg::OnBnClickedCancelbutton(WORD /*wNotifyCode*/, 
												 WORD /*wID*/, HWND /*hWndCtl*/, 
												 BOOL& /*bHandled*/)
{
	m_cmd->m_selectionMode = CCmdMakeShot::StartSelection;
	m_cmd->m_data->GetMap()->Invalidate( VARIANT_FALSE );
	SendMessage(WM_CLOSE);
	return 0;
}

LRESULT CCmdMakeShotDlg::OnEnChangeValue(WORD /*wNotifyCode*/, 
										 WORD wID, HWND hWndCtl, 
										 BOOL& /*bHandled*/)
{
	if(m_bUpdateLock) return 0;

	TCHAR *p = NULL;
	TCHAR tmpstr[50];
	::GetWindowText(hWndCtl, tmpstr, sizeof(tmpstr) / sizeof(TCHAR));
	
	switch(wID)
	{
		case IDC_WIDTHEDIT:
		{
			// locale-based function
			LONG tmp = ::_tcstol(tmpstr, &p, 0);
			if((NULL!=p && *p != 0) || tmp < m_cmd->m_data->MinImageWidth) 
				UpdateEdits(true, false, false); // Restore value
			else
			{
				m_scale = std::fabs(m_pMapRect->Width) / double(tmp);
				m_pImageSize->Width = tmp;
				m_pImageSize->Height = static_cast<long>(
					std::ceil(std::fabs(m_pMapRect->Height) / m_scale));

				UpdateEdits(false, true, true);
			}
			break;
		}
		case IDC_HEIGHTEDIT:
		{
			LONG tmp = ::_tcstol(tmpstr, &p, 0);
			if((NULL!=p && *p != 0) || tmp < m_cmd->m_data->MinImageHeight) 
				UpdateEdits(false, true, true);
			else
			{
				m_scale = std::fabs(m_pMapRect->Height) / double(tmp);
				m_pImageSize->Width = static_cast<long>(
					std::ceil(std::fabs(m_pMapRect->Width) / m_scale));
				m_pImageSize->Height = tmp;

				UpdateEdits(true, false, true);
			}
			break;
		}
		default:
		{
			DOUBLE tmp = ::_tcstod(tmpstr, &p);
			if((NULL!=p && *p != 0) || tmp < 0.001) 
				UpdateEdits(false, true, true);
			else
			{
				m_scale = m_cmd->m_data->MetersToInternal(tmp);
				m_pImageSize->Width = static_cast<long>(
					std::ceil(std::fabs(m_pMapRect->Width) / m_scale));
				m_pImageSize->Height = static_cast<long>(
					std::ceil(std::fabs(m_pMapRect->Height) / m_scale));

				UpdateEdits(true, true, false);
			}
		}
	}

	return 0;
}

/*
DOUBLE CCmdMakeShotDlg::get_Scale()
{
	return m_scale; 
}

void CCmdMakeShotDlg::put_Scale(DOUBLE scale)
{
	m_scale = scale;
	m_pImageSize->Width = static_cast<long>(std::ceil(m_pMapRect->Width / m_scale));
	m_pImageSize->Height = static_cast<long>(std::ceil(m_pMapRect->Height / m_scale));

	UpdateEdits(true, true, true);
}

GrymCore::IDevSizePtr CCmdMakeShotDlg::get_ImageSize()
{
	return m_pImageSize;
}
*/

void CCmdMakeShotDlg::put_MapRect(GrymCore::IMapRectPtr pMapRect)
{
	ATLASSERT(m_scale != 0.0);
	m_pMapRect = pMapRect;

	m_pImageSize->Width = static_cast<long>(std::ceil(std::fabs(m_pMapRect->Width) / m_scale));
	m_pImageSize->Height = static_cast<long>(std::ceil(std::fabs(m_pMapRect->Height) / m_scale));
	UpdateEdits(true, true, false);
}


void CCmdMakeShotDlg::UpdateEdits(bool bUpdateWidth, bool bUpdateHeight, 
								  bool bUpdateScale)
{
	if(m_bUpdateLock) return;
	m_bUpdateLock = true;
	TCHAR tmpstr[255];

	ATL::CWindow wnd = GetDlgItem(IDC_MAPSIZE);
	if(wnd)
	{
		const DOUBLE width = std::fabs(m_cmd->m_data->InternalToMeters(m_pMapRect->Width));
		const DOUBLE height = std::fabs(m_cmd->m_data->InternalToMeters(m_pMapRect->Height));
		_stprintf_s(tmpstr, szMapSizeFormat, width, height);

		wnd.SetWindowText(tmpstr);
	}

	if(bUpdateWidth)
	{
		
		ATL::CWindow wnd = GetDlgItem(IDC_WIDTHEDIT);
		if(wnd)
		{
			// locale-based function
			_stprintf_s(tmpstr, _T("%ld"), m_pImageSize->Width);
			wnd.SetWindowText(tmpstr);
		}
	}

	if(bUpdateHeight)
	{
		ATL::CWindow wnd = GetDlgItem(IDC_HEIGHTEDIT);
		if(wnd)
		{
			// locale-based function
			_stprintf_s(tmpstr, _T("%ld"), m_pImageSize->Height);
			wnd.SetWindowText(tmpstr);
		}
	}

	if(bUpdateScale)
	{
		ATL::CWindow wnd = GetDlgItem(IDC_SCALEEDIT);
		if(wnd)
		{
			// locale-based function
			_stprintf_s(tmpstr, _T("%lf"), m_cmd->m_data->InternalToMeters(m_scale));
			wnd.SetWindowText(tmpstr);
		}
	}
	m_bUpdateLock = false;
}

LRESULT CCmdMakeShotDlg::OnDeltaposScalespin(int /*idCtrl*/, 
											 LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	TCHAR tmpstr[50];
	DOUBLE tmpscale;

	ATL::CWindow scaleEdit = GetDlgItem(IDC_SCALEEDIT);
	scaleEdit.GetWindowText(tmpstr, sizeof(tmpstr) / sizeof(TCHAR));
	tmpscale = ::_tcstod(tmpstr, NULL) - 0.1*pNMUpDown->iDelta;
	_stprintf_s(tmpstr, _T("%lf"), tmpscale);
	scaleEdit.SetWindowText(tmpstr);

	return 0;
}

HRESULT CCmdMakeShotDlg::Save()
{
	if(m_pImageSize->Width > m_cmd->m_data->MaxImageWidth || 
		m_pImageSize->Height > m_cmd->m_data->MaxImageHeight)
	{
		if(IDNO == MessageBox(_T("Размер изображения слишком велик. ")
			_T("Для сохранения данного изображения может понадобиться ")
			_T("большое количество памяти. Вы действительно хотите продолжить?"), 
			_T("Внимание!"), MB_ICONWARNING | MB_YESNO) )
			return S_FALSE;
	}

	UINT nImageEncoder;
	TCHAR szFile[_MAX_PATH];
	_tcscpy_s(szFile, _T("2gismap"));

	if(S_OK == m_cmd->m_data->SaveImageDialog(szFile, _MAX_PATH, nImageEncoder))
	{
		m_cmd->m_selectionMode = CCmdMakeShot::StartSelection;
		m_cmd->m_data->GetMap()->Invalidate( VARIANT_FALSE );

		Gdiplus::Image *pImage = m_cmd->m_data->GetMapImage(m_pMapRect, m_pImageSize);
		if(NULL != pImage)
		{
			HRESULT hResult = m_cmd->m_data->SaveImageToFile(pImage, 
				szFile, nImageEncoder);
			delete pImage;

			if(S_OK == hResult)
			{
				SendMessage(WM_CLOSE);
				return S_OK;
			}
		}

		m_cmd->m_selectionMode = CCmdMakeShot::Selected;
		m_cmd->m_data->GetMap()->Invalidate( VARIANT_FALSE );

		MessageBox(_T("Произошла ошибка при сохранении карты"),
			_T("Ошибка"), MB_OK | MB_ICONEXCLAMATION);
	}
	
	return E_FAIL;
}
