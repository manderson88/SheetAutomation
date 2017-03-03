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
#include "Reporter.h"

//class           Reporter
//{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
Private void    dumpString
(
char *poo
)
    {
    printf ("%s \n",poo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     Reporter::log_printf
(
long  level,
char  *szFormat,
...
)
    {
    va_list     arg;
    char        szBuf[512];
    static int   count;
    
    if (level < -1)
        return;

    va_start (arg, szFormat);
    vsprintf (szBuf, szFormat, arg);

    dumpString (szBuf);
   
    }
//public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void         Reporter::DebugNode(WString info)
{
    XmlNodeRef parentNode = GetRootNodePtr();
    //XmlNodeRef dumpNode;
    XmlHelper::AddElementStringValue(parentNode, m_pXMLDom, L"debugInfo", info.GetMSWCharCP());
    //mdlXMLDom_createElement(&dumpNode,m_pXMLDom,L"debugInfo");
    //XmlHelper::AddAttributeStringValue (dumpNode,L"DBG:",info);
    //mdlXMLDomElement_appendChild(parentNode,dumpNode);

}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
XmlNodeRef       Reporter::GetRootNodePtr ()
    {
    return m_rootNodePtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool             Reporter::Save (WString xmlFileName)
    {
    return XmlHelper::Save (m_pXMLDom, xmlFileName.GetMSWCharCP());
    m_passCount = 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool             Reporter::CopyModel()
    {
    char targetName[512];
    char currentFileName[512];
    char dev[MAXDEVICELENGTH];
    char directory[MAXDIRLENGTH];
    char name[MAXNAMELENGTH];
    char noImodelName[MAXNAMELENGTH];
    char extension[MAXEXTENSIONLENGTH];
    int  status;
    status = mdlModelRef_getFileName (ACTIVEMODEL,currentFileName,512);
    mdlFile_parseName (currentFileName,dev,directory,name,extension);
    memset (noImodelName,'\0',sizeof noImodelName);
    strncpy (noImodelName,name,strlen(name)-2);
    mdlFile_buildName (targetName,dev,directory,noImodelName,"dgn");
    status = mdlWorkDgn_createFile (&m_copymodel,targetName,DGNFILE_FORMAT_V8,ACTIVEMODEL,SEED_CopyAllData|SEED_CreateDefaultModel,NULL,NULL,TRUE);
   // WString copyFileName = WString (targetName);
   // SetCopyFileName(copyFileName);
    status = mdlWorkDgn_closeFile(m_copymodel);
    return  true;
    }
DgnModelRefP     Reporter::GetCopyModelRef()
    {
    return  m_copymodel;
    }
WString          Reporter::GetCopyModelFileName()
    {
    WString currentTarget;
    char currentFileName[512];

   // mdlModelRef_getFileName (m_copymodel,currentFileName,512);
    
    return  (WString (currentFileName));
    }
void             Reporter::SetCopyFileName (WString name)
    {
    m_copyFileName = name;
    }
bool             Reporter::CloseCopyModel ()
    {
    //mdlWorkDgn_closeFile(m_copymodel);
    return  TRUE;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double Reporter::CvtUorToMaster (double uorValue)
    {
    if (m_currentModelRef)
        m_currentUorsToMaster=mdlModelRef_getUorPerMaster (m_currentModelRef);
    
    return uorValue/m_currentUorsToMaster;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
bool Reporter::SuppressArray (size_t count, MSWCharP outerName, XmlNodeRef parentNode)
    {
    if (m_showArrays)
        return false;
    XmlNodeRef arrayNode;// = m_pXMLDom->createElement (outerName);
    mdlXMLDom_createElement(&arrayNode,m_pXMLDom,outerName);
    mdlXMLDomElement_appendChild(parentNode,arrayNode);

    //parentNode->appendChild (arrayNode);
    XmlHelper::AddAttributeInt64Value  (arrayNode, L"count", count);
    XmlHelper::AddAttributeStringValue  (arrayNode, L"remark", L"Array Contents Omitted");
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void            Reporter::SetCurrentModelRef (DgnModelRefP  pModel)
    {
    m_currentModelRef = pModel;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelRefP    Reporter::GetCurrentModelRef ()
    {
    return  m_currentModelRef;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void            Reporter::SetGlobalOrigin ()
    {
    if (m_currentModelRef)
        mdlModelRef_getGlobalOrigin (m_currentModelRef , &m_currentGlobalOrigin);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void            Reporter::GetGlobalOrigin(DPoint3dP  originP)
    {
    originP->x = m_currentGlobalOrigin.x;
    originP->y = m_currentGlobalOrigin.y;
    originP->z = m_currentGlobalOrigin.z;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportDpoint3d (MSWCharCP elementName, const DPoint3d& point, XmlNodeRef parentNodePtr, AdjustmentType adjustment)
    {
    log_printf (8,"point data is %lf,%lf,%lf \n",point.x,point.y,point.z);

    int status;
    XmlNodeRef detailNodePtr;// = m_pXMLDom->createElement (elementName);
    status = mdlXMLDom_createElement(&detailNodePtr,m_pXMLDom,elementName);
    status = mdlXMLDomElement_appendChild(parentNodePtr,detailNodePtr);
    // use the following to test for DISCONNECT points since there is some loss of accuracy when reading from an XML string
    if (point.x == DISCONNECT || point.y == DISCONNECT || point.z == DISCONNECT)
        {
        XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"x", point.x);
        XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"y", point.y);
        XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"z", point.z);
        return;
        }

    DPoint3d adjustedPoint = point;

    if (Adjustment_GO_ToMaster == adjustment)
        {
        // adjust by global origin
        adjustedPoint.x -= m_currentGlobalOrigin.x;
        adjustedPoint.y -= m_currentGlobalOrigin.y;
        adjustedPoint.z -= m_currentGlobalOrigin.z;
        }

    if (Adjustment_None != adjustment)
        {
        // adjust from uor to master
        adjustedPoint.x = CvtUorToMaster(adjustedPoint.x);
        adjustedPoint.y = CvtUorToMaster(adjustedPoint.y);
        adjustedPoint.z = CvtUorToMaster(adjustedPoint.z);
        }

    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"x", adjustedPoint.x);
    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"y", adjustedPoint.y);
    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"z", adjustedPoint.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportDPoint2d (MSWCharCP elementName, const DPoint2d& point, XmlNodeRef parentNodePtr, AdjustmentType adjustment)
    {
    int status;
    XmlNodeRef detailNodePtr;// = m_pXMLDom->createElement (elementName);
    status = mdlXMLDom_createElement(&detailNodePtr,m_pXMLDom,elementName);

    if (NULL != detailNodePtr)
        {
        status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);

        if (point.x == DISCONNECT || point.y == DISCONNECT)
            {
            XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"x", point.x);
            XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"y", point.y);
            return;
            }

        DPoint2d adjustedPoint = point;

        if (Adjustment_GO_ToMaster == adjustment)
            {
            // adjust by global origin
            adjustedPoint.x -= m_currentGlobalOrigin.x;
            adjustedPoint.y -= m_currentGlobalOrigin.y;
            }

        if (Adjustment_None != adjustment)
            {
            // adjust from uor to master
            adjustedPoint.x = CvtUorToMaster(adjustedPoint.x);
            adjustedPoint.y = CvtUorToMaster(adjustedPoint.y);
            }

        XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"x", adjustedPoint.x);
        XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"y", adjustedPoint.y);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportArray (DPoint3dCP source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment)
    {
    if (NULL == source)
        return;
    if (SuppressArray (count, outerName, parentNode))
        return;
    int status;
    XmlNodeRef arrayNode;// = m_pXMLDom->createElement (outerName);
    status = mdlXMLDom_createElement(&arrayNode,m_pXMLDom,outerName);
    if (NULL != arrayNode)
        {
        status = mdlXMLDomElement_appendChild (parentNode,arrayNode);
        for (size_t i = 0; i < count; ++i)
            {
            ReportDpoint3d (innerName, source[i], arrayNode, Adjustment_GO_ToMaster);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportArray (DPoint2dCP source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment)
    {
    if (NULL == source)
        return;
    if (SuppressArray (count, outerName, parentNode))
        return;
    XmlNodeRef arrayNode;// = m_pXMLDom->createElement (outerName);
    mdlXMLDom_createElement (&arrayNode,m_pXMLDom,outerName);
    if (NULL != arrayNode)
        {
        //parentNode->appendChild (arrayNode);
        mdlXMLDomElement_appendChild(parentNode,arrayNode);

        for (size_t i = 0; i < count; ++i)
            {
            ReportDPoint2d (innerName, source[i], arrayNode, adjustment);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportArray (double *source, size_t count, MSWCharP outerName, MSWCharP innerName, XmlNodeRef parentNode, AdjustmentType adjustment)
    {
    if (NULL == source)
        return;
    if (SuppressArray (count, outerName, parentNode))
        return;
    int status;
    XmlNodeRef arrayNode;// = m_pXMLDom->createElement (outerName);
    status = mdlXMLDom_createElement (&arrayNode,m_pXMLDom,outerName);
    if (NULL != arrayNode)
        {
       //parentNode->appendChild (arrayNode);
        status = mdlXMLDomElement_appendChild(parentNode,arrayNode);

        for (size_t i = 0; i < count; ++i)
            {
            XmlHelper::AddElementDoubleValue (arrayNode, m_pXMLDom, innerName, source[i]);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportDVec3d (MSWCharCP elementName, const DVec3d& vec, XmlNodeRef parentNodePtr, AdjustmentType adjustment)
    {
    int status;
    XmlNodeRef detailNodePtr;// = m_pXMLDom->createElement (elementName);
    status = mdlXMLDom_createElement (&detailNodePtr,m_pXMLDom,elementName);
    if (NULL == detailNodePtr)
        return;
    status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);
    //parentNodePtr->appendChild (detailNodePtr);

    DVec3d adjustedVec = vec;

    if (Adjustment_GO_ToMaster == adjustment)
        {
        // adjust by global origin
        adjustedVec.x -= m_currentGlobalOrigin.x;
        adjustedVec.y -= m_currentGlobalOrigin.y;
        adjustedVec.z -= m_currentGlobalOrigin.z;
        }

    if (Adjustment_None != adjustment)
        {
        // adjust from uor to master
        adjustedVec.x = CvtUorToMaster(adjustedVec.x);
        adjustedVec.y = CvtUorToMaster(adjustedVec.y);
        adjustedVec.z = CvtUorToMaster(adjustedVec.z);
        }

    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"x", vec.x);
    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"y", vec.y);
    XmlHelper::AddAttributeDoubleValue (detailNodePtr, L"z", vec.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportRotMatrix (MSWCharCP elementName, const RotMatrix& orientation, XmlNodeRef parentNodePtr)
    {
    // Orientation
    int status;
    XmlNodeRef detailNodePtr;// = m_pXMLDom->createElement (elementName);
    status = mdlXMLDom_createElement (&detailNodePtr,m_pXMLDom,elementName);

    if (NULL != detailNodePtr)
        {
        //parentNodePtr->appendChild (detailNodePtr);
        status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);

        for (int iRow=0; iRow<3; ++iRow)
            {
            //MSWChar    rowLbl[24];
            //wsprintf (rowLbl, L"Row%d", iRow);
            WString pString = WString (L"Row%d",iRow);
            XmlNodeRef rowNodePtr;// = m_pXMLDom->createElement (rowLbl);
            status = mdlXMLDom_createElement(&rowNodePtr,m_pXMLDom,pString.GetMSWCharCP());
            if (NULL != rowNodePtr)
                {
                //detailNodePtr->appendChild (rowNodePtr);
                status = mdlXMLDomElement_appendChild (detailNodePtr,rowNodePtr);

                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col1", orientation.form3d[iRow][0]);
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col2", orientation.form3d[iRow][1]);
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col3", orientation.form3d[iRow][2]);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportTransformDetails (MSWCharCP elementName,const Transform& trans, XmlNodeRef parentNodePtr)
{
int status;
XmlNodeRef detailNodePtr;
status = mdlXMLDom_createElement (&detailNodePtr,m_pXMLDom,elementName);

m_currentTransform = trans;

    if (NULL != detailNodePtr)
        {
        //parentNodePtr->appendChild (detailNodePtr);
        status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);
        for (int iRow=0; iRow<3; ++iRow)
            {
            WString pString = WString (L"Row");
            XmlNodeRef rowNodePtr;// = m_pXMLDom->createElement (rowLbl);
            status = mdlXMLDom_createElement(&rowNodePtr,m_pXMLDom,pString.GetMSWCharCP());
            if (NULL != rowNodePtr)
                {
                status = mdlXMLDomElement_appendChild (detailNodePtr,rowNodePtr);
                
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col1", trans.form3d[iRow][0]);
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col2", trans.form3d[iRow][1]);
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col3", trans.form3d[iRow][2]);
                XmlHelper::AddAttributeDoubleValue (rowNodePtr, L"col4", trans.form3d[iRow][3]);
                }
            }
        }   
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportConeDetails (
                                                MSWCharCP    nodeName,        
                                                DVec3dCR  primary,
                                                DVec3dCR  secondary,
                                                DPoint3dCR p0,
                                                DPoint3dCR p1,
                                                double     r0,
                                                double     r1,
                                                bool       cap,
                                                XmlNodeRef parentNodePtr
                                              )
    {
    int status=ERROR;
    
    XmlNodeRef detailNodePtr=NULL;// = m_pXMLDom->createElement (elementName);
    status = mdlXMLDom_createElement (&detailNodePtr,m_pXMLDom,nodeName);
    if (NULL != detailNodePtr)
        {
        if (SUCCESS == status)
            {
            XmlHelper::AddElementDoubleValue (detailNodePtr, m_pXMLDom, L"radius_1",CvtUorToMaster( r0));
            XmlHelper::AddElementDoubleValue (detailNodePtr, m_pXMLDom, L"radius_2", CvtUorToMaster(r1));

            ReportDpoint3d (L"Primary_Point",p0,detailNodePtr,Adjustment_None);
            ReportDpoint3d (L"Secondary_Point",p1,detailNodePtr,Adjustment_None);
            ReportDVec3d (L"Primary_Vector",primary,detailNodePtr,Adjustment_None);
            ReportDVec3d (L"Secondary_Vector",secondary,detailNodePtr,Adjustment_None);

            XmlHelper::AddAttributeBoolValue (detailNodePtr,L"cap",cap);
            }
         status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportCurveDetails (MSWCharCP nodeName, MSBsplineCurveCR curve,bool filled, XmlNodeRef parentNodePtr)
    {
    int status;
    XmlNodeRef  detailNodePtr;
    status = mdlXMLDom_createElement (&detailNodePtr,m_pXMLDom,nodeName);

    if (NULL == detailNodePtr)
        return;

    XmlHelper::AddAttributeInt32Value (detailNodePtr,L"Bspline_Type ",curve.type);

    status = mdlXMLDomElement_appendChild (parentNodePtr,detailNodePtr);

    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportEllipseDetails (MSWCharCP nodeName, DEllipse3d& ellipse, XmlNodeRef parentNodePtr, bool includeAngles = true)
    {
    int status;
    DPoint3d  center;
    double    col0Scale, col1Scale, start, sweep;

    col0Scale = 1;
    col1Scale = 0;
    start = ellipse.start;
    sweep = ellipse.sweep;

    center = ellipse.center;

    XmlNodeRef currentNodePtr;
    status = mdlXMLDom_createElement (&currentNodePtr,m_pXMLDom,nodeName);

    if (NULL == currentNodePtr)
        return;

    status = mdlXMLDomElement_appendChild (parentNodePtr,currentNodePtr);

    ReportDpoint3d (L"Center", center, currentNodePtr, Adjustment_GO_ToMaster);
    
    XmlHelper::AddElementDoubleValue (currentNodePtr, m_pXMLDom, L"Col0Scale", CvtUorToMaster(col0Scale)); 
    XmlHelper::AddElementDoubleValue (currentNodePtr, m_pXMLDom, L"Col1Scale", CvtUorToMaster(col1Scale));
    if (includeAngles)
        {
        XmlHelper::AddElementDoubleValue (currentNodePtr, m_pXMLDom, L"StartAngle", start);
        XmlHelper::AddElementDoubleValue (currentNodePtr, m_pXMLDom, L"SweepAngle", sweep);
        }
    }                                                                  


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportPointStringDetails (DPoint3dCP points, size_t nPoints, bool isDisjoint, XmlNodeRef parentNodePtr)
    {
    int status;
    XmlNodeRef currentNodePtr;// = m_pXMLDom->createElement (L"PointString_Details");
    status = mdlXMLDom_createElement (&currentNodePtr,m_pXMLDom,L"PointString_Details");

    if (NULL != currentNodePtr)
        {
        XmlHelper::AddElementBoolValue (currentNodePtr, m_pXMLDom, L"IsDisjoint", isDisjoint); 
        status = mdlXMLDomElement_appendChild (parentNodePtr,currentNodePtr);
    
        XmlNodeRef pointArrayNodePtr;// = m_pXMLDom->createElement (L"Points");
        status = mdlXMLDom_createElement (&pointArrayNodePtr,m_pXMLDom,L"Points");

        if (NULL != pointArrayNodePtr)
            {
            status = mdlXMLDomElement_appendChild(currentNodePtr,pointArrayNodePtr);

            for (size_t iPoint=0; iPoint<nPoints; ++iPoint)
                ReportDpoint3d (L"Point", points[iPoint], pointArrayNodePtr, Adjustment_GO_ToMaster);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportLineStringDetails (DPoint3dCP points, size_t nPoints, XmlNodeRef parentNodePtr)
    {
    int status;
    XmlNodeRef currentNodePtr;// = m_pXMLDom->createElement (L"LineString_Details");
    status = mdlXMLDom_createElement (&currentNodePtr,m_pXMLDom,L"LineString_Details");

    if (NULL != currentNodePtr)
        {
        status = mdlXMLDomElement_appendChild (parentNodePtr,currentNodePtr);

        XmlNodeRef pointArrayNodePtr;
        status = mdlXMLDom_createElement(&pointArrayNodePtr,m_pXMLDom,L"Points");
        if (NULL != pointArrayNodePtr)
            {
            status = mdlXMLDomElement_appendChild (currentNodePtr,pointArrayNodePtr);

            for (size_t iPoint=0; iPoint<nPoints; ++iPoint)
                ReportDpoint3d (L"Point", points[iPoint], pointArrayNodePtr, Adjustment_GO_ToMaster);
            }
        }
    }
//////////////////////////////////////////////////////////////////////
///	Dump debug info about a solid
void	TraceSolid	(KIENTITY const* pBody, char const* message)
{
    printf ("TraceSolid %s\n", message);
    //	Debug only: this doesn't tell us much
    mdlKISolid_checkEntity (const_cast<KIENTITY*>(pBody));
    mdlKISolid_debugEntity (const_cast<KIENTITY*>(pBody), const_cast<char*>(message)); 
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void             Reporter::ReportLineDetails (DSegment3d& segment, XmlNodeRef parentNodePtr)
    {
    int status;
    XmlNodeRef currentNodePtr;// = m_pXMLDom->createElement (L"Line_Details");
    status = mdlXMLDom_createElement(&currentNodePtr,m_pXMLDom,L"Line_Details");

    if (NULL != currentNodePtr)
        {
        status = mdlXMLDomElement_appendChild(parentNodePtr,currentNodePtr);

        XmlNodeRef pointArrayNodePtr;// = m_pXMLDom->createElement (L"Points");
        status =  mdlXMLDom_createElement(&pointArrayNodePtr,m_pXMLDom,L"Points");

        if (NULL != pointArrayNodePtr)
            {
            status = mdlXMLDomElement_appendChild(currentNodePtr,pointArrayNodePtr);

            ReportDpoint3d (L"Point", segment.point[0], pointArrayNodePtr, Adjustment_GO_ToMaster);
            ReportDpoint3d (L"Point", segment.point[1], pointArrayNodePtr, Adjustment_GO_ToMaster);
            }
        }
    }
  /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by KISolid elements     
    * @param  pBody is the body in kernel independent form to navigate the topology of.
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
       void  Reporter::ReportEdge(KIEDGE* edgeP,XmlNodeRef loopNodeP,int pointNumber)
        {
        StatusInt  status; 
        KIVERTEX* startVertexP=NULL;
        KIVERTEX* endVertexP=NULL;
        double    startParam;
        double    endParam;
        DPoint3d  start;
        DPoint3d  end;
        BoolInt  reversed;

        status = mdlKISolid_edgeData(&startVertexP,&endVertexP,&start,&end,&startParam,&endParam,NULL,NULL,&reversed,edgeP);
        if (SUCCESS != status)
            return;
        DPoint3d vertexPoint;
        Transform solidToUOR;
        Transform mdlToSolid;
        Transform invSolidToUOR;
        Transform localTM;
        mdlCurrTrans_begin();
        mdlCurrTrans_masterUnitsIdentity(true);
        mdlCurrTrans_getTransforms (&solidToUOR,&mdlToSolid);
        //experiment with the transforms.
        mdlTMatrix_getInverse (&invSolidToUOR,&mdlToSolid);
        
        mdlTMatrix_multiply (&localTM,&mdlToSolid,&m_currentTransform);
        XmlHelper::AddAttributeInt32Value(loopNodeP,L"Number_Of_Vertices",pointNumber);                
        status = mdlKISolid_vertexData(&vertexPoint,startVertexP);
         if (SUCCESS != status)
            return;
        mdlTMatrix_transformPoint (&vertexPoint,&localTM);
        log_printf (8,"Point out 1\n");
        ReportDpoint3d (L"vertex_1",vertexPoint,loopNodeP,Adjustment_None);

        status = mdlKISolid_vertexData (&vertexPoint,endVertexP);
         if (SUCCESS != status)
            return;
        mdlTMatrix_transformPoint (&vertexPoint,&localTM);
        log_printf (8,"Point out 2 \n");
        ReportDpoint3d (L"vertex_2",vertexPoint,loopNodeP,Adjustment_None);
        mdlCurrTrans_end();
        }
  /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by KISolid elements     
    * @param  pBody is the body in kernel independent form to navigate the topology of.
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
       KICOEDGE*  Reporter::ReportCoEdge(KICOEDGE* curCoedgeP, XmlNodeRef loopNodeP,int pointNumber)
           {
           StatusInt status;
           KIEDGE*   edgeP=NULL;
           DPoint3d  start;
           DPoint3d  end;
           BoolInt  reversed;
           KICOEDGE* nextCoEdgeP=NULL;
           KICOEDGE* lastCoEdgeP = NULL;
           KICOEDGE* partnerEdgeP = NULL; //this is the "other" side shared edge.
           KIENTITY* ownerLoopP = NULL;
           status = mdlKISolid_coedgeData (&edgeP,&ownerLoopP,&partnerEdgeP,&nextCoEdgeP,&lastCoEdgeP,&start,&end,&reversed,curCoedgeP);

           if (SUCCESS != status)
               return nextCoEdgeP = NULL;
                        
           ReportEdge (edgeP,loopNodeP,pointNumber);
           
           return nextCoEdgeP;  //supposed to be the same as the first coedge that starts this call?
           }
  /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by KISolid elements     
    * @param  pBody is the body in kernel independent form to navigate the topology of.
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
    void  Reporter::ReportSolidDetails (KIENTITY*      pBody, XmlNodeRef parentNode,MSWCharCP tagName)
    {
    KILUMP* lumpP=NULL;
    KIWIRE* wireP=NULL;
    int status;
    mdlKISolid_beginCurrTrans (GetCurrentModelRef());
    status = mdlKISolid_bodyData (&wireP,&lumpP,NULL,pBody);
    KISHELL* shellP=NULL;
    KILUMP*  nextLumpP=NULL;
    //TraceSolid (wireP,"wire trace");
    //TraceSolid (lumpP, "lump trace");
    XmlNodeRef bodyNodeP = XmlHelper::AddElementStringValue (parentNode,m_pXMLDom,L"BodyNode",L"");
    if (SUCCESS == status)
    do {
        status = mdlKISolid_lumpData (&shellP,&nextLumpP,lumpP);
        //brings back kishell
        KIFACE*  faceP=NULL;
        KISHELL* nextShellP=NULL;
         //TraceSolid (shellP, "shell trace");
        XmlNodeRef lumpNodeP = XmlHelper::AddElementStringValue(bodyNodeP,m_pXMLDom,L"LumpNode",L"");
        if (SUCCESS == status)
        do {        
            status = mdlKISolid_shellData(&faceP,&nextShellP,shellP);
            //brings back face data
            KILOOP*  loopP=NULL;
            KIFACE*  nextFaceP=NULL;
            BoolInt  reversed;
            BoolInt  doubleSided;
             //TraceSolid (faceP, "face trace");
            XmlNodeRef shellNodeP = XmlHelper::AddElementStringValue(lumpNodeP,m_pXMLDom,L"ShellNode",L"");
            if (SUCCESS == status)
                do {
                    status = mdlKISolid_faceData (&loopP,&nextFaceP,NULL,&reversed,&doubleSided,faceP);
                    //brings back loop
                    KICOEDGE*  coedgeP=NULL;
                    KILOOP*    nextLoopP=NULL;
                     //TraceSolid (loopP, "loop trace");
                    XmlNodeRef faceNodeP = XmlHelper::AddElementStringValue(shellNodeP,m_pXMLDom,L"FaceNode",L"");
                    if (SUCCESS==status)
                        do {
                            status = mdlKISolid_loopData(NULL,&coedgeP,&nextLoopP,loopP);
                            int pointNumber = 1;
                            //need to make this loop on the set of coedges  the initial next and last should have the last and first coedges but how to compare?
                            KICOEDGE* curCoedgeP = coedgeP; 
                            XmlNodeRef loopNodeP = XmlHelper::AddElementStringValue (faceNodeP,m_pXMLDom,L"CoEdgeNode",L"");
                            if (SUCCESS == status)
                                do
                                   {
                                   curCoedgeP = ReportCoEdge(curCoedgeP,loopNodeP,pointNumber);
                                   pointNumber++;
                                   }while (mdlKISolid_getKernelEntity(curCoedgeP) != mdlKISolid_getKernelEntity(coedgeP));                   
                            mdlXMLDomNode_free (loopNodeP);
                            loopP = nextLoopP;
                            } while (loopP != NULL);
                        mdlXMLDomNode_free (faceNodeP);
                        faceP = nextFaceP;
                        } while (faceP != NULL);
                mdlXMLDomNode_free (shellNodeP);
                shellP = nextShellP;
                } while (shellP != NULL);
        mdlXMLDomNode_free (lumpNodeP);
        lumpP = nextLumpP;
        } while (lumpP != NULL);
    mdlXMLDomNode_free (bodyNodeP);
    //bodies are lumps.
    //lumps have loops
    //loops have faces?
    //faces have edges
    //edges have vertices
   /* End Cur Trans */
    mdlKISolid_endCurrTrans ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportSolid(MSWCharCP tagName,KIENTITY* pBody, XmlNodeRef parentNodePtr)
    {
   StatusInt   status = SUCCESS;
    XmlNodeRef headerNodePtr;// = m_pXMLDom->createElement (tagName);
    status = mdlXMLDom_createElement (&headerNodePtr,m_pXMLDom,tagName);

    if (NULL == headerNodePtr)
        return;
    XmlHelper::AddAttributeStringValue (headerNodePtr,tagName,WString ("Solids Reporting"));

    status = mdlXMLDomElement_appendChild(parentNodePtr,headerNodePtr);

    ReportSolidDetails(pBody, headerNodePtr,L"Body_Details");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportBsplineParams (MSWCharCP tagName, BsplineParam* params, XmlNodeRef      parentNode)
    {
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Order",params->order);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Closed",params->closed);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Num_Poles",params->numPoles);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Num_Knots",params->numKnots);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Num_Rules",params->numRules);
    return;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportBsplineDisplay (MSWCharCP tagName, BsplineDisplay* params, XmlNodeRef    parentNode)
    {
    //XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, Int32 val
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Polygon_Display",params->polygonDisplay);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Curve_Display",params->curveDisplay);
    XmlHelper::AddElementInt32Value (parentNode,m_pXMLDom,L"Rules_By_Length",params->rulesByLength);
    return;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportBsplineSurface (MSWCharCP tagName, MSBsplineSurface* surface, XmlNodeRef parentNode)
    {
    StatusInt status=SUCCESS;
    MSElementDescr* surfaceEDP;
    XmlNodeRef headerNodePtr;
    status = mdlXMLDom_createElement (&headerNodePtr,m_pXMLDom, tagName);
    if (NULL==headerNodePtr)
        return;
    XmlHelper::AddAttributeStringValue (headerNodePtr,tagName,WString ("Surface Reporting"));
    status = mdlXMLDomElement_appendChild(parentNode,headerNodePtr);

    MSElement       headerElm;
    int             type;
    int             rational;
    BsplineDisplay  displayParams;
    BsplineParam    uParams;
    BsplineParam    vParams;
    DPoint3d        *poles;
    double          *uKnots;
    double          *vKnots;
    double          *weights;
    int             holeOrigin;
    int             numBounds;
    BsurfBoundary   *surfBoundary;

    poles = (Dpoint3d *) 0;
    uKnots = vKnots = weights = (double *) 0;
    surfBoundary = (BsurfBoundary*) 0;
    mdlBspline_extractFromSurface (&surfaceEDP, surface);

    mdlBspline_extractSurface (&headerElm, &type, &rational, &displayParams, &uParams,
                           &vParams, &poles, &uKnots, &vKnots, &weights,
                   &holeOrigin, &numBounds, &surfBoundary, surfaceEDP);

    ReportBsplineDisplay (L"bspline_display_params", &displayParams,headerNodePtr);

    ReportBsplineParams (L"bspline_uParams",&uParams,headerNodePtr);
    ReportBsplineParams (L"bspline_vParams",&vParams,headerNodePtr);

    int count;
    int i;
    count = uParams.numPoles;
    for (i=0;i<count;++i )
        {
        ReportDpoint3d (L"U_Pole",poles[i],headerNodePtr,Adjustment_None);
        }

    count = vParams.numPoles;
    for (i=0;i<count;++i )
        {
        ReportDpoint3d (L"V_Pole",poles[i],headerNodePtr,Adjustment_None);
        }

    count = uParams.numKnots;
    for (i=0;i<count ;++i )
        {
        XmlHelper::AddElementDoubleValue(headerNodePtr, m_pXMLDom, L"uKnot", uKnots[i]);
        }

    count = vParams.numKnots;
    for (i=0;i<count ;++i )
        {
        XmlHelper::AddElementDoubleValue(headerNodePtr, m_pXMLDom, L"vKnot", vKnots[i]);
        }

    if (poles)	    dlmSystem_mdlFree (poles);
    if (uKnots)	    dlmSystem_mdlFree (uKnots);
    if (vKnots)	    dlmSystem_mdlFree (vKnots);
    if (weights)    dlmSystem_mdlFree (weights);
    if (surfBoundary) dlmSystem_mdlFree (surfBoundary);
    return;
    }

int Reporter::GetPassCount ()
{return  m_passCount;};

#if defined (BSI_SPECIAL)

DgnModelRefP TempGetAndOpenFile(int passCount)
     {
     DgnModelRefP pTarget;
    char targetName[512];
    char currentFileName[512];
    char dev[MAXDEVICELENGTH];
    char directory[MAXDIRLENGTH];
    char name[MAXNAMELENGTH];
    char noImodelName[MAXNAMELENGTH];
    char extension[MAXEXTENSIONLENGTH];
    int  status;
    //int is3D;
    //int fmt;    
    char pass[12];

    status = mdlModelRef_getFileName (ACTIVEMODEL,currentFileName,512);
    mdlFile_parseName (currentFileName,dev,directory,name,extension);
    memset (noImodelName,'\0',sizeof noImodelName);
    strncpy (noImodelName,name,strlen(name)-2);
    sprintf (pass,"%ld",passCount);
    strcat (noImodelName,pass);
    mdlFile_buildName (targetName,dev,directory,noImodelName,"dgn");
    status = mdlWorkDgn_createFile (&pTarget,targetName,DGNFILE_FORMAT_V8,ACTIVEMODEL,SEED_CopyAllData|SEED_CreateDefaultModel,NULL,NULL,TRUE);
    //status = mdlWorkDgn_openFile (&pTarget,&fmt,&is3D,targetName ,NULL,FALSE);
    return pTarget;
    //status = mdlWorkDgn_openFile (&pTarget,targetName,DGNFILE_FORMAT_V8,ACTIVEMODEL,SEED_CopyAllData|SEED_CreateDefaultModel,NULL,NULL,TRUE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Mark.Anderson                        02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConvertToMTG (MTGFacets* pMtgFacets,ElementFacetSet* pFacets)
    {
    EmbeddedIntArray *pIndexArray       = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYZArray    = jmdlEmbeddedDPoint3dArray_grab ();
    StatusInt status = ERROR;
    const Int32 * pFacetIndex = pFacets->GetPointIndices();
    int verticesPerFacet;
    int pointCount = pFacets->GetPointCount();
    jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXYZArray, pFacets->GetPoint(0), pFacets->GetPointCount());
   
    verticesPerFacet = pFacets->GetNumPerFace();
    int faceCount =  pFacets->GetFacetCount();
       
    jmdlEmbeddedIntArray_addIntArray (pIndexArray,pFacets->GetPointIndices(), pFacets->GetPointIndexCount());
    
    status = mdlMesh_polyfaceArraysToMTG (pMtgFacets, pIndexArray, pXYZArray,
                                1, pFacets->GetFacetCount());
    

    jmdlEmbeddedDPoint3dArray_drop (pXYZArray);
    jmdlEmbeddedIntArray_drop (pIndexArray);
    return status;
    }

StatusInt       ConvertToMesh (ElementFacetSet* pFacets,int passCount, bool bWriteToFile)
{
    EmbeddedIntArray *pIndexArray       = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYZArray    = jmdlEmbeddedDPoint3dArray_grab ();
    StatusInt status = ERROR;
    const Int32 * pFacetIndex = pFacets->GetPointIndices();
    int verticesPerFacet;
    MSElementDescrP edP;

    int pointCount = pFacets->GetPointCount();
    jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXYZArray, pFacets->GetPoint(0), pFacets->GetPointCount());
   
    verticesPerFacet = pFacets->GetNumPerFace();
    int faceCount =  pFacets->GetFacetCount();
       
    jmdlEmbeddedIntArray_addIntArray (pIndexArray,pFacets->GetPointIndices(), pFacets->GetPointIndexCount());
    
   // status = mdlMesh_newPolyfaceFromEmbeddedArrays (&edP,NULL,pIndexArray,verticesPerFacet,pXYZArray);
    
    MTGFacets *pMtgFacets = jmdlMTGFacets_grab ();
    jmdlMTGFacets_setNormalMode (pMtgFacets,MTG_Facets_VertexOnly,0,0);
    
    status = ConvertToMTG (pMtgFacets,pFacets);

    jmdlMTGFacets_stitchFacets (pMtgFacets,0.0,1.0e-8);
    mdlMesh_newPolyfaceFromMTGFacets (&edP,pMtgFacets);
    jmdlMTGFacets_drop (pMtgFacets);

    double tempVolume = SolidsFunctions::MeasureSupport::GetVolume(edP);
    char buffer[255];
    sprintf (buffer,"Element ID %I64u , volume of %lf ", mdlElement_getID (&edP->el), tempVolume);
    if (tempVolume > 0 )
        Reporter::log_printf (1,buffer);

    DgnModelRefP pTarget=NULL;

    if (bWriteToFile)
        {
        pTarget = TempGetAndOpenFile (passCount);
        mdlElmdscr_addByModelRef (edP,pTarget);
    
    //mdlMeasure_volumePropertiesFromMesh (&tempVolume, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,edP,false);
    
        mdlWorkDgn_closeFile(pTarget);
        }
    
    mdlElmdscr_freeAll (&edP);
    jmdlEmbeddedDPoint3dArray_drop (pXYZArray);
    jmdlEmbeddedIntArray_drop (pIndexArray);
    return status;
}
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Mark.Anderson                        02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertFacetsToSolids (ElementFacetSet* pFacets,int passCount, bool bWriteToFile)
    {
    int status;
    
    KIBODY* pBody=NULL;
    MTGFacets *pMtgFacets = jmdlMTGFacets_grab ();
    
    status = ConvertToMTG (pMtgFacets,pFacets);
    
    Transform   inverseTransform, dgnToSolid, solidToDgn;
     int bodyTag = 0;

    DPoint3d originPt;
    memset (&originPt, 0, sizeof (originPt));
    mdlSolid_getDgnTransform (&solidToDgn, &dgnToSolid, &originPt, ACTIVEMODEL);

    jmdlEmbeddedDPoint3dArray_multiplyByTransformInPlace (&pMtgFacets->vertexArrayHdr, &dgnToSolid);

    if (SUCCESS == mdlSolid_transformPointsIntoSolidModelingCube (jmdlEmbeddedDPoint3dArray_getPtr (&pMtgFacets->vertexArrayHdr, 0),
                                                                &dgnToSolid, &inverseTransform,
                                                                    jmdlEmbeddedDPoint3dArray_getCount (&pMtgFacets->vertexArrayHdr), ACTIVEMODEL)
       && SUCCESS == mdlSolid_bodyFromMTGFacets (&bodyTag, pMtgFacets))
        mdlSolid_applyTransform ((BODY*)bodyTag, &inverseTransform);
//?not sure yet.
     pBody = (KIBODY*)bodyTag;
    
     //double solidVolume = SolidsFunctions::MeasureSupport::GetSmartSolidBodyVolumeProp (pBody);

    jmdlMTGFacets_drop (pMtgFacets);
    //this is to debug create a model in the file named copysolid and the output will go to it.
    if (NULL != bodyTag)
        {
        MSElementDescrP edP;
        DgnModelRefP pTarget=NULL;
        
        if (bWriteToFile)
            pTarget = TempGetAndOpenFile (passCount);
        
        mdlSolid_bodyToElement(&edP,(BODY*)bodyTag,TRUE,-1,-1,SOLID_NOCACHE_NOCOPY,NULL,ACTIVEMODEL);
        double tempVolume = SolidsFunctions::MeasureSupport::GetVolume(edP);
        
        if (NULL !=pTarget)
            {
            mdlElmdscr_addByModelRef (edP,pTarget);
            mdlWorkDgn_closeFile(pTarget);
            }

        mdlSolid_freeBody((BODY*)bodyTag);
        mdlElmdscr_freeAll (&edP);
        }

    }
#endif

Private void  generateTest(ElementFacetSet* pFacets,int passCount )
{
#if defined (BSI_SPECIAL) 
    bool bWriteToFile = false;
    //ConvertFacetsToSolids (pFacets,passCount, bWriteToFile);
    ConvertToMesh (pFacets,passCount, bWriteToFile);
#endif
 }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportFacetSet (MSWCharCP tagName, ElementFacetSet* pFacets, XmlNodeRef parentNodePtr)
    {
    StatusInt status=SUCCESS;
    XmlNodeRef headerNodePtr;
#if defined (BSI_SPECIAL)
    generateTest (pFacets,m_passCount);
    return;
#endif
    m_passCount++;
    status = mdlXMLDom_createElement (&headerNodePtr,m_pXMLDom, tagName);
    if (NULL==headerNodePtr)
        return;
     XmlHelper::AddAttributeStringValue (headerNodePtr,tagName,WString ("Facet Reporting"));
    //parentNodePtr->appendChild (headerNodePtr);
    status = mdlXMLDomElement_appendChild(parentNodePtr,headerNodePtr);
    int pCount = pFacets->GetPointCount();
    int fCount = pFacets->GetPointIndexCount();

    int i;

    XmlNodeRef loopNodeP = XmlHelper::AddElementStringValue (headerNodePtr,m_pXMLDom,L"Facet_Details",L"");

    for (i=0;i<pCount;++i)
        {
        ReportDpoint3d (L"Facet_Point",*pFacets->GetPoint(i),loopNodeP,Adjustment_None);
        }

    const Int32 * pFacetIndex = pFacets->GetPointIndices();
   
    for(i=0;i<fCount;++i)
        {
        int intVal=0;

        intVal = pFacetIndex[i];
        XmlHelper::AddElementInt32Value(loopNodeP, m_pXMLDom, L"Point_Index",intVal);
        }
    mdlXMLDomNode_free (loopNodeP);
    return;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void  Reporter::ReportGPArray (MSWCharCP tagName, GPArrayP gpa, XmlNodeRef parentNodePtr)
    {
    StatusInt   status = SUCCESS;
    XmlNodeRef headerNodePtr;// = m_pXMLDom->createElement (tagName);
    status = mdlXMLDom_createElement (&headerNodePtr,m_pXMLDom,tagName);

    if (NULL == headerNodePtr)
        return;

    //parentNodePtr->appendChild (headerNodePtr);
    status = mdlXMLDomElement_appendChild(parentNodePtr,headerNodePtr);

    for (int i=0, count = gpa->GetCount(); i<count && SUCCESS == status; )
        {
        switch (gpa->GetCurveType (i))
            {
            case GPCURVETYPE_LineString:
                {
                int         nPoints;
                DPoint3d    points[MAX_VERTICES];

                if (SUCCESS != (status = gpa->GetLineString (&i, points, &nPoints, MAX_VERTICES)))
                    break;

                size_t numPoints = (size_t)nPoints;
                ReportLineStringDetails (points, numPoints, headerNodePtr);
                
                break;
                }

            case GPCURVETYPE_Ellipse:
                {
                DEllipse3d  ellipse;

                if (SUCCESS != (status = gpa->GetEllipse (&i, &ellipse)))
                    break;

                ReportEllipseDetails (L"PartialEllipse_Details", ellipse, headerNodePtr, true);

                break;
                }

            case GPCURVETYPE_BCurve:
            //case GPCURVETYPE_BSpline:
                {
                MSBsplineCurve  curve;

                if (SUCCESS != (status = gpa->GetBCurve (&i, &curve)))
                    break;

                ReportCurveDetails (L"GPA-BCurve",curve, false, headerNodePtr);
                mdlBspline_freeCurve(&curve);
                //curve.ReleaseMem ();

                break;
                }

            default:   // GPCURVETYPE_Invalid
                {
                //assert (false);
                i++;
                break;
                }
            }

        bool    endLoop = (i && gpa->IsMajorBreak (i-1)); // potential start of new loop...
        if (endLoop && i<count)
            {
            headerNodePtr;//= m_pXMLDom->createElement (tagName);
            mdlXMLDom_createElement(&headerNodePtr,m_pXMLDom,tagName);
            if (NULL == headerNodePtr)
                return;

            mdlXMLDomElement_appendChild(parentNodePtr,headerNodePtr);
            //parentNodePtr->appendChild (headerNodePtr);
            }
        }
    }
//};
