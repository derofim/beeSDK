#include "stdafx.h"
#include "room_layout.h"
#include "video_room.h"
#include <stdio.h>
#include <tchar.h>
#include <sstream>
#include <thread>

const wchar_t kVideoRoomWindowClassName[] = L"VideoRoomWindowClass";

///////////////////////////////////PartyWindow///////////////////////////////////////
PartyWindow::PartyWindow(RoomLayout *layout, HWND handle, int x, int y, int width, int height, bee_handle bee_session, const std::string &room_name, const std::string &stream_name, PartyType type)
    : layout_(layout),
      handle_(handle),
      x_(x),
      y_(y),
      width_(width),
      height_(height),
      ori_width_(width),
      ori_height_(height),
	  border_brush_(NULL),
      bee_session_(bee_session),
      room_name_(room_name),
      stream_name_(stream_name),
      type_(type) {
	border_brush_ = CreateSolidBrush(RGB(255, 0, 0));
    video_renderer_.reset(new WinVideoRendererD3D);
}

PartyWindow::~PartyWindow() {
	DeleteObject(border_brush_);
    CloseRender();
}

void PartyWindow::Show(int show) {
    ::MoveWindow(handle_, x_, y_, width_, height_, TRUE);
    ::ShowWindow(handle_, show);
}

bool PartyWindow::ProcessMessage(UINT msg, WPARAM wp, LPARAM lp) {
	bool ret = false;
	do {
		switch (msg)
		{
		case WM_PAINT:
			OnPaint(wp, lp);
			ret = true;
			break;
        case WM_LBUTTONDBLCLK:
            OnLbDbClicked(wp, lp);
            ret = true;
            break;
        case WM_LBUTTONDOWN:
            OnLbDown(wp, lp);
            ret = true;
            break;
        case WM_RBUTTONDOWN:
            OnRbDown(wp, lp);
            ret = true;
            break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            ret = true;
            break;
		default:
			break;
		}
	} while (0);
	return ret;
}

void PartyWindow::UpdatePos(int x, int y, int width, int height) {
    if (width != -1) {
        ori_width_ = width;
    }
    if (height != -1) {
        ori_height_ = height;
    }

    size_t last_width = width_;
    size_t last_height = height_;
    this->MoveWindow(x, y, width, height);
    handle_resize(last_width, last_height);
}

void PartyWindow::MoveWindow(int x, int y, int width, int height) {
    x_ = x;
    y_ = y;
    if (width != -1) {
        width_ = width;
    }
    if (height != -1) {
        height_ = height;
    }
    ::MoveWindow(handle_, x_, y_, width_, height_, FALSE);
    ::InvalidateRect(handle_, NULL, TRUE);
}

void PartyWindow::CloseWinow() {
    ::SendMessage(handle_, WM_CLOSE, NULL, NULL);
}

void PartyWindow::Close() {
    CloseWinow();
}

void PartyWindow::Disconnect() {
    std::string command = "DisconnectConfParty";
    char args[512] = { 0 };
    sprintf_s(args, sizeof(args), "{\"room_name\":\"%s\",\"stream_name\":\"%s\", \"type\":%d}", room_name_.c_str(), stream_name_.c_str(), type_);
    size_t args_len = strlen(args);
    //bee_execute(bee_session_, command.c_str(), command.size(), args, args_len, bee_timeout);
    
    Close();
}

void PartyWindow::handle_resize(size_t width, size_t height) {
    double width_rate = (double)width / ori_width_;
    double height_rate = (double)height / ori_height_;
    double stretch_rate = width_rate > height_rate ? width_rate : height_rate;
    size_t target_width = (size_t)(width / stretch_rate);
    size_t target_height = (size_t)(height / stretch_rate);

    this->MoveWindow(x_, y_, target_width, target_height);
}

void PartyWindow::OnPaint(WPARAM wp, LPARAM lp) {
    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint(handle_, &ps);
    RECT rect;
    ::GetClientRect(handle_, &rect);
    ::FrameRect(hDC, &rect, border_brush_);
    std::ostringstream os;
    os << stream_name_;
    std::string id_str = os.str();
    ::TextOutA(hDC, width_ / 2, height_ / 2, id_str.c_str(), id_str.size());
    EndPaint(handle_, &ps);
}

void PartyWindow::OnLbDbClicked(WPARAM wp, LPARAM lp) {
    if (layout_ != NULL) {
        layout_->OnPartyLbDbClicked(shared_from_this());
    }
}

void PartyWindow::OnLbDown(WPARAM wp, LPARAM lp) {
    if (layout_ != NULL) {
        layout_->OnPartyLbDown(shared_from_this());
    }
}

void PartyWindow::OnRbDown(WPARAM wp, LPARAM lp) {
    POINT point;
    point.x = LOWORD(lp);
    point.y = HIWORD(lp);
    ::ClientToScreen(handle_, &point);

    HMENU hPopupMenu = CreatePopupMenu();
    AppendMenu(hPopupMenu, MF_STRING, IDM_CLOSE, L"Close");
    TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, handle_, NULL);

    if (layout_ != NULL) {
        layout_->OnPartyRbDown(shared_from_this());
    }
}

void PartyWindow::OnCommand(WPARAM wp, LPARAM lp) {
    do {
        switch (LOWORD(wp)) {
        case IDM_CLOSE:
            if (layout_ != NULL) {
                Disconnect();
                layout_->OnPartyClose(shared_from_this());
            }
            break;
        default:
            break;
        }
    } while (0);
}

bool PartyWindow::OpenRender() {
    if (video_renderer_ != NULL) {
        return video_renderer_->open(handle_, width_, height_, shared_from_this());
    } else {
        return false;
    }    
}

void PartyWindow::CloseRender() {
    if (video_renderer_ != NULL) {
        video_renderer_->close();
        video_renderer_ = NULL;
    }
}

/////////////////////////////////////RoomLayout/////////////////////////////////////
ATOM RoomLayout::wnd_class_ = 0;
RoomLayout::RoomLayout(HINSTANCE instance, int width, int height, std::shared_ptr<VideoRoom> video_room) :
      width_(width),
      height_(height),
      background_wnd_(NULL),
      main_window_(NULL),
      instance_(instance),
      video_room_(video_room) {
    InitSubWindowSize();
    InitSubWindowPos();
}

RoomLayout::~RoomLayout() {

}

LRESULT CALLBACK RoomLayout::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    bool processed = false;    
    do {
        RoomLayout* me = reinterpret_cast<RoomLayout*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!me && WM_CREATE == msg) {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lp);
            me = reinterpret_cast<RoomLayout*>(cs->lpCreateParams);
            if (me != NULL) {
                ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(me));
            }
        }

        if (me == NULL) {
            break;
        }

		auto iter = me->hwnd_2_party_table_.find((bee_int64_t)hwnd);
		if (iter != me->hwnd_2_party_table_.end()) {
			processed = iter->second->ProcessMessage(msg, wp, lp);
		}

		if (!processed) {
			processed = me->ProcessMessage(hwnd, msg, wp, lp);
		}
    } while (0);

    if (processed) {
        return 0;
    } else {
		return ::DefWindowProc(hwnd, msg, wp, lp);
    }
}

bool RoomLayout::RegisterWindowClass() {
    if (wnd_class_ != NULL)
        return true;

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_DBLCLKS;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.lpfnWndProc = &WndProc;
    wcex.lpszClassName = kVideoRoomWindowClassName;
    wnd_class_ = ::RegisterClassEx(&wcex);
    return wnd_class_ != 0;
}

void RoomLayout::CreateBackground() {
    do {
        if (!RegisterWindowClass()) {
            break;
        }

        if (background_wnd_ != NULL) {
            break;
        }

        HMENU menu = CreateMenu();

        background_wnd_ = ::CreateWindowExW(
            WS_EX_OVERLAPPEDWINDOW, 
            kVideoRoomWindowClassName, 
            L"VideoRoom",
            WS_OVERLAPPEDWINDOW  | WS_CLIPCHILDREN,
            CW_USEDEFAULT, 
            CW_USEDEFAULT, 
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            menu,
            GetModuleHandle(NULL), 
            this);
        if (background_wnd_ == NULL) {
            break;
        }

        //Let client region exactly be the size of width and height.
        DWORD dwStyle = ::GetWindowLongPtr(background_wnd_, GWL_STYLE);
        DWORD dwExStyle = ::GetWindowLongPtr(background_wnd_, GWL_EXSTYLE);
        RECT rc = { 0, 0, width_, height_ };
        ::AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
        ::SetWindowPos(background_wnd_, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
        ::ShowWindow(background_wnd_, SW_SHOW);

        ::GetClientRect(background_wnd_, &rc);
    } while (0);
}

PartyWindow::Ptr RoomLayout::CreateLocalWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    PartyWindow::Ptr party_window = NULL;
    do {
        if (background_wnd_ == NULL) {
            break;
        }

        if (main_window_ != NULL) {
            break;
        }

        int width = width_ / 2;
        int height = height_;
        HWND hwnd = ::CreateWindow(
            kVideoRoomWindowClassName,
            L"Main",
            WS_CHILD | WS_POPUP,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            width,
            height,
            background_wnd_,
            NULL,
            instance_,
            this);

        if (hwnd == NULL) {
            break;
        }

        POINT point;
        point.x = 0;
        point.y = 0;
        ::ClientToScreen(background_wnd_, &point);

        main_window_.reset(new PartyWindow(this, hwnd, point.x, point.y, width, height, bee_session, room_name, stream_name, ePartyType_Local));
        main_window_->OpenRender();
        main_window_->Show(SW_SHOW);
        hwnd_2_party_table_.insert(std::make_pair((bee_int64_t)main_window_->get_handle(), main_window_));
        stream_2_local_party_table.insert(std::make_pair(stream_name, main_window_));
        ActiveBackground(); //Must be called to take it to front.
        UpdateMainWindowPos();
        party_window = main_window_;
    } while (0);
    return party_window;
}

PartyWindow::Ptr RoomLayout::CreateRemoteWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    PartyWindow::Ptr party_window = NULL;
	do {
		if (background_wnd_ == NULL) {
			break;
		}

        size_t count = sub_windows_.size();
        if (count >= MAX_SUB_PARTY_SIZE) {
            break;
        }

        if (stream_2_remote_party_table.find(stream_name) != stream_2_remote_party_table.end()) {
            break;
        }

		int width = party_sizes_[count].width;
        int height = party_sizes_[count].height;
		HWND hwnd = ::CreateWindow(
			kVideoRoomWindowClassName,
			L"Sub",
			WS_CHILD | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			background_wnd_,
			NULL,
			instance_,
			this);

		if (hwnd == NULL) {
			break;
		}

        PartyWindow::Ptr sub_window(new PartyWindow(this, hwnd, 0, 0, width, height, bee_session, room_name, stream_name, ePartyType_Remote));
        sub_window->OpenRender();
        hwnd_2_party_table_.insert(std::make_pair((bee_int64_t)sub_window->get_handle(), sub_window));
        stream_2_remote_party_table.insert(std::make_pair(stream_name, sub_window));
        auto iter = sub_windows_.insert(sub_windows_.end(), sub_window);
        sub_window->SetIterator(iter);

        UpdateSubWindowPos();
        sub_window->Show(SW_SHOW);
        ActiveBackground(); //Must be called to take it to front.
        party_window = sub_window;
	} while (0);
    return party_window;
}

void RoomLayout::CloseLocalWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    do {
        auto iter = stream_2_local_party_table.find(stream_name);
        if (iter == stream_2_local_party_table.end()) {
            break;
        }

        PartyWindow::Ptr party = iter->second;
        if (party == NULL) {
            break;
        }

        if (party == main_window_) {
            RemoveMainWindow();
            break;
        }

        RemoveSubWindow(party->get_handle());
    } while (0);
}

void RoomLayout::CloseRemotebWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    do {
        auto iter = stream_2_remote_party_table.find(stream_name);
        if (iter == stream_2_remote_party_table.end()) {
            break;
        }

        PartyWindow::Ptr party = iter->second;
        if (party == NULL) {
            break;
        }

        if (party == main_window_) {
            RemoveMainWindow();
            break;
        }

        RemoveSubWindow(party->get_handle());
    } while (0);
}

void RoomLayout::OpenLocalVideoRenderer(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    do {
        auto iter = stream_2_local_party_table.find(stream_name);
        if (iter == stream_2_local_party_table.end()) {
            break;
        }

        PartyWindow::Ptr party = iter->second;
        if (party == NULL) {
            break;
        }

        party->OpenRender();
    } while (0);
}

void RoomLayout::OpenRemoteVideoRenderer(bee_handle bee_session, const std::string &room_name, const std::string &stream_name) {
    do {
        auto iter = stream_2_remote_party_table.find(stream_name);
        if (iter == stream_2_remote_party_table.end()) {
            break;
        }

        PartyWindow::Ptr party = iter->second;
        if (party == NULL) {
            break;
        }

        party->OpenRender();
    } while (0);
}

void RoomLayout::RemoveMainWindow() {
    auto iter = hwnd_2_party_table_.find((bee_int64_t)main_window_->get_handle());
    if (iter != hwnd_2_party_table_.end()) {
        hwnd_2_party_table_.erase(iter);

        if (main_window_->type() == ePartyType_Local) {
            stream_2_local_party_table.erase(main_window_->GetStreamName());
        } else {
            stream_2_remote_party_table.erase(main_window_->GetStreamName());
        }

        ::SendMessage(main_window_->get_handle(), WM_CLOSE, NULL, NULL);
        main_window_.reset();

        if (!sub_windows_.empty()) {
            main_window_ = sub_windows_.front();
            sub_windows_.pop_front();
            UpdateMainWindowPos();
        }
        UpdateSubWindowPos();
    }
}

void RoomLayout::RemoveSubWindow(HWND hwnd) {
    auto iter = hwnd_2_party_table_.find((bee_int64_t)hwnd);
    if (iter != hwnd_2_party_table_.end()) {
        PartyWindow::Ptr party = iter->second;
        if (!sub_windows_.empty()) {
            auto iter1 = party->GetIterator();
            if (iter1 != sub_windows_.end()) {
                sub_windows_.erase(iter1);
            }
        }
        hwnd_2_party_table_.erase(iter);
        if (party->type() == ePartyType_Local) {
            stream_2_local_party_table.erase(party->GetStreamName());
        } else {
            stream_2_remote_party_table.erase(party->GetStreamName());
        }
        ::SendMessage(party->get_handle(), WM_CLOSE, NULL, NULL);
        party.reset();
        UpdateSubWindowPos();
    }
}

void RoomLayout::OnPartyLbDbClicked(PartyWindow::Ptr party) {
    do {
        if (party == NULL || main_window_ == NULL) {
            break;
        }

        if (party == main_window_) {
            break;
        }

        auto after = sub_windows_.erase(party->GetIterator());
        auto iter = sub_windows_.insert(after, main_window_);
        main_window_->SetIterator(iter);        
        main_window_ = party;

        UpdateMainWindowPos();
        UpdateSubWindowPos();
    } while (0);
    ActiveBackground(); //Must be called to take it to front.
}

void RoomLayout::OnPartyLbDown(PartyWindow::Ptr party) {
    ActiveBackground();
}

void RoomLayout::OnPartyRbDown(PartyWindow::Ptr party) {
    ActiveBackground();
}

void RoomLayout::OnPartyClose(PartyWindow::Ptr party) {
    do {
        if (party == NULL) {
            break;
        }

        if (party == main_window_) {
            RemoveMainWindow();
            break;
        }

        RemoveSubWindow(party->get_handle());
    } while (0);
}

void RoomLayout::OnPartyUnlisten(PartyWindow::Ptr party) {
    do {
        if (party == NULL) {
            break;
        }

    } while (0);
}

bool RoomLayout::ProcessMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	bool ret = false;
	do {
		if (hwnd != background_wnd_) {
			break;
		}

		switch (msg)
		{
		case WM_MOVE:
			OnMove(wp, lp);
			ret = true;
			break;
		case WM_SIZE:
			OnSize(wp, lp);
			ret = true;
			break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            break;
		default:
			break;
		}
	} while (0);
	return ret;
}

void RoomLayout::OnMove(WPARAM wp, LPARAM lp) {
    UpdateMainWindowPos();
    InitSubWindowPos();
    UpdateSubWindowPos();
}

void RoomLayout::OnSize(WPARAM wp, LPARAM lp) {
    width_ = LOWORD(lp);
    height_ = HIWORD(lp);

    InitSubWindowSize();
    UpdateMainWindowPos();
    InitSubWindowPos();
    UpdateSubWindowPos();
}

void RoomLayout::OnCommand(WPARAM wp, LPARAM lp) {
    do {
        switch (LOWORD(wp)) {
        case IDM_VR_LEAVE:
            OnLeave();
            break;
        case IDM_VR_UNPUBLISH:
            OnUnpulish();
            break;
        default:
            break;
        }
    } while (0);
}

void RoomLayout::InitSubWindowSize() {
    int party_total_width = width_ / 2;
    int party_total_height = height_;

    int party_width = 0;
    int party_height = 0;

    for (int i = 0; i < MAX_SUB_PARTY_SIZE; ++i) {
        if (i == 0) {
            party_width = party_total_width;
            party_height = party_total_height;
            party_sizes_[i].width = party_width;
            party_sizes_[i].height = party_height;
            continue;;
        }

        if (i < 4) {
            party_width = party_total_width / 2;
            party_height = party_total_height / 2;
            party_sizes_[i].width = party_width;
            party_sizes_[i].height = party_height;
            continue;
        }

        party_width = party_total_width / 3;
        party_height = party_total_height / 3;
        party_sizes_[i].width = party_width;
        party_sizes_[i].height = party_height;
    }
}

void RoomLayout::InitSubWindowPos() {
    POINT point;
    point.x = width_ / 2;
    point.y = 0;
    if (background_wnd_ != NULL) {
        ::ClientToScreen(background_wnd_, &point);
    }
   
    InitSubWindowPos1(point.x, point.y);
    InitSubWindowPos4(point.x, point.y);
    InitSubWindowPos9(point.x, point.y);
}

void RoomLayout::InitSubWindowPos1(int x, int y) { 
    party_pos_1_[0].x = x;
    party_pos_1_[0].y = y;
}

void RoomLayout::InitSubWindowPos4(int x, int y) {
    int party_total_width = width_ / 2;
    int party_total_height = height_;

    int party_width = party_total_width / 2;
    int party_height = party_total_height / 2;

    party_pos_4_[0].x = x;
    party_pos_4_[0].y = y;

    party_pos_4_[1].x = x;
    party_pos_4_[1].y = y + party_height;

    party_pos_4_[2].x = x + party_width;
    party_pos_4_[2].y = y;

    party_pos_4_[3].x = x + party_width;
    party_pos_4_[3].y = y + party_height;
}

void RoomLayout::InitSubWindowPos9(int x, int y) {
    int party_total_width = width_ / 2;
    int party_total_height = height_;

    int party_width = party_total_width / 3;
    int party_height = party_total_height / 3;

    party_pos_9_[0].x = x;
    party_pos_9_[0].y = y;

    party_pos_9_[1].x = x;
    party_pos_9_[1].y = y + party_height;

    party_pos_9_[2].x = x;
    party_pos_9_[2].y = y + party_height * 2;

    party_pos_9_[3].x = x + party_width;
    party_pos_9_[3].y = y;

    party_pos_9_[4].x = x + party_width;
    party_pos_9_[4].y = y + party_height;

    party_pos_9_[5].x = x + party_width;
    party_pos_9_[5].y = y + party_height * 2;

    party_pos_9_[6].x = x + party_width * 2;
    party_pos_9_[6].y = y;

    party_pos_9_[7].x = x + party_width * 2;
    party_pos_9_[7].y = y + party_height;

    party_pos_9_[8].x = x + party_width * 2;
    party_pos_9_[8].y = y + party_height * 2;
}

void RoomLayout::UpdateMainWindowPos() {
    if (main_window_ != NULL) {
        POINT point;
        point.x = 0;
        point.y = 0;
        ::ClientToScreen(background_wnd_, &point);
        main_window_->MoveWindow(point.x, point.y, width_ / 2, height_);
    }
}

void RoomLayout::UpdateSubWindowPos() {
    do {
        size_t count = sub_windows_.size();
        if (count == 0) {
            break;
        }

        POINT *pos = NULL;
        if (count == 1) {
            pos = party_pos_1_;
        } else if (count <= 4) {
            pos = party_pos_4_;
        } else {
            pos = party_pos_9_;
        }

        int i = 0;
        auto iter = sub_windows_.begin();
        for (; iter != sub_windows_.end(); ++iter) {
            PartyWindow::Ptr party = *iter;
            party->UpdatePos(pos[i].x, pos[i].y, party_sizes_[count - 1].width, party_sizes_[count - 1].height);
            i++;
        }
    } while (0);
}

HMENU RoomLayout::CreateMenu() {
    HMENU hMenu = ::CreateMenu();
    HMENU hMenuPop = ::CreateMenu();
    AppendMenu(hMenuPop, MF_STRING, IDM_VR_LEAVE, _T("Leave"));
    AppendMenu(hMenuPop, MF_STRING, IDM_VR_UNPUBLISH, _T("Unpublish"));
    AppendMenu(hMenu, MF_POPUP, (unsigned int)hMenuPop, _T("Operation"));
    return hMenu;
}

void RoomLayout::ActiveBackground() {
    if (background_wnd_ != NULL)
        ::SetForegroundWindow(background_wnd_);
}

void RoomLayout::OnLeave() {
    if (background_wnd_ != NULL) {
        ::SendMessage(background_wnd_, WM_CLOSE, NULL, NULL);
    }

    if (video_room_ != NULL) {
        video_room_->leave();
    }

    if (main_window_ != NULL) {
        main_window_->Close();
        main_window_.reset();
    }

    auto iter = sub_windows_.begin();
    for (; iter != sub_windows_.end(); ++iter) {
        PartyWindow::Ptr party = *iter;
        party->Close();
        party.reset();
    }
    sub_windows_.clear();

    if (video_room_ != NULL) {
        video_room_->on_room_layout_close();
    }
}

void RoomLayout::OnUnpulish() {

}
