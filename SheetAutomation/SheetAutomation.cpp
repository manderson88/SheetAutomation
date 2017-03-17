/*--------------------------------------------------------------------------------------+
//----------------------------------------------------------------------------
// DOCUMENT ID:   
// LIBRARY:       
// CREATOR:       Mark Anderson
// DATE:          05-05-2016
//
// NAME:          SheetAutomation
//
// DESCRIPTION:   
//
// REFERENCES:
//
// ---------------------------------------------------------------------------
// NOTICE
//    NOTICE TO ALL PERSONS HAVING ACCESS HERETO:  This document or
//    recording contains computer software or related information
//    constituting proprietary trade secrets of Black & Veatch, which
//    have been maintained in "unpublished" status under the copyright
//    laws, and which are to be treated by all persons having acdcess
//    thereto in manner to preserve the status thereof as legally
//    protectable trade secrets by neither using nor disclosing the
//    same to others except as may be expressly authorized in advance
//    by Black & Veatch.  However, it is intended that all prospective
//    rights under the copyrigtht laws in the event of future
//    "publication" of this work shall also be reserved; for which
//    purpose only, the following is included in this notice, to wit,
//    "(C) COPYRIGHT 1997 BY BLACK & VEATCH, ALL RIGHTS RESERVED"
// ---------------------------------------------------------------------------
/*
/* CHANGE LOG
 * $Archive: /MDL/ISOSheetAutomation/SheetAutomation/SheetAutomation/SheetAutomation.cpp $
 * $Revision: 2 $
 * $Modtime: 3/14/17 11:04a $
 * $History: SheetAutomation.cpp $
 * 
 * *****************  Version 2  *****************
 * User: Mark.anderson Date: 3/15/17    Time: 12:50p
 * Updated in $/MDL/ISOSheetAutomation/SheetAutomation/SheetAutomation
 * updating comments and refactoring 
 * 
 * *****************  Version 1  *****************
 * User: Mark.anderson Date: 3/06/17    Time: 12:00p
 * Created in $/MDL/ISOSheetAutomation/SheetAutomation/SheetAutomation
 * Initial checkin
 * 
 +--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define UNICODE 1


#include <mdl.h>
#include <MicroStationAPI.h>
#include "SheetAutomation.h"
#include "MyElementGraphicsProcessor.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <mselemen.fdf>
#include <mselmdsc.fdf>
#include <mslinkge.fdf>
#include <msscancrit.fdf>
#include <mstagdat.fdf>
#include <mselems.h>
#include <mscell.fdf>
#include <leveltable.fdf>
#include <mslstyle.fdf>
#include <msstrlst.h>
#include <mscnv.fdf>
#include <msdgnobj.fdf>
#include <msmodel.fdf>
#include <msview.fdf>
#include <msviewinfo.fdf>
#include <msvar.fdf>
#include <dlmsys.fdf>
#include <msdialog.fdf>
#include <mswrkdgn.fdf>
#include <msrmgr.h>
#include <mssystem.fdf>
#include <msparse.fdf>
#include <msfile.fdf>
#include <toolsubs.h>
#include <mssheetdef.h>
#include <msreffil.fdf>
//scan code
#include <elementref.h>
//locate code
#include <msdependency.fdf>
#include <msassoc.fdf>
#include <msmisc.fdf>
#include <mslocate.fdf>
#include <msstate.fdf>
#include <msoutput.fdf>
#define EXTRA_TESTING
//place command
#include <mstmatrx.fdf>
#undef CF_MAX

#include <aawindms.h>
#include <aaapi.h>
#include <aadmsapi.h>
#include <aawinapi.h>
#include <mcmlib.fdf>
#include <aadmsapi.fdf>

#include <fstream>
#include "SheetAutomationCmd.h"

#include <interface/element/Handler.h>
#include <interface/element/DisplayHandler.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_USTN
USING_NAMESPACE_BENTLEY_USTN_ELEMENT

extern "C" int dgnFileObj_setDefaultModelID(DgnFile* pFile,ModelID id);
extern "C" ModelID dgnFileObj_getDefaultModelID(DgnFile* dgnFileP);
extern "C" int mdlModelRef_saveModelInfo(DgnModelRefP pModel, BoolInt flag);

#define DIM(a)      (  (sizeof a) / sizeof *(a)  )
//#include "Reporter.h"
static WString gDataSource;
static BOOL g_isLoggedIn = FALSE;
static long g_iProjectID;
static long g_iDocID;
//-----------------------------------------------------------------------------
//a method to write to a file for process logging
//-----------------------------------------------------------------------------
void WriteLogFile(const char* szString)
{
 // #if defined PRINTLOG_STATE
    char* tempDir = mdlSystem_getExpandedCfgVar ("MS_TMP");
     // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   char* dt = ctime(&now);
  
    char  outfile[MAXFILELENGTH];
    sprintf(outfile,"%sSheetAutomation-LogFile.txt",tempDir);

    FILE* fp = mdlTextFile_open(outfile,TEXTFILE_APPEND);
    //std::string text(szString);
    //text.append(" -- ");
    //text.append(dt);

    //std::ofstream log_file(
      //  outfile.c_str(), std::ios_base::out | std::ios_base::app );
    char buffer[5000];

    //log_file << text << std::endl;
    sprintf(buffer,"%s \n",szString);
    
    mdlTextFile_putString(buffer,fp,TEXTFILE_DEFAULT);

    mdlTextFile_close(fp);
    
    mdlSystem_freeCfgVarBuffer(tempDir);
 
  //#endif
 
}
//-----------------------------------------------------------------------------
//a method to print to the MicroStation text window.
// Input:   poo the string to print.
//---------------+---------------+---------------+---------------+-------------
Private void    dumpString(char *poo)
    {
    printf ("%s \n",poo);
    WriteLogFile(poo);
    }
//-----------------------------------------------------------------------------
// a simple printf wrapper for controled print out
// Input:   level an integer representing a filter for what to print
//          szFormat a format statement to print out
//          ... a variable arg to print.
//---------------+---------------+---------------+---------------+-------------
extern "C"  void     log_printf(long  level,char  *szFormat,...)
    {
    va_list     arg;
    char        szBuf[512];
    static int   count;
    
    va_start (arg, szFormat);
    vsprintf (szBuf, szFormat, arg);

    dumpString (szBuf);
    }
/*------------------------------------------------------------------------------+
| SheetAutomation_pwErrorInformation - single error information code. This will |
| be the single point for all PW errors to be processed.                        |
+------------------------------------------------------------------------------*/
void SheetAutomation_pwErrorInformation()
{
    long    errNo = aaApi_GetLastErrorId();
    LPCWSTR lastMessage = aaApi_GetLastErrorMessage();
    LPCWSTR lastDetail = aaApi_GetLastErrorDetail();
    log_printf(1, "PWERROR %S - %S",lastMessage,lastDetail);
}
/* -----------------------------------------------------------------------------+
|  Gets the cached datasource name.                                             |
|                                                                               |
+------------------------------------------------------------------------------*/
void GetDataSourceName (WStringR dsName)
{
    dsName = gDataSource;
    return;
}
/*------------------------------------------------------------------------------+
|  Sets the cached datasource name                                              |
|                                                                               |
+------------------------------------------------------------------------------*/
void SetDataSourceName(WStringR dsName)
{
    gDataSource = dsName;
}
#if defined (GRAPHICS_TEST)
/*--------------------------------------------------------------------------**//**
* @description This function processes all the elements as solids or GPA.  
*              WIP to handle Building elements?  
* @param  eh the element handle to process.
* @param  reporter the reporting object.
* @bsimethod                       @author BSI
+---------------------------------------------------------------------------*/
Private void ProcessSpecial (ElemHandle eh, Reporter reporter)
    {
    KIBODY* bodyP;
    KIENTITY_LIST *listP= NULL;
    int count;
    int solidstatus;
    solidstatus = mdlKISolid_listCreate (&listP);    
    
    solidstatus = mdlKISolid_elementToBodyList(&listP,NULL,NULL,NULL,
                               const_cast <MSElementDescr*>(eh.GetElemDescrCP()),
                               eh.GetModelRef(),0,TRUE,TRUE,TRUE);

    if (SUCCESS == solidstatus)
    {
        solidstatus = mdlKISolid_listCount (&count,listP);   
        if (SUCCESS == solidstatus){
            for (int i =0;SUCCESS== mdlKISolid_listNthEl (&bodyP,listP,i);++i)
                {
                reporter.ReportSolid (L"DirectSolid",bodyP,reporter.GetRootNodePtr());
                mdlKISolid_freeBody(bodyP);
                }
        }
    }

    mdlKISolid_listDelete (&listP);
//done with trying it as a solid now trying it as a handler interface.
    Bentley::WString nameInfo1;
    Handler& tempHandle = eh.GetHandler(MISSING_HANDLER_PERMISSION_All_);
   // tempHandle.GetDescription(eh,nameInfo1,512);
    ElemDisplayParams dispParams;

    tempHandle.GetDisplayHandler()->GetElemDisplayParams(eh,dispParams, true);
    {
        GPArrayP pArray=GPArray::Grab ();
                      // if (baseHandler.GetDisplayHandler()->IsVisible()) does not have a viewcontext so fallback to old school element info.
        int fillGpaStatus = tempHandle.GetDisplayHandler()->FillGPA (eh, pArray);
        if (fillGpaStatus == SUCCESS)
            reporter.ReportGPArray(L"DirectGPA",pArray,reporter.GetRootNodePtr()); 
        pArray->Drop();
    }
    if (dispParams.m_transparency != 0)
         printf("this is not a solid \n");
    ICurveQueryP pCurve = tempHandle.GetICurveQuery();
    if (pCurve)
    {
        double len;
        CurveQuery::GetLength (eh,len,NULL);
    }


    //playing around with the iterators.
    for (ChildElemIter child(eh,EXPOSECHILDREN_Count );child.IsValid();child= child.ToNext())
        {
        Bentley::WString nameInfo;
        Handler& baseHandler = child.GetHandler(MISSING_HANDLER_PERMISSION_Draw );
        baseHandler.GetDescription(child,nameInfo,512);

        GPArrayP pArray=GPArray::Grab ();
                      // if (baseHandler.GetDisplayHandler()->IsVisible()) does not have a viewcontext so fallback to old school element info.
        int fillGpaStatus = baseHandler.GetDisplayHandler()->FillGPA (child, pArray);
        if (fillGpaStatus == SUCCESS)
            reporter.ReportGPArray(L"DirectGPA",pArray,reporter.GetRootNodePtr()); 
        pArray->Drop();
        }
    //yet more testing: did not work here with the 106 elements.  need to make them into mesh elements first.  see reporter
    //double volume = SolidsFunctions::MeasureSupport::GetVolume (eh.GetElemDescrCP());
    return;
    }

/*------------------------------------------------------------------------------+
|   imodelVisitor_processModel this is a way of processing a model by iterating |
|   over the elements.  It will get only the top level elements.                |
|                                                                               |
+------------------------------------------------------------------------------*/
int imodelVisitor_processModel (DgnModelRefP pModel)
     {
     MSDgnFileP pFile;
     DgnCacheP pCache = pModel->GetDgnCache();
     pFile = pModel->GetDgnFile();
     UInt32 count = pCache->GetElementCount (DGNCACHE_SECTION_GRAPHIC_ELMS);
     DgnElmListIterator iter;
     int elCounter=0;
     ModelRefIteratorP  mrIterator;
     DgnModelRefP       modelP;
     Reporter  s_reporter(L"_",pFile,true,false);
     //Reporter s_reporter;
     MyElementGraphicsProcessor s_graphicProcessor (s_reporter,s_reporter.GetRootNodePtr());
     
     ISessionMgrR    pSessionMgr=ISessionMgr::GetManager();
     
     
     mdlModelRefIterator_create (&mrIterator,pModel,MRITERATE_Root|MRITERATE_PrimaryChildRefs,-1);

     //if ((pHandlers->GetDataHandler()->GetActiveSchema().CompareToI(WString("*"))==0)||(pHandlers->GetDataHandler()->SchemaInModel(pModel,pHandlers->GetDataHandler()->GetActiveSchema())))
         {        
         //if (pFile->IsIModel()||g_bProcessAll)
             while(NULL!=(modelP= mdlModelRefIterator_getNext(mrIterator)))
                 {
                 pCache = mdlModelRef_getCache(modelP);
                
                 if (pCache && (!pCache->IsFilled(DGNCACHE_SECTION_GRAPHIC_ELMS)))
                    pCache->FillSections (DGNCACHE_SECTION_GRAPHIC_ELMS);
                 if (pCache)
                 for (CacheElemRef elRef = iter.FirstCacheElm(pCache->GetGraphicElms()); NULL != elRef; elRef = iter.NextCacheElm (false, false))
                   {
                   ElemHandle eh = ElemHandle(elRef,pModel);
                    //this method will enable us to process either DGN or XGraphics independent of the formatting.
#if defined (EXTRA_TESTING)
                   if (106==elementRef_getElemType(elRef))
                   {
                       MSElement el;
                       MSWChar cellName[512];
                       elementRef_getElement (elRef,&el,sizeof el);
                       mdlCell_extractName (cellName,512,&el);
                       printf ("processing Cell name %S \n",cellName);
                       ProcessSpecial (eh,s_reporter);
                   }
#endif
                   Bentley::Ustn::ElementGraphicsOutput::Process(eh,s_graphicProcessor);

                   elCounter++;
                   }
                 }
             mdlModelRefIterator_free(&mrIterator);
             }
         s_reporter.Save(WString("testing.xml"));
    return elCounter;
    }
#endif
/*------------------------------------------------------------------------------+
|  SheetAutomation_ModelTest - this is for testing the special element          |
|  processing.                                                                  |
|                                                                               |
+------------------------------------------------------------------------------*/
extern "C" DLLEXPORT void SheetAutomation_ModelTest(char* unparsed)
{
    imodelVisitor_processModel(ACTIVEMODEL);
}


//-----------------------------------------------------------------------------
// check the file into the ProjectWise repository if the file already exists
// the file is checked in.  If the file does  not exist the entry is created
// in the repository.
// Input:   location - the  location of the  work file to check in
//          filename - the name of the file being checked in                     
//
// Output: SUCCESS if the file is checked in or created.
//------------------------------------------------------------------------------
long SheetAutomation_getCurrentProjectID(DgnModelRefP pModel)
{
    long        projID = 0;
    MSWChar     wfullFilePath[MAXFILELENGTH];
    StatusInt   status=ERROR;
    status = mdlModelRef_getFileNameW(pModel,wfullFilePath,MAXFILELENGTH);

    if(SUCCESS != status)
        return -1;  //most likely bad name but this should not happen.

    projID = aaApi_GetProjectIdFromFileName2(wfullFilePath);

    if (0==projID)
        SheetAutomation_pwErrorInformation();

    //this is so we can capture the project from the command line parameters.
    if(projID == 0)
        projID = g_iProjectID;

    return projID;
}
/*------------------------------------------------------------------------------+
|  SheetAutomation_getCurrentDocumentID - this will get the current doc id      |
|  based on the mcm API - not the best but it works some times.                 |
|                                                                               |
+------------------------------------------------------------------------------*/
long SheetAutomation_getCurrentDocumentID(DgnModelRefP pModel)               
{
    long         projID = 0;
    long         docID = 0;
    MSWChar      wfullFilePath[MAXFILELENGTH];
    char         fullFilePath[MAXFILELENGTH];
    HAADMSBUFFER docBuffer;
    StatusInt    status = ERROR;
    LPGUID       pGUIDs;
    int          nGUIDs;

    //not going to worry about the status for now.
    status = mdlModelRef_getFileName(pModel,fullFilePath,MAXFILELENGTH);

    status = mdlModelRef_getFileNameW(pModel,wfullFilePath,MAXFILELENGTH);


    //try using this function it has been deprecated.
    //mcmMain_GetDocumentIdByFilePath(wfullFilePath,&projID,&docID);
    status = aaApi_GetGuidsFromFileName(&pGUIDs,&nGUIDs,fullFilePath,FALSE);
    
    docBuffer = aaApi_SelectDocumentDataBufferByFilePath(wfullFilePath);

    if (NULL == docBuffer)
        SheetAutomation_pwErrorInformation();
    else
    {
        docID = aaApi_DmsDataBufferGetNumericProperty(docBuffer,DOC_PROP_ID,0);
        projID = aaApi_DmsDataBufferGetNumericProperty(docBuffer,DOC_PROP_PROJECTID,0);
        aaApi_DmsDataBufferFree(docBuffer);
    }

    if (0==projID)
        SheetAutomation_pwErrorInformation();

    if(docID == 0)
        docID = g_iDocID;
    
    if(nGUIDs>0)
        aaApi_Free(pGUIDs);

    return docID;
}
/*------------------------------------------------------------------------------+
|  SheetAutomation_findFolderID - finds a node that is a child folder           |
|                                                                               |
|  Input: startingFolder the parent folder to start from.                       |
|         targetName - the name of the leaf to search for.                      |
|  Return: the id of the folder                                                 |
+------------------------------------------------------------------------------*/
long SheetAutomation_findFolderID (long startingFolder,LPCWSTR targetName)
{
    long         folderID = 0;
    HAADMSBUFFER buffer;
    
    buffer = aaApi_SelectProjectDataBufferChilds2(startingFolder,false);
    
    if (NULL == buffer)
        return -1;

    int childCount = aaApi_DmsDataBufferGetCount(buffer);
    for (int i = 0;i<childCount; i++)
    {
        LPCWSTR childName = aaApi_DmsDataBufferGetStringProperty(buffer,PROJ_PROP_NAME,i);
        if (wcscmp(childName,targetName) == 0)
            folderID = aaApi_DmsDataBufferGetNumericProperty(buffer,PROJ_PROP_ID,i);
    }

    return folderID;
}
/*------------------------------------------------------------------------------+
| SheetAutomation_findLeafCreateIfMissing - this function will find the node    |
| that matches the name passed in.  If the node does not exist then it is       |
| created.                                                                      |
| Input: parentID - the project ID of the parent                                |
|        leafName - the name of the target node.                                |
| Output: the id of the leaf node.                                              |
+------------------------------------------------------------------------------*/
long SheetAutomation_findLeafCreateIfMissing(long parentID, LPCWSTR leafName)
{
    long         leafID = 0;
    BOOL         bStatus;
    HAADMSBUFFER buffer = aaApi_SelectProjectDataBufferChilds2(parentID,false);
    HAADMSBUFFER parentbuffer = aaApi_SelectProjectDataBuffer(parentID);
    
    if((NULL == buffer)||(NULL == parentbuffer))
        return -1;

    int ssChildCount = aaApi_DmsDataBufferGetCount(buffer);

    for (int k = 0;k<ssChildCount;k++)
    {
        LPCWSTR leaf = aaApi_DmsDataBufferGetStringProperty(buffer,PROJ_PROP_NAME,k);
        if(wcscmp(leaf,leafName)==0)
            leafID = aaApi_DmsDataBufferGetNumericProperty(buffer,PROJ_PROP_ID,k);
    }
    //if there is no project already in Piping ISO then create one.
    if(leafID==0)
    {
        long lStorageID;
        long lMgrID;
        long lWorkFlowID;
        long lWorkSpaceID;
        lMgrID = aaApi_DmsDataBufferGetNumericProperty(parentbuffer,PROJ_PROP_MANAGERID,1);
        lStorageID = aaApi_DmsDataBufferGetNumericProperty(parentbuffer,PROJ_PROP_STORAGEID,1);
        lWorkFlowID = aaApi_DmsDataBufferGetNumericProperty(parentbuffer,PROJ_PROP_WORKFLOWID,1);
        lWorkSpaceID = aaApi_DmsDataBufferGetNumericProperty(parentbuffer,PROJ_PROP_WSPACEPROFID, 1);
        bStatus = aaApi_CreateProject(&leafID,parentID,lStorageID,lMgrID,AADMS_PROJECT_TYPE_NORMAL,lWorkFlowID,lWorkSpaceID,0,leafName,L"ISO Sheet Model");
        if(!bStatus)
            SheetAutomation_pwErrorInformation();
    }

    aaApi_DmsDataBufferFree(buffer);
    aaApi_DmsDataBufferFree(parentbuffer);
    
    return leafID;
}
/*------------------------------------------------------------------------------+
| SheetAutomation_getSheetPath - gets the sheet path from the ProjectID for the 
| model.  This is fairly hard coded in that the standard is that the design
| folder structure mirrors to the sheet folder.  This is per user definition.
| This function returns the new project folder id to use for the sheets.  It 
| will return -1 if the folder cannot be created.  Most likely cause is the 
| login failed.
+------------------------------------------------------------------------------*/
long SheetAutomation_getSheetPath(long startProjID)
{
    long rtnID=0;
    long status = 0;
    HAADMSBUFFER buffer = aaApi_SelectProjectDataBuffer(startProjID);

    if (NULL == buffer)
        return -1;

    //this should be the name that is compared to...
    LPCWSTR name = aaApi_DmsDataBufferGetStringProperty(buffer,PROJ_PROP_NAME ,0);
    
    //back up one
    long parentID;
    status = aaApi_SelectParentProject(startProjID);
    //if I get here and things are bad return and stop the insanity.
    if (-1 == status)
        return status;

    parentID = aaApi_GetProjectNumericProperty(PROJ_PROP_ID,0);
    //back up two
    status = aaApi_SelectParentProject(parentID);
    parentID = aaApi_GetProjectNumericProperty(PROJ_PROP_ID,0);

    //back up three...this should be the Mechanical folder
    status = aaApi_SelectParentProject(parentID);
    
    parentID = aaApi_GetProjectNumericProperty(PROJ_PROP_ID,0);

#if !defined OLDCODE
    parentID = SheetAutomation_findFolderID(parentID,L"Sheets");
    
    parentID = SheetAutomation_findFolderID(parentID,L"Piping Isometrics");

    rtnID = SheetAutomation_findLeafCreateIfMissing(parentID,name);
#else
    buffer = aaApi_SelectProjectDataBufferChilds2(parentID,false);
    int childCount = aaApi_DmsDataBufferGetCount(buffer);
    for (int i = 0;i<childCount; i++)
    {
        LPCWSTR childName = aaApi_DmsDataBufferGetStringProperty(buffer,PROJ_PROP_NAME,i);
        if (wcscmp(childName,L"Sheets") == 0)
        {
            long pid = aaApi_DmsDataBufferGetNumericProperty(buffer,PROJ_PROP_ID,i);
            HAADMSBUFFER b2 = aaApi_SelectProjectDataBufferChilds2(pid,false);
            int subChildCount = aaApi_DmsDataBufferGetCount(b2);
            
            for (int j = 0;j<subChildCount;j++)
            {
                 LPCWSTR childName = aaApi_DmsDataBufferGetStringProperty(b2,PROJ_PROP_NAME,j);

                 if (wcscmp(childName,L"Piping Isometrics")==0)
                 {
                     long p = aaApi_DmsDataBufferGetNumericProperty(b2,PROJ_PROP_ID,j);
                     HAADMSBUFFER b3 = aaApi_SelectProjectDataBufferChilds2(p,false);
                     int ssChildCount = aaApi_DmsDataBufferGetCount(b3);
                     for (int k = 0;k<ssChildCount;k++)
                     {
                         LPCWSTR leaf = aaApi_DmsDataBufferGetStringProperty(b3,PROJ_PROP_NAME,k);
                         if(wcscmp(leaf,name)==0)
                         {
                             rtnID = aaApi_DmsDataBufferGetNumericProperty(b3,PROJ_PROP_ID,k);
                         }
                     }
                     //if there is no project already in Piping ISO then create one.
                     if(rtnID==0)
                     {
                         long lStorageID;
                         long lMgrID;
                         long lWorkFlowID;
                         long lWorkSpaceID;
                         lMgrID = aaApi_DmsDataBufferGetNumericProperty(b2,PROJ_PROP_MANAGERID,j);
                         lStorageID = aaApi_DmsDataBufferGetNumericProperty(b2,PROJ_PROP_STORAGEID,j);
                         lWorkFlowID = aaApi_DmsDataBufferGetNumericProperty(b2,PROJ_PROP_WORKFLOWID,j);
                         lWorkSpaceID = aaApi_DmsDataBufferGetNumericProperty(b2,PROJ_PROP_WSPACEPROFID, j);
                         aaApi_CreateProject(&rtnID,p,lStorageID,lMgrID,AADMS_PROJECT_TYPE_NORMAL,lWorkFlowID,lWorkSpaceID,0,name,L"ISO Sheet Model");
                     }
                     aaApi_DmsDataBufferFree(b3);

                 }

            }
            aaApi_DmsDataBufferFree(b2);
        }
    }

#endif

    aaApi_DmsDataBufferFree(buffer);

    return rtnID;
}
/*------------------------------------------------------------------------------+
| SheetAutomation_commitToPW - this will take the new model ref and the old     |
| document and project ids.  it will call the function to walk back up the tree |
| and over to the new location.                                                 |
+------------------------------------------------------------------------------*/
int SheetAutomation_commitToPW(DgnModelRefP pModel,long projID,long parentID)
{
    long    iFileType = AADMS_FTYPE_SHEET;
    long    iItemType = AADMS_ITYPE_UNKNOWN;
    long    iAppType=0 ;
    long    iDepType=0;
    long    iWorkspace=0;
    long    iStorageID = 0;
    BOOL    bLeaveOut = FALSE;
    long    ulFlags = AADMSDOCCREF_DEFAULT;
    long    ulMask = 0x0;
    wchar_t workingFileName[MAXFILELENGTH*4];//was coming out walked on...
    long    bufferSize = 1024;
    long    attrId;
    MSWChar docName[MAXFILELENGTH];
    MSWChar fileName[MAXFILELENGTH];
    MSWChar baseName[MAXNAMELENGTH];
    MSWChar dev[MAXDEVICELENGTH];
    MSWChar dir[MAXDIRLENGTH];
    MSWChar ext[MAXEXTENSIONLENGTH];
    MSWChar baseFileName[MAXFILELENGTH];
    BOOL    bStatus = false;
    long    docID=0;
    WString description;
    
    mdlModelRef_getFileNameW(pModel,fileName,MAXFILELENGTH);
    mdlFile_parseNameW(fileName,dev,dir,baseName,ext);
    mdlFile_buildNameW(docName,NULL,NULL,baseName,L"SHT");
    mdlFile_buildNameW(baseFileName,NULL,NULL,baseName,ext);
    
    long    outProjID = SheetAutomation_getSheetPath(projID);
    
    if (outProjID <=0)
        return outProjID;

    //MSWCharCP outPath = L"..\\..\\..\\Sheets\\Piping Isometrics\\CCH\\";
    description = WString ("ISOAutomation Model");

    //need to get the out project ID
    bStatus = aaApi_CreateDocument (&docID,outProjID,iStorageID,
                                    iFileType,iItemType,iAppType,iDepType,
                                    iWorkspace,fileName,baseFileName,docName,
                                    description.GetMSWCharCP(),
                                    NULL,bLeaveOut,ulFlags,workingFileName,
                                    bufferSize,&attrId);  

    //if the file is checked in then make a set of things.    
    if(TRUE == bStatus)
    {
        long  setId;
        long  memberID;
        log_printf(1, "PW file commited to the store \n");

        bStatus = aaApi_CreateLSet (&setId,outProjID,docID,projID,parentID,AADMS_SETMEM_REF,AADMS_SETMEM_COPY,&memberID);

        log_printf(0,"PW set created %ld, member id  %ld, outProject = %ld, document id = %ld, project ID = %ld, parent ID = %ld \n",setId,memberID,outProjID,docID,projID,parentID);
    }

//taking this out since I am setting the file type in the create step.
#if defined OLDCODE
    //set the file type to sheet.
    if(TRUE == bStatus)
    {
        log_printf(0, "PW Doc Set created");
        bStatus = aaApi_SetDocumentFileType(projID,docID,AADMS_FTYPE_SHEET);
    }
#endif

   //did not create or  checkin a doc
    if (FALSE == bStatus)
        {
            SheetAutomation_pwErrorInformation(); 
            return !SUCCESS;
        }
    
    log_printf(0," File CREATED, SET CREATED, and APPTYPE applied ");
    return SUCCESS;
}
/*------------------------------------------------------------------------------+
| SheetAutomation_attachReference this will attach the reference file.          |
| if the bFlag is true then it will do a rotation adjustment.  I don't think    |
| this is being used..                                                          |
+------------------------------------------------------------------------------*/
void SheetAutomation_attachReference(DgnModelRefP pModel, DgnModelRefP attachModel, int nestFlag, bool bFlag)
{
    DgnModelRefP   refAttachment;
    char           fileName[MAXFILELENGTH];
    MSWChar        refAttModelName[MAX_MODEL_NAME_LENGTH];
    MSWChar        logicalName[MAX_MODEL_NAME_LENGTH];
    MSWChar        description[MAX_MODEL_DESCR_LENGTH];
    int            status;
    ReferenceFileP refP;

    //get the information to populate the reference file attachment.
    mdlModelRef_getFileName(attachModel,fileName,MAXFILELENGTH);
    mdlModelRef_getModelName   (attachModel,refAttModelName);
    mdlModelRef_getModelDescription(attachModel,logicalName);
    mdlModelRef_getModelDescription(attachModel,description);
    
    char attachmentFileName[1024];
    char dev[MAXDEVICELENGTH];
    char dir[MAXDIRLENGTH];
    char attName[MAXFILELENGTH];
    char ext[MAXEXTENSIONLENGTH];

    mdlFile_parseName(fileName,dev,dir,attName,ext);
    
    if(g_iProjectID>0)
        sprintf(attachmentFileName,"PW_WORKDIR:dms%05d\\%s.%s",g_iProjectID,attName,ext);
    else
        strcpy(attachmentFileName,fileName);
    
    log_printf(0,"creating attachment ");
    
    status = mdlRefFile_beginAttachmentToModel(&refAttachment,attachmentFileName,refAttModelName,logicalName,description,pModel);
    if(0 == status)
        log_printf(0,"begin succeeded  ");
    //any steps needed in the attachment process.
    if(bFlag)
        {
            RotMatrix rMatrix;
            int       attachMethod = ATTACHMETHOD_Isometric;
            //at one time I thought I had to rotate the reference file.
            //turns out I did not just attach to the correct model.
            //left this in for reference on how to work with the reference during
            //attach process.
            mdlView_getStandard(&rMatrix,STDVIEW_ISO);
            mdlRefFile_setParameters (&rMatrix,REFERENCE_ROTATION,refAttachment);
            mdlRefFile_setParameters(&attachMethod,REFERENCE_ATTACHMETHOD, refAttachment);
            refP = mdlRefFile_getInfo(refAttachment);
            //printf("the master orig is %lf, %lf, %lf and the ref orig is %lf, %lf, %lf \n", refP->display.mast_org.x, refP->display.mast_org.y,refP->display.mast_org.z,
            //    refP->display.ref_org.x,refP->display.ref_org.y, refP->display.ref_org.z);
            mdlRefFile_setCoincidentWorldOrigin(refAttachment);
        }
    
    status = mdlRefFile_completeAttachment(refAttachment,nestFlag,-1,FALSE);

    if (SUCCESS == status)
        log_printf(0, "Reference File Attachment completed ");
}
/*------------------------------------------------------------------------------+
| SheetAutomation_createSheet - this creates the sheet model.                   |
|                                                                               |
+------------------------------------------------------------------------------*/
void SheetAutomation_createSheet(void)
{
    int             status;
    char            fileName[MAXFILELENGTH];
    DgnModelRefP    outModel;
    BoolInt         is3dFile;
    char            baseFullFileName[MAXFILELENGTH];
    char            baseDevName[MAXDEVICELENGTH];
    char            baseDirName[MAXDIRLENGTH];
    char            baseExtName[MAXEXTENSIONLENGTH];
    char            baseFileName[MAXFILELENGTH];
    DgnModelRefP    sheetModelP=NULL;
    DgnModelRefP    origSheetModelP=NULL;
    DgnFileObjP     currentFileP = ISessionMgr::GetActiveDgnFile();
    DgnFileObjP     destFileP;
    MSWChar         wrkFile[MAXFILELENGTH];

    //copy out the file to make sure we have it for reference.
    aaApi_CopyOutDocument(g_iProjectID,g_iDocID,NULL,wrkFile,MAXFILELENGTH);
    
    //break down the location.
    mdlModelRef_getFileName(ACTIVEMODEL,baseFullFileName,MAXFILELENGTH);
    mdlFile_parseName (baseFullFileName,baseDevName,baseDirName,baseFileName,baseExtName);
    mdlFile_buildName(fileName,baseDevName,baseDirName,baseFileName,"SHT");
    is3dFile = mdlModelRef_is3D(ACTIVEMODEL);
    
    //create a file with a default model.
    status = mdlWorkDgn_createFile (&outModel, fileName, DGNFILE_FORMAT_V8, ACTIVEMODEL, SEED_CopyDefaultData, NULL, NULL, is3dFile); 
    mdlWorkDgn_saveChanges(outModel);
    log_printf(0," work file %s created ", fileName);

    //attach the original design model the design model in the SHT file.
    SheetAutomation_attachReference(outModel,ACTIVEMODEL,REFATTACH_NEST_NONE,false);
    mdlWorkDgn_saveChanges(outModel); 
    
    //is there a sheet model then we will copy it over to the new file and attach 
    //the design model as a reference to it.
    status = mdlModelRef_createWorkingByName(&origSheetModelP,currentFileP,L"SheetView",false,false);
    if(SUCCESS == status)
        {
            SheetDef* sdP=NULL;
            status = mdlModelRef_getSheetDef(origSheetModelP,sdP);
        
            destFileP = mdlModelRef_getDgnFile(outModel);
            //copy the  sheet model from the source to the new file.
            status = mdlModelRef_copyModel (&sheetModelP,origSheetModelP,destFileP,L"SheetView",L"Sheet View");
            
            destFileP->ProcessChanges(DGNSAVE_SAVE_SETTINGS,0);
            ModelID sheetID = sheetModelP->GetModelID();
            //not documented function pulled it out of the library.
            dgnFileObj_setDefaultModelID (destFileP,sheetID);

            destFileP->ProcessChanges(DGNSAVE_SAVE_SETTINGS,0);
            
            sheetID = dgnFileObj_getDefaultModelID (destFileP);
            
            mdlWorkDgn_saveChanges(outModel); 
        
            log_printf(0,"the sheet model is copied ");

            if (SUCCESS == status)
                {
                    SheetDef* sheetDefP=NULL;
                    //attach the design model in the SHT file to the sheet model in the SHT file.
                    SheetAutomation_attachReference(sheetModelP,origSheetModelP,REFATTACH_NEST_DISPLAY, false);
                    mdlModelRef_setModelType(sheetModelP,MODEL_TYPE_Sheet);
                    mdlModelRef_saveModelInfo(sheetModelP,TRUE);
                    mdlModelRef_freeWorking(sheetModelP);
                
                    if(sdP!=NULL)
                        mdlSheetDef_free(&sdP);
                
                    mdlWorkDgn_saveChanges(outModel);
                }                
            mdlModelRef_freeWorking(origSheetModelP);
        }

    //these will get the project id from the current and uses
    //these as a reference model.
    long projID = SheetAutomation_getCurrentProjectID(ACTIVEMODEL);

    long docID = SheetAutomation_getCurrentDocumentID(ACTIVEMODEL);
    
    if(projID>0)
        status = SheetAutomation_commitToPW(outModel,projID,docID);
    
    if(SUCCESS==status)
        log_printf(0,"Commited Project ID %ld ",status);
    else
        log_printf(0,"error commiting to PW store");

    //make sheet model active model save settings????
    mdlWorkDgn_closeFile(outModel);

    return;
}
/*---------------------------------------------------------------------------------**//**
* @description  SheetAutomation_mdlCommand
* @param 	unparsed      The project and document id passed in as an unparsed arg
* @bsimethod 							BSI             06/03
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" DLLEXPORT void SheetAutomation_mdlCommand 
(
char * unparsed
)
    {
        if((unparsed) && (strlen(unparsed)>0))
        {
            int   status = 0;
            int     argc, numchars;
            char  **argv, *argStrings;

                // Determine the space needed for argc/argv
            mdlString_parseIntoArgcArgv (unparsed, NULL, NULL, &argc, &numchars);
            argv       = (char**)_alloca ((argc+1) * sizeof(char *) );
            argStrings = (char*)_alloca ((numchars+1) * sizeof(char) );

            // Actually split up the arguments
            mdlString_parseIntoArgcArgv (unparsed, argv, argStrings, &argc, &numchars);
    
            if (argc >= 2)
            {
                long iProjectID = atoi(argv[0]);
                long iDocumentID = atoi(argv[1]);
                g_iProjectID = iProjectID;
                g_iDocID = iDocumentID;
            }
        }

    SheetAutomation_createSheet();
    }
/*-----------------------------------------------------------------------------+
| SheetAutomation_PwLogin - does the login to the datasource.                  |
|                                                                              |
|  Input: dsName the datasource to login to                                    |
|         userName the user name for logging in.                               |
|         password the password                                                |
|                                                                              |
| return: 1 succeeded.                                                         |
+-----------------------------------------------------------------------------*/
 int SheetAutomation_pwLogin(char* dsName, char* userName, char* passwd)
{
    wchar_t wdsName[512];
    wchar_t wuserName[512];
    wchar_t wpasswd[512];
    size_t convSize;
    mbstowcs_s(&convSize,wdsName,512,dsName,512);
    mbstowcs_s(&convSize,wuserName,512,userName,512);
    mbstowcs_s(&convSize,wpasswd,512,passwd,512);

    aaApi_Initialize(0);
    BOOL loginState = FALSE;
    HDSOURCE hdSource = NULL;
    hdSource = aaApi_GetActiveDatasource ( );
    
    if (NULL == hdSource)
        loginState =  aaApi_Login(10,wdsName,wuserName,wpasswd,NULL);
    else
        loginState = true;
    
    log_printf(0,"logging into PW returned %ld status ",loginState);

    g_isLoggedIn = loginState;
    SetDataSourceName(WString (wdsName));
    
    return g_isLoggedIn?SUCCESS:!SUCCESS;

}
/*-----------------------------------------------------------------------------+
|  SheetAutomation_loginCMD - This is a command to login to PW                  |
|                             The command CMD_SHEETAUTOMATION_LOGIN is connected|
|                             This command is passed through the AS framework.  |
|  Input:  unparsed string containing the datasource name: the login:           |
|          the password: the project id: the document id                        |
|                                                                               |
+------------------------------------------------------------------------------*/
extern "C" DLLEXPORT void SheetAutomation_loginCMD(char* unparsed)
{
    int   status = 0;
    int   argc, numchars;
    char  **argv, *argStrings;

    printf("The login keyin is %s \n",unparsed);

    // Determine the space needed for argc/argv
    mdlString_parseIntoArgcArgv (unparsed, NULL, NULL, &argc, &numchars);
    argv       = (char**)_alloca ((argc+1) * sizeof(char *) );
    argStrings = (char*)_alloca ((numchars+1) * sizeof(char) );

    // Actually split up the arguments
    mdlString_parseIntoArgcArgv (unparsed, argv, argStrings, &argc, &numchars);
    
    if (argc > 2)
        status = SheetAutomation_pwLogin (argv[0],argv[1],argv[2]);

    if(status == SUCCESS)
    {
        long iProjectID = atoi(argv[3]);
        long iDocumentID = atoi(argv[4]);
        //aaApi_CopyOutDocument(iProjectID,iDocumentID,NULL,NULL,0);
        //SheetAutomation_createSheet();
    }
    if(argc > 3)
    {
    g_iDocID = atoi(argv[4]);
    g_iProjectID = atoi(argv[3]);
    }
    return;
}
/*---------------------------------------------------------------------------------**//**
* @description  MdlMain
* @param 	argc      The number of command line parameters sent to the application.
* @param 	argv[]    The array of strings sent to the application on the command line.
* @bsimethod 							BSI             06/03
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" DLLEXPORT  int MdlMain
(
int         argc,
char        *argv[]
)
    {
    RscFileHandle   rfHandle;

    mdlResource_openFile (&rfHandle,NULL,RSC_READONLY);

    // Map command name to function (usage: MDL COMMAND COMPEXPORT)
    static  MdlCommandName cmdNames[] = 
    {
        {SheetAutomation_mdlCommand, "SheetAutomation_mdlCommand"  },
        0,
    };

    mdlSystem_registerCommandNames (cmdNames);

    // Map key-in to function
    static MdlCommandNumber cmdNumbers[] =
    {
        {SheetAutomation_mdlCommand,  CMD_SHEETAUTOMATION_CREATE_SHEET },
        {SheetAutomation_ModelTest, CMD_SHEETAUTOMATION_CREATE_MODEL},
        {SheetAutomation_loginCMD,CMD_SHEETAUTOMATION_LOGIN},
        0,
    };

    mdlSystem_registerCommandNumbers (cmdNumbers);

    mdlParse_loadCommandTable (NULL);

    return SUCCESS;
    }
