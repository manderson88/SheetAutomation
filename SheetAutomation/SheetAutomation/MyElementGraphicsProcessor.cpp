#include "MyElementGraphicsProcessor.h"
/*--------------------------------------------------------------------------**//**
* @description This class is the implementation for the IElementGraphicsProcessor.  This class
* contains the methods for processing i-model graphics.  The caller will pass an element handle
* and a reference to this class.     
* @bsiclass                       @author BSI
+---------------------------------------------------------------------------*/

MyElementGraphicsProcessor::MyElementGraphicsProcessor (Reporter reporter, XmlNodeRef currentNode)
    {
    m_reporter = reporter;
    m_currentNode = currentNode;
    m_FacetOptions = NULL;
    mdlTMatrix_getIdentity(&m_currentTransform);
    }

/*--------------------------------------------------------------------------**//**
* @description WriteOutElementFile - this will write the data to a psuedo xml format     
* @param  gpaLine the gpa for  the linestring 
* @bsimethod                       @author BSI
+---------------------------------------------------------------------------*/
bool GetFileName(WStringR fileName)
    {
      char	      xmlFilename[MAXFILELENGTH];
      MSWChar     wsXmlFilename[MAXFILELENGTH];
      char*       dataLocationPath;
        
      dataLocationPath = mdlSystem_expandCfgVar ("MS_DATA");
      
      if ((SUCCESS == mdlDialog_fileCreate (xmlFilename, NULL, 0, "sample.xml", "*.xml", dataLocationPath , "Save As XML File"))
                 && (0 < strlen(xmlFilename)))
             {
             mdlCnv_convertMultibyteToUnicode(xmlFilename, -1, wsXmlFilename, MAXFILELENGTH);
                 /* Save the XML Dom out to a FILE   */
             fileName = WString (xmlFilename);
             return true;
             }
      mdlSystem_freeCfgVarBuffer (dataLocationPath);
      return false;
    }
/*--------------------------------------------------------------------------**//**
    * @description Write the DOM out to a file     
    * @param  fileName The full filespec to write out to.
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
void MyElementGraphicsProcessor::WriteReportToFile ()
    {
    WString fileName;
    if (GetFileName(fileName))
        m_reporter.Save(fileName);
    }
/*--------------------------------------------------------------------------**//**
* @description This will allow the overriding of the facet options.  Return NULL to allow the 
* default processing.
* @bsimethod                       @author BSI
+---------------------------------------------------------------------------*/
ElementFacetOptions* MyElementGraphicsProcessor::_GetFacetOptions() {return NULL;}
/*---------------------------------------------------------------------------------**//**
* description This method sets the current transformation information that is being used by the xgraphics system.
* @param trans This is the transformation matrix that is applied to the element that is being processed.
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void MyElementGraphicsProcessor::_AnnounceTransform (TransformCP trans)
        {
        if (NULL != trans)
            {
            memcpy (&m_currentTransform,trans,sizeof (Transform));
            m_reporter.ReportTransformDetails (L"Transform", m_currentTransform ,m_currentNode);
            }
        }
/*---------------------------------------------------------------------------------**//**
* Description This method set the current Material information that is being used by the xgraphics system.
* @param matSymb the symbology for the current element.
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void MyElementGraphicsProcessor::_AnnounceElemMatSymb (ElemMatSymbCR matSymb)
    {
    __super::_AnnounceElemMatSymb (matSymb);
    }
/*---------------------------------------------------------------------------------**//**
* description The current display parameters that are applied to the current element.
* @params displayParams The display information that is applied to the graphic element currently being processed.
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
void MyElementGraphicsProcessor::_AnnounceElemDisplayParams (ElemDisplayParamsCR displayParams)
    {
    __super::_AnnounceElemDisplayParams (displayParams);
    }

    /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by a solid entity     
    * @param  surface the Bspline Surface 
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
    StatusInt MyElementGraphicsProcessor::_ProcessParasolidBody(KIBodyCP pBody) 
        {
        m_reporter.ReportSolid (L"Solid",(KIENTITY*)pBody,m_currentNode);
        return SUCCESS;
        }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt MyElementGraphicsProcessor::_ProcessAcisBody(KIBodyCP pBody)
        {
        return SUCCESS;
        }

    /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by a Graphics Point Array.     
    * @param  gpa is the graphics point array to interrogate
    * @param  isClosed is the array a closed region
    * @param  isFilled is the array a filled region
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/    
   BentleyStatus MyElementGraphicsProcessor::_ProcessGPA (GPArrayP gpa, bool isClosed, bool isFilled)
        {
        StatusInt   status = SUCCESS;
        m_reporter.ReportGPArray (L"GPA",gpa,m_currentNode);
        return SUCCESS;
        }

    /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by Facet Sets     
    * @param  pFacets the geometry in a set of facets. 
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
     StatusInt MyElementGraphicsProcessor::_ProcessFacetSet (ElementFacetSet* facets) 
        {
        m_reporter.ReportFacetSet(L"Facet_Set",facets,m_currentNode);
        return SUCCESS;
        }
    /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by BsplineSurface elements     
    * @param  surface the Bspline Surface 
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
    BentleyStatus MyElementGraphicsProcessor::_ProcessSurface (MSBsplineSurface* surface) 
        {
        m_reporter.ReportBsplineSurface(L"Surface",surface,m_currentNode); 
        return SUCCESS;
        }
    /*--------------------------------------------------------------------------**//**
    * @description Process the Geometry that is represented by a cone.     
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
     BentleyStatus MyElementGraphicsProcessor::_ProcessCone 
        (
        DVec3dCR  primary,
        DVec3dCR  secondary,
        DPoint3dCR p0,
        DPoint3dCR p1,
        double     r0,
        double     r1,
        bool       cap
        ) 
        {
        m_reporter.ReportConeDetails(L"Cone",primary,secondary,p0,p1,r0,r1,cap,m_currentNode);
        return SUCCESS;
        }
    /*--------------------------------------------------------------------------**//**
    * @description Process the geometry that is represented by BsplineCurve elements     
    * @param  curve
    * @param  filled - the curve is a filled curve?
    * @bsimethod                       @author BSI
    +---------------------------------------------------------------------------*/
     BentleyStatus MyElementGraphicsProcessor::_ProcessCurve 
        (
        MSBsplineCurveCR curve,
        bool             filled
        ) 
        {
        m_reporter.ReportCurveDetails (L"Bspline_Curve",curve,filled,m_currentNode);
        return SUCCESS;
        }

//end of MyElementGraphicsProcessor
