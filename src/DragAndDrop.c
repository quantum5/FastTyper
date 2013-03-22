/*
  Copyright (C) 2008 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*
 * File: DragAndDrop.c
 * A layer for implemention Drag and Drop in Windows for C-applications. This enables
 * C applications to easily implement drag and drop, without having to deal with any OLE or
 * something like that. Some shortcuts has been tahken, such as only one TYMED is supported (HGLOBAL)
 * and only the DVASPECT_CONTENT aspect.
 * Both drag-drop target and source is supported.
 */
#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <tchar.h>
#include "DragAndDrop.h"
#include "resource.h"

/* Macros. */
#define MYDD_HEAP (g_hHeap == NULL ? (g_hHeap = GetProcessHeap()) : g_hHeap)

typedef struct tagMYIDROPTARGET {
	IDropTarget idt;
	LONG lRefCount;
	ULONG lNumFormats;
	UINT *pFormat;
	HWND hWnd;
	BOOL bAllowDrop;
	DWORD dwKeyState;
	IDataObject *pDataObject;
	UINT nMsg;
	void *pUserData;
	MYDDCALLBACK pDropProc;
} MYIDROPTARGET, *PMYIDROPTARGET;

typedef struct MYIDROPTARGET_VTBL {
	BEGIN_INTERFACE
	HRESULT (STDMETHODCALLTYPE *QueryInterface)(PMYIDROPTARGET pThis, REFIID riid,
	  void  **ppvObject);
	ULONG (STDMETHODCALLTYPE *AddRef)(PMYIDROPTARGET pThis);
	ULONG (STDMETHODCALLTYPE *Release)(PMYIDROPTARGET pThis);
	HRESULT (STDMETHODCALLTYPE *DragEnter)(PMYIDROPTARGET pThis, IDataObject *pDataObject,
	  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT (STDMETHODCALLTYPE *DragOver)(PMYIDROPTARGET pThis, DWORD dwKeyState, POINTL pt,
	  DWORD *pdwEffect);
	HRESULT (STDMETHODCALLTYPE *DragLeave)(PMYIDROPTARGET pThis);
	HRESULT (STDMETHODCALLTYPE *Drop)(PMYIDROPTARGET pThis, IDataObject *pDataObject,
	  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
	END_INTERFACE
} MYIDROPTARGET_VTBL, *PMYIDROPTARGET_VTBL;

/* Static globals. */
static HANDLE g_hHeap = NULL;

/* MYIDROPTARGET functions. */
static BOOL MYIDROPTARGET_QueryDataObject(PMYIDROPTARGET pDropTarget, IDataObject *pDataObject);
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_QueryInterface(PMYIDROPTARGET pThis, REFIID riid,
  LPVOID *ppvObject);
static ULONG STDMETHODCALLTYPE MYIDROPTARGET_AddRef(PMYIDROPTARGET pThis);
static ULONG STDMETHODCALLTYPE MYIDROPTARGET_Release(PMYIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragEnter(PMYIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragOver(PMYIDROPTARGET pThis, DWORD dwKeyState, POINTL pt,
  DWORD *pdwEffect);
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragLeave(PMYIDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_Drop(PMYIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect);


/* Public functions. */
/*
 * Function: MyDragDropInit()
 * Initialize the DragDrop library and OLE itself.
 * Arguments:
 * HANDLE hHeap - Handle to the heap memory handle to use for MyDragDropInit().
 */
void MyDragDropInit(HANDLE hHeap)
{
	/* Only initialize this if not already set or used before. */
	if (g_hHeap == NULL && hHeap == NULL)
		g_hHeap = GetProcessHeap();
	else if (g_hHeap == NULL)
		g_hHeap = hHeap;

	/* Initialize OLE, to be sure. */
	OleInitialize(NULL);
} /* End of MyDragDropInit(). */

/*
 * Function: MyRegisterDragDrop()
 * Simplified registreation for a drop target.
 * Arguments:
 * HWND hWnd - The window where drops occur and messages are sent etc.
 * UINT *pFormat - An array of UINTs supported.
 * ULONG lFmt - The count of UINTS in the pFormat array.
 * HWND hWnd - The window that is the parent of this target.
 * UINT nMsg - The message to send to hWnd when data is dropped. Set to WM_NULL if we are not
 *   to send any messages.
 * MYDDCALLBACK pDropProc - The callback to call when stuff is dropped. Set to NULL
 *   to send nMsg instead of doing the callback.
 * void *pUserData - A user defined data pointer, that is passed back in the
 *   callback or message, whichever method is used.
 * Returns:
 * PMYDROPTARGET - The target object, NULL if there was an error.
 */
PMYDROPTARGET MyRegisterDragDrop(HWND hWnd, UINT *pFormat, ULONG lFmt, UINT nMsg,
  MYDDCALLBACK pDropProc, void *pUserData)
{
	IDropTarget *pTarget;

	/* First, create the target. */
	if ((pTarget = CreateDropTarget(pFormat, lFmt, hWnd, nMsg, pDropProc, pUserData)) == NULL)
		return NULL;

	/* Now, register for drop. If this fails, free my target the old-faschioned way, as noone knows about it anyway. */
	if (RegisterDragDrop(hWnd, pTarget) != S_OK) {
		HeapFree(MYDD_HEAP, 0, pTarget);
		return NULL;
	}

	return (PMYDROPTARGET) pTarget;
} /* End of MyRegisterDragDrop(). */


/*
 * Function: MyRevokeDragDrop()
 * Convenience function to revoke drag drop and also free the associated IDropTarget().
 * Arguments:
 * PMYDROPTARGET - The target to free and revoke drop from.
 * Returns:
 * PMYDROPTARGET - NULL
 */
PMYDROPTARGET MyRevokeDragDrop(PMYDROPTARGET pTarget)
{
	if (pTarget == NULL)
		return NULL;

	/* If there is a HWND, then revoke it as a drop object. */
	if (((PMYIDROPTARGET) pTarget)->hWnd != NULL)
		/* Now, this is a little precaution to know that this is an OK PMIIDROPTARGET object. */
		if(GetWindowLong(((PMYIDROPTARGET) pTarget)->hWnd, GWL_WNDPROC) != 0)
			RevokeDragDrop(((PMYIDROPTARGET) pTarget)->hWnd);

	/* Now, release the target. */
	((IDropTarget *) pTarget)->lpVtbl->Release((IDropTarget *) pTarget);

	return NULL;
} /* End of MyRevokeDragDrop(). */


/*
 * Function: CreateDropTarget()
 * Create a IDropTarget, that has some added private members, used only here. When
 * data is dropped, either pDropProc is called, or if this is NULL, then nMsg is
 * sent to hWnd instead. When a message is sent, a pointer to a MYDROPDATA
 * is passed a wParam, and the user defined data pointer, pUserData, is
 * passed as lParam.
 * Arguments:
 * UINT *pFormat - An array of UINTs supported.
 * ULONG lFmt - The count of UINTS in the pFormat array.
 * HWND hWnd - The window that is the parent of this target.
 * UINT nMsg - The message to send to hWnd when data is dropped. Set to WM_NULL if we are not
 *   to send any messages.
 * DWORD (*pDropProc)() - The callback to call when stuff is dropped. Set to NULL
 *   to send nMsg instead of doing the callback.
 * void *pUserData - A user defined data pointer, that is passed back in the
 *   callback or message, whichever method is used.
 * Returns:
 * IDropTarget * - A pointert to a created IDropTarget, NULL if there is an error.
 */
IDropTarget *CreateDropTarget(UINT *pFormat, ULONG lFmt, HWND hWnd, UINT nMsg,
  DWORD (*pDropProc)(UINT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData),
  void *pUserData)
{
	PMYIDROPTARGET pRet;
	static MYIDROPTARGET_VTBL idt_vtbl = {
		MYIDROPTARGET_QueryInterface,
		MYIDROPTARGET_AddRef,
		MYIDROPTARGET_Release,
		MYIDROPTARGET_DragEnter,
		MYIDROPTARGET_DragOver,
		MYIDROPTARGET_DragLeave,
		MYIDROPTARGET_Drop
	};

	/* Allocate the nasty little thing, and supply space for UINT array in the process. */
	if ((pRet = HeapAlloc(MYDD_HEAP, 0, sizeof(MYIDROPTARGET) + lFmt * sizeof(UINT))) == NULL)
		return NULL;
	pRet->pFormat = (UINT *) (((char *) pRet) + sizeof(MYIDROPTARGET));

	/* Set up the struct members. */
	pRet->idt.lpVtbl = &idt_vtbl;
	pRet->lRefCount = 1;
	pRet->hWnd = hWnd;
	pRet->nMsg = nMsg;
	pRet->bAllowDrop = FALSE;
	pRet->dwKeyState = 0;
	pRet->lNumFormats = lFmt;
	pRet->pDropProc = pDropProc;
	pRet->pUserData = pUserData;

	/* Set the format members. */
	for (lFmt = 0; lFmt < pRet->lNumFormats; lFmt++)
		pRet->pFormat[lFmt] = pFormat[lFmt];

	return (IDropTarget *) pRet;
} /* End of CreateDropTarget(). */


/*
 * MYIDROPTARGET functions.
 * These functions are just implementations of the standard IDropTarget methods.
 */
static BOOL MYIDROPTARGET_QueryDataObject(PMYIDROPTARGET pDropTarget, IDataObject *pDataObject)
{
	ULONG lFmt;
	FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	
	for (lFmt = 0; lFmt < pDropTarget->lNumFormats; lFmt++) {
		fmtetc.cfFormat = (ULONG) pDropTarget->pFormat[lFmt];
		if (pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
			return TRUE;
	}
	return FALSE;
} /* End of MYIDROPTARGET_QueryDataObject(). */


static DWORD MYIDROPTARGET_DropEffect(DWORD dwKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	if (dwKeyState & MK_CONTROL)
		dwEffect = dwAllowed & DROPEFFECT_COPY;
	else if (dwKeyState & MK_SHIFT)
		dwEffect = dwAllowed & DROPEFFECT_MOVE;

	if (dwEffect == 0) {
		if (dwAllowed & DROPEFFECT_COPY)
			dwEffect = DROPEFFECT_COPY;
		if (dwAllowed & DROPEFFECT_MOVE)
			dwEffect = DROPEFFECT_MOVE;
	}
	return dwEffect;
}

/* IDropTarget private methods. */
static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_QueryInterface(PMYIDROPTARGET pThis, REFIID riid,
  LPVOID *ppvObject)
{
	*ppvObject = NULL;

	if (IsEqualGUID(riid, &IID_IUnknown)) {
		MYIDROPTARGET_AddRef(pThis);
		*ppvObject = pThis;
		return S_OK;
	} else if (IsEqualGUID(riid, &IID_IDropTarget)) {
		MYIDROPTARGET_AddRef(pThis);
		*ppvObject = pThis;
		return S_OK;
	}

	return E_NOINTERFACE;
} /* End of MYIDROPTARGET_QueryInterface(). */


static ULONG STDMETHODCALLTYPE MYIDROPTARGET_AddRef(PMYIDROPTARGET pThis)
{
	return InterlockedIncrement(&pThis->lRefCount);
} /* End of MYIDROPTARGET_AddRef(). */


static ULONG STDMETHODCALLTYPE MYIDROPTARGET_Release(PMYIDROPTARGET pThis)
{
	LONG nCount;
	if ((nCount = InterlockedDecrement(&pThis->lRefCount)) == 0) {
		HeapFree(MYDD_HEAP, 0, pThis);
		return 0;
	}
	return nCount;
} /* End of MYIDROPTARGET_Release(). */


static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragEnter(PMYIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (pThis->bAllowDrop = MYIDROPTARGET_QueryDataObject(pThis, pDataObject)) {
		*pdwEffect = MYIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
		SetFocus(pThis->hWnd);
	} else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
} /* End of MYIDROPTARGET_DragEnter(). */


static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragOver(PMYIDROPTARGET pThis, DWORD dwKeyState, POINTL pt,
  DWORD *pdwEffect)
{
	if (pThis->bAllowDrop) {
		pThis->dwKeyState = dwKeyState;

		*pdwEffect = MYIDROPTARGET_DropEffect(dwKeyState, pt, *pdwEffect);
	} else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
} /* End of MYIDROPTARGET_DragEffect(). */


static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_DragLeave(PMYIDROPTARGET pThis)
{
	return S_OK;
} /* End of MYIDROPTARGET_DragLeave(). */


static HRESULT STDMETHODCALLTYPE MYIDROPTARGET_Drop(PMYIDROPTARGET pThis, IDataObject *pDataObject,
  DWORD dwKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC fmtetc = {CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium;
	ULONG lFmt;
	MYDROPDATA DropData;

	if (pThis->bAllowDrop) {
		/* Find the first matching UINT that I can handle. */
		for (lFmt = 0; lFmt < pThis->lNumFormats; lFmt++) {
			fmtetc.cfFormat = (ULONG) pThis->pFormat[lFmt];
			if (pDataObject->lpVtbl->QueryGetData(pDataObject, &fmtetc) == S_OK)
				break;
		}
		/* If we found a matching format, then handle it now. */
		if (lFmt < pThis->lNumFormats) {
			/* Get the data being dragged. */
			pDataObject->lpVtbl->GetData(pDataObject, &fmtetc, &medium);
			*pdwEffect = DROPEFFECT_NONE;
			/* If a callback procedure is defined, then use that. */
			if (pThis->pDropProc != NULL)
				*pdwEffect = (*pThis->pDropProc)(pThis->pFormat[lFmt], medium.hGlobal, pThis->hWnd, pThis->dwKeyState,
				  pt, pThis->pUserData);
			/* Else, if a message is valid, then send that. */
			else if (pThis->nMsg != WM_NULL) {
				/* Fill the struct with the relevant data. */
				DropData.cf = pThis->pFormat[lFmt];
				DropData.dwKeyState = pThis->dwKeyState;
				DropData.hData = medium.hGlobal;
				DropData.pt = pt;
				/* And send the message. */
				*pdwEffect = (DWORD) SendMessage(pThis->hWnd, pThis->nMsg, (WPARAM) &DropData,
				  (LPARAM) pThis->pUserData);
			}
			/* Release the medium, if it was used. */
			if (*pdwEffect != DROPEFFECT_NONE)
				ReleaseStgMedium(&medium);
			}
		}
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
} /* End of MYIDROPTARGET_Drop(). */
