
// mfcTestView.h : interface of the CmfcTestView class
//


#pragma once


class CmfcTestView : public CView
{
protected: // create from serialization only
	CmfcTestView();
	DECLARE_DYNCREATE(CmfcTestView)

// Attributes
public:
	CmfcTestDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CmfcTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in mfcTestView.cpp
inline CmfcTestDoc* CmfcTestView::GetDocument() const
   { return reinterpret_cast<CmfcTestDoc*>(m_pDocument); }
#endif

