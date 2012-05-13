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

#include <cmath>

class ATL_NO_VTABLE CMapShotData
{
public:
	CMapShotData(GrymCore::IGrym *grymRoot, 
		GrymCore::IBaseViewThread *baseView);
	~CMapShotData();

	GrymCore::IMapPtr GetMap();
	HWND GetMapHWND();
	HWND GetParentHWND();
	GrymCore::IGrymObjectFactoryPtr GetFactory();
	_bstr_t LoadString(UINT id);
	HCURSOR LoadCursor(UINT id);
	GrymCore::IRasterPtr LoadRaster(UINT id);

	GrymCore::IMapPointPtr DeviceToMap(LONG x, LONG y);
	GrymCore::IDevPointPtr MapToDevice(GrymCore::IMapPointPtr pMapPoint);

	GrymCore::IMapRectPtr SwapMapPoints(GrymCore::IMapPointPtr pStartMapPoint,
					GrymCore::IMapPointPtr pEndMapPoint);

	HRESULT SaveImageDialog(TCHAR* szFile, size_t sizeInWords, 
		UINT32 &nImageEncoder);

	Gdiplus::Image *GetMapImage(GrymCore::IMapRectPtr pMapRect,
		GrymCore::IDevSizePtr pImageSize);
	HRESULT SaveImageToFile(Gdiplus::Image *pImage,
											   const TCHAR* pszName, UINT nImageEncoder);

	DOUBLE InternalToMeters(DOUBLE value);
	DOUBLE MetersToInternal(DOUBLE value);

	const DOUBLE MinMapHeight;
	const DOUBLE MinMapWidth;
	const LONG MinImageHeight;
	const LONG MinImageWidth;
	const LONG MaxImageHeight;
	const LONG MaxImageWidth;
	BOOL InvertedYAxis;

private:
	HWND m_hMap;
	HWND m_hParent;

    GrymCore::IGrymPtr m_grymRoot;
    GrymCore::IBaseViewThreadPtr m_baseView; 
	GrymCore::IGrymObjectFactoryPtr m_pFactory;

	ULONG_PTR m_gdiplusToken;
	Gdiplus::ImageCodecInfo *pEncoders;
	UINT nEncoders;
};

