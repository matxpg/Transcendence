//	CDockScreenList.cpp
//
//	CDockScreenList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int PICKER_ROW_HEIGHT	=	96;
const int PICKER_ROW_COUNT =	4;

bool CDockScreenList::EvalBool (const CString &sCode, bool *retbResult, CString *retsError)

//	EvalBool
//
//	Evaluates the given string

	{
	CCodeChainCtx Ctx;
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	char *pPos = sCode.GetPointer();
	ICCItem *pExp = Ctx.Link(sCode, 1, NULL);

	ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
	Ctx.Discard(pExp);

	if (pResult->IsError())
		{
		*retsError = pResult->GetStringValue();
		Ctx.Discard(pResult);
		return false;
		}

	*retbResult = !pResult->IsNil();
	Ctx.Discard(pResult);

	return true;
	}

bool CDockScreenList::EvalString (const CString &sString, bool bPlain, ECodeChainEvents iEvent, CString *retsResult)

//	EvalString
//
//	Evaluates the given string.

	{
	CCodeChainCtx Ctx;
	Ctx.SetEvent(iEvent);
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	return Ctx.RunEvalString(sString, bPlain, retsResult);
	}

void CDockScreenList::OnDeleteCurrentItem (int iCount)

//	OnDeleteCurrentItem
//
//	Delete the current item.

	{
	m_pItemListControl->DeleteAtCursor(iCount);
	ShowItem();
	}

const CItem &CDockScreenList::OnGetCurrentItem (void) const

//	OnGetCurrentItem
//
//	Returns the currently selected item

	{
	return m_pItemListControl->GetItemAtCursor();
	}

ICCItem *CDockScreenList::OnGetCurrentListEntry (void) const

//	OnGetCurrentListEntry
//
//	Returns the current list entry.

	{
	return m_pItemListControl->GetEntryAtCursor();
	}

IDockScreenDisplay::EResults CDockScreenList::OnHandleAction (DWORD dwTag, DWORD dwData)

//	OnHandleAction
//
//	Handle an action

	{
	if (dwTag == m_dwID)
		{
		switch (dwData)
			{
			//	Should never happen.

			case ITEM_LIST_AREA_PAGE_DOWN_ACTION:
			case ITEM_LIST_AREA_PAGE_UP_ACTION:
				return resultNone;

			default:
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_pItemListControl->SetCursor(dwData);
				return resultShowPane;
			}
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenList::OnHandleKeyDown (int iVirtKey)

//	OnHandleKeyDown
//
//	Handles key down. If we don't handle the given key, we return resultNone.

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_LEFT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectPrevItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_DOWN:
		case VK_RIGHT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectNextItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_PRIOR:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectPrevItem();
				SelectPrevItem();
				SelectPrevItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				m_pItemListControl->Invalidate();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_NEXT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectNextItem();
				SelectNextItem();
				SelectNextItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				m_pItemListControl->Invalidate();
				return resultShowPane;
				}
			else
				return resultHandled;

		default:
			return resultNone;
		}
	}

ALERROR CDockScreenList::OnInit (SInitCtx &Ctx, CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	m_pDockScreen = Ctx.pDockScreen;
	m_pPlayer = Ctx.pPlayer;
	m_pLocation = Ctx.pLocation;
	m_pData = Ctx.pData;
	m_dwID = Ctx.dwFirstID;

	//	Calculate some basic metrics

	RECT rcList = Ctx.rcRect;
	rcList.left += 12;
	rcList.right -= 44;
	rcList.top += 12;
	rcList.bottom = rcList.top + (PICKER_ROW_COUNT * PICKER_ROW_HEIGHT);

	//	Create the picker control

	m_pItemListControl = new CGItemListArea;
	if (m_pItemListControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	m_pItemListControl->SetUIRes(&g_pTrans->GetUIRes());
	m_pItemListControl->SetFontTable(Ctx.pFontTable);

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pItemListControl, rcList, m_dwID);

	//	Let our subclass initialize

	if (error = OnInitList(Ctx, retsError))
		return error;

	return NOERROR;
	}

bool CDockScreenList::OnIsCurrentItemValid (void) const

//	OnIsCurrentItemValid
//
//	Returns TRUE if current item is valid

	{
	return m_pItemListControl->IsCursorValid();
	}

IDockScreenDisplay::EResults CDockScreenList::OnResetList (CSpaceObject *pLocation)

//	OnResetList
//
//	Reset the list

	{
	if (m_pItemListControl->GetSource() == pLocation)
		{
		m_pItemListControl->ResetCursor();
		m_pItemListControl->MoveCursorForward();
		ShowItem();
		return resultShowPane;
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenList::OnSetListCursor (int iCursor)

//	OnSetListCursor
//
//	Sets the list cursor

	{
	m_pItemListControl->SetCursor(iCursor);
	ShowItem();
	return resultShowPane;
	}

IDockScreenDisplay::EResults CDockScreenList::OnSetListFilter (const CItemCriteria &Filter)

//	OnSetListFilter
//
//	Sets the list filter

	{
	m_pItemListControl->SetFilter(Filter);
	ShowItem();
	return resultShowPane;
	}

bool CDockScreenList::OnSelectNextItem (void)

//	OnSelectNextItem
//
//	Selects the next item

	{
	return m_pItemListControl->MoveCursorForward();
	}

bool CDockScreenList::OnSelectPrevItem (void)

//	OnSelectPrevItem
//
//	Selects the previous item

	{
	return m_pItemListControl->MoveCursorBack();
	}

void CDockScreenList::OnShowItem (void)

//	OnShowItem
//
//	Show the current item

	{
	m_pItemListControl->SyncCursor();

	//	If we've got an installed armor segment selected, then highlight
	//	it on the armor display

	if (m_pItemListControl->IsCursorValid())
		{
		const CItem &Item = m_pItemListControl->GetItemAtCursor();
		if (Item.IsInstalled() && Item.GetType()->IsArmor())
			{
			int iSeg = Item.GetInstalled();
			g_pTrans->SelectArmor(iSeg);
			}
		else
			g_pTrans->SelectArmor(-1);
		}
	else
		g_pTrans->SelectArmor(-1);
	}

void CDockScreenList::OnShowPane (bool bNoListNavigation)

//	OnShowPane
//
//	Handle case where the pane is shown

	{
	//	Update armor items to match the current state (the damaged flag)

	CSpaceObject *pLocation = m_pItemListControl->GetSource();
	if (pLocation)
		pLocation->UpdateArmorItems();

	//	Update the item list

	ShowItem();

	//	If this is set, don't allow the list selection to change

	m_bNoListNavigation = bNoListNavigation;
	}
