////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The nzg software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including nzg as 
//  long as they comply with the license.BSD 2 - Clause License
// 
//  Copyright(c) 2024, TOPCON, All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met :
// 
//  1. Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and /or other materials provided with the distribution.
// 
//  3. The software package includes some companion executive binaries or shared 
//  libraries necessary to execute APs on Windows. These licenses succeed to the 
//  original ones of these software.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// 	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// 	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// 	OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// 	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include "pch.h"

#include "TreeCtrlEx.h"

using namespace nzg;

const TreeCursor& TreeCursor::operator =(const TreeCursor& posSrc)
{
	if (&posSrc != this) {
		m_hTreeItem = posSrc.m_hTreeItem;
		m_pTree = posSrc.m_pTree;
	}
	return *this;
}


TreeCursor TreeCursor::_insert(LPCTSTR strItem, int nImageIndex, HTREEITEM hAfter)
{
	TV_INSERTSTRUCT ins;
	ins.hParent = m_hTreeItem;
	ins.hInsertAfter = hAfter;
	ins.item.mask = TVIF_TEXT | TVIF_PARAM;
	ins.item.pszText = (LPTSTR)strItem;
	ins.item.lParam = NULL;
	if (nImageIndex != -1) {
		ins.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		ins.item.iImage = nImageIndex;
		ins.item.iSelectedImage = nImageIndex;
	}
	return TreeCursor(m_pTree->InsertItem(&ins), m_pTree);
}

int TreeCursor::getImageID()
{
	TV_ITEM item;
	item.mask = TVIF_HANDLE | TVIF_IMAGE;
	item.hItem = m_hTreeItem;
	m_pTree->GetItem(&item);
	return item.iImage;
}

bool TreeCursor::moveToNextByData(DWORD_PTR dwData, int nImageID)
{
	while (TRUE)
	{
		if (m_hTreeItem == NULL)
			return FALSE;

		if ((getData() == dwData) && (getImageID() == nImageID))
			return TRUE;

		*this = getNext(TVGN_NEXT);
	}

	ASSERT(FALSE); // Never get here
	return FALSE;
}

CImageList* TreeCursor::getImageList(UINT nImageList)
{
	ASSERT(m_pTree != NULL);
	if (m_pTree == NULL)
		return NULL;
	return m_pTree->GetImageList(nImageList);
}

bool TreeCursor::collapseAllChilds(TreeCursor tExcept)
{
	if (m_hTreeItem == (HTREEITEM)tExcept)
		return FALSE;

	bool bRet = true;
	for (TreeCursor tNode = getChild(); (HTREEITEM)tNode != NULL; tNode = tNode.getNextSibling())
	{
		if (!tNode.collapseAllChilds(tExcept))
			bRet = false;
	}

	if (bRet)
	{
		expand(TVE_COLLAPSE);
	}

	return bRet;
}

void TreeCursor::removeAllChilds()
{
	for (TreeCursor t = getChild(); (HTREEITEM)t != NULL; )
	{
		TreeCursor tNext = t.getNextSibling();
		t.deleteItem();
		t = tNext;
	}
}

void TreeCursor::setThreeStateCheck(int nCheck)
{
	ASSERT(::IsWindow(m_pTree->m_hWnd));
	TVITEM item;
	item.mask = TVIF_HANDLE | TVIF_STATE;
	item.hItem = m_hTreeItem;
	item.stateMask = TVIS_STATEIMAGEMASK;

	/*
	Since state images are one-based, 1 in this macro turns the check off, and
	2 turns it on.
	*/
	item.state = INDEXTOSTATEIMAGEMASK(nCheck + 1); //(fCheck ? 2 : 1));

	::SendMessage(m_pTree->m_hWnd, TVM_SETITEM, 0, (LPARAM)&item);
}

int TreeCursor::getThreeStateCheck()
{
	ASSERT(::IsWindow(m_pTree->m_hWnd));
	TVITEM item;
	item.mask = TVIF_HANDLE | TVIF_STATE;
	item.hItem = m_hTreeItem;
	item.stateMask = TVIS_STATEIMAGEMASK;

	/*
	Since state images are one-based, 1 in this macro turns the check off, and
	2 turns it on.
	*/
	//item.state = INDEXTOSTATEIMAGEMASK(nCheck + 1); //(fCheck ? 2 : 1));

	::SendMessage(m_pTree->m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
	return (item.state >> 12);
}

TreeCursor TreeCursor::findItem(DWORD_PTR dwData)
{
	DWORD_PTR dw = getData();
	if (dw == dwData)
		return *this;

	TreeCursor t;

	for (t = getChild(); (HTREEITEM)t != NULL; t = t.getNextSibling())
	{
		TreeCursor tFound = t.findItem(dwData);
		if ((HTREEITEM)tFound != NULL)
			return tFound;
	}

	return t;
}

TreeCursor TreeCursor::findChild(DWORD_PTR dwData, bool recursion)
{
	TreeCursor t;

	for (t = getChild(); t; t = t.getNextSibling())
	{
		if (t.getData() == dwData)
			return t;
		if (recursion)
		{
			TreeCursor tf = t.findChild(dwData, recursion);
			if (tf)
				return tf;
		}
	}

	return t;
}

bool TreeCursor::setChildren(bool bChildren)
{
	ASSERT(::IsWindow(m_pTree->m_hWnd));

	TVITEM tv;
	memset(&tv, 0, sizeof(TVITEM));
	tv.mask = TVIF_HANDLE | TVIF_CHILDREN;
	tv.hItem = m_hTreeItem;
	tv.cChildren = bChildren ? 1 : 0;

	return m_pTree->SetItem(&tv);
}

void TreeCursor::copyFrom(TreeCursor& tFrom)
{
	ASSERT(getChild().isNull());

	setText(tFrom.getText().c_str());
	setData(tFrom.getData());
	setCheck(tFrom.getCheck());
	int nImage = -1;
	int nSelImage = -1;
	if (tFrom.getImage(nImage, nSelImage))
	{
		setImage(nImage, nSelImage);
	}

	for (TreeCursor t = tFrom.getChild(); !t.isNull(); t = t.getNextSibling())
	{
		TreeCursor tChild = addTail(t.getText().c_str(), t.getImageID());
		tChild.copyFrom(t);
	}

	UINT nState = tFrom.getState(TVIS_EXPANDED);
	if (nState & TVIS_EXPANDED)
		expand();
	//SetState (nState, TVIS_EXPANDED);
}

TreeCursor TreeCursor::insertNextSibling(LPCTSTR szText, int nImageIndex)
{
	TV_INSERTSTRUCT ins;
	ins.hParent = getParent();
	ins.hInsertAfter = m_hTreeItem;
	ins.item.mask = TVIF_TEXT | TVIF_PARAM;
	ins.item.pszText = (LPTSTR)szText;
	ins.item.lParam = NULL;
	if (nImageIndex != -1) {
		ins.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		ins.item.iImage = nImageIndex;
		ins.item.iSelectedImage = nImageIndex;
	}
	return TreeCursor(m_pTree->InsertItem(&ins), m_pTree);

}

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlEx

TreeCursor TreeCtrlEx::findItem(DWORD_PTR dwData)
{
	TreeCursor tRoot = getRootItem();

	TreeCursor t;

	for (t = tRoot; (HTREEITEM)t != NULL; t = t.getNextSibling())
	{
		TreeCursor tFound = t.findItem(dwData);
		if ((HTREEITEM)tFound != NULL)
			return tFound;
	}

	return t;
}

TreeCursor TreeCtrlEx::copyItem(HTREEITEM hSource, HTREEITEM hNewParent, HTREEITEM hInsertAfter)
{
	TreeCursor tSource(hSource, this);
	std::string text = tSource.getText();
	DWORD_PTR dwData = tSource.getData();
	int nImage;
	int nSelectedImage;
	tSource.getImage(nImage, nSelectedImage);
	UINT nState = tSource.getState(TVIF_STATE);
	BOOL bCheck = tSource.getCheck();

	TreeCursor tNew = insertItem(text.c_str(), hNewParent, hInsertAfter);

	for (TreeCursor tChild = tSource.getChild(); (HTREEITEM)tChild != NULL; tChild = tChild.getNextSibling())
	{
		copyItem((HTREEITEM)tChild, (HTREEITEM)tNew, TVI_LAST);
	}

	tNew.setData(dwData);
	tNew.setImage(nImage, nSelectedImage);
	tNew.setState(nState, TVIF_STATE);
	tNew.setCheck(bCheck);

	return tNew;
}


