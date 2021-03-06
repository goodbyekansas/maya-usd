//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "nurbsCurveWriter.h"

#include <numeric>

#include <maya/MDoubleArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPointArray.h>

#include <pxr/pxr.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/curves.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>

#include <mayaUsd/fileio/primWriter.h>
#include <mayaUsd/fileio/primWriterRegistry.h>
#include <mayaUsd/fileio/utils/adaptor.h>
#include <mayaUsd/fileio/utils/writeUtil.h>
#include <mayaUsd/fileio/writeJobContext.h>

PXR_NAMESPACE_OPEN_SCOPE

PXRUSDMAYA_REGISTER_WRITER(nurbsCurve, PxrUsdTranslators_NurbsCurveWriter);
PXRUSDMAYA_REGISTER_ADAPTOR_SCHEMA(nurbsCurve, UsdGeomNurbsCurves);


PxrUsdTranslators_NurbsCurveWriter::PxrUsdTranslators_NurbsCurveWriter(
        const MFnDependencyNode& depNodeFn,
        const SdfPath& usdPath,
        UsdMayaWriteJobContext& jobCtx) :
    UsdMayaPrimWriter(depNodeFn, usdPath, jobCtx)
{
    if (!TF_VERIFY(GetDagPath().isValid())) {
        return;
    }

    UsdGeomNurbsCurves primSchema =
        UsdGeomNurbsCurves::Define(GetUsdStage(), GetUsdPath());
    if (!TF_VERIFY(
            primSchema,
            "Could not define UsdGeomNurbsCurves at path '%s'\n",
            GetUsdPath().GetText())) {
        return;
    }
    _usdPrim = primSchema.GetPrim();
    if (!TF_VERIFY(
            _usdPrim,
            "Could not get UsdPrim for UsdGeomNurbsCurves at path '%s'\n",
            primSchema.GetPath().GetText())) {
        return;
    }
}

/* virtual */
void
PxrUsdTranslators_NurbsCurveWriter::Write(const UsdTimeCode& usdTime)
{
    UsdMayaPrimWriter::Write(usdTime);

    UsdGeomNurbsCurves primSchema(_usdPrim);
    writeNurbsCurveAttrs(usdTime, primSchema);
}

bool
PxrUsdTranslators_NurbsCurveWriter::writeNurbsCurveAttrs(
        const UsdTimeCode& usdTime,
        UsdGeomNurbsCurves& primSchema)
{
    MStatus status = MS::kSuccess;

    // Return if usdTime does not match if shape is animated
    if (usdTime.IsDefault() == _HasAnimCurves() ) {
        // skip shape as the usdTime does not match if shape isAnimated value
        return true;
    }

    MFnDependencyNode fnDepNode(GetDagPath().node(), &status);
    MString name = fnDepNode.name();

    MFnNurbsCurve curveFn(GetDagPath(), &status);
    if (!status) {
        TF_RUNTIME_ERROR(
                "MFnNurbsCurve() failed for curve at DAG path: %s",
                GetDagPath().fullPathName().asChar());
        return false;
    }

    // How to repeat the end knots.
    bool wrap = false;
    MFnNurbsCurve::Form form(curveFn.form());
    if (form == MFnNurbsCurve::kClosed ||
        form == MFnNurbsCurve::kPeriodic){
        wrap = true;
    }

    // Get curve attrs ======
    unsigned int numCurves = 1; // Assuming only 1 curve for now
    VtIntArray curveOrder(numCurves);
    VtIntArray curveVertexCounts(numCurves);
    VtFloatArray curveWidths(numCurves);
    VtVec2dArray ranges(numCurves);

    curveOrder[0] = curveFn.degree()+1;
    curveVertexCounts[0] = curveFn.numCVs();
    if (!TF_VERIFY(curveOrder[0] <= curveVertexCounts[0])) {
        return false;
    }
    curveWidths[0] = 1.0; // TODO: Retrieve from custom attr

    double mayaKnotDomainMin;
    double mayaKnotDomainMax;
    status = curveFn.getKnotDomain(mayaKnotDomainMin, mayaKnotDomainMax);
    CHECK_MSTATUS_AND_RETURN(status, false);
    ranges[0][0] = mayaKnotDomainMin;
    ranges[0][1] = mayaKnotDomainMax;

    MPointArray mayaCurveCVs;
    status = curveFn.getCVs(mayaCurveCVs, MSpace::kObject);
    CHECK_MSTATUS_AND_RETURN(status, false);
    VtVec3fArray points(mayaCurveCVs.length()); // all CVs batched together
    for (unsigned int i=0; i < mayaCurveCVs.length(); i++) {
        points[i].Set(mayaCurveCVs[i].x, mayaCurveCVs[i].y, mayaCurveCVs[i].z);
    }

    MDoubleArray mayaCurveKnots;
    status = curveFn.getKnots(mayaCurveKnots);
    CHECK_MSTATUS_AND_RETURN(status, false);
    VtDoubleArray curveKnots(mayaCurveKnots.length() + 2); // all knots batched together
    for (unsigned int i = 0; i < mayaCurveKnots.length(); i++) {
        curveKnots[i + 1] = mayaCurveKnots[i];
    }
    if (wrap) {
        curveKnots[0] = curveKnots[1] - (curveKnots[curveKnots.size() - 2] -
                                         curveKnots[curveKnots.size() - 3]);
        curveKnots[curveKnots.size() - 1] =
            curveKnots[curveKnots.size() - 2] + (curveKnots[2] - curveKnots[1]);
    } else {
        curveKnots[0] = curveKnots[1];
        curveKnots[curveKnots.size() - 1] = curveKnots[curveKnots.size() - 2];
    }

    // Gprim
    VtVec3fArray extent(2);
    UsdGeomCurves::ComputeExtent(points, curveWidths, &extent);
    UsdMayaWriteUtil::SetAttribute(primSchema.CreateExtentAttr(), &extent, usdTime, _GetSparseValueWriter());

    // find the number of segments: (vertexCount - order + 1) per curve
    // varying interpolation is number of segments + number of curves
    size_t accumulatedVertexCount =
        std::accumulate(curveVertexCounts.begin(), curveVertexCounts.end(), 0);
    size_t accumulatedOrder =
        std::accumulate(curveOrder.begin(), curveOrder.end(), 0);
    size_t expectedSegmentCount =
        accumulatedVertexCount - accumulatedOrder + curveVertexCounts.size();
    size_t expectedVaryingSize =
        expectedSegmentCount + curveVertexCounts.size();

    if (curveWidths.size() == 1)
        primSchema.SetWidthsInterpolation(UsdGeomTokens->constant);
    else if (curveWidths.size() == points.size())
        primSchema.SetWidthsInterpolation(UsdGeomTokens->vertex);
    else if (curveWidths.size() == curveVertexCounts.size())
        primSchema.SetWidthsInterpolation(UsdGeomTokens->uniform);
    else if (curveWidths.size() == expectedVaryingSize)
        primSchema.SetWidthsInterpolation(UsdGeomTokens->varying);
    else {
        TF_WARN(
            "MFnNurbsCurve has unsupported width size "
            "for standard interpolation metadata: %s",
            GetDagPath().fullPathName().asChar());
    }

    // Curve
    // not animatable
    UsdMayaWriteUtil::SetAttribute(primSchema.GetOrderAttr(), curveOrder, UsdTimeCode::Default(), _GetSparseValueWriter());
    UsdMayaWriteUtil::SetAttribute(primSchema.GetCurveVertexCountsAttr(), &curveVertexCounts, UsdTimeCode::Default(), _GetSparseValueWriter());
    UsdMayaWriteUtil::SetAttribute(primSchema.GetWidthsAttr(), &curveWidths, UsdTimeCode::Default(), _GetSparseValueWriter());

    UsdMayaWriteUtil::SetAttribute(primSchema.GetKnotsAttr(), &curveKnots, UsdTimeCode::Default(), _GetSparseValueWriter()); // not animatable
    UsdMayaWriteUtil::SetAttribute(primSchema.GetRangesAttr(), &ranges, UsdTimeCode::Default(), _GetSparseValueWriter()); // not animatable
    UsdMayaWriteUtil::SetAttribute(primSchema.GetPointsAttr(), &points, usdTime, _GetSparseValueWriter()); // CVs

    // TODO: Handle periodic and non-periodic cases

    return true;
}

/* virtual */
bool
PxrUsdTranslators_NurbsCurveWriter::ExportsGprims() const
{
    return true;
}


PXR_NAMESPACE_CLOSE_SCOPE
