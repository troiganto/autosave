// IconBitmap.h : Converts an icon into a bitmap and manages its handle.
// Functions may throw AutoSaveException on failure.

#pragma once

#include "stdafx.h"
#include "AutoSaveException.h"
#include <wingdi.h>
#include <ShObjIdl.h>

class IconBitmap
{
public:
	IconBitmap(LPCTSTR resourceName, int size);
	IconBitmap(HIMAGELIST himl, int idx);
	~IconBitmap() { DeleteBitmap(m_hbmp); }

	const HBITMAP& hBitmap = m_hbmp;
	void fillSHDragImage(SHDRAGIMAGE* shdi) const;

private:
	static int getImageListIconSize(HIMAGELIST himl);
	void drawIconFromResource(HDC hdc, LPCTSTR resourceName) const;

	int m_size;
	HBITMAP m_hbmp;
};


