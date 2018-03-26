#pragma once

#include "AL/maya/utils/ForwardDeclares.h"

/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, the status is returned.
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR(status, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if (!_status_##__LINE__) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
    return status; \
  } }

/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, the program execution continues.
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR2(status, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if ((_status_##__LINE__) != MS::kSuccess) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
  } }

/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, then a "continue" statement is issued (and so should be used in loops).
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR_CONTINUE(status, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if (!_status_##__LINE__) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
    continue; \
  } }

/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, a null MObject is returned.
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR_RETURN_NULL_MOBJECT(status, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if (!_status_##__LINE__) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
    return MObject::kNullObj; \
  } }

/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, then exit immediately, with no return.
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR_RETURN(status, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if (!_status_##__LINE__) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
    return; \
  } }


/// \brief  Given the status, validates that the status is ok. If not, an error is logged using the specified error
///         message. If an error occurs, then exit immediately, returning the given value.
/// \ingroup   mayautils
#define AL_MAYA_CHECK_ERROR_RETURN_VAL(status, returnVal, ErrorString) { \
  MStatus _status_##__LINE__ = status; \
  if (!_status_##__LINE__) \
  { \
    MString maya_error_string = __FILE__ ":"; \
    maya_error_string += __LINE__; \
    maya_error_string += " "; \
    maya_error_string += _status_##__LINE__.errorString(); \
    maya_error_string += " : "; \
    maya_error_string += ErrorString; \
    MGlobal::displayError(maya_error_string); \
    return returnVal; \
  } }

/// \ingroup   mayautils
/// \brief  utility macro to check that an SdfLayerHandle is actually valid.
#define LAYER_HANDLE_CHECK(X) \
  if(!X) \
    std::cout << "Layer is invalid " << __FILE__ << " " << __LINE__ << std::endl;

#if AL_GENERATING_DOCS
/// \brief  use this macro one within an MPxCommand derived class
/// \ingroup   mayautils
# define AL_MAYA_DECLARE_COMMAND()

/// \brief  Use this macro in a cpp file to implement some of the boiler plate code for your MEL command.
///         Specify the Command itself, and a namespace which will be prefixed to the maya node name. E.g.
///         If the command class is MyMelCommand and the namespace is AL_usdmaya, then the resulting command
//          name in Maya will be "AL_usdmaya_MyMelCommand"
/// \ingroup   mayautils
# define AL_MAYA_DEFINE_COMMAND(COMMAND, NAMESPACE)

/// \brief  Use this macro within an MPxNode derived class
/// \ingroup   mayautils
# define AL_MAYA_DECLARE_NODE()

/// \brief  Use this macro in a cpp file to implement some of the boiler plate code for your custom Maya node.
///         Specify the Node class itself, its typeId, and a namespace which will be prefixed to the maya node
///         name. E.g. If the node class is MyNode and the namespace is AL_usdmaya, then the resulting node
//          name in Maya will be "AL_usdmaya_MyNode"
/// \ingroup   mayautils
# define AL_MAYA_DEFINE_NODE(NODE, TYPEID, NAMESPACE)

/// \brief  Use this macro within your commands doIt method to implement the help text printed.
/// \ingroup   mayautils
# define AL_MAYA_COMMAND_HELP(database, __helpText)
#else
# define AL_MAYA_DECLARE_COMMAND() \
    static const char* const g_helpText; \
    static void* creator(); \
    static MSyntax createSyntax(); \
    static const MString kName;

# define AL_MAYA_DEFINE_COMMAND(COMMAND, NAMESPACE) \
    void* COMMAND :: creator() { return new COMMAND; } \
    const MString COMMAND :: kName(#NAMESPACE "_" #COMMAND);

# define AL_MAYA_DECLARE_NODE() \
    static void* creator(); \
    static MStatus initialise(); \
    static const MString kTypeName; \
    static const MTypeId kTypeId;

# define AL_MAYA_DEFINE_NODE(NODE, TYPEID, NAMESPACE) \
    void* NODE :: creator() { return new NODE; } \
    const MString NODE :: kTypeName(#NAMESPACE "_" #NODE); \
    const MTypeId NODE :: kTypeId(TYPEID);

# define AL_MAYA_COMMAND_HELP(database, __helpText) \
    if(database.isFlagSet("-h")) { \
      MGlobal::displayInfo( __helpText ); \
      return MS::kSuccess; \
    }
#endif

/// a macro to register an MPxCommand derived command with maya
/// \ingroup   mayautils
#define AL_REGISTER_COMMAND(plugin, X) { \
  MStatus status = plugin.registerCommand( \
      X ::kName, \
      X ::creator, \
      X ::createSyntax); \
  if(!status) { \
    status.perror("unable to register command " #X); \
    return status; \
  }}

/// a macro to register an MPxFileTranslator derived command with maya
/// \ingroup   mayautils
#define AL_REGISTER_TRANSLATOR(plugin, X) { \
  MStatus status = X ::registerTranslator(plugin); \
  if(!status) { \
    status.perror("unable to register file translator " #X); \
    return status; \
  }}

/// a macro to register an MPxNode derived node with maya
/// \ingroup   mayautils
#define AL_REGISTER_DEPEND_NODE(plugin, X){ \
  MStatus status = plugin.registerNode( \
      X ::kTypeName, \
      X ::kTypeId, \
      X ::creator, \
      X ::initialise); \
  if(!status) { \
    status.perror("unable to register depend node " #X); \
    return status; \
  }}

/// a macro to register an MPxShape derived node with maya
/// \ingroup   mayautils
#define AL_REGISTER_SHAPE_NODE(plugin, X, UI, DRAW){ \
  MStatus status = plugin.registerShape( \
      X ::kTypeName, \
      X ::kTypeId, \
      X ::creator, \
      X ::initialise, \
      UI ::creator, \
      &DRAW ::kDrawDbClassification); \
  if(!status) { \
    status.perror("unable to register shape node " #X); \
    return status; \
  }}

/// a macro to register an MPxTransform derived node with maya
/// \ingroup   mayautils
#define AL_REGISTER_TRANSFORM_NODE(plugin, NODE, MATRIX){ \
  MStatus status = plugin.registerTransform( \
    NODE ::kTypeName, \
    NODE ::kTypeId, \
    NODE ::creator, \
    NODE ::initialise, \
    MATRIX ::creator, \
    MATRIX ::kTypeId); \
  if(!status) { \
    status.perror("unable to register transform node " #NODE); \
    return status; \
  }}

/// a macro to register an MPxData derived object with maya
/// \ingroup   mayautils
#define AL_REGISTER_DATA(plugin, X){ \
  MStatus status = plugin.registerData( \
    X ::kName, \
    X ::kTypeId, \
    X ::creator); \
  if(!status) { \
    status.perror("unable to register data " #X); \
    return status; \
  }}

/// a macro to register a custom draw override with maya
/// \ingroup   mayautils
#define AL_REGISTER_DRAW_OVERRIDE(plugin, X) { \
  MStatus status = MHWRender::MDrawRegistry::registerDrawOverrideCreator( \
    X ::kDrawDbClassification, \
    X ::kDrawRegistrantId, \
    X ::creator); \
  if (!status) { \
    status.perror("unable to register draw override " #X); \
    return status; \
  }}

/// a macro to unregister a MEL command from maya
/// \ingroup   mayautils
#define AL_UNREGISTER_COMMAND(plugin, X) { \
  MStatus status = plugin.deregisterCommand(X ::kName); \
  if (!status) { \
      status.perror("deregisterCommand AL::usdmaya::" #X); \
      return status; \
  }}
/// a macro to unregister a custom node from maya
/// \ingroup   mayautils
#define AL_UNREGISTER_NODE(plugin, X) { \
  MStatus status = plugin.deregisterNode(X ::kTypeId); \
  if (!status) { \
      status.perror("deregisterNode AL::usdmaya::" #X); \
      return status; \
  }}
/// a macro to unregister a custom MPxData derived object from maya
/// \ingroup   mayautils
#define AL_UNREGISTER_DATA(plugin, X) {\
  MStatus status = plugin.deregisterData(X  ::kTypeId); \
  if (!status) { \
      status.perror("deregisterData AL::usdmaya::" #X); \
      return status; \
  }}
/// a macro to unregister a custom MPxFileTranslator derived object from maya
/// \ingroup   mayautils
#define AL_UNREGISTER_TRANSLATOR(plugin, X) {\
  MStatus status = X ::deregisterTranslator(plugin); \
  if(!status) { \
    status.perror("deregisterTranslator AL::usdmaya::" #X); \
    return status; \
  }}
/// a macro to unregister a custom draw override from Maya
/// \ingroup   mayautils
#define AL_UNREGISTER_DRAW_OVERRIDE(plugin, X) { \
  MStatus status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator( \
      X ::kDrawDbClassification, \
      X ::kDrawRegistrantId); \
  if (!status) { \
    status.perror("deregisterDrawOverrideCreator " #X); \
    return status; \
  }}