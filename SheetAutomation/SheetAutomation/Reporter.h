/*----------------------------------------------------------------------+
|									|
| Copyright , Bentley Systems, Incorporated, 			|
| All rights reserved.							|
|									|
|									|
| Limited permission is hereby granted to reproduce and modify this	|
| copyrighted material provided that the resulting code is used only in |
| conjunction with Bentley Systems products under the terms of the	|
| license agreement provided therein, and that this notice is retained	|
| in its entirety in any such reproduction or modification.		|
|									|
+----------------------------------------------------------------------*/
//#if !defined(resource)
#include    <mdl.h>
#include <GPArray.h>
#include <ElementGraphics.h>
#include <imodel\xmlinstanceschemamanager.h>
#include <IModel\xmlinstanceapi.h>
#include <IModel\stringlisthandle.h>
#include <IModel\imodelpublisher.h>

#include    <MicroStationAPI.h>
#include    <stdio.h>
#include    <string.h>
#include    <malloc.h>
#include    <mselemen.fdf>
#include    <mselmdsc.fdf>
#include    <mslinkge.fdf>
#include    <msscancrit.fdf>
#include    <mstagdat.fdf>
#include    <mselems.h>
#include    <mscell.fdf>
#include    <leveltable.fdf>
#include    <mslstyle.fdf>
#include    <msstrlst.h>
#include    <mscnv.fdf>
#include    <msdgnobj.fdf>
#include    <msmodel.fdf>
#include    <msview.fdf>
#include    <msviewinfo.fdf>
#include    <msvar.fdf>
#include    <dlmsys.fdf>
#include    <msdialog.fdf>

#include    <msrmgr.h>
#include    <mssystem.fdf>
#include    <msparse.fdf>

#include	<toolsubs.h>

//scan code
#include	<elementref.h>
//locate code
#include	<msdependency.fdf>
#include	<msassoc.fdf>
#include	<msmisc.fdf>
#include	<mslocate.fdf>
#include	<msstate.fdf>
#include	<msoutput.fdf>
#include    <mskisolid.h>
#include    <mdlxmltools.fdf>
#include    <mskisolid.fdf>
#include    <mdlxmltoolslink.fdf>
#include    <msnamedgroup.fdf>
#if defined (BSI)
#include  <jmtg.h>
#include <mimesh.h>
#include <mimdlmeshmtg.fdf>

#include <mspsolid.fdf>
//#include "SolidConverter.fdf"
#endif
#include <msbsplin.fdf>
#include <msmdlmesh.fdf>

#include <wchar.h>
//place command
#include	<mstmatrx.fdf>
#include     "XmlHelper.h"
//#endif
//#define BSI_SPECIAL

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_USTN
USING_NAMESPACE_BENTLEY_USTN_ELEMENT
USING_NAMESPACE_BENTLEY_XMLINSTANCEAPI_NATIVE

enum AdjustmentType
      {
      Adjustment_None = 0,
      Adjustment_GO_ToMaster = 1
      };

//this is used to determine the kernel that was used to build the solids.  It is not normally used in applications.
extern "C" void* mdlKISolid_getKernelEntity 
( 
KIENTITY    *kiEntityP 
);

extern "C" void        mdlSolid_getDgnTransform (TransformP solidToDGN, TransformP dgnToSolid, DPoint3dCP origin, DgnModelRefP modelRef);

extern "C" int      mdlSolid_transformPointsIntoSolidModelingCube
(
DPoint3d        *pPoints,           // in master units
Transform       *pFwdTransform,     // optional
Transform       *pInvTransform,     // optional
int             numPoints,
DgnModelRefP    modelRef
);

//extern "C" BODY*  ConvertFacetsToSolids (ElementFacetSet* pFacets);
/*
A simple class to report on the geometry of an i-model.
*/
class           Reporter
{


public:
    DgnFileP        m_dgnFile;
    XmlNodeRef      m_rootNodePtr;
    Transform       m_currentTransform;
    XmlDomRef       m_pXMLDom;
    bool            m_processXMLFragments;
    bool            m_showArrays;
    double          m_currentUorsToMaster;
    DPoint3d        m_currentGlobalOrigin;
    WString         m_copyFileName;
    DgnModelRefP    m_currentModelRef;
    DgnModelRefP    m_copymodel;
    int             m_passCount;
/*
The constructor(s) for the class.
*/
    Reporter (){};
    Reporter (wchar_t* rootElementName, DgnFileP dgnFile, bool showArrays, bool processXMLFragments) :
                  m_pXMLDom (NULL), m_dgnFile (dgnFile), m_showArrays (showArrays), m_processXMLFragments (processXMLFragments)
    {
    int status;
    XmlHelper::CreateDomInstance (&m_pXMLDom);
    m_dgnFile = dgnFile;
    m_copyFileName=WString(L"");
    // create the report XML document and its root node
  
    status = mdlXMLDom_createElement (&m_rootNodePtr,m_pXMLDom,rootElementName);
    status = mdlXMLDom_appendChild (m_pXMLDom,m_rootNodePtr);
    status = mdlXMLDom_setRootElement (m_pXMLDom,m_rootNodePtr);
  
    mdlTMatrix_getIdentity(&m_currentTransform);
    
    m_currentUorsToMaster   = 1.0;
    
    m_currentGlobalOrigin.x = 0.0;
    m_currentGlobalOrigin.y = 0.0;
    m_currentGlobalOrigin.z = 0.0;
    m_currentModelRef = MASTERFILE;
    m_passCount = 0;
    }

    static void log_printf(long  level,char  *szFormat,...);
/*
The methods that the class instance supports.  This is the public API.
*/
    int    GetPassCount();
    void   SetPassCount(int value){m_passCount = value;};
    double CvtUorToMaster (double uorValue);
    bool  SuppressArray (size_t count, MSWCharP outerName, XmlNodeRef parentNode);
    void  SetGlobalOrigin ();
    void  GetGlobalOrigin(DPoint3dP  originP);
    bool  Save (WString xmlFileName);
    bool  CopyModel();
    DgnModelRefP GetCopyModelRef();
    bool  CloseCopyModel();
    WString GetCopyModelFileName();
    void    SetCopyFileName (WString name);
    XmlNodeRef Reporter::GetRootNodePtr ();
    void  DebugNode(WString info);
    void  SetCurrentModelRef (DgnModelRefP  pModel);
    DgnModelRefP    GetCurrentModelRef ();
    Transform       GetCurrentTransform(){return m_currentTransform;};
    void            SetCurrentTransform(Transform& t){m_currentTransform = t;};
    void  ReportDpoint3d (MSWCharCP elementName, const DPoint3d& point, XmlNodeRef parentNodePtr, AdjustmentType adjustment);
    void  ReportDPoint2d (MSWCharCP elementName, const DPoint2d& point, XmlNodeRef parentNodePtr, AdjustmentType adjustment);
    void  ReportArray (DPoint3dCP source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment);
    void  ReportArray (DPoint2dCP source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment);
    void  ReportArray (double *source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment);
    void  ReportDVec3d (MSWCharCP elementName, const DVec3d& vec, XmlNodeRef parentNodePtr, AdjustmentType adjustment);
    void  ReportRotMatrix (MSWCharCP elementName, const RotMatrix& orientation, XmlNodeRef parentNodePtr);
    void  ReportTransformDetails (MSWCharCP elementName,const Transform& trans, XmlNodeRef parentNodePtr);
    void  ReportConeDetails (MSWCharCP    nodeName,DVec3dCR  primary, DVec3dCR  secondary,DPoint3dCR p0, DPoint3dCR p1, double     r0, double     r1, bool       cap, XmlNodeRef parentNodePtr);
    void  ReportCurveDetails (MSWCharCP nodeName, MSBsplineCurveCR curve,bool filled, XmlNodeRef parentNodePtr);
    void  ReportEllipseDetails (MSWCharCP nodeName, DEllipse3d& ellipse, XmlNodeRef parentNodePtr, bool includeAngles);
    void  ReportPointStringDetails (DPoint3dCP points, size_t nPoints, bool isDisjoint, XmlNodeRef parentNodePtr);
    void  ReportLineStringDetails (DPoint3dCP points, size_t nPoints, XmlNodeRef parentNodePtr);
    void  ReportLineDetails (DSegment3d& segment, XmlNodeRef parentNodePtr);
    void  ReportEdge(KIEDGE* edgeP,XmlNodeRef loopNodeP,int pointNumber);
    KICOEDGE*  ReportCoEdge(KICOEDGE* curCoedgeP, XmlNodeRef loopNodeP,int pointNumber);
    void  ReportSolidDetails (KIENTITY*      pBody, XmlNodeRef parentNode,MSWCharCP tagName);
    void  ReportSolid(MSWCharCP tagName,KIENTITY* pBody, XmlNodeRef parentNodePtr);
    void  ReportBsplineParams (MSWCharCP tagName, BsplineParam* params, XmlNodeRef      parentNode);
    void  ReportBsplineDisplay (MSWCharCP tagName, BsplineDisplay* params, XmlNodeRef    parentNode);
    void  ReportBsplineSurface (MSWCharCP tagName, MSBsplineSurface* surface, XmlNodeRef parentNode);
    void  ReportFacetSet (MSWCharCP tagName, ElementFacetSet* pFacets, XmlNodeRef parentNodePtr);
    void  ReportGPArray (MSWCharCP tagName, GPArrayP gpa, XmlNodeRef parentNodePtr);
};
