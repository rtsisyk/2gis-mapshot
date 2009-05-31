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
#include "MapShotPluginImpl.h"

CMapShotPluginImpl::CMapShotPluginImpl() 
	: m_data(NULL)
{

}

CMapShotPluginImpl::~CMapShotPluginImpl() 
{

}

GrymCore::IGrymPluginPtr CMapShotPluginImpl::CreateInstance()
{
    ATL::CComObject<CMapShotPluginImpl> *obj;
    ATLVERIFY(S_OK == ATL::CComObject<CMapShotPluginImpl>::CreateInstance(&obj));
	GrymCore::IGrymPluginPtr rv = obj;
	ATLASSERT(NULL != rv);

	return rv;
}

/*
	Initialization
*/
STDMETHODIMP CMapShotPluginImpl::raw_Initialize(GrymCore::IGrym *grymRoot, 
												  GrymCore::IBaseViewThread *baseView)
{
	if(!grymRoot || !baseView)
		return E_INVALIDARG;

	try {
		m_data = new CMapShotData(grymRoot, baseView);
		if(NULL == m_data)
			return E_FAIL;

		// Добавление в ribbonBar
		GrymCore::IRibbonBarPtr ribbonBar = GrymCore::IBaseViewFramePtr(baseView)->
			GetMainRibbonBar();

		GrymCore::IRibbonTabPtr ribbonTab = ribbonBar->GetTab("Grym.ToolsTab");
		GrymCore::IRibbonGroupPtr ribbonGroup = ribbonTab->CreateGroup(
			GrymCore::RibbonGroupTypeSimple, _bstr_t(OLESTR("MapShot")),
			_bstr_t(OLESTR("MapShotGroup:1200")), m_data->LoadString(IDS_CMDMAKESHOT));

		m_ctrlSet = ribbonGroup;

		m_cmdMakeShot = CCmdMakeShot::CreateInstance(m_data);
		m_ctrlSet->AddControl(m_cmdMakeShot);

		return S_OK;
	}
	catch(...) {};

	return E_FAIL;
}

/*
	Termination
*/
STDMETHODIMP CMapShotPluginImpl::raw_Terminate()
{
	m_data->GetMap()->Tools->SetDefaultTool();
	m_data->GetMap()->Invalidate(VARIANT_TRUE);

	delete m_data;
	m_data = NULL;

	return S_OK;
}

/*
	XML information about plugin
*/
STDMETHODIMP CMapShotPluginImpl::get_XMLInfo(BSTR *pValue)
{
	try
	{
		if (!pValue) return E_POINTER;
		*pValue = NULL;

		static const _bstr_t data(OLESTR(
			"<grym_plugin_info>\
			<name>Снимок карты</name>\
			<description>Плагин позволяет сохранять фрагменты карты в файл.</description>\
			<copyright>Roman Tsisyck, Barnaul 2009.</copyright>\
			<tag>GrymPlugin.MapShot</tag>\
			<requirements>\
				<requirement_api>API-1.0</requirement_api>\
			</requirements>\
			</grym_plugin_info>")
		);

		*pValue = data.copy();
		return S_OK;
	}
	catch(...) {};

	return E_FAIL;
}
