#include "stdafx.h"
#include "IconBitmap.h"


IconBitmap::IconBitmap(LPCTSTR iconName, int size)
	: m_size(size)
{
	m_hbmp = throwIfZero<AutoSaveException>(
		CreateBitmap(m_size, m_size, 1, 32, NULL));

	HDC hdc = throwIfZero<AutoSaveException>(CreateCompatibleDC(NULL));
	HBITMAP hPrevBitmap = SelectBitmap(hdc, m_hbmp);
	drawIconFromResource(hdc, iconName);
	SelectBitmap(hdc, hPrevBitmap);
	DeleteDC(hdc);
}



IconBitmap::IconBitmap(HIMAGELIST himl, int idx)
	: m_size(getImageListIconSize(himl))
{
	m_hbmp = throwIfZero<AutoSaveException>(
		CreateBitmap(m_size, m_size, 1, 32, NULL));

	HDC hdc = throwIfZero<AutoSaveException>(CreateCompatibleDC(NULL));
	HBITMAP hPrevBitmap = SelectBitmap(hdc, m_hbmp);
	ImageList_DrawEx(himl, idx, hdc, 2, 2, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
	SelectBitmap(hdc, hPrevBitmap);
	DeleteDC(hdc);
}



void IconBitmap::fillSHDragImage(SHDRAGIMAGE* shdi) const
{
	*shdi = {
			{ m_size, m_size },			// sizeDragImage
			{ m_size / 2, m_size / 2 },	// ptOffset
			hBitmap,					// hbmpDragImage;
			CLR_NONE,					// crColorKey;
	};
}



int IconBitmap::getImageListIconSize(HIMAGELIST himl)
{
	int cx, cy;
	ImageList_GetIconSize(himl, &cx, &cy);
	return cx;
}



void IconBitmap::drawIconFromResource(HDC hdc, LPCTSTR resourceName) const
{
	RECT r = { 0, 0, m_size, m_size };
	FillRect(hdc, &r, GetSysColorBrush(COLOR_WINDOW));

	HICON hIcon;
	throwOnFailure<AutoSaveException>(
		LoadIconWithScaleDown(GetModuleHandle(NULL), resourceName, m_size, m_size, &hIcon));
	//hIcon = LoadIcon(GetModuleHandle(NULL), resourceName);
	DrawIconEx(hdc, 0, 0, hIcon, m_size, m_size, 0, NULL, DI_NORMAL);
	DestroyIcon(hIcon);
}

