
// mfcTestView.cpp : implementation of the CmfcTestView class
//

#include "stdafx.h"
#include "mfcTest.h"

#include "mfcTestDoc.h"
#include "mfcTestView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmfcTestView

IMPLEMENT_DYNCREATE(CmfcTestView, CView)

BEGIN_MESSAGE_MAP(CmfcTestView, CView)
END_MESSAGE_MAP()

// CmfcTestView construction/destruction

CmfcTestView::CmfcTestView()
{
	// TODO: add construction code here

}

CmfcTestView::~CmfcTestView()
{
}

BOOL CmfcTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CmfcTestView drawing

void CmfcTestView::OnDraw(CDC* /*pDC*/)
{
	CmfcTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CmfcTestView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CmfcTestView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CmfcTestView diagnostics

#ifdef _DEBUG
void CmfcTestView::AssertValid() const
{
	CView::AssertValid();
}

void CmfcTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CmfcTestDoc* CmfcTestView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CmfcTestDoc)));
	return (CmfcTestDoc*)m_pDocument;
}
#endif //_DEBUG


// CmfcTestView message handlers
