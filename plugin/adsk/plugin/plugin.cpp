// ===========================================================================
// Copyright 2019 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
#include "pxr/base/tf/envSetting.h"

#include "base/api.h"
#include "importTranslator.h"
#include "ProxyShape.h"

#include <mayaUsd/render/vp2RenderDelegate/proxyRenderDelegate.h>
#include <mayaUsd/nodes/proxyShapeBase.h>
#include <mayaUsd/nodes/stageData.h>
#include <mayaUsd/nodes/proxyShapePlugin.h>
#include <mayaUsd/render/pxrUsdMayaGL/proxyShapeUI.h>

#if defined(WANT_UFE_BUILD)
#include <mayaUsd/ufe/Global.h>
#endif

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MDrawRegistry.h>

PXR_NAMESPACE_USING_DIRECTIVE

MAYAUSD_PLUGIN_PUBLIC
MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

    status = plugin.registerFileTranslator(
        "mayaUsdImport",
        "",
        UsdMayaImportTranslator::creator,
        "usdTranslatorImport", // options script name
        const_cast<char*>(UsdMayaImportTranslator::GetDefaultOptions().c_str()),
        false);
    if (!status) {
        status.perror("mayaUsdPlugin: unable to register import translator.");
    }

    status = MayaUsdProxyShapePlugin::initialize(plugin);
    CHECK_MSTATUS(status);

#if defined(WANT_UFE_BUILD)
    status = MayaUsd::ufe::initialize();
    if (!status) {
        status.perror("mayaUsdPlugin: unable to initialize ufe.");
    }
#endif

    status = plugin.registerShape(
        MayaUsd::ProxyShape::typeName,
        MayaUsd::ProxyShape::typeId,
        MayaUsd::ProxyShape::creator,
        MayaUsd::ProxyShape::initialize,
        UsdMayaProxyShapeUI::creator,
        MayaUsdProxyShapePlugin::getProxyShapeClassification());
    CHECK_MSTATUS(status);

    return status;
}

MAYAUSD_PLUGIN_PUBLIC
MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    MStatus status;

    status = plugin.deregisterFileTranslator("mayaUsdImport");
    if (!status) {
        status.perror("mayaUsdPlugin: unable to deregister import translator.");
    }

    status = plugin.deregisterNode(MayaUsd::ProxyShape::typeId);
    CHECK_MSTATUS(status);

    status = MayaUsdProxyShapePlugin::finalize(plugin);
    CHECK_MSTATUS(status);

#if defined(WANT_UFE_BUILD)
    status = MayaUsd::ufe::finalize();
    CHECK_MSTATUS(status);
#endif

    return status;
}
