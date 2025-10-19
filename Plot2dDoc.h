#pragma once

#include "Plot2dNode.h"

class CPlot2dDoc : public CDocument
{
	DECLARE_DYNCREATE(CPlot2dDoc)

public:
	CPlot2dDoc();
	virtual ~CPlot2dDoc();

	nzg::Plot2dNode* m_pn;

#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()

};