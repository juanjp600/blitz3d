
#ifndef HTMLHELP_H
#define HTMLHELP_H

class BHtmlHelp;

class HelpListener{
public:
	virtual void helpOpen( BHtmlHelp *help,const string &file )=0;
	virtual void helpTitleChange( BHtmlHelp *help,const string &title )=0;
};

class BHtmlHelp : public CHtmlView{
public:
	BHtmlHelp( HelpListener *l ):listener(l){}

	string getTitle();

DECLARE_DYNAMIC( BHtmlHelp )
DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEraseBkgnd( CDC *dc );

private:
	virtual void OnTitleChange( LPCTSTR t );
	virtual void OnBeforeNavigate2( LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel );

	string title;
	HelpListener *listener;
};

#endif
