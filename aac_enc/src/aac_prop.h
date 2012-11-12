//-----------------------------------------------------------------------------
//
//	Monogram AAC Encoder
//
//	Base on GNU LGPL libfaac
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CAACPropertyPage class
//
//-----------------------------------------------------------------------------
class CAACPropertyPage : public CBasePropertyPage
{
public:

	// enkoder
	IMonogramAACEncoder		*encoder;
	int						def_output;

	void UpdateStats();
	void UpdateConfig();
public:
	CAACPropertyPage(LPUNKNOWN lpUnk, HRESULT *phr);
	virtual ~CAACPropertyPage();
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr);

	// CBasePropertyPage
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	void OnVersionChange();

	// message
	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetDirty();
};

