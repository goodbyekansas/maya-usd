//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/pxr.h"
#include "pxrUsdMayaGL/proxyShapeUI.h"

#include "pxrUsdMayaGL/batchRenderer.h"
#include "pxrUsdMayaGL/renderParams.h"
#include "pxrUsdMayaGL/shapeAdapter.h"
#include "usdMaya/proxyShape.h"

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/timeCode.h"

#include <maya/M3dView.h>
#include <maya/MBoundingBox.h>
#include <maya/MDagPath.h>
#include <maya/MDrawInfo.h>
#include <maya/MDrawRequest.h>
#include <maya/MDrawRequestQueue.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MSelectInfo.h>
#include <maya/MSelectionList.h>
#include <maya/MSelectionMask.h>


PXR_NAMESPACE_OPEN_SCOPE


/* static */
void*
UsdMayaProxyShapeUI::creator()
{
    UsdMayaGLBatchRenderer::Init();
    return new UsdMayaProxyShapeUI();
}

bool
UsdMayaProxyShapeUI::_SyncShapeAdapter(
        UsdMayaProxyShape* shape,
        const MDagPath& objPath,
        const M3dView::DisplayStyle displayStyle,
        const M3dView::DisplayStatus displayStatus) const
{
    if (!shape) {
        return false;
    }

    // XXX: Note that for now we must populate the properties on the shape
    // adapter that will be used to compute its delegateId *before* we call
    // AddShapeAdapter(), since adding the shape adapter the first time will
    // invoke its Init() method. See the comment in the implementation of
    // PxrMayaHdShapeAdapter::Init() for more detail.
    const UsdPrim usdPrim = shape->usdPrim();
    const SdfPathVector excludedPrimPaths = shape->getExcludePrimPaths();
    _shapeAdapter._shapeDagPath = objPath;
    _shapeAdapter._rootPrim = usdPrim;
    _shapeAdapter._excludedPrimPaths = excludedPrimPaths;

    UsdMayaGLBatchRenderer::GetInstance().AddShapeAdapter(&_shapeAdapter);

    return _shapeAdapter.Sync(shape, displayStyle, displayStatus);
}

/* virtual */
void
UsdMayaProxyShapeUI::getDrawRequests(
        const MDrawInfo& drawInfo,
        bool /* isObjectAndActiveOnly */,
        MDrawRequestQueue& requests)
{
    MDrawRequest request = drawInfo.getPrototype(*this);

    const MDagPath shapeDagPath = drawInfo.multiPath();
    UsdMayaProxyShape* shape =
        UsdMayaProxyShape::GetShapeAtDagPath(shapeDagPath);
    if (!shape) {
        return;
    }

    if (!_SyncShapeAdapter(shape,
                           shapeDagPath,
                           drawInfo.displayStyle(),
                           drawInfo.displayStatus())) {
        return;
    }

    bool drawShape;
    bool drawBoundingBox;
    PxrMayaHdRenderParams params =
        _shapeAdapter.GetRenderParams(&drawShape, &drawBoundingBox);

    // Only query bounds if we're drawing bounds...
    //
    if (drawBoundingBox) {
        const MBoundingBox bounds = shape->boundingBox();

        // Note that drawShape is still passed through here.
        UsdMayaGLBatchRenderer::GetInstance().QueueShapeForDraw(
            &_shapeAdapter,
            this,
            request,
            params,
            drawShape,
            &bounds);
    }
    //
    // Like above but with no bounding box...
    else if (drawShape) {
        UsdMayaGLBatchRenderer::GetInstance().QueueShapeForDraw(
            &_shapeAdapter,
            this,
            request,
            params,
            drawShape,
            nullptr);
    }
    else
    {
        // we weren't asked to do anything.
        return;
    }

    //
    // add the request to the queue
    //
    requests.add(request);
}

/* virtual */
void
UsdMayaProxyShapeUI::draw(const MDrawRequest& request, M3dView& view) const
{
    view.beginGL();

    UsdMayaGLBatchRenderer::GetInstance().Draw(request, view);

    view.endGL();
}

/* virtual */
bool
UsdMayaProxyShapeUI::select(
        MSelectInfo& selectInfo,
        MSelectionList& selectionList,
        MPointArray& worldSpaceSelectedPoints) const
{
    MSelectionMask objectsMask(MSelectionMask::kSelectObjectsMask);

    // selectable() takes MSelectionMask&, not const MSelectionMask.  :(.
    if (!selectInfo.selectable(objectsMask)) {
        return false;
    }

    M3dView view = selectInfo.view();

    // Note that we cannot use UsdMayaProxyShape::GetShapeAtDagPath() here.
    // selectInfo.selectPath() returns the dag path to the assembly node, not
    // the shape node, so we don't have the shape node's path readily available.
    UsdMayaProxyShape* shape = static_cast<UsdMayaProxyShape*>(surfaceShape());

    if (!_SyncShapeAdapter(shape,
                           selectInfo.selectPath(),
                           view.displayStyle(),
                           view.displayStatus(selectInfo.selectPath()))) {
        return false;
    }

    GfVec3f hitPoint;
    const bool didHit =
        UsdMayaGLBatchRenderer::GetInstance().TestIntersection(
            &_shapeAdapter,
            view,
            selectInfo.singleSelection(),
            &hitPoint);

    if (didHit) {
        MSelectionList newSelectionList;
        newSelectionList.add(selectInfo.selectPath());

        MPoint mayaHitPoint = MPoint(hitPoint[0], hitPoint[1], hitPoint[2]);

        selectInfo.addSelection(
            newSelectionList,
            mayaHitPoint,
            selectionList,
            worldSpaceSelectedPoints,

            // even though this is an "object", we use the "meshes" selection
            // mask here.  This allows us to select usd assemblies that are
            // switched to "full" as well as those that are still collapsed.
            MSelectionMask(MSelectionMask::kSelectMeshes),

            false);
    }

    return didHit;
}

UsdMayaProxyShapeUI::UsdMayaProxyShapeUI() : MPxSurfaceShapeUI()
{
}

/* virtual */
UsdMayaProxyShapeUI::~UsdMayaProxyShapeUI()
{
    UsdMayaGLBatchRenderer::GetInstance().RemoveShapeAdapter(&_shapeAdapter);
}


PXR_NAMESPACE_CLOSE_SCOPE