#ifndef __ROOM_LAYOUT_H__
#define __ROOM_LAYOUT_H__

#include "bee/base/bee_define.h"
#include "bee/win/win_video_renderer_d3d.h"
#include <unordered_map>
#include <list>

using namespace bee;

const int MAX_SUB_PARTY_SIZE = 9;

#define IDM_VR_LEAVE     50001
#define IDM_VR_UNPUBLISH 50002
#define IDM_CLOSE        50003
#define IDM_UNLISTEN     50004

typedef enum PartyType {
    ePartyType_Local = 0,
    ePartyType_Remote
}PartyType;

/////////////////////////////////////PartySize/////////////////////////////////////
typedef struct PartySize{
    int  width;
    int  height;
    bool active;

    PartySize() {
        width = 0;
        height = 0;
        active = false;
    }
}PartySize;

///////////////////////////////////PartyWindow///////////////////////////////////////
class RoomLayout;
class PartyWindow : public WinD3DResizeHandler, public std::enable_shared_from_this<PartyWindow> {
public:
    typedef std::shared_ptr<PartyWindow> Ptr;
    PartyWindow(RoomLayout *layout, HWND handle, int x, int y, int width, int height, bee_handle bee_session, const std::string &room_name, const std::string &stream_name, PartyType type);
    ~PartyWindow();

public:
    void Show(int show);
    HWND get_handle() { return handle_; }
	bool ProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    void SetIterator(const std::list<std::shared_ptr<PartyWindow> >::iterator &iter) { iterator_ = iter; }
    std::list<std::shared_ptr<PartyWindow> >::iterator &GetIterator() { return iterator_; }
    void UpdatePos(int x, int y, int width, int height);
    void MoveWindow(int x, int y, int width, int height);
    void CloseWinow();
    const std::string &GetStreamName() const { return stream_name_; }
    void Close();
    void Disconnect();
    PartyType type() { return type_; }
    std::shared_ptr<WinVideoRendererD3D> get_video_renderer() { return video_renderer_; }
    virtual void handle_resize(size_t width, size_t height);
    bool OpenRender();
    void CloseRender();

protected:
    void OnPaint(WPARAM wp, LPARAM lp);
    void OnLbDbClicked(WPARAM wp, LPARAM lp);
    void OnLbDown(WPARAM wp, LPARAM lp);
    void OnRbDown(WPARAM wp, LPARAM lp);
    void OnCommand(WPARAM wp, LPARAM lp);

protected:
    RoomLayout *layout_;
    HWND handle_;
    int x_;
    int y_;
    int width_;
    int height_;
    int ori_width_;
    int ori_height_;
	HBRUSH border_brush_;
    std::list<std::shared_ptr<PartyWindow> >::iterator iterator_;
    bee_handle bee_session_ = -1;
    std::string room_name_;
    std::string stream_name_;
    int bee_timeout = 5000;
    PartyType type_;
    std::shared_ptr<WinVideoRendererD3D> video_renderer_;
};

///////////////////////////////////RoomLayout///////////////////////////////////////
class VideoRoom;
class RoomLayout {
public:
	typedef std::unordered_map<bee_int64_t, std::shared_ptr<PartyWindow> > PartyWinTable;
    typedef std::unordered_map<std::string, std::shared_ptr<PartyWindow> > PartyStreamTable;
    RoomLayout(HINSTANCE instance, int width, int height, std::shared_ptr<VideoRoom> video_room);
    ~RoomLayout();

public:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static bool RegisterWindowClass();

public:
    void CreateBackground();
    PartyWindow::Ptr CreateLocalWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    PartyWindow::Ptr CreateRemoteWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    void CloseLocalWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    void CloseRemotebWindow(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    void OpenLocalVideoRenderer(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    void OpenRemoteVideoRenderer(bee_handle bee_session, const std::string &room_name, const std::string &stream_name);
    void RemoveMainWindow();
    void RemoveSubWindow(HWND hwnd);
    void OnPartyLbDbClicked(PartyWindow::Ptr party);
    void OnPartyLbDown(PartyWindow::Ptr party);
    void OnPartyRbDown(PartyWindow::Ptr party);
    void OnPartyClose(PartyWindow::Ptr party);
    void OnPartyUnlisten(PartyWindow::Ptr party);
    HWND GetMainWnd() { return main_window_ == NULL ? NULL : main_window_->get_handle(); }
    
protected:
	bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	void OnMove(WPARAM wp, LPARAM lp);
	void OnSize(WPARAM wp, LPARAM lp);
    void OnCommand(WPARAM wp, LPARAM lp);
    void InitSubWindowSize();
    void InitSubWindowPos();
    void InitSubWindowPos1(int x, int y);
    void InitSubWindowPos4(int x, int y);
    void InitSubWindowPos9(int x, int y);
    void UpdateMainWindowPos();
    void UpdateSubWindowPos();
    HMENU CreateMenu();
    void ActiveBackground();
    void OnLeave();
    void OnUnpulish();

protected:
    int width_;
    int height_;
    HWND background_wnd_;
    std::shared_ptr<PartyWindow> main_window_;
	std::list<std::shared_ptr<PartyWindow> > sub_windows_;
    HINSTANCE instance_;
    static ATOM wnd_class_;
    PartyWinTable hwnd_2_party_table_;
    PartyStreamTable stream_2_local_party_table;
    PartyStreamTable stream_2_remote_party_table;
    PartySize party_sizes_[MAX_SUB_PARTY_SIZE];
    POINT party_pos_1_[1];
    POINT party_pos_4_[4];
    POINT party_pos_9_[9];
    std::shared_ptr<VideoRoom> video_room_;
};

#endif
