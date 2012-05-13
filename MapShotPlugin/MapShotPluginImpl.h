/*******************************************************************************
*  This file is part of the MapShot plugin
*  Copyright (C) 2009 Roman Tsisyk <roman@tsisyk.com>
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
#include "CmdMakeShot.h"

class ATL_NO_VTABLE CMapShotPluginImpl : 
	public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
    public GrymCore::IGrymPlugin,
    public GrymCore::IGrymPluginInfo
{
public:
    static GrymCore::IGrymPluginPtr CreateInstance();

protected:
    CMapShotPluginImpl();
    ~CMapShotPluginImpl();

	BEGIN_COM_MAP(CMapShotPluginImpl)
		COM_INTERFACE_ENTRY(GrymCore::IGrymPlugin)
		COM_INTERFACE_ENTRY(GrymCore::IGrymPluginInfo)
	END_COM_MAP()

public: 
	// GrymCore::IGrymPlugin
    STDMETHOD(raw_Initialize)(GrymCore::IGrym *pRoot, GrymCore::IBaseViewThread *pBaseView);
    STDMETHOD(raw_Terminate)();

	// GrymCore::IGrymPluginInfo
	STDMETHOD(get_XMLInfo)(BSTR *pVal);

private:
	CMapShotData *m_data;
	GrymCore::ICommandActionPtr m_cmdMakeShot;
	GrymCore::IControlSetPtr m_ctrlSet;

};

class CSaveImagePluginModule: public ATL::CAtlDllModuleT< CSaveImagePluginModule >
{

};

extern class CSaveImagePluginModule _AtlModule;
