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
#pragma once


namespace nzg
{
	class TreeCtrlEx;

	/////////////////////////////////////////////////////////////////////////////
	// CTreeCursor

	class TreeCursor
	{
		// Attributes
	protected:
		HTREEITEM	m_hTreeItem;
		TreeCtrlEx* m_pTree;

		// Implementation
	protected:
		TreeCursor TreeCursor::_insert(LPCTSTR strItem, int nImageIndex, HTREEITEM hAfter);

		// Operation
	public:
		bool isValid() const;
		bool isNull() const { return m_hTreeItem == NULL; }
		TreeCursor() : m_hTreeItem(NULL), m_pTree(NULL) {}
		TreeCursor(HTREEITEM hTreeItem, TreeCtrlEx* pTree) : m_hTreeItem(hTreeItem), m_pTree(pTree)	{ }
		TreeCursor(const TreeCursor& a) { *this = a; }
		~TreeCursor() {}
		void reset() {	m_pTree = nullptr;	m_hTreeItem = NULL;	}
		const TreeCursor& operator =(const TreeCursor& posSrc);
		operator HTREEITEM() {	return m_hTreeItem;	}
		operator bool() { return m_pTree != nullptr && m_hTreeItem != NULL;	}
		TreeCtrlEx* getTreeCtrl() const { return m_pTree; }
		bool operator==(const TreeCursor& t) const { return m_hTreeItem == t.m_hTreeItem; }
		bool operator!=(const TreeCursor& t) const { return m_hTreeItem != t.m_hTreeItem; }

		TreeCursor insertAfter(LPCTSTR strItem, HTREEITEM hAfter, int nImageIndex = -1) { 
			return _insert(strItem, nImageIndex, hAfter); 
		}
		TreeCursor addHead(LPCTSTR strItem, int nImageIndex = -1) { return _insert(strItem, nImageIndex, TVI_FIRST); }
		TreeCursor addTail(LPCTSTR strItem, int nImageIndex = -1) {	return _insert(strItem, nImageIndex, TVI_LAST); }

		int getImageID();
		bool moveToNextByData(DWORD_PTR dwData, int nImageID);
		CImageList* getImageList(UINT nImageList);

		// Returns FALSE when some of childs is the same as tExcept
		bool collapseAllChilds(TreeCursor tExcept);

		bool getRect(LPRECT lpRect, bool bTextOnly);
		TreeCursor getNext(UINT nCode);
		TreeCursor getChild();
		TreeCursor getNextSibling();
		TreeCursor getPrevSibling();
		TreeCursor getParent();
		TreeCursor getFirstVisible();
		TreeCursor getNextVisible();
		TreeCursor getPrevVisible();
		TreeCursor getSelected();
		TreeCursor getDropHilight();
		TreeCursor getRoot();
		std::string getText();
		bool getImage(int& nImage, int& nSelectedImage);
		UINT getState(UINT nStateMask);
		DWORD_PTR getData();
		bool set(UINT nMask, LPCTSTR lpszItem, int nImage,
			int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam);
		bool setChildren(bool bChildren);
		bool setText(LPCTSTR lpszItem);
		bool setImage(int nImage, int nSelectedImage);
		bool setState(UINT nState, UINT nStateMask);
		bool setData(DWORD_PTR dwData);
		bool hasChildren();
		BOOL getCheck();
		BOOL setCheck(BOOL bCheck);
		void setThreeStateCheck(int nCheck); // 0 - No check, 1 - check, 2 - intermediate
		int getThreeStateCheck();
		// Operations
		bool deleteItem();

		bool expand(UINT nCode = TVE_EXPAND);
		TreeCursor select(UINT nCode);
		TreeCursor select();
		TreeCursor selectDropTarget();
		TreeCursor selectSetFirstVisible();
		CEdit* editLabel();
		CImageList* createDragImage();
		bool sortChildren();
		void removeAllChilds();
		bool ensureVisible();

		TreeCursor findItem(DWORD_PTR dwData);
		TreeCursor findChild(DWORD_PTR dwData, bool recursion = false);
		
		void copyFrom(TreeCursor& tFrom);

		TreeCursor insertNextSibling(LPCTSTR szText, int nImage = -1);
	};

	/////////////////////////////////////////////////////////////////////////////
	// TreeCtrlEx

	class TreeCtrlEx : public CTreeCtrl
	{
		// Attributes
	protected:

		// Operation
	public:
		TreeCtrlEx() {}
		~TreeCtrlEx() {}
		CImageList* setImageList(CImageList* pImageList, int nImageListType = TVSIL_NORMAL)	{
			return CTreeCtrl::SetImageList(pImageList, nImageListType);
		}

		TreeCursor getNextItem(HTREEITEM hItem, UINT nCode)	{
			return TreeCursor(CTreeCtrl::GetNextItem(hItem, nCode), this);
		}
		TreeCursor getChildItem(HTREEITEM hItem) {
			return TreeCursor(CTreeCtrl::GetChildItem(hItem), this);
		}
		TreeCursor getNextSiblingItem(HTREEITEM hItem) {
			return TreeCursor(CTreeCtrl::GetNextSiblingItem(hItem), this);
		}
		TreeCursor getPrevSiblingItem(HTREEITEM hItem)	{
			return TreeCursor(CTreeCtrl::GetPrevSiblingItem(hItem), this);
		}
		TreeCursor getParentItem(HTREEITEM hItem) {
			return TreeCursor(CTreeCtrl::GetParentItem(hItem), this);
		}
		TreeCursor getFirstVisibleItem() {
			return TreeCursor(CTreeCtrl::GetFirstVisibleItem(), this);
		}
		TreeCursor getNextVisibleItem(HTREEITEM hItem)	{
			return TreeCursor(CTreeCtrl::GetNextVisibleItem(hItem), this);
		}
		TreeCursor getPrevVisibleItem(HTREEITEM hItem)	{
			return TreeCursor(CTreeCtrl::GetPrevVisibleItem(hItem), this);
		}
		TreeCursor getSelectedItem() {
			return TreeCursor(CTreeCtrl::GetSelectedItem(), this);
		}
		TreeCursor getDropHilightItem()	{
			return TreeCursor(CTreeCtrl::GetDropHilightItem(), this);
		}
		TreeCursor getRootItem() {
			return TreeCursor(CTreeCtrl::GetRootItem(), this);
		}
		TreeCursor insertItem(LPTV_INSERTSTRUCT lpInsertStruct)	{
			return TreeCursor(CTreeCtrl::InsertItem(lpInsertStruct), this);
		}
		TreeCursor insertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
			int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
			HTREEITEM hParent, HTREEITEM hInsertAfter) {
			return TreeCursor(CTreeCtrl::InsertItem(nMask, lpszItem, nImage,
				nSelectedImage, nState, nStateMask, lParam, hParent, hInsertAfter), this);
		}
		TreeCursor insertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST) {
			return TreeCursor(CTreeCtrl::InsertItem(lpszItem, hParent, hInsertAfter), this);
		}
		TreeCursor insertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,	HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST) {
			return TreeCursor(CTreeCtrl::InsertItem(lpszItem, nImage, nSelectedImage,
				hParent, hInsertAfter), this);
		}

		TreeCursor select(HTREEITEM hItem, UINT nCode) {
			CTreeCtrl::Select(hItem, nCode); 
			return TreeCursor(CTreeCtrl::GetSelectedItem(), this);
		}
		TreeCursor selectItem(HTREEITEM hItem) {
			CTreeCtrl::SelectItem(hItem); 
			return TreeCursor(CTreeCtrl::GetSelectedItem(), this);
		}

		TreeCursor selectDropTarget(HTREEITEM hItem) {
			CTreeCtrl::SelectDropTarget(hItem); 
			return TreeCursor(CTreeCtrl::GetDropHilightItem(), this);
		}

		TreeCursor selectSetFirstVisible(HTREEITEM hItem) {
			CTreeCtrl::SelectSetFirstVisible(hItem); 
			return TreeCursor(CTreeCtrl::GetFirstVisibleItem(), this);
		}

		TreeCursor hitTest(CPoint pt, UINT* pFlags = NULL) {
			return TreeCursor(CTreeCtrl::HitTest(pt, pFlags), this);
		}

		TreeCursor hitTest(TV_HITTESTINFO* pHitTestInfo) {
			return TreeCursor(CTreeCtrl::HitTest(pHitTestInfo), this);
		}

		TreeCursor findItem(DWORD_PTR dwData);

		TreeCursor copyItem(HTREEITEM hSource, HTREEITEM hNewParent, HTREEITEM hInsertAfter);
	};


	inline bool TreeCursor::isValid() const
	{
		return (m_pTree != NULL) && ::IsWindow(m_pTree->m_hWnd);
	}
	inline bool TreeCursor::getRect(LPRECT lpRect, bool bTextOnly)
	{
		return m_pTree->GetItemRect(m_hTreeItem, lpRect, bTextOnly);
	}
	inline TreeCursor TreeCursor::getNext(UINT nCode)
	{
		return m_pTree->getNextItem(m_hTreeItem, nCode);
	}
	inline TreeCursor TreeCursor::getChild()
	{
		return m_pTree->getChildItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getNextSibling()
	{
		return m_pTree->getNextSiblingItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getPrevSibling()
	{
		return m_pTree->getPrevSiblingItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getParent()
	{
		return m_pTree->getParentItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getFirstVisible()
	{
		return m_pTree->getFirstVisibleItem();
	}
	inline TreeCursor TreeCursor::getNextVisible()
	{
		return m_pTree->getNextVisibleItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getPrevVisible()
	{
		return m_pTree->getPrevVisibleItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::getSelected()
	{
		return m_pTree->getSelectedItem();
	}
	inline TreeCursor TreeCursor::getDropHilight()
	{
		return m_pTree->getDropHilightItem();
	}
	inline TreeCursor TreeCursor::getRoot()
	{
		return m_pTree->getRootItem();
	}
	inline std::string TreeCursor::getText()
	{
		return std::string ((const char *)m_pTree->GetItemText(m_hTreeItem));
	}
	inline bool TreeCursor::getImage(int& nImage, int& nSelectedImage)
	{
		return m_pTree->GetItemImage(m_hTreeItem, nImage, nSelectedImage);
	}
	inline UINT TreeCursor::getState(UINT nStateMask)
	{
		return m_pTree->GetItemState(m_hTreeItem, nStateMask);
	}
	inline DWORD_PTR TreeCursor::getData()
	{
		return m_pTree->GetItemData(m_hTreeItem);
	}
	inline BOOL TreeCursor::getCheck()
	{
		return m_pTree->GetCheck(m_hTreeItem);
	}
	inline BOOL TreeCursor::setCheck(BOOL bCheck)
	{
		return m_pTree->SetCheck(m_hTreeItem, bCheck);
	}
	inline bool TreeCursor::setText(LPCTSTR lpszItem)
	{
		return m_pTree->SetItemText(m_hTreeItem, lpszItem);
	}
	inline bool TreeCursor::setImage(int nImage, int nSelectedImage)
	{
		return m_pTree->SetItemImage(m_hTreeItem, nImage, nSelectedImage);
	}
	inline bool TreeCursor::setState(UINT nState, UINT nStateMask)
	{
		return m_pTree->SetItemState(m_hTreeItem, nState, nStateMask);
	}
	inline bool TreeCursor::setData(DWORD_PTR dwData)
	{
		return m_pTree->SetItemData(m_hTreeItem, dwData);
	}
	inline bool TreeCursor::hasChildren()
	{
		return m_pTree->ItemHasChildren(m_hTreeItem);
	}
	// Operations
	inline bool TreeCursor::deleteItem()
	{
		bool bOk = m_pTree->DeleteItem(m_hTreeItem);
		reset();
		return bOk;
	}
	inline bool TreeCursor::expand(UINT nCode)
	{
		return m_pTree->Expand(m_hTreeItem, nCode);
	}
	inline TreeCursor TreeCursor::select(UINT nCode)
	{
		return m_pTree->select(m_hTreeItem, nCode);
	}
	inline TreeCursor TreeCursor::select()
	{
		return m_pTree->selectItem(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::selectDropTarget()
	{
		return m_pTree->selectDropTarget(m_hTreeItem);
	}
	inline TreeCursor TreeCursor::selectSetFirstVisible()
	{
		return m_pTree->selectSetFirstVisible(m_hTreeItem);
	}
	inline CEdit* TreeCursor::editLabel()
	{
		return m_pTree->EditLabel(m_hTreeItem);
	}
	inline CImageList* TreeCursor::createDragImage()
	{
		return m_pTree->CreateDragImage(m_hTreeItem);
	}
	inline bool TreeCursor::sortChildren()
	{
		return m_pTree->SortChildren(m_hTreeItem);
	}
	inline bool TreeCursor::ensureVisible()
	{
		return m_pTree->EnsureVisible(m_hTreeItem);
	}



}