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
#ifndef PXRUSDMAYA_READ_JOB_H
#define PXRUSDMAYA_READ_JOB_H

/// \file usdMaya/readJob.h

#include "jobArgs.h"
#include "../primReaderContext.h"

#include "pxr/pxr.h"

#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/primRange.h"

#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>

#include <map>
#include <string>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class UsdMayaPrimReaderArgs;

class UsdMaya_ReadJob
{
public:
    MAYAUSD_CORE_PUBLIC
    UsdMaya_ReadJob(
            const std::string& iFileName,
            const std::string& iPrimPath,
            const std::map<std::string, std::string>& iVariants,
            const UsdMayaJobImportArgs & iArgs);

    MAYAUSD_CORE_PUBLIC
    virtual ~UsdMaya_ReadJob();

    /// Reads the USD stage specified by the job file name and prim path.
    /// Newly-created DAG paths are inserted into the vector \p addedDagPaths.
    MAYAUSD_CORE_PUBLIC
    bool Read(std::vector<MDagPath>* addedDagPaths);

    /// Redoes a previous Read() operation after Undo() has been called.
    /// If Undo() hasn't been called, does nothing.
    MAYAUSD_CORE_PUBLIC
    bool Redo();

    /// Undoes a previous Read() operation, removing all added nodes.
    MAYAUSD_CORE_PUBLIC
    bool Undo();

    // Getters/Setters
    MAYAUSD_CORE_PUBLIC
    void SetMayaRootDagPath(const MDagPath &mayaRootDagPath);
    MAYAUSD_CORE_PUBLIC
    const MDagPath& GetMayaRootDagPath() const;

protected:

    MAYAUSD_CORE_PUBLIC
    virtual bool DoImport(UsdPrimRange& range, const UsdPrim& usdRootPrim);

    // Hook for derived classes to override the prim reader.  Returns true if
    // override was done, false otherwise.  Implementation in this class
    // returns false.
    MAYAUSD_CORE_PUBLIC
    virtual bool OverridePrimReader(
        const UsdPrim&               usdRootPrim,
        const UsdPrim&               prim,
        const UsdMayaPrimReaderArgs& args,
        UsdMayaPrimReaderContext&    readCtx,
        UsdPrimRange::iterator&      primIt
    );

    // Engine method for DoImport().  Covers the functionality of a regular
    // usdImport.
    MAYAUSD_CORE_PUBLIC
    bool _DoImport(UsdPrimRange& range, const UsdPrim& usdRootPrim);

    // Data
    UsdMayaJobImportArgs mArgs;
    std::string mFileName;
    std::map<std::string,std::string> mVariants;
    UsdMayaPrimReaderContext::ObjectRegistry mNewNodeRegistry;

private:

    // Data
    std::string mPrimPath;
    MDagModifier mDagModifierUndo;
    bool mDagModifierSeeded;
    MDagPath mMayaRootDagPath;
};


PXR_NAMESPACE_CLOSE_SCOPE


#endif
