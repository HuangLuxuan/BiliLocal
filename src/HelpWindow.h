#ifndef HELPWINDOW_H
#define HELPWINDOW_H
#include <windows.h>
#include <CommCtrl.h>
#include <richedit.h>
#include <deque>
#include <iostream>
#include "HelpWindowRes.h"
class HelpWindow{
public:
    HelpWindow(HICON hIcon=0);
    static HelpWindow *getHelpWindow(HICON hIcon=0);
private:
    HWND hwnd;
    HINSTANCE hInstance;
    static ATOM classAtom;
    //static HelpWindow *instance;
    static std::atomic<HelpWindow*> instance;
    static LRESULT CALLBACK WindowProc_S(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT onCreatWindow();
    LRESULT onResize(int width,int height);
    LRESULT onEnLink(ENLINK* enlink);
    LRESULT onClickLink(wchar_t *link);
    int compareUrl(const wchar_t*u1,const wchar_t*u2);
    HWND themeList,contentRichText,forwardButton,backwardButton;
    static HFONT wingdings;
    typedef struct RESFILE{
        const wchar_t *path;
        LPCWSTR id;
    } ResFile;
    typedef struct HELPFILE{
        ResFile file;
        const wchar_t *title;
    } HelpFile;
    enum LocationType{
        NONE,
        T_Index,
        T_ResFile,
        T_HelpFile
    };
    static HelpFile helpFiles[],extraHelpFiles[];
    static ResFile extraResFiles[];
    void initThemeList();
    void loadContent(int index);
    void loadContent(HelpFile file);
    void loadContent(ResFile file);
    void tryJumpAnchor(wchar_t *link);
    typedef struct LOCATION{
        union{
            int index;
            ResFile resFile;
            HelpFile helpFile;
        };
        LocationType type;
        CHARRANGE selection;
        POINT scrollPosition;
    }Location;
    Location currentLocation;
    unsigned locationQueueMaxDepth=128;
    std::deque<Location> locationQueue;
    std::deque<Location> locationQueue2;
    void loadCurrentLocation();
    void forward();
    void backward();
    void pushLocation();
};

#endif // HELPWINDOW_H
