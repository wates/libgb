#include "viewport.h"

#include <chrono>
#include <functional>
#include <vector>

using namespace gh;

const int font[128][16] = { {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{96,96,96,96,96,96,96,96,0,96,96,96,0,0,0,0},{472,408,144,144,0,0,0,0,0,0,0,0,0,0,0,0},{416,432,496,1016,504,144,144,504,508,216,216,216,0,0,0,0},{480,504,408,408,24,120,240,448,384,384,504,248,96,32,0,0},{56,108,108,108,312,480,112,472,864,864,864,448,0,0,0,0},{112,248,216,216,248,120,504,504,460,396,504,496,0,0,0,0},{96,96,96,96,0,0,0,0,0,0,0,0,0,0,0,0},{192,192,96,96,96,96,48,32,96,96,96,96,192,128,0,0},{48,48,96,96,96,96,224,96,96,96,96,32,48,16,0,0},{96,96,504,240,240,144,144,0,0,0,0,0,0,0,0,0},{0,0,0,96,96,96,504,1020,96,96,96,96,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,96,96,48,48,48,0},{0,0,0,0,0,0,240,240,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,96,240,96,0,0,0,0},{384,384,384,192,192,96,96,48,48,48,24,24,8,0,0,0},{112,504,408,408,408,1020,1020,412,408,408,504,240,0,0,0,0},{192,224,240,216,200,192,192,192,192,192,192,192,0,0,0,0},{112,504,448,384,384,448,192,224,112,56,504,504,0,0,0,0},{504,504,448,224,112,240,496,384,384,384,504,248,0,0,0,0},{128,448,480,480,496,432,408,408,1020,1020,448,448,0,0,0,0},{504,504,24,24,24,504,408,384,384,384,504,248,0,0,0,0},{192,240,56,24,248,504,412,924,924,408,504,240,0,0,0,0},{504,504,384,384,192,192,224,96,96,112,48,48,0,0,0,0},{240,504,924,908,408,248,504,408,780,924,504,240,0,0,0,0},{240,504,408,412,924,920,504,504,384,448,240,48,0,0,0,0},{0,0,0,0,96,112,96,0,0,96,240,96,0,0,0,0},{0,0,0,0,96,224,96,0,0,0,96,112,48,48,16,0},{0,0,0,256,448,240,56,24,56,224,384,256,0,0,0,0},{0,0,0,0,0,504,0,0,504,504,0,0,0,0,0,0},{0,0,0,8,56,240,448,384,448,112,24,8,0,0,0,0},{240,508,896,896,384,192,96,96,0,32,112,112,0,0,0,0},{0,0,240,440,264,488,364,300,300,364,488,24,504,224,0,0},{96,96,240,240,240,144,408,408,504,408,392,780,0,0,0,0},{120,504,408,408,408,248,504,408,920,408,504,248,0,0,0,0},{480,496,440,408,408,24,24,24,24,24,496,480,0,0,0,0},{120,248,472,408,408,920,920,408,408,408,504,248,0,0,0,0},{504,504,24,24,24,504,504,24,24,24,504,504,0,0,0,0},{504,504,24,24,24,504,504,24,24,24,24,24,0,0,0,0},{480,496,56,24,28,476,476,412,408,408,504,496,0,0,0,0},{408,408,408,408,408,504,504,408,408,408,408,408,0,0,0,0},{504,504,96,96,96,96,96,96,96,96,504,504,0,0,0,0},{240,504,448,448,448,448,448,448,448,192,248,120,0,0,0,0},{280,408,408,216,216,120,120,216,216,408,408,408,0,0,0,0},{24,24,24,24,24,24,24,24,24,24,504,504,0,0,0,0},{408,504,504,504,504,504,504,360,360,392,408,408,0,0,0,0},{408,440,440,440,440,440,504,472,472,472,472,472,0,0,0,0},{112,504,408,408,408,924,924,412,408,408,504,240,0,0,0,0},{120,504,408,920,920,408,504,248,24,24,24,24,0,0,0,0},{112,504,408,408,408,924,924,412,408,408,504,496,896,768,0,0},{120,504,408,408,408,408,504,248,216,216,408,408,0,0,0,0},{480,504,408,408,24,120,240,448,384,384,504,248,0,0,0,0},{504,504,96,96,96,96,96,96,96,96,96,96,0,0,0,0},{408,408,408,408,408,408,408,408,408,408,504,240,0,0,0,0},{268,780,408,408,408,408,144,240,240,240,112,96,0,0,0,0},{780,780,876,876,876,360,504,504,504,504,504,408,0,0,0,0},{392,408,408,240,240,240,240,240,240,408,408,408,0,0,0,0},{264,408,408,408,240,240,96,96,96,96,96,96,0,0,0,0},{504,504,448,192,192,96,96,112,48,56,504,504,0,0,0,0},{496,48,48,48,48,48,48,48,48,48,48,48,496,240,0,0},{24,24,24,48,48,96,96,192,192,192,384,384,256,0,0,0},{248,192,192,192,192,192,192,192,192,192,192,192,248,240,0,0},{96,240,240,240,408,408,408,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,1020,1020,0},{48,48,96,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,504,392,384,504,408,408,504,496,0,0,0,0},{24,24,24,24,504,408,408,408,408,408,504,248,0,0,0,0},{0,0,0,0,496,504,408,24,24,24,504,496,0,0,0,0},{384,384,384,384,504,408,408,408,408,408,504,496,0,0,0,0},{0,0,0,0,240,408,408,504,504,24,504,496,0,0,0,0},{480,480,96,96,504,504,96,96,96,96,96,96,0,0,0,0},{0,0,0,0,504,408,408,408,408,408,504,480,384,224,120,0},{24,24,24,24,504,408,408,408,408,408,408,408,0,0,0,0},{224,224,64,0,248,224,192,192,192,192,192,192,0,0,0,0},{224,224,64,0,248,224,192,192,192,192,192,192,192,112,120,0},{24,24,24,24,408,216,216,120,248,216,472,408,0,0,0,0},{120,120,96,96,96,96,96,96,96,96,480,480,0,0,0,0},{0,0,0,0,508,1020,876,876,876,876,876,876,0,0,0,0},{0,0,0,0,504,408,408,408,408,408,408,408,0,0,0,0},{0,0,0,0,248,504,408,408,408,408,504,240,0,0,0,0},{0,0,0,0,248,408,408,408,408,408,504,248,24,24,24,0},{0,0,0,0,504,408,408,408,408,408,504,496,384,384,384,0},{0,0,0,0,496,112,48,48,48,48,48,48,0,0,0,0},{0,0,0,0,504,408,24,120,480,384,504,248,0,0,0,0},{0,48,48,112,508,112,48,48,48,48,496,480,0,0,0,0},{0,0,0,0,408,408,408,408,408,408,504,496,0,0,0,0},{0,0,0,0,408,408,408,408,240,240,240,96,0,0,0,0},{0,0,0,0,876,876,876,504,504,504,504,408,0,0,0,0},{0,0,0,0,408,408,240,240,240,240,408,408,0,0,0,0},{0,0,0,0,408,408,408,400,240,240,224,96,96,112,56,0},{0,0,0,0,504,448,192,96,112,48,504,504,0,0,0,0},{480,96,96,96,96,48,56,56,96,96,96,96,480,448,0,0},{96,96,96,96,96,96,96,96,96,96,96,96,96,96,96,0},{120,96,96,96,96,192,448,448,96,96,96,96,120,56,0,0},{0,0,0,0,0,0,824,1020,460,0,0,0,0,0,0,0},{} };
struct Canvas {
  uint8_t* rgb = 0;
  int width = 768;
  int height = 512;
  void Putc(int x, int y, char c, uint32_t color) {
    if (c < 33 || c >= 127)return;
    for (int v = 0; v < 16; v++) {
      for (int u = 0; u < 16; u++) {
        if ((font[c][v] >> u) % 2) {
          rgb[((y + v) * width + x + u) * 3 + 0] = 0xff & (color >> 16);
          rgb[((y + v) * width + x + u) * 3 + 1] = 0xff & (color >> 8);
          rgb[((y + v) * width + x + u) * 3 + 2] = 0xff & (color);
        }
      }
    }
  }
  void String(int x, int y, const char* s, uint32_t color) {
    for (int i = 0; i < strlen(s); i++) {
      Putc(x + i * 9, y, s[i], color);
    }
  }
  void Rect(int left, int top, int right, int bottom, uint32_t color) {
    for (int y = top; y < bottom; y++) {
      for (int x = left; x < right; x++) {
        rgb[(y * width + x) * 3 + 0] = 0xff & (color >> 16);
        rgb[(y * width + x) * 3 + 1] = 0xff & (color >> 8);
        rgb[(y * width + x) * 3 + 2] = 0xff & (color);
      }
    }
  }
  void ByteBlt(int x, int y, int w, int h, const uint8_t* bytes) {
    for (int v = 0; v < h; v++) {
      for (int u = 0; u < w; u++) {
        rgb[((y + v) * width + (x + u)) * 3 + 0] = bytes[v * w + u];
        rgb[((y + v) * width + (x + u)) * 3 + 1] = bytes[v * w + u];
        rgb[((y + v) * width + (x + u)) * 3 + 2] = bytes[v * w + u];
      }
    }
  }
  uint8_t pal[4] = { 0xff, 0xbb, 0x88, 0x44 };
  void IndexBlt(int x, int y, int w, int h, const uint8_t* bytes) {
    for (int v = 0; v < h; v++) {
      for (int u = 0; u < w; u++) {
        rgb[((y + v) * width + (x + u)) * 3 + 0] = pal[bytes[v * w + u]];
        rgb[((y + v) * width + (x + u)) * 3 + 1] = pal[bytes[v * w + u]];
        rgb[((y + v) * width + (x + u)) * 3 + 2] = pal[bytes[v * w + u]];
      }
    }
  }

}f;

struct VP :public ViewportMessage {
  void CreateFail() {

  }
  void CloseButton() {
    is_close = true;
  }
  void Mouse(int x, int y, int z, unsigned int button) {
    if (button & ViewportMessage::MOUSE_L) {
      //ltouch = true;
    }
    else {
      //ltouch = false;
    }
  }
  void Open() {
    is_open = true;
  }
  void Close() {
    is_close = true;
  }
  void Resize(int w, int h) {
  }
  bool is_open = false;
  bool is_close = false;
  gh::Viewport* vp;
};

#include "libgb.h"

#include "bincc/bincc.h"
#include <sstream>

#include <Windows.h>
#include <WinUser.h>
#pragma comment(lib,"winmm.lib")

gb::Machine* vm;

void FetchTile(int index, uint8_t* u88) {
  int p = 0x8000 + index * 16;
  for (int l = 0; l < 8; l++) {
    int low = vm->read(p++);
    int high = vm->read(p++);
    for (int x = 0; x < 8; x++) {
      int g = ((low >> (7 - x)) & 1) + (((high >> (7 - x)) & 1) << 1);
      u88[l * 8 + x] = g;
    }
  }
}

void FetchBgTile(int index, uint8_t* u88) {
  uint8_t lcdc = vm->read(0xff40);
  uint8_t pal = vm->read(0xff47);
  int c[4] = { pal & 3,(pal >> 2) & 3,(pal >> 4) & 3 ,pal >> 6 };
  if ((lcdc & 0x10) == 0 && index < 128) {
    index += 256;
  }

  int p = 0x8000 + index * 16;
  for (int l = 0; l < 8; l++) {
    int low = vm->read(p++);
    int high = vm->read(p++);
    for (int x = 0; x < 8; x++) {
      int g = ((low >> (7 - x)) & 1) + (((high >> (7 - x)) & 1) << 1);
      u88[l * 8 + x] = g;
    }
  }
}

void FetchTileLine(int index, int y, uint8_t* u8) {
  int low = vm->read(0x8000 + index * 16 + y * 2);
  int high = vm->read(0x8000 + index * 16 + y * 2 + 1);
  for (int x = 0; x < 8; x++) {
    int g = ((low >> (7 - x)) & 1) + (((high >> (7 - x)) & 1) << 1);
    u8[x] = g;
  }
}

void AllSprite(uint8_t* u160x144) {
  memset(u160x144, 0, 160 * 144);
  uint8_t lcdc = vm->read(0xff40);
  uint8_t sp[8 * 8 * 2];
  for (int i = 0; i < 40; i++) {
    uint8_t oy = vm->read(0xfe00 + i * 4 + 0);
    uint8_t ox = vm->read(0xfe00 + i * 4 + 1) - 8;
    uint8_t tile = vm->read(0xfe00 + i * 4 + 2);
    uint8_t flag = vm->read(0xfe00 + i * 4 + 3);
    FetchTile(tile, sp);
    FetchTile(tile + 1, sp + 64);
    for (int y = 0; y < (lcdc & 0x40 ? 16 : 8); y++) {
      for (int x = 0; x < 8; x++) {
        if ((oy + y - 16) < 0 || (oy + y - 16) >= 144 || (ox + x) < 0 || (ox + x) >= 160)continue;
        u160x144[(oy + y - 16) * 160 + (ox + x)] = sp[y * 8 + x];
      }
    }
  }
}

int main(int, char**)
{
  VP m;
  ViewportInitializeParameter p;
  p.using_framebuffer = true;
  p.height = f.height;
  p.width = f.width;
  p.scale_framebuffer = 2;

  m.vp = CreateViewport(p, &m);
  while (!m.is_open) {
    m.vp->MessageFetch();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  using namespace app_bincc_data;
  vm = gb::CreateMachine();
  vm->setCartridge(cpu_instrs_gb, cpu_instrs_gb_size);
  //vm->setCartridge(dmg_acid2_gb, dmg_acid2_gb_size);

  uint8_t* rgb = m.vp->Framebuffer();
  f.rgb = rgb;
  bool debug_step = false;
  bool debug_step_trigger = false;
  int count = 0;
  uint32_t start = timeGetTime();
  while (!m.is_close) {
    m.vp->MessageFetch();
    if (GetAsyncKeyState('B')) {
      debug_step = true;
      vm->updateNextFrame();
      vm->updateNextFrame();
    }
    if (!debug_step_trigger && GetAsyncKeyState('N')) {
      debug_step = true;
      vm->updateNextFrame();
    }
    debug_step_trigger = GetAsyncKeyState('N');
    if (GetAsyncKeyState('M')) {
      debug_step = false;
    }
    if (GetAsyncKeyState('R')) {
      vm->reset();
    }

    int pad = 0;
    if (GetAsyncKeyState('E')) {
      pad |= gb::PAD_UP;
    }
    if (GetAsyncKeyState('D')) {
      pad |= gb::PAD_DOWN;
    }
    if (GetAsyncKeyState('S')) {
      pad |= gb::PAD_LEFT;
    }
    if (GetAsyncKeyState('F')) {
      pad |= gb::PAD_RIGHT;
    }
    if (GetAsyncKeyState(VK_SHIFT)) {
      pad |= gb::PAD_SELECT;
    }
    if (GetAsyncKeyState(VK_SPACE)) {
      pad |= gb::PAD_START;
    }
    if (GetAsyncKeyState('J')) {
      pad |= gb::PAD_A;
    }
    if (GetAsyncKeyState('K')) {
      pad |= gb::PAD_B;
    }
    vm->updatePadState(pad);
    if (!debug_step) {
      vm->updateNextFrame();
    }

    //tile1
    for (int i = 0; i < 128; i++) {
      uint8_t tile[64];
      FetchTile(i, tile);
      f.IndexBlt(360 + (i % 8) * 8, 8 + (i / 8) * 8, 8, 8, tile);
    }
    //tile2
    for (int i = 0; i < 128; i++) {
      uint8_t tile[64];
      FetchTile(i + 128, tile);
      f.IndexBlt(360 + 72 + (i % 8) * 8, 8 + (i / 8) * 8, 8, 8, tile);
    }
    //tile3
    for (int i = 0; i < 128; i++) {
      uint8_t tile[64];
      FetchTile(i + 256, tile);
      f.IndexBlt(360 + 144 + (i % 8) * 8, 8 + (i / 8) * 8, 8, 8, tile);
    }

    //Bg
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 32; x++) {
        uint8_t tile[64];
        uint8_t i = vm->read(0x9800 + y * 32 + x);
        FetchBgTile(i, tile);
        f.IndexBlt(x * 8, 160 + y * 8, 8, 8, tile);
      }
    }
    //Window
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 32; x++) {
        uint8_t tile[64];
        uint8_t i = vm->read(0x9c00 + y * 32 + x);
        FetchBgTile(i, tile);
        f.IndexBlt(256 + x * 8, 160 + y * 8, 8, 8, tile);
      }
    }
    f.IndexBlt(8, 8, 160, 144, vm->getLcdBuffer());

    uint8_t scr[160 * 144];
    AllSprite(scr);
    f.IndexBlt(188, 8, 160, 144, scr);

    //full mem
    for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
        uint8_t c = vm->read(y * 256 + x);
        int pos = ((160 + y) * f.width + (512 + x)) * 3;

        f.rgb[pos + 0] = c << 5;
        f.rgb[pos + 1] = c << 2;
        f.rgb[pos + 2] = c;
      }
    }

    count++;
    int i = count * 1000 / 60 - (timeGetTime() - start);
    if (i > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(i));
    }
  }
  DeleteViewport(m.vp);
}

