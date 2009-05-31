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

#include "StdAfx.h"
#include "MapShotData.h"

CMapShotData::CMapShotData(GrymCore::IGrym *grymRoot,
												 GrymCore::IBaseViewThread *baseView):
	m_grymRoot(grymRoot),
	m_baseView(baseView),
	MinMapHeight(100.0),
	MinMapWidth(100.0),
	MinImageHeight(5),
	MinImageWidth(5),
	MaxImageHeight(6000),
	MaxImageWidth(6000)
{
	ATLASSERT(NULL != m_grymRoot);
	ATLASSERT(NULL != m_baseView);
	m_hMap = NULL;
	m_hParent = NULL;
	m_pFactory = NULL;

	// GDI+ Initialisation
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if(Gdiplus::Ok == Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL))
	{
		// Get list of available codes
		UINT  size;
		Gdiplus::GetImageEncodersSize(&nEncoders, &size);
		pEncoders = new Gdiplus::ImageCodecInfo[size];
		Gdiplus::GetImageEncoders(nEncoders, size, pEncoders); 
	}
	else m_gdiplusToken = 0;
}

CMapShotData::~CMapShotData()
{
	if(m_gdiplusToken)
	{
		if(pEncoders) delete[] pEncoders;
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
	}

	m_grymRoot = NULL;
    m_baseView = NULL; 
	m_pFactory = NULL;
}

/*
	Convert internal 2gis cm to m 
*/
DOUBLE CMapShotData::InternalToMeters(DOUBLE value)
{
	return value*0.01;
}

/*
	Convert meters to internal 2gis cm
*/
DOUBLE CMapShotData::MetersToInternal(DOUBLE value)
{
	return value*100;
}

/*
	Return IMap pointer
*/
GrymCore::IMapPtr CMapShotData::GetMap()
{
	GrymCore::IMapPtr pMap = (m_baseView == 0 ) 
		? GrymCore::IMapPtr()
		: GrymCore::IBaseViewFramePtr(m_baseView)->GetMap();
	ATLASSERT(NULL != pMap);

	return pMap;
}

/*
	Return HWND of map window
*/
HWND CMapShotData::GetMapHWND()
{
	if(NULL == m_hMap)
	{
		GrymCore::IMapPtr pMap = GetMap();
		OLE_HANDLE ohWnd = GetMap()->GetHWindow();
		m_hMap = *reinterpret_cast<HWND *>(&ohWnd);
		ATLASSERT(NULL!=m_hMap);
	}

	return m_hMap;
}

/*
	Return HWND of main window
*/
HWND CMapShotData::GetParentHWND()
{
	if(NULL == m_hParent)
	{
		HWND parentHWND = GetMapHWND();
		while (parentHWND != NULL && (::GetWindowLong(
			parentHWND, GWL_STYLE) & WS_CHILD))
			parentHWND = ::GetParent(parentHWND);
		m_hParent = parentHWND;
		ATLASSERT(NULL!=m_hParent);
	}

	return m_hParent;
}

/*
	Return factory pointer
*/
GrymCore::IGrymObjectFactoryPtr CMapShotData::GetFactory()
{
	if(NULL == m_pFactory)
	{
		m_pFactory = m_baseView->GetFactory(); 
		ATLASSERT(NULL!=m_pFactory);
	}
	return m_pFactory;
}

/*
	Load string from resources
*/
_bstr_t CMapShotData::LoadString(UINT id)
{
  ATL::CStringW str;
  str.LoadString(id);
  return _bstr_t(str.AllocSysString(), false);
}

/*
	Load cursor from resources
*/
HCURSOR CMapShotData::LoadCursor(UINT id)
{
	HMODULE hModule = ATL::_AtlBaseModule.GetResourceInstance();
	return ::LoadCursor(hModule, MAKEINTRESOURCE(id));
}

/*
	Load PNG from resources
*/
GrymCore::IRasterPtr CMapShotData::LoadRaster(UINT id)
{
	GrymCore::IGrymObjectFactoryPtr pFactory = GetFactory();
	GrymCore::IRasterPtr result = NULL;

	HMODULE hModule = ATL::_AtlBaseModule.GetResourceInstance();
	HRSRC hResource = ::FindResource(hModule, MAKEINTRESOURCE(id), _T("PNG"));

	ATLASSERT(NULL != hResource);
	ULONG size = ::SizeofResource(hModule, hResource);

	if (size)
	{
		HGLOBAL hGlobal = ::LoadResource(hModule, hResource);
		ATLASSERT(NULL != hGlobal);
		BYTE *data = static_cast<BYTE *>(::LockResource(hGlobal));
		ATL::CComSafeArray<BYTE> saBuf(size);
		BYTE HUGEP *pSAMem;

		if(S_OK == ::SafeArrayAccessData(saBuf.m_psa, (void HUGEP**)(&pSAMem)))
		{
			::memcpy(pSAMem, data, size);
			::SafeArrayUnaccessData(saBuf.m_psa);

			result = pFactory->CreateRasterFromMemory(saBuf.m_psa);
		}

		::FreeResource(hGlobal);
	}

	return result;
}

/*
	Convert device coordinates to map coordinates
*/
GrymCore::IMapPointPtr CMapShotData::DeviceToMap(LONG x, LONG y)
{
	GrymCore::IGrymObjectFactoryPtr ptrFactory = GetFactory();
	GrymCore::IDevPointPtr pDevPoint = ptrFactory-> CreateDevPoint(x,y);

	GrymCore::IMapDevicePtr ptrDevice = GetMap();
		
	return ptrDevice->DeviceToMap(pDevPoint);
}

/*
	Convert map coordinates to device coordinates
*/
GrymCore::IDevPointPtr CMapShotData::MapToDevice(GrymCore::IMapPointPtr pMapPoint)
{
	GrymCore::IMapDevicePtr ptrDevice = GetMap();	
	return ptrDevice->MapToDevice(pMapPoint);
}

/*
	Swap IMapPoint
*/
void CMapShotData::SwapMapPoints(GrymCore::IMapPointPtr pStartMapPoint,
					GrymCore::IMapPointPtr pEndMapPoint)
{
	DOUBLE minX, minY, maxX, maxY;

	if( pStartMapPoint->X < pEndMapPoint->X )
	{
		minX = pStartMapPoint->X;
		maxX = pEndMapPoint->X;
	}
	else
	{
		minX = pEndMapPoint->X;
		maxX = pStartMapPoint->X;
	}

	if( pStartMapPoint->Y < pEndMapPoint->Y )
	{
		minY = pStartMapPoint->Y;
		maxY = pEndMapPoint->Y;
	}
	else
	{
		minY = pEndMapPoint->Y;
		maxY = pStartMapPoint->Y;
	}

	pStartMapPoint->X = minX;
	pEndMapPoint->X = maxX;

	pStartMapPoint->Y = minY;
	pEndMapPoint->Y = maxY;
}

/*
	Get Image of pMapRect with size pImageSize 
*/
Gdiplus::Image *CMapShotData::GetMapImage(GrymCore::IMapRectPtr pMapRect,
	GrymCore::IDevSizePtr pImageSize)
{
	Gdiplus::Bitmap *pResultBitmap;
	Gdiplus::Graphics *pGraphics;
	HDC hDC = NULL;
	HDC hMemDC = NULL;

	GrymCore::IMapPtr pMap = GetMap();
	HWND hMap = GetMapHWND();

	// Get initial map position
	GrymCore::IMapRectPtr pInitalRect = pMap->GetMapVisibleRect();

	// Get visible area
	RECT rcClient;
	::GetClientRect(hMap, &rcClient);
	LONG visibleHeight = rcClient.bottom -  rcClient.top;
	LONG visibleWidth = rcClient.right -  rcClient.left;

	// Calc scale
	DOUBLE scale = pMapRect->Width / (double) pImageSize->Width;

	// Calc number of pieces
	UINT countV = static_cast<UINT>(std::ceil(pImageSize->Height / double(visibleHeight)));
	UINT countH = static_cast<UINT>(std::ceil(pImageSize->Width / double(visibleWidth)));

	// Create temporary bitmap
	pResultBitmap = new Gdiplus::Bitmap(pImageSize->Width, pImageSize->Height, PixelFormat24bppRGB );
	// Create temporary Graphics object for gluing
	pGraphics = Gdiplus::Graphics::FromImage(pResultBitmap);
	// Get system palette
	HPALETTE hPal = (HPALETTE) ::GetStockObject(DEFAULT_PALETTE);
	// Create DC
	hDC = ::GetDC( hMap );
	if(hDC) hMemDC = ::CreateCompatibleDC(hDC);
	// Create Temporary GDI HBITMAP
	HBITMAP hTmpBitmap = ::CreateCompatibleBitmap(hDC, visibleWidth, visibleHeight);

	if (NULL != hMemDC && pResultBitmap != NULL && pGraphics!= NULL && hTmpBitmap != NULL)
	{
		pGraphics->SetPageUnit(Gdiplus::UnitPixel);
		HBITMAP hBitmapOld = (HBITMAP) ::SelectObject(hMemDC, hTmpBitmap); 

		GrymCore::IGrymObjectFactoryPtr pFactory = GetFactory(); 	
		GrymCore::IDevPointPtr pDevPoint = pFactory->CreateDevPoint(0,0);

		GrymCore::IDevRectPtr pDevRect = pFactory->CreateDevRect( 
			rcClient.left, rcClient.top, rcClient.right, rcClient.bottom );

		DOUBLE mapDeltaV = visibleHeight * scale;
		DOUBLE mapDeltaH = visibleWidth * scale;

		LONG imageCurPosV = 0;
		LONG imageCurPosH = 0;

		DOUBLE mapCurPosH = pMapRect->MinX;
		DOUBLE mapCurPosV = pMapRect->MinY;
		DOUBLE mapNextPosH = mapCurPosH;
		DOUBLE mapNextPosV = mapCurPosV;

		UINT32 i;
		UINT32 j;
		for(j=0; j < countV; j++)
		{
			mapNextPosV += mapDeltaV;
			mapCurPosH = pMapRect->MinX;
			mapNextPosH = mapCurPosH;
			imageCurPosH = 0;

			for(i=0; i< countH; i++)
			{
				mapNextPosH += mapDeltaH;

				// Get next piece
				GrymCore::IMapRectPtr pTmpRect = pFactory->CreateMapRect(
					mapCurPosH, mapCurPosV, mapNextPosH, mapNextPosV);
				pMap->SetMapVisibleRect(pTmpRect, TRUE);
				pMap->CopyMap( pDevRect, reinterpret_cast<OLE_HANDLE>(hMemDC), pDevPoint);

				// Paint piece on Graphics
				Gdiplus::Bitmap *pTmpBitmap = Gdiplus::Bitmap::FromHBITMAP(hTmpBitmap, hPal);
				pGraphics->DrawImage(pTmpBitmap, imageCurPosH, imageCurPosV, visibleWidth, visibleHeight);
				delete pTmpBitmap;

				mapCurPosH = mapNextPosH;
				imageCurPosH+=visibleWidth;
			}

			mapCurPosV = mapNextPosV;
			imageCurPosV+=visibleHeight;
		}

		// Revert initial map position
		pMap->SetMapVisibleRect(pInitalRect, TRUE);
		::SelectObject(hMemDC, hBitmapOld); 
	}
	else
	{
		// Some errors was occured
		if(NULL != pResultBitmap)
			delete pResultBitmap;
		pResultBitmap = NULL;
	}

	// Cleanup
	if(NULL != hMemDC)
		::DeleteDC(hMemDC);
	if(NULL != hDC)
		::ReleaseDC(hMap, hDC);
	if(NULL != pGraphics)
		delete pGraphics;
	if(NULL != hTmpBitmap)
		::DeleteObject(hTmpBitmap);

	return pResultBitmap;
}

/*
	Show standart SaveFile dialog 
	[in, out] pszFile - filename
	[in] sizeInWords - size of pszFile
	[out] nImageEncoder - type of selected file
*/
HRESULT CMapShotData::SaveImageDialog(TCHAR* pszFile, size_t sizeInWords,
												 UINT32 &nImageEncoder)
{
	TCHAR szFilter[512];
	UINT nFilterIndex = 0;

	*szFilter = '\0';
	for( UINT i = 0; i < nEncoders; ++i )
	{
		_tcscat_s( szFilter, pEncoders[i].FormatDescription );
		_tcscat_s( szFilter, _T(" ("));
		_tcscat_s( szFilter, pEncoders[i].FilenameExtension );
		_tcscat_s( szFilter, _T(")|") );
		_tcscat_s( szFilter, pEncoders[i].FilenameExtension );
		_tcscat_s( szFilter, _T("|") );

		// Select default codec (PNG)
		if(_tcscmp(pEncoders[i].MimeType, _T("image/png")) == 0)
			nFilterIndex = i+1;
	}
	_tcscat_s( szFilter, _T("|") );

	// Replace | with null
	for( TCHAR *pch = szFilter; *pch != 0; ++pch )
	{
		if ( *pch == '|' ) 
			*pch = 0;
	}

	OPENFILENAME Ofn;
	memset( &Ofn, 0, sizeof(OPENFILENAME) );
	Ofn.lStructSize = sizeof(OPENFILENAME); 
	Ofn.hwndOwner = GetParentHWND();
	Ofn.lpstrFilter = szFilter; 
	Ofn.lpstrFile= pszFile; 
	Ofn.nMaxFile = sizeInWords; 
	Ofn.nFilterIndex = nFilterIndex;
	Ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER; 

	if ( !::GetSaveFileName(&Ofn) )
		return E_ABORT;
	
	nImageEncoder = (Ofn.nFilterIndex > 0) ? Ofn.nFilterIndex-1 : 0;

	TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFname[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath_s(pszFile, szDrive, szDir, szFname, szExt);

	if(*szExt == 0)
	{
		UINT len = _tcscspn(pEncoders[nImageEncoder].FilenameExtension, _T(";"));
		_tcsncpy_s(szExt, pEncoders[nImageEncoder].FilenameExtension+1, len-1);
		_tcslwr_s(szExt);

		_tmakepath_s(pszFile, sizeInWords, szDrive, szDir, szFname, szExt);
	}

	return S_OK;
}

/*
	Save Image to file with name pszName and type nImageEncoder
*/
HRESULT CMapShotData::SaveImageToFile(Gdiplus::Image *pImage,
											   const TCHAR* pszName, UINT nImageEncoder)
{
	if(Gdiplus::Ok == pImage->Save(pszName, &pEncoders[nImageEncoder].Clsid, NULL))
		return S_OK;

	return E_FAIL;
}

