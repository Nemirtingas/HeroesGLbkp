/*
	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"
#include "mmsystem.h"
#include "Hooks.h"
#include "GLib.h"
#include "Main.h"
#include "Config.h"
#include "Window.h"
#include "Resource.h"
#include "Mods.h"

BOOL __stdcall DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: {
		CHAR* line = GetCommandLine();
		do
		{
			line = StrChar(line, ' ');
			if (!line || !*(++line))
				break;

			if (StrStr(line, "-nogl") == line)
			{
				config.isDDraw = TRUE;
				break;
			}
		} while (TRUE);

		LoadKernel32();

		hDllModule = hModule;
		if (Hooks::Load())
		{
			if (!config.isDDraw)
			{
				Mods::Load();

				Window::SetCaptureKeys(TRUE);

				{
					WNDCLASS wc = {
						CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
						DefWindowProc,
						0, 0,
						hDllModule,
						NULL, (HCURSOR)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(132), IMAGE_CURSOR, 32, 32, LR_CREATEDIBSECTION),
						(HBRUSH)GetStockObject(BLACK_BRUSH), NULL,
						WC_DRAW
					};

					RegisterClass(&wc);
				}
			}

			if (CreateActCtxC)
			{
				CHAR path[MAX_PATH];
				GetModuleFileName(hModule, path, MAX_PATH);

				ACTCTX actCtx = {};
				actCtx.cbSize = sizeof(ACTCTX);
				actCtx.lpSource = path;
				actCtx.hModule = hModule;
				actCtx.lpResourceName = MAKEINTRESOURCE(IDR_MANIFEST);
				actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
				hActCtx = CreateActCtxC(&actCtx);
			}

			LoadShcore();
			if (SetProcessDpiAwarenessC)
				SetProcessDpiAwarenessC(PROCESS_PER_MONITOR_DPI_AWARE);

			timeBeginPeriod(1);
		}
		else
			hDllModule = NULL;

		break;
	}

	case DLL_PROCESS_DETACH:
		if (hDllModule)
		{
			timeEndPeriod(1);

			if (hActCtx && hActCtx != INVALID_HANDLE_VALUE)
				ReleaseActCtxC(hActCtx);

			if (!config.isDDraw)
			{
				UnregisterClass(WC_DRAW, hDllModule);
				ClipCursor(NULL);
				Window::SetCaptureKeys(FALSE);
			}
		}

		break;

	default:
		break;
	}
	return TRUE;
}