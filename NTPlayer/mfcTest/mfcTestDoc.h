
// mfcTestDoc.h : interface of the CmfcTestDoc class
//


#pragma once


class CmfcTestDoc : public CDocument
{
protected: // create from serialization only
	CmfcTestDoc();
	DECLARE_DYNCREATE(CmfcTestDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CmfcTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


