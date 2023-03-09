
#include "viewport.h"

#include "msgq.h"

#include <Windows.h>

namespace
{
  using namespace gh;

  struct Pipe
  {
    gh::MessageQueue* to;
    gh::MessageQueue* from;

    Pipe();
    ~Pipe();
  };

  Pipe::Pipe()
    :to(MessageQueue::Create())
    , from(MessageQueue::Create())
  {
  }

  Pipe::~Pipe()
  {
  }

  struct ThreadMessage
  {
    enum ThreadMessageType
    {
      OpenWindow,
      CloseWindow,

      WindowCreateFail,
      WindowOpen,
      WindowCloseButton,
      WindowClose,

      WindowSize,

      Mouse,

    }type;

    ThreadMessage(ThreadMessageType t)
      :type(t) {}
    virtual ~ThreadMessage() {}
  };

  struct ThreadMessageOpenWindow
    :public ThreadMessage
  {
    ViewportInitializeParameter ip;
    ThreadMessageOpenWindow(ViewportInitializeParameter ip)
      :ThreadMessage(OpenWindow), ip(ip) {}
  };

  struct ThreadMessageWindowCreated
    :public ThreadMessage
  {
    HWND hwnd;
    uint8_t* framebuffer;
    ThreadMessageWindowCreated(HWND hwnd, uint8_t* framebuffer)
      :ThreadMessage(WindowOpen), hwnd(hwnd), framebuffer(framebuffer) {}
  };

  struct ThreadMessageWindowSize
    :public ThreadMessage
  {
    SIZE size;
    ThreadMessageWindowSize(SIZE size)
      :ThreadMessage(WindowSize), size(size) {}
  };

  struct ThreadMessageMouse
    :public ThreadMessage
  {
    int x, y, z;
    unsigned int button;
    ThreadMessageMouse()
      :ThreadMessage(Mouse) {}
  };

  struct MessageFromWindow
    :public Endpoint
  {
  public:
    MessageFromWindow();
    void Open();
    void Close();
    int Transfer(const uint8_t* buffer, int length);
    bool is_open()const;
    bool is_close()const;
    void SetCallback(ViewportMessage* callback);
    void* WindowHandle();
    inline SIZE Size()const
    {
      return size_;
    }

    ViewportMessage* callback_;
    HWND hwnd_;
    uint8_t* framebuffer_;
    SIZE size_;
    bool is_open_;
    bool is_close_;
  };

  MessageFromWindow::MessageFromWindow()
    :is_open_(false), is_close_(false)
  {
  }

  void MessageFromWindow::Open()
  {
    is_open_ = true;
  }

  bool MessageFromWindow::is_open()const
  {
    return is_open_;
  }

  void MessageFromWindow::Close()
  {
    is_close_ = true;
  }

  bool MessageFromWindow::is_close()const
  {
    return is_close_;
  }

  void* MessageFromWindow::WindowHandle()
  {
    return hwnd_;
  }

  void MessageFromWindow::SetCallback(ViewportMessage* callback)
  {
    this->callback_ = callback;
  }

  int MessageFromWindow::Transfer(const uint8_t* buffer, int length)
  {
    if (length >= sizeof(ThreadMessage::ThreadMessageType))
    {
      const ThreadMessage* t = reinterpret_cast<const ThreadMessage*>(buffer);
      if (ThreadMessage::WindowCreateFail == t->type)
      {
        callback_->CreateFail();
      }
      else if (ThreadMessage::WindowOpen == t->type)
      {
        hwnd_ = (static_cast<const ThreadMessageWindowCreated*>(t))->hwnd;
        framebuffer_ = (static_cast<const ThreadMessageWindowCreated*>(t))->framebuffer;
        callback_->Open();
      }
      else if (ThreadMessage::WindowClose == t->type)
      {
        callback_->Close();
      }
      else if (ThreadMessage::WindowCloseButton == t->type)
      {
        callback_->CloseButton();
      }
      else if (ThreadMessage::Mouse == t->type)
      {
        const ThreadMessageMouse* tmm = static_cast<const ThreadMessageMouse*>(t);
        callback_->Mouse(tmm->x, tmm->y, tmm->z, tmm->button);
      }
      else if (ThreadMessage::WindowSize == t->type)
      {
        const ThreadMessageWindowSize* wsz = static_cast<const ThreadMessageWindowSize*>(t);
        size_ = wsz->size;
        callback_->Resize(size_.cx, size_.cy);
      }
      t->~ThreadMessage();
    }
    return length;
  }
}

#include <thread>

namespace gh {
  void ViewportMessage::Mouse(int x, int y, int z, unsigned int button)
  {
  }

  void ViewportMessage::Resize(int w, int h)
  {
  }

  /////////////////////////////
  // Thread Messages
  void ViewportThread(Pipe&);

  class ViewportBody
    :public Viewport
  {
  public:
    void Open(const ViewportInitializeParameter& ip, ViewportMessage* cb);
    void Close();
    void* GetWindowHandle();
    uint8_t* Framebuffer() {
      return pass_.framebuffer_;
    }
    void MessageFetch();
    void ChangeSize(int w, int h);
    ViewportBody();
    ~ViewportBody();
    int Width()const
    {
      return pass_.Size().cx;
    }
    int Height()const
    {
      return pass_.Size().cy;
    }
    void SetSize(int w, int h)
    {
    }
  private:
    Pipe pipe_;
    ViewportInitializeParameter ip_;
    std::thread th;
    MessageFromWindow pass_;
  };

  ViewportBody::ViewportBody()
    :pipe_()
    , th(ViewportThread, std::ref(pipe_))
  {
    pipe_.from->SetNext(&pass_);
  }

  ViewportBody::~ViewportBody()
  {
    pipe_.to->Close();
    while (!pass_.is_close())
    {
      pipe_.from->Fetch();
      Sleep(1);
    }
    th.join();
  }

  void* ViewportBody::GetWindowHandle()
  {
    return pass_.WindowHandle();
  }

  void ViewportBody::Close()
  {
    uint8_t* context;
    pipe_.to->Lock(&context, sizeof(ThreadMessage));
    new(context)ThreadMessage(ThreadMessage::CloseWindow);
    pipe_.to->Unlock();
  }

  void ViewportBody::MessageFetch()
  {
    pipe_.from->Fetch();
  }

  void ViewportBody::Open(const ViewportInitializeParameter& ip, ViewportMessage* cb)
  {
    pass_.SetCallback(cb);

    uint8_t* context;
    pipe_.to->Lock(&context, sizeof(ThreadMessageOpenWindow));
    new(context)ThreadMessageOpenWindow(ip);
    pipe_.to->Unlock();
  }

  void ViewportBody::ChangeSize(int w, int h)
  {
  }

  /////////////////////////////////////
  // mixed 


  Viewport* CreateViewport(const ViewportInitializeParameter& ip, ViewportMessage* cb)
  {
    ViewportBody* vp = new ViewportBody;
    vp->Open(ip, cb);
    return vp;
  }

  void DeleteViewport(Viewport* p)
  {
    ViewportBody* vp = static_cast<ViewportBody*>(p);
    delete vp;
  }

  ////////////////////////////////////////
  // Window Private Thread

  struct TLS
  {
    ViewportInitializeParameter param;

    Pipe* pipe_;
    uint8_t* backbuffer = nullptr;
    uint8_t* frontbuffer = nullptr;
    HBITMAP dib;

    int mouse_x;
    int mouse_y;
    int mouse_z;
    unsigned int mouse_button;

    TLS()
      :mouse_x(0)
      , mouse_y(0)
      , mouse_z(0)
      , mouse_button(0)
    {}
    void UpdateMousePos(HWND hWnd)
    {
      POINT pos;
      GetCursorPos(&pos);
      ScreenToClient(hWnd, &pos);
      mouse_x = pos.x;
      mouse_y = pos.y;
      mouse_z = 0;
    }
    void SendMouse()
    {
      uint8_t* context;
      pipe_->from->Lock(&context, sizeof(ThreadMessageMouse));
      new(context)ThreadMessageMouse();
      ThreadMessageMouse* tmm = reinterpret_cast<ThreadMessageMouse*>(context);
      tmm->button = mouse_button;
      tmm->x = mouse_x;
      tmm->y = mouse_y;
      tmm->z = mouse_z;
      pipe_->from->Unlock();
    }
  };


  static LRESULT APIENTRY ViewportWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    static POINT mouse;
    static bool down = false;
    TLS* tls = (TLS*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    auto update = [&](HDC dc) {
      if (tls->frontbuffer && tls->param.using_framebuffer) {
        auto bmp = CreateCompatibleDC(dc);
        auto old = (HBITMAP)SelectObject(bmp, tls->dib);
        for (int y = 0; y < tls->param.height; y++) {
          for (int x = 0; x < tls->param.width; x++) {
            tls->frontbuffer[(y * tls->param.width + x) * 4 + 0] = tls->backbuffer[(y * tls->param.width + x) * 3 + 2];
            tls->frontbuffer[(y * tls->param.width + x) * 4 + 1] = tls->backbuffer[(y * tls->param.width + x) * 3 + 1];
            tls->frontbuffer[(y * tls->param.width + x) * 4 + 2] = tls->backbuffer[(y * tls->param.width + x) * 3 + 0];
            tls->frontbuffer[(y * tls->param.width + x) * 4 + 3] = 0;
          }
        }
        if (tls->param.scale_framebuffer == 1) {
          BitBlt(dc, 0, 0, tls->param.width, tls->param.height, bmp, 0, 0, SRCCOPY);
        }
        else {
          StretchBlt(dc, 0, 0, tls->param.width * tls->param.scale_framebuffer, tls->param.height * tls->param.scale_framebuffer, bmp, 0, 0, tls->param.width, tls->param.height, SRCCOPY);
        }
        SelectObject(bmp, old);
      }
    };
    switch (message)
    {
    case WM_CREATE:
      SetTimer(hWnd, 0, 16, 0);
      break;
    case WM_DESTROY:
    {
      uint8_t* context;
      tls->pipe_->from->Lock(&context, sizeof(ThreadMessage));
      new(context)ThreadMessage(ThreadMessage::WindowClose);
      tls->pipe_->from->Unlock();
    }
    break;
    case WM_TIMER:
    {
      HDC dc = GetDC(hWnd);
      update(dc);
      ReleaseDC(hWnd, dc);
    }
    break;
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
    }
    break;
    case WM_MOUSEMOVE:
      tls->UpdateMousePos(hWnd);
      tls->SendMouse();
      if constexpr (0 && down)
      {
        POINT to;
        RECT rc;
        GetCursorPos(&to);
        GetWindowRect(hWnd, &rc);
        SetWindowPos(hWnd, HWND_TOP, rc.left + to.x - mouse.x, rc.top + to.y - mouse.y, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW | SWP_NOSIZE);
        mouse = to;
      }
      break;
    case WM_MOUSEWHEEL:
      tls->UpdateMousePos(hWnd);
      tls->mouse_z = (short)HIWORD(wParam);
      tls->SendMouse();
      break;
    case WM_LBUTTONDOWN:
      tls->UpdateMousePos(hWnd);
      tls->mouse_button |= ViewportMessage::MOUSE_L;
      tls->SendMouse();
      down = true;
      GetCursorPos(&mouse);
      break;
    case WM_LBUTTONUP:
      tls->UpdateMousePos(hWnd);
      tls->mouse_button &= ~ViewportMessage::MOUSE_L;
      tls->SendMouse();
      down = false;
      break;
    case WM_SIZE:
      if (tls)
      {
        SIZE sz;
        RECT rc;
        GetClientRect(hWnd, &rc);
        sz.cx = rc.right - rc.left;
        sz.cy = rc.bottom - rc.top;
        uint8_t* context;
        tls->pipe_->from->Lock(&context, sizeof(ThreadMessageWindowSize));
        new(context)ThreadMessageWindowSize(sz);
        tls->pipe_->from->Unlock();
      }
      break;
    case WM_CLOSE:
    {
      uint8_t* context;
      tls->pipe_->from->Lock(&context, sizeof(ThreadMessage));
      new(context)ThreadMessage(ThreadMessage::WindowCloseButton);
      tls->pipe_->from->Unlock();
    }
    break;
    case WM_IME_SETCONTEXT:
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
  }

  void ViewportThread(Pipe& thread_parameter)
  {
    TLS tls;
    tls.pipe_ = &thread_parameter;

    class MessageFromMaster
      :public Endpoint
    {
      HWND hwnd;
      std::wstring wname;
      void Open()
      {
      }
      void Close()
      {
        is_close = true;
      }
      int Transfer(const uint8_t* buffer, int length)
      {
        if (length >= sizeof(ThreadMessage))
        {
          const ThreadMessage* t = reinterpret_cast<const ThreadMessage*>(buffer);
          if (ThreadMessage::OpenWindow == t->type)
          {
            const ThreadMessageOpenWindow* tm = static_cast<const ThreadMessageOpenWindow*>(t);

            WNDCLASS wc;
            wc.hInstance = GetModuleHandle(NULL);
            wchar_t ws[1024];
            std::mbstowcs(ws, tm->ip.name, 1024);
            wname = ws;
            wc.lpszClassName = ws;
            wc.lpfnWndProc = (WNDPROC)ViewportWndProc;
            wc.style = NULL;
            wc.hIcon = LoadIcon(wc.hInstance, L"");
            wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
            wc.lpszMenuName = NULL;
            wc.cbClsExtra = NULL;
            wc.cbWndExtra = NULL;
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
            if (!RegisterClass(&wc))
            {
              uint8_t* context;
              tls->pipe_->from->Lock(&context, sizeof(ThreadMessage));
              new(context)ThreadMessage(ThreadMessage::WindowCreateFail);
              tls->pipe_->from->Unlock();
            }
            else
            {
              const int W = tm->ip.width * tm->ip.scale_framebuffer;
              const int H = tm->ip.height * tm->ip.scale_framebuffer;
              RECT rect;
              rect.left = 0;
              rect.top = 0;
              rect.right = W;
              rect.bottom = H;
              DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
              AdjustWindowRect(&rect, WindowStyle, FALSE);
              if (tm->ip.layered) {
                hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, wc.lpszClassName, wc.lpszClassName, 0, CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, wc.hInstance, NULL);
              }
              else {
                hwnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
              }
              if (!hwnd)
              {
                uint8_t* context;
                tls->pipe_->from->Lock(&context, sizeof(ThreadMessage));
                new(context)ThreadMessage(ThreadMessage::WindowCreateFail);
                tls->pipe_->from->Unlock();
              }
              else
              {
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)tls);
                ShowWindow(hwnd, SW_SHOW);
                UpdateWindow(hwnd);

                uint8_t* framebuf = nullptr;

                if (tm->ip.using_framebuffer) {
                  framebuf = new uint8_t[tm->ip.width * tm->ip.height * 3];
                  BITMAPINFOHEADER bmi;
                  memset(&bmi, 0, sizeof(bmi));
                  bmi.biSize = sizeof(bmi);
                  bmi.biWidth = tm->ip.width;
                  bmi.biHeight = -tm->ip.height;
                  bmi.biBitCount = 32;
                  bmi.biPlanes = 1;
                  bmi.biSizeImage = tm->ip.width * tm->ip.height * 4;
                  tls->dib = CreateDIBSection(NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&tls->frontbuffer, NULL, 0);
                  for (int y = 0; y < tm->ip.height; y++) {
                    for (int x = 0; x < tm->ip.width; x++) {
                      uint8_t c = ((x / 32 + y / 32) % 2) ? 80 : 120;
                      framebuf[(y * tm->ip.width + x) * 3 + 0] = c;
                      framebuf[(y * tm->ip.width + x) * 3 + 1] = c*2;
                      framebuf[(y * tm->ip.width + x) * 3 + 2] = c*2;
                    }
                  }
                }
                tls->param = tm->ip;
                tls->backbuffer = framebuf;
                uint8_t* context;
                tls->pipe_->from->Lock(&context, sizeof(ThreadMessageWindowCreated));
                new(context)ThreadMessageWindowCreated(hwnd, framebuf);
                tls->pipe_->from->Unlock();
              }
            }
          }
          else if (ThreadMessage::CloseWindow == t->type)
          {
            DestroyWindow(hwnd);
            UnregisterClass(wname.c_str(), GetModuleHandle(0));
          }
          t->~ThreadMessage();
        }
        return length;
      }
    public:
      bool is_close;
      TLS* tls;
    }mfm;
    mfm.is_close = false;
    mfm.tls = &tls;

    tls.pipe_->to->SetNext(&mfm);
    tls.pipe_->from->Open();
    MSG msg;
    while (!mfm.is_close)
    {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      tls.pipe_->to->Fetch();
      Sleep(1);
    }
    tls.pipe_->from->Close();
  }

}