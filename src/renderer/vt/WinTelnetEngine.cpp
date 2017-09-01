/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "precomp.h"
#include "WinTelnetEngine.hpp"
#include "..\..\inc\Viewport.hpp"
#pragma hdrstop
using namespace Microsoft::Console::Render;

WinTelnetEngine::WinTelnetEngine(HANDLE hPipe)
    : VtEngine(hPipe)
{
}


// Routine Description:
// - This method will set the GDI brushes in the drawing context (and update the hung-window background color)
// Arguments:
// - wTextAttributes - A console attributes bit field specifying the brush colors we should use.
// Return Value:
// - S_OK if set successfully or relevant GDI error via HRESULT.
HRESULT WinTelnetEngine::UpdateDrawingBrushes(_In_ COLORREF const colorForeground, _In_ COLORREF const colorBackground, _In_ WORD const legacyColorAttribute, _In_ bool const fIncludeBackgrounds)
{
    UNREFERENCED_PARAMETER(colorForeground);
    UNREFERENCED_PARAMETER(colorBackground);
    UNREFERENCED_PARAMETER(legacyColorAttribute);
    UNREFERENCED_PARAMETER(fIncludeBackgrounds);

    try
    {
        if (colorForeground != _LastFG)
        {
            char* fmt = IsFlagSet(legacyColorAttribute, FOREGROUND_INTENSITY)? "\x1b[1;%dm" : "\x1b[%dm";
            int fg = 30
                     + IsFlagSet(legacyColorAttribute,FOREGROUND_RED)? 1 : 0
                     + IsFlagSet(legacyColorAttribute,FOREGROUND_GREEN)? 2 : 0
                     + IsFlagSet(legacyColorAttribute,FOREGROUND_BLUE)? 4 : 0
                     ;
            int cchNeeded = _scprintf(fmt, fg);
            wistd::unique_ptr<char[]> psz = wil::make_unique_nothrow<char[]>(cchNeeded + 1);
            RETURN_IF_NULL_ALLOC(psz);

            int cchWritten = _snprintf_s(psz.get(), cchNeeded + 1, cchNeeded, fmt, fg);
            _Write(psz.get(), cchWritten);

            _LastFG = colorForeground;
            
        }
        if (colorBackground != _LastBG) 
        {
            char* fmt = IsFlagSet(legacyColorAttribute, BACKGROUND_INTENSITY)? "\x1b[1;%dm" : "\x1b[%dm";

            int bg = 40
                     + IsFlagSet(legacyColorAttribute,BACKGROUND_RED)? 1 : 0
                     + IsFlagSet(legacyColorAttribute,BACKGROUND_GREEN)? 2 : 0
                     + IsFlagSet(legacyColorAttribute,BACKGROUND_BLUE)? 4 : 0
                     ;

            int cchNeeded = _scprintf(fmt, bg);
            wistd::unique_ptr<char[]> psz = wil::make_unique_nothrow<char[]>(cchNeeded + 1);
            RETURN_IF_NULL_ALLOC(psz);

            int cchWritten = _snprintf_s(psz.get(), cchNeeded + 1, cchNeeded, fmt, bg);
            _Write(psz.get(), cchWritten);
            _LastBG = colorBackground;
        }
    }
    CATCH_RETURN();
    return S_OK;
}

HRESULT WinTelnetEngine::_MoveCursor(COORD const coord)
{
    // don't try and be clever about moving the cursor.
    // Always just use the full sequence
    if (coord.X != _lastText.X || coord.Y != _lastText.Y)
    {
        try
        { 
            PCSTR pszCursorFormat = "\x1b[%d;%dH";
            COORD coordVt = coord;
            coordVt.X++;
            coordVt.Y++;

            int cchNeeded = _scprintf(pszCursorFormat, coordVt.Y, coordVt.X);
            wistd::unique_ptr<char[]> psz = wil::make_unique_nothrow<char[]>(cchNeeded + 1);
            RETURN_IF_NULL_ALLOC(psz);

            int cchWritten = _snprintf_s(psz.get(), cchNeeded + 1, cchNeeded, pszCursorFormat, coordVt.Y, coordVt.X);
            _Write(psz.get(), cchWritten);

            _lastText = coord;
        }
        CATCH_RETURN();
    }
    return S_OK;
}

HRESULT WinTelnetEngine::ScrollFrame()
{
    // win-telnet doesn't know anything about scroll vt sequences
    // every frame, we're repainitng everything, always.
    return S_OK;
}

// Routine Description:
// - Notifies us that the console is attempting to scroll the existing screen area
// Arguments:
// - pcoordDelta - Pointer to character dimension (COORD) of the distance the console would like us to move while scrolling.
// Return Value:
// - HRESULT S_OK, GDI-based error code, or safemath error
HRESULT WinTelnetEngine::InvalidateScroll(_In_ const COORD* const pcoordDelta)
{
    UNREFERENCED_PARAMETER(pcoordDelta);
    // win-telnet doesn't know anything about scrolling. 
    //  Every invalidate action should just repaint everything.
    return InvalidateAll();
}
