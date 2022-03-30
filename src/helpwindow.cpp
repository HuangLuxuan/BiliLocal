#include "HelpWindow.h"
ATOM HelpWindow::classAtom=0;
//HelpWindow *HelpWindow::instance=NULL;
std::atomic<HelpWindow*> HelpWindow::instance;
HFONT HelpWindow::wingdings=NULL;
HelpWindow::HelpWindow(HICON hIcon){
    hInstance=(HINSTANCE)GetWindowLongW(GetForegroundWindow(), GWL_HINSTANCE);
    const wchar_t CLASS_NAME[]=L"BiliLocalHelpWindow";
    if(!classAtom){
        WNDCLASSW wc={};
        wc.lpfnWndProc=HelpWindow::WindowProc_S;
        wc.hInstance=hInstance;
        wc.lpszClassName=CLASS_NAME;
        wc.hIcon=hIcon;
        classAtom= RegisterClassW(&wc);
    }
    assert(classAtom);
    if(!wingdings)
        wingdings=CreateFontW(30,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SYMBOL_CHARSET,OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH|FF_DECORATIVE,L"Wingdings");
    assert(wingdings);
    hwnd=CreateWindowExW(0,CLASS_NAME,L"BiliLocal帮助",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,480,360,GetForegroundWindow(),NULL,hInstance,this);
    assert(hwnd);
    ShowWindow(hwnd,SW_SHOWNORMAL);
    //instance=this;
    instance.store(this);
}
HelpWindow *HelpWindow::getHelpWindow(HICON hIcon){
    //HelpWindow* window= instance?instance:new HelpWindow(hIcon);
    HelpWindow* window=instance.load();
    if(!window)window=new HelpWindow(hIcon);
    assert(window->hwnd);
    SetFocus(window->hwnd);
    return window;
}
LRESULT HelpWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return onCreatWindow();
    case WM_DESTROY:
    {
        //if(instance==this)instance=NULL;
        HelpWindow *cmp=this;
        instance.compare_exchange_strong(cmp,NULL);
        delete this;
        return 0;
    }
    case WM_SIZE:
        return onResize(LOWORD(lParam),HIWORD(lParam));
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NOTIFY:
    {
        NMHDR *nmhdr=(NMHDR*)lParam;
        switch(nmhdr->code){
            case EN_LINK:
                if(nmhdr->hwndFrom==contentRichText)
                    return onEnLink((ENLINK*)lParam);
            break;
        }
        return 0;
    }
    case WM_COMMAND:
    {
        //WORD id=LOWORD(wParam);
        WORD notification=HIWORD(wParam);
        //std::cout<<std::to_string(id)<<","<<std::to_string(notification)<<","<<std::to_string(wParam)<<std::endl;
        //std::cout<<std::hex<<lParam<<","<<std::hex<<themeList<<std::endl;
        if((HWND)lParam==themeList&&notification==LBN_SELCHANGE){
            pushLocation();
            loadContent(SendMessageW(themeList,LB_GETCURSEL,0,0));
        }
        if(notification==BN_CLICKED){
            if((HWND)lParam==forwardButton)
                forward();
            else if((HWND)lParam==backwardButton)
                backward();
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK HelpWindow::WindowProc_S(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        CREATESTRUCTW* createStruct=(CREATESTRUCTW*)lParam;
        assert(createStruct->lpCreateParams);
        SetPropW(hwnd,L"HelpWindowInstance",createStruct->lpCreateParams);
        ((HelpWindow*)createStruct->lpCreateParams)->hwnd=hwnd;
        break;
    }
    HelpWindow* window=(HelpWindow*)GetPropW(hwnd,L"HelpWindowInstance");
    if(window)
        return window->WindowProc(uMsg,wParam,lParam);
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
LRESULT HelpWindow::onCreatWindow(){
    themeList=CreateWindowExW(WS_EX_CLIENTEDGE,WC_LISTBOXW ,L"themeList",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_DISABLENOSCROLL|LBS_NOINTEGRALHEIGHT|LBS_HASSTRINGS|LBS_NOTIFY,0,30,160,330,hwnd,NULL,hInstance,0);
    assert(themeList);
    assert(LoadLibraryW(L"Msftedit.dll"));
    contentRichText=CreateWindowExW(WS_EX_CLIENTEDGE,MSFTEDIT_CLASS ,L"contentRichText",WS_CHILD|WS_VISIBLE|ES_DISABLENOSCROLL|WS_VSCROLL|ES_NOIME|ES_MULTILINE|ES_READONLY,160,30,320,330,hwnd,NULL,hInstance,0);
    assert(contentRichText);
    long mask=SendMessageW(contentRichText,EM_GETEVENTMASK,0,0);
    mask|=ENM_LINK;
    SendMessageW(contentRichText,EM_SETEVENTMASK,0,mask);
    backwardButton=CreateWindowW(WC_BUTTON,L"\xef",WS_CHILD|WS_VISIBLE|BS_TEXT,0,0,30,30,hwnd,NULL,hInstance,0);
    assert(backwardButton);
    SendMessageW(backwardButton,WM_SETFONT,(WPARAM)wingdings,TRUE);
    forwardButton=CreateWindowW(WC_BUTTON,L"\xf0",WS_CHILD|WS_VISIBLE|BS_TEXT,30,0,30,30,hwnd,NULL,hInstance,0);
    assert(forwardButton);
    SendMessageW(forwardButton,WM_SETFONT,(WPARAM)wingdings,TRUE);
    initThemeList();
    return 0;
}
LRESULT HelpWindow::onResize(int width,int height){
    int width1=160;
    if(width1>width/2)width1=width/2;
    MoveWindow(themeList,0,30,width1,height-30,TRUE);
    MoveWindow(contentRichText,width1,30,width-width1,height-30,TRUE);
    return 0;
}
HelpWindow::HelpFile HelpWindow::helpFiles[]={
    {{TEXT(PathHelpIntro),MAKEINTRESOURCEW(HelpIntro)},L"前言"},
    {{TEXT(PathHelpCmd),MAKEINTRESOURCEW(HelpCmd)},L"命令行"},
    {{TEXT(PathHelpConfigJSON),MAKEINTRESOURCEW(HelpConfigJSON)},L"Config.txt"},
    {{TEXT(PathHelpInterface),MAKEINTRESOURCEW(HelpInterface)},L"设置界面"},
    {{TEXT(PathHelpFAQ),MAKEINTRESOURCEW(HelpFAQ)},L"常见问题"},
    {{TEXT(PathTHEFUCKINGCURSEDSHITTYQT),MAKEINTRESOURCEW(THEFUCKINGCURSEDSHITTYQT)},L"咸因"}
};
void HelpWindow::initThemeList(){
    for(int i=0,size=SendMessageW(themeList,LB_GETCOUNT,0,0);i<size;i++)
        SendMessageW(themeList,LB_DELETESTRING,size-i-1,0);
    for(HelpFile f:helpFiles)
        SendMessageW(themeList,LB_ADDSTRING,0,(LPARAM)f.title);
    assert(SendMessageW(themeList,LB_SETCURSEL,0,0)!=LB_ERR);
    currentLocation.type=NONE;
    loadContent(0);
}
void HelpWindow::loadContent(int index){
    loadContent(helpFiles[index]);
    assert(SendMessageW(themeList,LB_SETCURSEL,index,0)!=LB_ERR);
    currentLocation.type=T_Index;
    currentLocation.index=index;
}
void HelpWindow::loadContent(HelpFile file){
    loadContent(file.file);
    std::wstring title(L"BiliLocal帮助 - ");
    title+=file.title;
    SetWindowTextW(hwnd,title.c_str());
    currentLocation.type=T_HelpFile;
    currentLocation.helpFile=file;
}
void HelpWindow::loadContent(ResFile file){
    HRSRC hrsrc=FindResourceW(hInstance,file.id,RT_RCDATA);
    assert(hrsrc);
    unsigned size=SizeofResource(hInstance,hrsrc);
    HANDLE handle=LoadResource(hInstance,hrsrc);
    assert(handle);
    const char *data=(char*)LockResource(handle);
    char *data2=new char[size+1];
    memcpy(data2,data,size);
    data2[size]=0;
    SETTEXTEX setTextEx={ST_DEFAULT,936};
    assert(SendMessageW(contentRichText,EM_SETTEXTEX,(WPARAM)&setTextEx,(LPARAM)data2));
    POINT point={0,0};
    SendMessageW(contentRichText,EM_SETSCROLLPOS,0,(LPARAM)&point);
    delete[]data2;
    currentLocation.type=T_ResFile;
    currentLocation.resFile=file;
}

LRESULT HelpWindow::onEnLink(ENLINK *enlink){
    if(enlink->msg==WM_LBUTTONUP){
        wchar_t *buf=new wchar_t[enlink->chrg.cpMax-enlink->chrg.cpMin];
        TEXTRANGEW textRange={enlink->chrg,buf};
        assert(SendMessageW(contentRichText,EM_GETTEXTRANGE,0,(LPARAM)&textRange));
        long result=onClickLink(buf);
        delete[] buf;
        return result;
    }
   return 0;
}
HelpWindow::HelpFile HelpWindow::extraHelpFiles[]={
    {{TEXT(PathHelpCmdExample),MAKEINTRESOURCEW(HelpCmdExample)},L"命令行示例"}
};
HelpWindow::ResFile HelpWindow::extraResFiles[]={
    {TEXT(PathOpenGLRenderCPP),MAKEINTRESOURCEW(OpenGLRenderCPP)},
    {TEXT(PathAsyncRasterSpriteCPP),MAKEINTRESOURCEW(AsyncRasterSpriteCPP)},
    {TEXT(PathParseCPP),MAKEINTRESOURCEW(ParseCPP)},
    {TEXT(PathRunningCPP),MAKEINTRESOURCEW(RunningCPP)}
};
int HelpWindow::compareUrl(const wchar_t*u1,const wchar_t*u2){
    for(;*u1&&*u2&&*u1!='#'&&*u2!='#';u1++,u2++){
        if((*u1=='\\'||*u1=='/')&&(*u2=='\\'||*u2=='/'))
                continue;
        else if(*u1!=*u2)
                return *u1-*u2;
    }
    return 0;
}
LRESULT HelpWindow::onClickLink(wchar_t *link){
    if(*link=='#'){
        pushLocation();
        tryJumpAnchor(link);
        return 1;
    }
    const wchar_t *resProtocol=L"res://";
    const size_t resProtocolLen=lstrlenW(resProtocol);
    if(wcsnicmp(link,resProtocol,resProtocolLen)){
        const wchar_t *httpProtocol=L"http://";
        const size_t httpProtocolLen=lstrlenW(httpProtocol);
        const wchar_t *httpsProtocol=L"https://";
        const size_t httpsProtocolLen=lstrlenW(httpsProtocol);
        if(wcsnicmp(link,httpProtocol,httpProtocolLen)&&wcsnicmp(link,httpsProtocol,httpsProtocolLen))return 0;
        if(IDNO==MessageBoxW(hwnd,(std::wstring(L"是否打开链接\n")+link).c_str(),L"是否打开链接",MB_ICONWARNING|MB_YESNO))return 0;
        INT_PTR ret=(INT_PTR)ShellExecuteW(hwnd,L"open",link,NULL,NULL,SW_SHOWNORMAL);
        if(ret<=32){
            DWORD errCode=GetLastError();
            TCHAR *message;
            if(FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,errCode,0,(LPTSTR)&message,64,NULL)){
                MessageBoxW(hwnd,(std::wstring(L"发生错误(")+std::to_wstring(errCode)+L")\n"+message).c_str(),L"发生错误",MB_OK|MB_ICONERROR);
                LocalFree(message);
                return 0;
            }
            MessageBoxW(hwnd,(std::wstring(L"发生错误(")+std::to_wstring(errCode)+L")").c_str(),L"发生错误",MB_OK|MB_ICONERROR);
            return 0;
        }
        return 1;
    }
    for(unsigned i=0;i<sizeof(helpFiles)/sizeof(HelpFile);i++){
        if(compareUrl(helpFiles[i].file.path,link+resProtocolLen)==0)
        {
            pushLocation();
            loadContent(i);
            tryJumpAnchor(link);
            return 1;
        }
    }
    for(HelpFile f:extraHelpFiles){
        if(compareUrl(f.file.path,link+resProtocolLen)==0)
        {
            pushLocation();
            loadContent(f);
            SendMessageW(themeList,LB_SETCURSEL,-1,0);
            tryJumpAnchor(link);
            return 1;
        }
    }
    for(ResFile f:extraResFiles){
        if(compareUrl(f.path,link+resProtocolLen)==0)
        {
            pushLocation();
            loadContent(f);
            SendMessageW(themeList,LB_SETCURSEL,-1,0);
            SetWindowTextW(hwnd,(std::wstring(L"BiliLocal帮助 - res://")+f.path).c_str());
            tryJumpAnchor(link);
            return 1;
        }
    }
    MessageBoxW(hwnd,(std::wstring(L"没有找到资源文件")+link).c_str(),L"错误",MB_ICONERROR|MB_OK);
    return 1;
}
void HelpWindow::tryJumpAnchor(wchar_t *link){
    CHARRANGE charrange;
    SendMessageW(contentRichText,EM_EXGETSEL,0,(LPARAM)&charrange);
    POINT point;
    SendMessageW(contentRichText,EM_GETSCROLLPOS,0,(LPARAM)&point);
    while(*link&&*link!='#')link++;
    if(!*link)return;
    link++;
    if(!*link)return;
    std::wstring findStr(L"\r");
    findStr+=link;
    FINDTEXTEXW findtextexw={};
    findtextexw.chrg.cpMin=0;
    findtextexw.chrg.cpMax=-1;
    findtextexw.lpstrText=findStr.c_str();
    findtextexw.chrgText.cpMax=0;
    PARAFORMAT2 paraformat2={};
    paraformat2.cbSize=sizeof(PARAFORMAT2);
    while(1){
        findtextexw.chrg.cpMin=findtextexw.chrgText.cpMax;
        if(-1==SendMessageW(contentRichText,EM_FINDTEXTEXW,FR_DOWN|FR_MATCHCASE,(LPARAM)&findtextexw)){
            SendMessageW(contentRichText,EM_EXSETSEL,0,(LPARAM)&charrange);
            SendMessageW(contentRichText,EM_SETSCROLLPOS,0,(LPARAM)&point);
            std::wstring message(L"没有找到锚点");
            message+=link;
            MessageBoxW(hwnd,message.c_str(),L"错误",MB_ICONERROR|MB_OK);
            return;
        }
        findtextexw.chrgText.cpMin++;
        SendMessageW(contentRichText,EM_EXSETSEL,0,(LPARAM)&findtextexw.chrgText);
        SendMessageW(contentRichText,EM_GETPARAFORMAT,0,(LPARAM)&paraformat2);
        if(!(paraformat2.dwMask&PFM_NUMBERING)||paraformat2.wNumbering!=PFN_BULLET)continue;
        return;
    }
}
void HelpWindow::loadCurrentLocation(){
    switch(currentLocation.type){
    case T_Index:
        loadContent(currentLocation.index);
        break;
    case T_HelpFile:
        loadContent(currentLocation.helpFile);
        break;
    case T_ResFile:
        loadContent(currentLocation.resFile);
        break;
    case NONE:
        return;
    }
    SendMessageW(contentRichText,EM_EXSETSEL,0,(LPARAM)&currentLocation.selection);
    SendMessageW(contentRichText,EM_SETSCROLLPOS,0,(LPARAM)&currentLocation.scrollPosition);
}
void HelpWindow::forward(){
    if(locationQueue2.size()<=0)return;
    SendMessageW(contentRichText,EM_EXGETSEL,0,(LPARAM)&currentLocation.selection);
    SendMessageW(contentRichText,EM_GETSCROLLPOS,0,(LPARAM)&currentLocation.scrollPosition);
    locationQueue.push_front(currentLocation);
    if(locationQueue.size()>locationQueueMaxDepth)
        locationQueue.pop_back();
    currentLocation=locationQueue2.back();
    locationQueue2.pop_back();
    loadCurrentLocation();
}
void HelpWindow::backward(){
    if(locationQueue.size()<=0)return;
    SendMessageW(contentRichText,EM_EXGETSEL,0,(LPARAM)&currentLocation.selection);
    SendMessageW(contentRichText,EM_GETSCROLLPOS,0,(LPARAM)&currentLocation.scrollPosition);
    locationQueue2.push_back(currentLocation);
    currentLocation=locationQueue.front();
    locationQueue.pop_front();
    loadCurrentLocation();
}
void HelpWindow::pushLocation(){
    SendMessageW(contentRichText,EM_EXGETSEL,0,(LPARAM)&currentLocation.selection);
    SendMessageW(contentRichText,EM_GETSCROLLPOS,0,(LPARAM)&currentLocation.scrollPosition);
    locationQueue.push_front(currentLocation);
    if(locationQueue.size()>locationQueueMaxDepth)
        locationQueue.pop_back();
    locationQueue2.clear();
}
