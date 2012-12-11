
#ifndef _DSHOW_CONTROL_H_
#define _DSHOW_CONTROL_H_

class CDShowControl
{
public:
	CDShowControl();
	~CDShowControl();
public:

	HRESULT			Create(CDShowEngine* pEngine);
	void			Destroy(void);
};


#endif