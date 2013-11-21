
// mfcTestDoc.cpp : implementation of the CmfcTestDoc class
//

#include "stdafx.h"
#include "mfcTest.h"

#include "mfcTestDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmfcTestDoc

IMPLEMENT_DYNCREATE(CmfcTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CmfcTestDoc, CDocument)
END_MESSAGE_MAP()


// CmfcTestDoc construction/destruction

CmfcTestDoc::CmfcTestDoc()
{
	// TODO: add one-time construction code here

}

CmfcTestDoc::~CmfcTestDoc()
{
}

BOOL CmfcTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CmfcTestDoc serialization

void CmfcTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CmfcTestDoc diagnostics

#ifdef _DEBUG
void CmfcTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CmfcTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CmfcTestDoc commands
