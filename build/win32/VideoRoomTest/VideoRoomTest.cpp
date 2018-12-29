// VideoRoomTest.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "VideoRoomTest.h"
#include "src/video_room.h"
#include "comLib/SafeQueue.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <cctype>

#define MAX_LOADSTRING 100

//#define LEAK_CHECK
#ifdef LEAK_CHECK
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

typedef enum LoginChildWindowID {
    eLoginChildWindowID_Room_Id_Static = 1,
    eLoginChildWindowID_Room_Id_Edit,
    eLoginChildWindowID_My_Name_Static,
    eLoginChildWindowID_My_Name_Edit,
    eLoginChildWindowID_Media_Type_Static,
    eLoginChildWindowID_Media_Type_Audio_CheckBox,
    eLoginChildWindowID_Media_Type_Video_CheckBox,
    eLoginChildWindowID_Create_Button,
    eLoginChildWindowID_Join_Button,
    eLoginChildWindowID_Cancel_Button,
}LoginChildWindowID;

const DWORD UI_THREAD_CALLBACK = WM_APP + 1;

// Child windows.
HWND g_login_wnd = NULL;
HWND g_room_id_static = NULL;
HWND g_room_id_edit = NULL;
HWND g_my_name_static = NULL;
HWND g_my_name_edit = NULL;
HWND g_media_type_static = NULL;
HWND g_media_type_audio_check_box = NULL;
HWND g_media_type_video_check_box = NULL;
HWND g_create_button = NULL;
HWND g_join_button = NULL;
HWND g_cancel_button = NULL;

size_t login_wnd_width = 0;
size_t login_wnd_height = 0;
size_t room_id_static_x = 0;
size_t room_id_static_y = 0;
size_t room_id_static_width = 0;
size_t room_id_static_height = 0;
size_t room_id_edit_x = 0;
size_t room_id_edit_y = 0;
size_t room_id_edit_width = 0;
size_t room_id_edit_height = 0;
size_t my_name_static_x = 0;
size_t my_name_static_y = 0;
size_t my_name_static_width = 0;
size_t my_name_static_height = 0;
size_t my_name_edit_x = 0;
size_t my_name_edit_y = 0;
size_t my_name_edit_width = 0;
size_t my_name_edit_height = 0;
size_t media_type_static_x = 0;
size_t media_type_static_y = 0;
size_t media_type_static_width = 0;
size_t media_type_static_height = 0;
size_t media_type_audio_check_box_x = 0;
size_t media_type_audio_check_box_y = 0;
size_t media_type_audio_check_box_width = 0;
size_t media_type_audio_check_box_height = 0;
size_t media_type_video_check_box_x = 0;
size_t media_type_video_check_box_y = 0;
size_t media_type_video_check_box_width = 0;
size_t media_type_video_check_box_height = 0;
size_t create_button_x = 0;
size_t create_button_y = 0;
size_t create_button_width = 0;
size_t create_button_height = 0;
size_t join_button_x = 0;
size_t join_button_y = 0;
size_t join_button_width = 0;
size_t join_button_height = 0;
size_t cancel_button_x = 0;
size_t cancel_button_y = 0;
size_t cancel_button_width = 0;
size_t cancel_button_height = 0;

std::string g_login_ini = "login.ini";
std::shared_ptr<VideoRoom> g_video_room;
DWORD g_ui_thread_id = 0;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                CreateChildWindow(HWND* wnd, int id, const wchar_t* class_name, DWORD control_style, DWORD ex_style, int x, int y, int width, int height, const wchar_t *window_name = _T(""));
HFONT               GetDefaultFont();
VOID                InitLoginWindowLayout();
VOID                LoadLoginConfig();
VOID                SaveLoginConfig(const std::string &room_name, const std::string &my_name, int push_audio, int push_video);
std::string         GetWindowText(HWND wnd);

bool PreTranslateMessage(MSG* msg) {
    if (msg->hwnd == NULL && msg->message == UI_THREAD_CALLBACK && g_video_room != NULL) {
        g_video_room->ui_thread_callback(static_cast<UIThreadMsg>(msg->wParam), reinterpret_cast<void*>(msg->lParam));
        return true;
    } else {
        return false;
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#ifdef LEAK_CHECK
    VLDGlobalEnable();
    VLDReportLeaks();
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
  
    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEOROOMTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    g_ui_thread_id = ::GetCurrentThreadId();

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEOROOMTEST));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) && !PreTranslateMessage(&msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#ifdef LEAK_CHECK
    VLDGlobalDisable();
#endif
    return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDEOROOMTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中
   InitLoginWindowLayout();

   int screen_width = GetSystemMetrics(SM_CXSCREEN);
   int screen_height = GetSystemMetrics(SM_CYSCREEN);

   int x = (screen_width - login_wnd_width) / 2;
   int y = (screen_height - login_wnd_height) / 2;

   DWORD style = WS_OVERLAPPEDWINDOW;
   style = style &~ WS_MAXIMIZEBOX;
   g_login_wnd = CreateWindowW(
       szWindowClass, 
       L"登录视频会议", 
       style,
       x, 
       y,
       login_wnd_width,
       login_wnd_height,
       nullptr, 
       nullptr, 
       hInstance, 
       nullptr);

   if (!g_login_wnd)
   {
      return FALSE;
   }

   CreateChildWindow(
       &g_room_id_static, 
       eLoginChildWindowID_Room_Id_Static, 
       L"Static", 
       BS_CENTER | ES_READONLY  | SS_CENTER, 
       0,
       room_id_static_x,
       room_id_static_y,
       room_id_static_width,
       room_id_static_height);
   ::SetWindowText(g_room_id_static, L"会议室");

   CreateChildWindow(
       &g_room_id_edit,
       eLoginChildWindowID_Room_Id_Edit,
       L"Edit",
       ES_LEFT | ES_NOHIDESEL | WS_TABSTOP, 
       WS_EX_CLIENTEDGE,
       room_id_edit_x,
       room_id_edit_y,
       room_id_edit_width,
       room_id_edit_height);

   CreateChildWindow(
       &g_my_name_static,
       eLoginChildWindowID_My_Name_Static,
       L"Static",
       BS_CENTER | ES_READONLY | SS_CENTER,
       0,
       my_name_static_x,
       my_name_static_y,
       my_name_static_width,
       my_name_static_height);
   ::SetWindowText(g_my_name_static, L"名 字");

   CreateChildWindow(
       &g_my_name_edit,
       eLoginChildWindowID_My_Name_Edit,
       L"Edit",
       ES_LEFT | ES_NOHIDESEL | WS_TABSTOP,
       WS_EX_CLIENTEDGE,
       my_name_edit_x,
       my_name_edit_y,
       my_name_edit_width,
       my_name_edit_height);

   CreateChildWindow(
       &g_media_type_static,
       eLoginChildWindowID_Media_Type_Static,
       L"Static",
       BS_CENTER | ES_READONLY | SS_CENTER,
       0,
       media_type_static_x,
       media_type_static_y,
       media_type_static_width,
       media_type_static_height);
   ::SetWindowText(g_media_type_static, L"推 流");

   CreateChildWindow(
       &g_media_type_audio_check_box,
       eLoginChildWindowID_Media_Type_Audio_CheckBox,
       L"Button",
       WS_VISIBLE | BS_AUTOCHECKBOX,
       0,
       media_type_audio_check_box_x,
       media_type_audio_check_box_y,
       media_type_audio_check_box_width,
       media_type_audio_check_box_height,
       _T("CheckBox"));
   ::SetWindowText(g_media_type_audio_check_box, L"音频");

   CreateChildWindow(
       &g_media_type_video_check_box,
       eLoginChildWindowID_Media_Type_Audio_CheckBox,
       L"Button",
       WS_VISIBLE | BS_AUTOCHECKBOX,
       0,
       media_type_video_check_box_x,
       media_type_video_check_box_y,
       media_type_video_check_box_width,
       media_type_video_check_box_height,
       _T("CheckBox"));
   ::SetWindowText(g_media_type_video_check_box, L"视频");

   CreateChildWindow(
       &g_create_button,
       eLoginChildWindowID_Create_Button,
       L"Button",
       BS_CENTER | WS_TABSTOP,
       0,
       create_button_x,
       create_button_y,
       create_button_width,
       create_button_height);
   ::SetWindowText(g_create_button, L"创 建");

   CreateChildWindow(
       &g_join_button,
       eLoginChildWindowID_Join_Button,
       L"Button",
       BS_CENTER | WS_TABSTOP,
       0,
       join_button_x,
       join_button_y,
       join_button_width,
       join_button_height);
   ::SetWindowText(g_join_button, L"加 入");

   CreateChildWindow(
       &g_cancel_button,
       eLoginChildWindowID_Cancel_Button,
       L"Button",
       BS_CENTER | WS_TABSTOP,
       0,
       cancel_button_x,
       cancel_button_y,
       cancel_button_width,
       cancel_button_height);
   ::SetWindowText(g_cancel_button, L"取 消");
   ::SetFocus(g_room_id_edit);

   LoadLoginConfig();

   ShowWindow(g_login_wnd, nCmdShow);
   UpdateWindow(g_login_wnd);

   return TRUE;
}

void PostToUIThread(UIThreadMsg msg_id, void *data) {
    ::PostThreadMessage(g_ui_thread_id, UI_THREAD_CALLBACK, static_cast<WPARAM>(msg_id), reinterpret_cast<LPARAM>(data));
}

void OnVideoRoomClosed() {
    if (g_video_room != NULL) {
        g_video_room.reset();
    }

    ::ShowWindow(g_login_wnd, SW_SHOW);
}

VOID OnCreateVideoConf(HWND hWnd, const std::string room_name, const std::string &my_name, int push_audio, int push_video) {
    do {
        if (room_name.empty() || my_name.empty()) {
            MessageBoxA(hWnd, "Invalid Input", "Error", MB_OK);
            break;
        }

        g_video_room.reset(new VideoRoom(PostToUIThread, OnVideoRoomClosed));
        BeeErrorCode ec1 = kBeeErrorCode_Success;
        if (!g_video_room->init(hInst, ec1) || ec1 != kBeeErrorCode_Success) {
            char msg[512] = { 0 };
            sprintf_s(msg, sizeof(msg), "Bee init fail, error:%d", ec1);
            MessageBoxA(hWnd, msg, "Error", MB_OK);
            break;
        }

        if (!g_video_room->do_create(room_name, my_name, push_audio, push_video, ec1) || ec1 != kBeeErrorCode_Success) {
            char msg[512] = { 0 };
            sprintf_s(msg, sizeof(msg), "Bee create fail, error:%d", ec1);
            MessageBoxA(hWnd, msg, "Error", MB_OK);
            break;
        }

        SaveLoginConfig(room_name, my_name, push_audio, push_video);
        ::ShowWindow(g_login_wnd, SW_HIDE);
    } while (0);
}

VOID OnJoinVideoConf(HWND hWnd, const std::string room_name, const std::string &my_name, int push_audio, int push_video) {
    do {
        if (room_name.empty()) {
            MessageBoxA(hWnd, "Invalid Input", "Error", MB_OK);
            break;
        }

        g_video_room.reset(new VideoRoom(PostToUIThread, OnVideoRoomClosed));
        BeeErrorCode ec1 = kBeeErrorCode_Success;
        if (!g_video_room->init(hInst, ec1) || ec1 != kBeeErrorCode_Success) {
            char msg[512] = { 0 };
            sprintf_s(msg, sizeof(msg), "Bee init fail, error:%d", ec1);
            MessageBoxA(hWnd, msg, "Error", MB_OK);
            break;
        }

        if (!g_video_room->do_join(room_name, my_name, push_audio, push_video, ec1) || ec1 != kBeeErrorCode_Success) {
            char msg[512] = { 0 };
            sprintf_s(msg, sizeof(msg), "Bee join fail, error:%d", ec1);
            MessageBoxA(hWnd, msg, "Error", MB_OK);
            break;
        }

        SaveLoginConfig(room_name, my_name, push_audio, push_video);
        ::ShowWindow(g_login_wnd, SW_HIDE);
    } while (0);
}

bool CALLBACK OnLoginWindowCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    bool ret = true;
    int id = LOWORD(wParam);
    switch (id) {
    case eLoginChildWindowID_Create_Button:
        if (BN_CLICKED == HIWORD(wParam)) {
            std::string room_name(GetWindowText(g_room_id_edit));
            std::string my_name(GetWindowText(g_my_name_edit));
            int push_audio = SendMessage(g_media_type_audio_check_box, BM_GETCHECK, 0, 0);
            int push_video = SendMessage(g_media_type_video_check_box, BM_GETCHECK, 0, 0);
            OnCreateVideoConf(hWnd, room_name, my_name, push_audio, push_video);
            ret = true;
        }
        break;
    case eLoginChildWindowID_Join_Button:
        if (BN_CLICKED == HIWORD(wParam)) {
            std::string room_name(GetWindowText(g_room_id_edit));
            std::string my_name(GetWindowText(g_my_name_edit));
            int push_audio = SendMessage(g_media_type_audio_check_box, BM_GETCHECK, 0, 0);
            int push_video = SendMessage(g_media_type_video_check_box, BM_GETCHECK, 0, 0);
            OnJoinVideoConf(hWnd, room_name, my_name, push_audio, push_video);
            ret = true;
        }
        break;
    case eLoginChildWindowID_Cancel_Button:
        VideoRoom::uninit_on_exit();
        ::PostQuitMessage(0);
        break;
    case eLoginChildWindowID_Media_Type_Audio_CheckBox:
        break;
    case eLoginChildWindowID_Media_Type_Video_CheckBox:
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (!OnLoginWindowCommand(hWnd, message, wParam, lParam)) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CreateChildWindow(HWND* wnd, int id, const wchar_t* class_name, DWORD control_style, DWORD ex_style, int x, int y, int width, int height, const wchar_t *window_name) {
    if (::IsWindow(*wnd))
        return;

    // Child windows are invisible at first, and shown after being resized.
    DWORD style = WS_CHILD | control_style;
    *wnd = ::CreateWindowEx(
        ex_style, 
        class_name, 
        window_name,
        style,
        x, 
        y, 
        width, 
        height, 
        g_login_wnd,
        reinterpret_cast<HMENU>(id),
        GetModuleHandle(NULL), 
        NULL);
    ::SendMessage(*wnd, WM_SETFONT, reinterpret_cast<WPARAM>(GetDefaultFont()), TRUE);
    ::ShowWindow(*wnd, SW_SHOW);
}

HFONT GetDefaultFont() {
    // 设置字体参数
    static LOGFONT LogFont;
    ::memset(&LogFont, 0, sizeof(LOGFONT));
    lstrcpy(LogFont.lfFaceName, L"黑体");
    LogFont.lfWeight = 0;
    LogFont.lfHeight = -15; // 字体大小
    LogFont.lfCharSet = 134;
    LogFont.lfOutPrecision = 3;
    LogFont.lfClipPrecision = 2;
    LogFont.lfOrientation = 45;
    LogFont.lfQuality = PROOF_QUALITY;
    LogFont.lfPitchAndFamily = 2;

    HFONT hFont = CreateFontIndirect(&LogFont);
    return hFont;
}

VOID InitLoginWindowLayout() {
    size_t space1 = 40;
    size_t space2 = 50;
    size_t space3 = 70;
    size_t space4 = 20;
    size_t height1 = 22;

    login_wnd_width = 435;
    login_wnd_height = 320;

    room_id_static_x = 80;
    room_id_static_y = 40;
    room_id_static_width = 50;
    room_id_static_height = height1;

    room_id_edit_x = room_id_static_x + room_id_static_width + space1;
    room_id_edit_y = room_id_static_y;
    room_id_edit_width = 170;
    room_id_edit_height = height1;

    my_name_static_x = room_id_static_x;
    my_name_static_y = room_id_static_y + space2;
    my_name_static_width = room_id_static_width;
    my_name_static_height = height1;

    my_name_edit_x = my_name_static_x + my_name_static_width + space1;
    my_name_edit_y = my_name_static_y;
    my_name_edit_width = room_id_edit_width;
    my_name_edit_height = height1;

    media_type_static_x = my_name_static_x;
    media_type_static_y = my_name_static_y + space2;
    media_type_static_width = my_name_static_width;
    media_type_static_height = height1;

    media_type_audio_check_box_x = my_name_edit_x;
    media_type_audio_check_box_y = media_type_static_y;
    media_type_audio_check_box_width = 70;
    media_type_audio_check_box_height = media_type_static_height;

    media_type_video_check_box_x = my_name_edit_x + media_type_audio_check_box_width + 30;
    media_type_video_check_box_y = media_type_static_y;
    media_type_video_check_box_width = media_type_audio_check_box_width;
    media_type_video_check_box_height = media_type_audio_check_box_height;

    create_button_x = media_type_static_x;
    create_button_y = media_type_static_y + 60;
    create_button_width = 75;
    create_button_height = 30;

    join_button_x = create_button_x + create_button_width + space4;
    join_button_y = create_button_y;
    join_button_width = create_button_width;
    join_button_height = create_button_height;

    cancel_button_x = join_button_x + join_button_width  + space4;
    cancel_button_y = create_button_y;
    cancel_button_width = create_button_width;
    cancel_button_height = create_button_height;
}

bool is_number(const std::string &str) {
    bool ret = true;
    const char *p = str.c_str();
    size_t n = str.size();
    for (size_t i = 0; i < n; ++i) {
        if (!isdigit(p[i])) {
            ret = false;
            break;
        }
    }
    return ret;
}

VOID LoadLoginConfig() {
    try {
        do {
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(g_login_ini, pt);
            std::string room_name = pt.get<std::string>("room_name");
            std::string my_name = pt.get <std::string>("my_name");
            int push_audio = pt.get<int>("push_audio");
            int push_video = pt.get<int>("push_video");
            if (g_room_id_edit == NULL || g_my_name_edit == NULL || g_create_button == NULL || g_join_button == NULL) {
                break;
            }

            ::SetWindowTextA(g_room_id_edit, room_name.c_str());
            ::SetWindowTextA(g_my_name_edit, my_name.c_str());
            ::SendMessage(g_media_type_audio_check_box, BM_SETCHECK, push_audio, 0);
            ::SendMessage(g_media_type_video_check_box, BM_SETCHECK, push_video, 0);
            ::SetFocus(g_create_button);
        } while (0);
    } catch (std::exception &e) {
        printf("Load config fail error %s\n", e.what());
    }
}

VOID SaveLoginConfig(const std::string &room_name, const std::string &my_name, int push_audio, int push_video) {
    FILE *fp = NULL;
    int err = fopen_s(&fp, g_login_ini.c_str(), "wb+");
    if (fp != NULL && err == 0) {
        fprintf(fp, "room_name = %s\nmy_name=%s\npush_audio=%d\npush_video=%d\n", 
            room_name.c_str(),
            my_name.c_str(),
            push_audio,
            push_video);
        fclose(fp);
    }
}

std::string GetWindowText(HWND wnd) {
    char text[MAX_PATH] = { 0 };
    ::GetWindowTextA(wnd, &text[0], ARRAYSIZE(text));
    return text;
}
