/*--------------------------------------------------------------------------------------+
|     $Source: /miscdev-root/miscdev/mdl/examples/imodelvisitor/XmlHelper.h,v $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// This file should be included directly from a cpp file and not in a .h file that
// may get used for a PCH. The .cpp file that includes this must also be compiled
// outside of a multi-compile block for the #import statement to work properly.
//////////////////////////////////////////////////////////////////////////////////////////

#include <WString.h>
#include <mdlxmltools.fdf>
#include <mdlxmltoolslink.fdf>

USING_NAMESPACE_BENTLEY

/*=================================================================================**//**
*
* Utility Class to work with MSXML 6 
*
* @bsiclass                                     		BentleySystems  
+===============+===============+===============+===============+===============+======*/
struct XmlHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    XmlHelper::CreateDomInstance (XmlDomRef* pXMLDom, bool bFreeThread=false)
    {
    int status;
    if (bFreeThread)
       status = mdlXMLDom_createFreeThreaded(pXMLDom);
    else
        status = mdlXMLDom_create (pXMLDom);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    XmlHelper::LoadXML (XmlDomRef pXMLDom, MSWCharCP xmlString)
    {
   
    return (SUCCESS == mdlXMLDom_load(pXMLDom,FILESPEC_LOCAL,xmlString,NULL,NULL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeStringValue (WStringR val, XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    int status = ERROR;
    val = L""; // initialize return value to empty string
    MSWChar pVal[512];
    int     pValSize=512;
    XmlNodeRef pNode;
    XmlNodeRef parentNode = parentNodePtr;
    status = mdlXMLDomNode_selectSingleNode (&pNode,parentNode,path);
    status = mdlXMLDomNode_getValue (pVal,&pValSize,pNode);
    val=Bentley::WString(pVal);
    return (SUCCESS == status);
 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeDoubleValue (double& val,  XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    WString valString;

    if (GetNodeStringValue (valString, parentNodePtr, path))
        {
        val = _wtof (valString.c_str());
        return true;
        }

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeInt32Value (Int32& val,  XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    XmlNodeRef nodePtr=NULL;
    WString valueString;
    
    if  (GetNodeStringValue(valueString,parentNodePtr,path))
        {
        val = _wtoi (valueString.c_str());
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeUInt32Value (UInt32& val,  XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    XmlNodeRef nodePtr = NULL;
    WString valueString;
    if  (GetNodeStringValue(valueString,parentNodePtr,path))
        {
        val = wcstoul (valueString.c_str(),NULL,10);
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeUInt64Value (UInt64& val,  XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    XmlNodeRef nodePtr = NULL;
    WString valueString;
    if  (GetNodeStringValue(valueString,parentNodePtr,path))
        { //this is only going to convert to int32
        val = wcstoul (valueString.c_str(),NULL,10);
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeBoolValue (bool& val,  XmlNodeRef& parentNodePtr, MSWCharCP path=NULL)
    {
    WString boolVal;

    if (XmlHelper::GetNodeStringValue (boolVal, parentNodePtr, path))
        {
        val = (0 == _wcsicmp(boolVal.c_str(), L"true" ));
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef   XmlHelper::AddElementStringValue (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, MSWCharCP val)
    {
    int status;
    XmlNodeRef pNode;
    status = mdlXMLDom_createElement(&pNode,pXMLDom,name);
    status = mdlXMLDomElement_setValue (pNode,val);
    status = mdlXMLDomElement_appendChild(parentElementPtr,pNode);
    return pNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef   XmlHelper::AddElementInt32Value (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, Int32 val)
    {
    int status;
    XmlNodeRef  elementPtr;
    status = mdlXMLDom_addElement(&elementPtr,pXMLDom,parentElementPtr,XMLDATATYPE_INT32,name,&val);
    status = mdlXMLDomElement_appendChild(parentElementPtr,elementPtr);
    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef     XmlHelper::AddElementUInt32Value (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, UInt32 val)
    {
    int status;
    XmlNodeRef  elementPtr;
    status = mdlXMLDom_addElement(&elementPtr,pXMLDom,parentElementPtr,XMLDATATYPE_UINT32,name,&val);
    status = mdlXMLDomElement_appendChild(parentElementPtr,elementPtr);
    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef   XmlHelper::AddElementUInt64Value (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, UInt64 val)
    {
    XmlNodeRef elementPtr;
    int status;
    status = mdlXMLDom_addElement(&elementPtr,pXMLDom,parentElementPtr, XMLDATATYPE_UINT64,name,&val);
    status = mdlXMLDomElement_appendChild(parentElementPtr,elementPtr);
    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef   XmlHelper::AddElementBoolValue (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, bool val)
    {
    return XmlHelper::AddElementStringValue (parentElementPtr, pXMLDom, name, val?L"true":L"false");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlNodeRef   XmlHelper::AddElementDoubleValue (XmlNodeRef& parentElementPtr, XmlDomRef& pXMLDom, MSWCharCP name, double val)
    {
    int status;
    XmlNodeRef  elementPtr;
    status = mdlXMLDom_addElement(&elementPtr,pXMLDom,parentElementPtr,XMLDATATYPE_DOUBLE,name,&val);
    status = mdlXMLDomElement_appendChild(parentElementPtr,elementPtr);

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeStringValue (XmlNodeRef elementPtr, MSWCharCP name, WStringR val)
    {
    int status;
    char  buffer[512];
    int   bufferLen = 512;
    status = mdlXMLDomElement_getAttributeExt (buffer,&bufferLen,elementPtr,name,XMLDATATYPE_STRING,true);
    if (ERROR == status)
        return false;
    val = Bentley::WString (buffer);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeInt32Value (XmlNodeRef& elementPtr, MSWCharCP name, Int32& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtoi (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeUInt32Value (XmlNodeRef& elementPtr, MSWCharCP name, UInt32& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;
    val = _wtoi(valString.c_str());
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeInt64Value (XmlNodeRef& elementPtr, MSWCharCP name, __int64& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtoi64 (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeUInt64Value (XmlNodeRef& elementPtr, MSWCharCP name, unsigned __int64& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;
        //this is only going to convert to int32
    val = wcstoul (valString.c_str(),NULL,10);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeDoubleValue (XmlNodeRef& elementPtr, MSWCharCP name, double& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtof (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeBoolValue (XmlNodeRef& elementPtr, MSWCharCP name, bool& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = (0 == _wcsicmp(valString.c_str(), L"true" ));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void    XmlHelper::AddAttributeStringValue (XmlNodeRef& elementPtr, MSWCharCP name, MSWCharCP val)
    {
    int status;
    if (NULL == val)
        return;
    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_WIDESTRING,name,val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void    XmlHelper::AddAttributeStringValue (XmlNodeRef& elementPtr, MSWCharCP name, WStringCR val)
    {
    int status;
    MSWCharCP   wVal = val.c_str ();
    if (NULL == wVal)
        return;

    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_WIDESTRING,name,wVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeInt32Value (XmlNodeRef& elementPtr, MSWCharCP name, Int32 val)
    {
    int status;
    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_INT32,name,&val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void    XmlHelper::AddAttributeDoubleValue (XmlNodeRef& elementPtr, MSWCharCP name, double val)
    {
    mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_DOUBLE,name,&val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeUInt32Value (XmlNodeRef& elementPtr, MSWCharCP name, UInt32 val)
    {
    int status;
    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_UINT32,name,&val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeUInt64Value (XmlNodeRef& elementPtr, MSWCharCP name, unsigned __int64 val)
    {
    int status;
    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_UINT64,name,&val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeInt64Value (XmlNodeRef& elementPtr, MSWCharCP name, const __int64 val)
    {
    int status;
    status = mdlXMLDomElement_addAttribute (elementPtr,XMLDATATYPE_INT64,name,&val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeBoolValue (XmlNodeRef& elementPtr, MSWCharCP name, bool val)
    {
    AddAttributeStringValue (elementPtr, name, val?L"true":L"false");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeXmlText (XmlNodeRef& elementPtr, Bentley::WString& xmlText)
    {
    int status;
    if (NULL == elementPtr)
        return false;

    MSWChar pVal[512];
    int     pValLen = 512;
    status = mdlXMLDomNode_getXmlText(pVal,&pValLen,elementPtr);
    xmlText = Bentley::WString(pVal);
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems                  
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::Save (XmlDomRef& pXMLDomPtr, MSWCharCP xmlFileName, MSWCharCP encoding=NULL)
    {
    return (SUCCESS == mdlXMLDom_save(pXMLDomPtr,FILESPEC_LOCAL,xmlFileName,NULL, NULL,  L"Windows-1252", FALSE, TRUE, TRUE));
    
    }
};
