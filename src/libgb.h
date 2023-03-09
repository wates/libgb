
#pragma once
#include <cstdint>
#include <cstddef>
namespace gb {

  static const int PAD_A = 1 << 0;
  static const int PAD_B = 1 << 1;
  static const int PAD_SELECT = 1 << 2;
  static const int PAD_START = 1 << 3;
  static const int PAD_RIGHT = 1 << 4;
  static const int PAD_LEFT = 1 << 5;
  static const int PAD_UP = 1 << 6;
  static const int PAD_DOWN = 1 << 7;

  static const int LCD_WIDTH = 160;
  static const int LCD_HEIGHT = 144;

  struct Machine {
    virtual void setCartridge(const uint8_t* rom, size_t size) = 0;
    virtual void reset() = 0;
    virtual void updatePadState(int pad) = 0;
    virtual void updateNextFrame() = 0;
    virtual const uint8_t* getLcdBuffer() = 0;
    virtual uint8_t read(int address) = 0;
  };

  Machine* CreateMachine();

}
