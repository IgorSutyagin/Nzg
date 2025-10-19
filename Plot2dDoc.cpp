#include "pch.h"

#include "Plot2dDoc.h"

IMPLEMENT_DYNCREATE(CPlot2dDoc, CDocument)

CPlot2dDoc::CPlot2dDoc()
{
	m_pn = nullptr;
}

BOOL CPlot2dDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

CPlot2dDoc::~CPlot2dDoc()
{
}


BEGIN_MESSAGE_MAP(CPlot2dDoc, CDocument)
END_MESSAGE_MAP()


// CPlot2dDoc diagnostics

#ifdef _DEBUG
void CPlot2dDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CPlot2dDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CPlot2dDoc serialization

void CPlot2dDoc::Serialize(CArchive& ar)
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
#endif


