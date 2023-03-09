#include "libgb.h"

namespace gb {
  static const uint8_t LCD_STAT_HBLANK = 1 << 3;
  static const uint8_t LCD_STAT_VBLANK = 1 << 4;
  static const uint8_t LCD_STAT_OAM = 1 << 5;
  static const uint8_t LCD_STAT_LY_LYC = 1 << 6;

  static const uint8_t INT_VBLANK = 1 << 0;
  static const uint8_t INT_LCD_STAT = 1 << 1;
  static const uint8_t INT_TIMER = 1 << 2;
  static const uint8_t INT_SERIAL = 1 << 3;
  static const uint8_t INT_JOYPAD = 1 << 4;

  struct Mem {
    uint8_t work_ram[0x1000];
    uint8_t bank_ram[0x1000];
    uint8_t vram[0x2000];
    uint8_t extram_bank[4][0x2000];
    uint8_t oam[0x100];
    uint8_t io_reg[0x80];
    uint8_t hram[0x80];
    const uint8_t* rom;
    uint8_t* extram;
    size_t rom_size;
    bool enable_ram;
    int rom_bank;
    int pad;
    void Reset() {
      const uint8_t* rr = rom;
      size_t rs = rom_size;
      for (int i = 0; i < sizeof(Mem); i++) {
        (reinterpret_cast<uint8_t*>(this))[i] = 0;
      }
      rom = rr;
      rom_size = rs;
      rom_bank = 1;
      extram = extram_bank[0];
    }
    Mem() {
      Reset();
    }
    uint8_t read(int address) {
      if (address >= 0x0000 && address <= 0x3fff) {
        return rom[address];
      }
      else if (address >= 0x4000 && address <= 0x7fff) {
        return rom[(address - 0x4000) + 0x4000 * rom_bank];
      }
      else if (address >= 0x8000 && address <= 0x9fff) {
        return vram[address - 0x8000];
      }
      else if (address >= 0xa000 && address <= 0xbfff) {
        return extram[address - 0xa000];
      }
      else if (address >= 0xC000 && address <= 0xCfff) {
        return work_ram[address - 0xC000];
      }
      else if (address >= 0xD000 && address <= 0xDfff) {
        return bank_ram[address - 0xD000];
      }
      else if (address >= 0xfe00 && address <= 0xfeff) {
        return oam[address - 0xfe00];
      }
      else if (address >= 0xff00 && address <= 0xff7f) {
        if (address == 0xff00) {
          io_reg[0] &= 0xf0;
          if (io_reg[0] & 0x10) {
            io_reg[0] |= (~pad & 0x0f);
          }
          if (io_reg[0] & 0x20) {
            io_reg[0] |= ((~pad & 0xf0) >> 4);
          }
        }
        return io_reg[address - 0xFF00];
      }
      else if (address >= 0xff80 && address <= 0xffff) {
        return hram[address - 0xff80];
      }
      else {
        return 0;
      }
    }
    uint16_t read16(int address) {
      uint16_t low = read(address);
      uint16_t high = read(address + 1);
      return low | (high << 8);
    }
    void write(int address, uint8_t d) {
      if (address >= 0x0000 && address <= 0x1fff) {
        enable_ram = true;
      }
      else if (address >= 0x2000 && address <= 0x3fff) {
        rom_bank = d & 0x1f;
        if (rom_bank == 0)rom_bank = 1;
      }
      else if (address >= 0x4000 && address <= 0x5fff) {
        extram = extram_bank[d & 3];
      }
      else if (address >= 0x8000 && address <= 0x9fff) {
        vram[address - 0x8000] = d;
      }
      else if (address >= 0xa000 && address <= 0xbfff) {
        extram[address - 0xa000] = d;
      }
      else if (address >= 0xC000 && address <= 0xCfff) {
        work_ram[address - 0xC000] = d;
      }
      else if (address >= 0xD000 && address <= 0xDfff) {
        bank_ram[address - 0xD000] = d;
      }
      else if (address >= 0xfe00 && address <= 0xfeff) {
        oam[address - 0xfe00] = d;
      }
      else if (address >= 0xFF00 && address <= 0xFF7F) {
        if (address == 0xff02) {
          if (d == 0x81) {
            //printf("%c", io_reg[0x01]);
          }
        }
        else if (address == 0xff46) {
          uint16_t src = d;
          src *= 0x100;
          for (int i = 0; i < 0xa0; i++) {
            write(0xfe00 + i, read(src + i));
          }
        }
        io_reg[address - 0xFF00] = d;
      }
      else if (address >= 0xff80 && address <= 0xffff) {
        hram[address - 0xff80] = d;
      }
      else {
        //__debugbreak();
      }
    }
    void write16(int address, uint16_t d) {
      write(address, d & 0xff);
      write(address + 1, d >> 8);
    }
  };

  struct GPU {
    uint8_t lcd[160 * 144];
    int scan_obj[40];
    Mem* mem;
    int scan_line;
    int window_line;
    int scan_cycle;
    int horizontal_cycle;
    int mode;
    int num_obj;
    int frame_count;

    void Reset() {
      scan_line = 0;
      scan_cycle = 0;
      horizontal_cycle = 0;
      mode = 2;
      frame_count = 0;
      window_line = 0;
    }
    GPU() {
      Reset();
    }
    void Scan(int y) {
      uint8_t* line = &lcd[y * 160];
      uint8_t scy = mem->read(0xff42);
      uint8_t scx = mem->read(0xff43);
      uint8_t lcdc = mem->read(0xff40);
      bool lcd_enable = lcdc & 0x80;
      uint16_t window_map = lcdc & 0x40 ? 0x9c00 : 0x9800;
      bool window_enable = lcdc & 0x20;
      uint16_t bg_map = lcdc & 0x08 ? 0x9c00 : 0x9800;
      bool obj_16px = lcdc & 0x04;
      if (lcdc & 0x01) {
        uint8_t bg[256];
        uint8_t pal = mem->read(0xff47);
        int c[4] = { pal & 3,(pal >> 2) & 3,(pal >> 4) & 3 ,pal >> 6 };
        auto Fetch = [&](int index, int y, uint8_t* u8) {
          if ((lcdc & 0x10) == 0 && index < 128) {
            index += 256;
          }
          int low = mem->read(0x8000 + index * 16 + y * 2);
          int high = mem->read(0x8000 + index * 16 + y * 2 + 1);
          for (int x = 0; x < 8; x++) {
            int g = ((low >> (7 - x)) & 1) + (((high >> (7 - x)) & 1) << 1);
            u8[x] = c[g];
          }
        };

        int scan = (y + scy) & 0xff;
        for (int x = 0; x < 32; x++) {
          uint8_t tile = mem->read(bg_map + (scan / 8) * 32 + x);
          Fetch(tile, scan % 8, &bg[x * 8]);
        }
        for (int x = 0; x < 160; x++) {
          line[x] = bg[(x + scx) & 0xff];
        }
        uint8_t wy = mem->read(0xff4a);
        uint8_t wx = mem->read(0xff4b);
        if (window_enable && wy <= scan_line && wx < 166) {
          for (int x = wx - 7; x < 160; x += 8) {
            uint8_t tile = mem->read(window_map + ((window_line) / 8) * 32 + (x - wx + 7) / 8);
            uint8_t win[8];
            Fetch(tile, (window_line) % 8, win);
            for (int xx = 0; xx < 8 && x + xx < 160; xx++) {
              line[x + xx] = win[xx];
            }
          }
          window_line++;
        }
      }
      else {
        for (int x = 0; x < 160; x++) {
          line[x] = 0;
        }
      }
      if (lcdc & 0x02)
      {
        uint8_t pal0 = mem->read(0xff48);
        uint8_t pal1 = mem->read(0xff49);
        int pal[2][4] = { { 0,(pal0 >> 2) & 3,(pal0 >> 4) & 3 ,pal0 >> 6 },
          { 0,(pal1 >> 2) & 3,(pal1 >> 4) & 3 ,pal1 >> 6 } };
        for (int i = 0; i < num_obj; i++) {
          uint8_t oy = mem->read(0xfe00 + scan_obj[i] * 4 + 0) - 16;
          uint8_t ox = mem->read(0xfe00 + scan_obj[i] * 4 + 1) - 8;
          uint8_t tile = mem->read(0xfe00 + scan_obj[i] * 4 + 2);
          uint8_t flag = mem->read(0xfe00 + scan_obj[i] * 4 + 3);
          int flop = (flag & 0x40) ? 7 - (y - oy) : y - oy;
          if (obj_16px) {
            tile &= 0xfe;
            flop = (flag & 0x40) ? 15 - (y - oy) : y - oy;
          }
          int low = mem->read(0x8000 + tile * 16 + flop * 2);
          int high = mem->read(0x8000 + tile * 16 + flop * 2 + 1);
          for (int x = 0; x < 8; x++) {
            int flip = (flag & 0x20) ? x : 7 - x;
            int px = ((low >> flip) & 1) + (((high >> flip) & 1) << 1);
            if (px == 0)continue;
            if (ox + x < 0 || ox + x >= 160)continue;
            if (flag & 0x80 && line[ox + x])continue;
            line[ox + x] = pal[(flag >> 4) & 1][px];
          }
        }
      }
    }
    bool befor_stat;
    void Run(int cycles) {
      horizontal_cycle += cycles;
      uint8_t lcd_status = mem->read(0xff41);
      uint8_t lyc = mem->read(0xff45);
      bool stat_level = befor_stat;

      switch (mode) {
      case 0:
        stat_level = lcd_status & LCD_STAT_HBLANK;
        if (horizontal_cycle > 456) {
          horizontal_cycle -= 456;
          scan_line++;
          mem->write(0xff44, scan_line);
          if (scan_line == 144) {
            frame_count++;
            mode = 1;
          }
          else {
            mode = 2;
          }
        }
        break;
      case 1:
        stat_level = lcd_status & LCD_STAT_VBLANK;
        mem->write(0xff0f, mem->read(0xff0f) | INT_VBLANK);
        if (horizontal_cycle > 456) {
          horizontal_cycle -= 456;
          scan_line++;
          if (scan_line == 154) {
            scan_line = 0;
            mode = 2;
            window_line = 0;
          }
          mem->write(0xff44, scan_line);
        }
        break;
      case 2:
        stat_level = lcd_status & LCD_STAT_OAM;
        if (horizontal_cycle > 80) {
          int h = (mem->read(0xff40) & 0x04) ? 16 : 8;
          num_obj = 0;
          for (int i = 0; i < 40; i++) {
            uint8_t oy = mem->read(0xfe00 + i * 4 + 0) - 16;
            uint8_t ox = mem->read(0xfe00 + i * 4 + 1) - 16;
            if (scan_line < oy + h && scan_line >= oy) {
              if (num_obj == 10) {
                for (int j = 0; j < num_obj; j++) {
                  uint8_t tx = mem->read(0xfe00 + scan_obj[j] * 4 + 1) - 16;
                  if (ox < tx) {
                    scan_obj[j] = i;
                    break;
                  }
                }
              }
              else {
                bool samecollum = false;
                for (int j = 0; j < num_obj; j++) {
                  uint8_t tx = mem->read(0xfe00 + scan_obj[j] * 4 + 1) - 16;
                  samecollum = samecollum || tx == ox;
                }
                if (!samecollum) {
                  scan_obj[num_obj++] = i;
                }
              }
            }
          }
          mode = 3;
          Scan(scan_line);
        }
        break;
      case 3:
        if (horizontal_cycle > 252) {
          mode = 0;
        }
        break;
      }
      lcd_status &= 0xf8;
      if (scan_line == lyc) {
        stat_level = lcd_status & LCD_STAT_LY_LYC;
        lcd_status |= 0x04;
      }
      lcd_status |= mode;
      mem->write(0xff41, lcd_status);
      if (!befor_stat && stat_level) {
        mem->write(0xff0f, mem->read(0xff0f) | INT_LCD_STAT);
      }
      befor_stat = stat_level;
    }
  };

  struct CPU {
    Mem* mem;
    GPU* gpu;

    uint16_t af, bc, de, hl, sp, pc;
    bool zero, sub, halfc, carry;
    enum Register {
      A, B, C, D, E, H, L,
      AF, BC, DE, HL, SP, PC,
      HLI, HLD
    };
    bool ime;// interrupts enable
    bool halt;
    int steps;
    int timer_cycles;
    int exec_cycles;
    int last_cycles;
    int total_cycles;

    template<bool high> struct r8 {
      uint16_t* target;
      explicit r8(uint16_t* b) :target(b) {}
      uint8_t get() {
        return *(reinterpret_cast<uint8_t*>(target) + (high ? 1 : 0));
      }
      void set(uint8_t i) {
        *(reinterpret_cast<uint8_t*>(target) + (high ? 1 : 0)) = i;
      }
    };
    struct r16 {
      uint16_t* target;
      r16(uint16_t* b) :target(b) {}
      uint16_t get() {
        return *target;
      }
      void set(uint16_t i) {
        *target = i;
      }
    };

    template<Register N, typename T = void>struct r;
    template<typename T>struct r<A, T> :public r8<true> {
      r(CPU* a) :r8(&a->af) {}
    };
    template<typename T>struct r<B, T> :public r8<true> {
      r(CPU* a) :r8(&a->bc) {}
    };
    template<typename T>struct r<C, T> :public r8<false> {
      r(CPU* a) :r8(&a->bc) {}
    };
    template<typename T>struct r<D, T> :public r8<true> {
      r(CPU* a) :r8(&a->de) {}
    };
    template<typename T>struct r<E, T> :public r8<false> {
      r(CPU* a) :r8(&a->de) {}
    };
    template<typename T>struct r<H, T> :public r8<true> {
      r(CPU* a) :r8(&a->hl) {}
    };
    template<typename T>struct r<L, T> :public r8<false> {
      r(CPU* a) :r8(&a->hl) {}
    };
    template<typename T>struct r<AF, T> {
      CPU* cpu;
      r(CPU* c) :cpu(c) {}
      uint16_t get() {
        cpu->af &= 0xff00;
        cpu->af |= (cpu->zero ? 0x80 : 0) | (cpu->sub ? 0x40 : 0) | (cpu->halfc ? 0x20 : 0) | (cpu->carry ? 0x10 : 0);
        return cpu->af;
      }
      void set(uint16_t i) {
        cpu->af = i;
        cpu->zero = cpu->af & 0x80;
        cpu->sub = cpu->af & 0x40;
        cpu->halfc = cpu->af & 0x20;
        cpu->carry = cpu->af & 0x10;
      }
    };
    template<typename T>struct r<BC, T> :public r16 {
      r(CPU* a) :r16(&a->bc) {}
    };
    template<typename T>struct r<DE, T> :public r16 {
      r(CPU* a) :r16(&a->de) {}
    };
    template<typename T>struct r<HL, T> :public r16 {
      r(CPU* a) :r16(&a->hl) {}
    };
    template<typename T>struct r<SP, T> :public r16 {
      r(CPU* a) :r16(&a->sp) {}
    };
    template<Register N, typename T = void>struct p :public r<N> {
      CPU* cpu;
      p(CPU* a) :cpu(a), r<N>(a) {}
      uint8_t get() {
        uint16_t addr = r<N>::get();
        return cpu->mem->read(addr);
      }
      void set(uint8_t i) {
        uint16_t addr = r<N>::get();
        cpu->mem->write(addr, i);
      }
    };
    template<typename T>struct p<C, T> {
      CPU* cpu;
      p(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->mem->read((cpu->bc & 0xff) + 0xff00);
      }
      void set(uint8_t i) {
        cpu->mem->write((cpu->bc & 0xff) + 0xff00, i);
      }
    };
    template<typename T>struct p<HLI, T> {
      CPU* cpu;
      p(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->mem->read(cpu->hl++);
      }
      void set(uint8_t i) {
        cpu->mem->write(cpu->hl++, i);
      }
    };
    template<typename T>struct p<HLD, T> {
      CPU* cpu;
      p(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->mem->read(cpu->hl--);
      }
      void set(uint8_t i) {
        cpu->mem->write(cpu->hl--, i);
      }
    };

    uint8_t fetch() {
      return mem->read(pc++);
    }
    uint16_t fetch16() {
      uint16_t low = fetch();
      uint16_t high = fetch();
      return low + (high << 8);
    }

    struct d8 {
      CPU* cpu;
      d8(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->fetch();
      }
    };
    struct s8 {
      CPU* cpu;
      s8(CPU* c) :cpu(c) {}
      int8_t get() {
        return (int8_t)cpu->fetch();
      }
    };
    template<int I>struct i8 {
      constexpr i8(CPU*) {}
      constexpr uint8_t get() {
        return I;
      }
    };
    struct d16 {
      CPU* cpu;
      d16(CPU* c) :cpu(c) {}
      uint16_t get() {
        return cpu->fetch16();
      }
    };
    struct pa8 {
      CPU* cpu;
      pa8(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->mem->read(cpu->fetch16());
      }
      void set(uint8_t a) {
        cpu->mem->write(cpu->fetch16(), a);
      }
    };
    struct pa16 {
      CPU* cpu;
      pa16(CPU* c) :cpu(c) {}
      void set(uint16_t a) {
        cpu->mem->write16(cpu->fetch16(), a);
      }
    };
    struct a8 {
      CPU* cpu;
      a8(CPU* c) :cpu(c) {}
      uint8_t get() {
        return cpu->mem->read(0xff00 + cpu->fetch());
      }
      void set(uint8_t a) {
        cpu->mem->write(0xff00 + cpu->fetch(), a);
      }
    };
    typedef d16 a16;

    void push(uint16_t i) {
      mem->write(--sp, i >> 8);//high
      mem->write(--sp, i & 0xff);//low
    }
    uint16_t pop() {
      uint16_t low = mem->read(sp++);
      uint16_t high = mem->read(sp++);
      return low | (high << 8);
    }

    struct Z {
      template<typename ...T>constexpr Z(T...) {}
    };
    struct NZ {
      template<typename ...T>constexpr NZ(T...) {}
    };
    struct NC {
      template<typename ...T>constexpr NC(T...) {}
    };
    struct NOP {
      template<typename ...T>constexpr NOP(T...) {}
    };
    struct LD {
      template<typename Op1, typename Op2>
      LD(CPU* cpu, Op1& x, Op2& y) {
        x.set(y.get());
      }
    };
    struct LDHL {
      template<typename Op2>
      LDHL(CPU* cpu, Op2& y) {
        auto l = cpu->sp;
        auto r = y.get();
        cpu->zero = cpu->sub = false;
        cpu->halfc = (l & 15) + (r & 15) > 15;
        cpu->carry = ((l + r) & 0xff) < (l & 0xff);
        cpu->hl = cpu->sp + r;
      }
    };
    template<typename I>static constexpr I hMAX(I) {
      return ((I)-1) >> 4;
    }
    template<typename I>static constexpr I cMAX(I) {
      return (I)-1;
    }
    struct INC {
      template<typename Op1>INC(CPU* cpu, Op1& x) {
        decltype(x.get()) i = x.get() + 1;
        flag(cpu, i);
        x.set(i);
      }
      void flag(CPU* cpu, uint8_t i) {
        cpu->zero = i == 0;
        cpu->sub = false;
        cpu->halfc = (i & 15) == 0;
      }
      constexpr void flag(CPU*, uint16_t) { }
    };
    struct DEC {
      template<typename Op1>
      DEC(CPU* cpu, Op1& x) {
        decltype(x.get()) i = x.get() - 1;
        flag(cpu, i);
        x.set(i);
      }
      void flag(CPU* cpu, uint8_t i) {
        cpu->zero = i == 0;
        cpu->sub = true;
        cpu->halfc = (i & 15) == 15;
      }
      constexpr void flag(CPU*, uint16_t) { }
    };
    struct ADD {
      template<typename Op1, typename Op2>
      ADD(CPU* cpu, Op1& x, Op2& y) {
        auto l = x.get();
        auto r = y.get();
        if (cMAX(l) == 0xff)
          cpu->zero = ((l + r) & cMAX(l)) == 0;
        cpu->sub = false;
        cpu->halfc = (l & hMAX(l)) + (r & hMAX(l)) > hMAX(l);
        cpu->carry = ((int)l + r) > cMAX(l);
        x.set(l + r);
      }
      template<typename Op2>
      ADD(CPU* cpu, r<SP>& x, Op2& y) {
        auto l = x.get();
        auto r = y.get();
        cpu->zero = cpu->sub = false;
        cpu->halfc = (l & 15) + (r & 15) > 15;
        cpu->carry = ((l + r) & 0xff) < (l & 0xff);
        x.set(l + r);
      }

    };
    struct ADC {
      template<typename Op1, typename Op2>
      ADC(CPU* cpu, Op1& x, Op2& y) {
        auto l = x.get();
        auto r = y.get();
        uint8_t c = (cpu->carry ? 1 : 0);
        cpu->zero = ((l + r + c) & cMAX(l)) == 0;
        cpu->sub = false;
        cpu->halfc = (l & hMAX(l)) + (r & hMAX(l)) + c > hMAX(l);
        cpu->carry = ((int)l + r + c) > cMAX(l);
        x.set(l + r + c);
      }
    };
    struct SUB {
      template<typename Op1>
      SUB(CPU* cpu, Op1& y) {
        r<A> x(cpu);
        auto l = x.get();
        auto r = y.get();
        cpu->sub = true;
        cpu->zero = l - r == 0;
        cpu->carry = l < r;
        cpu->halfc = (l & hMAX(l)) < (r & hMAX(l));
        x.set(l - r);
      }
    };
    struct SBC {
      template<typename Op1, typename Op2>
      SBC(CPU* cpu, Op1& x, Op2& y) {
        auto l = x.get();
        auto r = y.get();
        uint8_t c = (cpu->carry ? 1 : 0);
        cpu->sub = true;
        cpu->zero = ((l - r - c) & cMAX(l)) == 0;
        cpu->carry = l < (r + c);
        cpu->halfc = (l & hMAX(l)) < ((r & hMAX(l)) + c);
        x.set(l - r - c);
      }
    };
    struct RLCA {
      RLCA(CPU* cpu) {
        r<A> x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 0x80;
        cpu->halfc = cpu->sub = cpu->zero = false;
        l = (l << 1) | (l >> 7);
        x.set(l);
      }
    };
    struct RRCA {
      RRCA(CPU* cpu) {
        r<A> x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 1;
        cpu->halfc = cpu->sub = cpu->zero = false;
        l = (l << 7) | (l >> 1);
        x.set(l);
      }
    };
    struct RLA {
      RLA(CPU* cpu) {
        r<A> x(cpu);
        uint8_t l = x.get();
        uint8_t lsb = cpu->carry ? 1 : 0;
        cpu->carry = l & 0x80;
        cpu->halfc = cpu->sub = cpu->zero = false;
        l = (l << 1) | lsb;
        x.set(l);
      }
    };
    struct RRA {
      RRA(CPU* cpu) {
        r<A> x(cpu);
        uint8_t l = x.get();
        uint8_t msb = cpu->carry ? 0x80 : 0;
        cpu->carry = l & 1;
        cpu->halfc = cpu->sub = cpu->zero = false;
        l = msb | (l >> 1);
        x.set(l);
      }
    };
    struct STOP {
      template<typename Op1>
      STOP(CPU* cpu, Op1& x) {
        x.get();
      }
    };
    struct JP {
      template<typename Op2> JP(CPU* cpu, NZ&, Op2& y) {
        if (!cpu->zero) {
          cpu->pc = y.get();
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>JP(CPU* cpu, Z&, Op2& y) {
        if (cpu->zero) {
          cpu->pc = y.get();
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>JP(CPU* cpu, NC&, Op2& y) {
        if (!cpu->carry) {
          cpu->pc = y.get();
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>JP(CPU* cpu, r<C>&, Op2& y) {
        if (cpu->carry) {
          cpu->pc = y.get();
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op1>JP(CPU* cpu, Op1& x) {
        cpu->pc = x.get();
        cpu->exec_cycles += 4;
      }
    };
    struct JR {
      template<typename Op2> JR(CPU* cpu, Z&, Op2& y) {
        if (cpu->zero) {
          jr(cpu, y);
        }
        else {
          cpu->pc++;
        }
      }
      template<typename Op2> JR(CPU* cpu, NZ&, Op2& y) {
        if (!cpu->zero) {
          jr(cpu, y);
        }
        else {
          cpu->pc++;
        }
      }
      template<typename Op2> JR(CPU* cpu, r<C>&, Op2& y) {
        if (cpu->carry) {
          jr(cpu, y);
        }
        else {
          cpu->pc++;
        }
      }
      template<typename Op2> JR(CPU* cpu, NC&, Op2& y) {
        if (!cpu->carry) {
          jr(cpu, y);
        }
        else {
          cpu->pc++;
        }
      }
      template<typename Op2>
      JR(CPU* cpu, Op2& y) {
        jr(cpu, y);
      }
      template<typename Op2>
      void jr(CPU* cpu, Op2& y) {
        int8_t dst = y.get();
        cpu->pc = cpu->pc + dst;
        cpu->exec_cycles += 4;
      }
    };
    struct CALL {
      template<typename Op2>CALL(CPU* cpu, Z&, Op2& y) {
        if (cpu->zero) {
          call(cpu, y);
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>CALL(CPU* cpu, NZ&, Op2& y) {
        if (!cpu->zero) {
          call(cpu, y);
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>CALL(CPU* cpu, r<C>&, Op2& y) {
        if (cpu->carry) {
          call(cpu, y);
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op2>CALL(CPU* cpu, NC&, Op2& y) {
        if (!cpu->carry) {
          call(cpu, y);
        }
        else {
          cpu->pc += 2;
        }
      }
      template<typename Op1>
      CALL(CPU* cpu, Op1& x) {
        call(cpu, x);
      }
      template<typename Op1>
      void call(CPU* cpu, Op1& x) {
        uint16_t landing = x.get();// increment pc
        cpu->push(cpu->pc);
        cpu->pc = landing;
        cpu->exec_cycles += 12;
      }
    };
    struct RET {
      RET(CPU* cpu, Z&) {
        if (cpu->zero) {
          ret(cpu);
        }
      }
      RET(CPU* cpu, NZ&) {
        if (!cpu->zero) {
          ret(cpu);
        }
      }
      RET(CPU* cpu, r<C>&) {
        if (cpu->carry) {
          ret(cpu);
        }
      }
      RET(CPU* cpu, NC&) {
        if (!cpu->carry) {
          ret(cpu);
        }
      }
      RET(CPU* cpu) {
        ret(cpu);
      }
      void ret(CPU* cpu) {
        cpu->pc = cpu->pop();
        cpu->exec_cycles += 12;
      }
    };
    struct RETI {
      RETI(CPU* cpu) {
        cpu->ime = true;
        cpu->pc = cpu->pop();
      }
    };
    struct RST {
      template<typename Op1>
      RST(CPU* cpu, Op1& x) {
        cpu->push(cpu->pc);
        cpu->pc = x.get();
      }
    };
    typedef RST INT;
    struct POP {
      template<typename Op1>
      POP(CPU* cpu, Op1& x) {
        x.set(cpu->pop());
      }
    };
    struct PUSH {
      template<typename Op1>
      PUSH(CPU* cpu, Op1& x) {
        cpu->push(x.get());
      }
    };
    struct DI {
      DI(CPU* cpu) {
        cpu->ime = false;
      }
    };
    struct EI {
      EI(CPU* cpu) {
        cpu->ime = true;
      }
    };
    struct HALT {
      HALT(CPU* cpu) {
        cpu->halt = true;
      }
    };
    struct AND {
      template<typename Op1>
      AND(CPU* cpu, Op1& y) {
        r<A> x(cpu);
        auto l = x.get();
        auto r = y.get();
        cpu->zero = (l & r) == 0;
        cpu->carry = cpu->sub = false;
        cpu->halfc = true;
        x.set(l & r);
      }
    };
    struct OR {
      template<typename Op1>
      OR(CPU* cpu, Op1& y) {
        r<A> x(cpu);
        auto l = x.get();
        auto r = y.get();
        cpu->zero = (l | r) == 0;
        cpu->halfc = cpu->carry = cpu->sub = false;
        x.set(l | r);
      }
    };
    struct XOR {
      template<typename Op1>
      XOR(CPU* cpu, Op1& y) {
        r<A> x(cpu);
        auto l = x.get();
        auto r = y.get();
        cpu->zero = (l ^ r) == 0;
        cpu->halfc = cpu->carry = cpu->sub = false;
        x.set(l ^ r);
      }
    };
    struct CP {
      template<typename Op1>
      CP(CPU* cpu, Op1& y) {
        r<A> x(cpu);
        auto l = x.get();
        auto r = y.get();
        cpu->sub = true;
        cpu->zero = l - r == 0;
        cpu->carry = l < r;
        cpu->halfc = (l & 15) < (r & 15);
      }
    };
    struct CPL {
      CPL(CPU* cpu) {
        r<A> x(cpu);
        cpu->sub = true;
        cpu->halfc = true;
        x.set(~x.get());
      }
    };
    typedef LD LDH;
    struct SCF {
      SCF(CPU* cpu) {
        cpu->sub = false;
        cpu->halfc = false;
        cpu->carry = true;
      }
    };
    struct CCF {
      CCF(CPU* cpu) {
        cpu->sub = false;
        cpu->halfc = false;
        cpu->carry = !cpu->carry;
      }
    };
    struct DAA {
      DAA(CPU* cpu) {
        r<A> x(cpu);
        uint8_t a = x.get();
        uint8_t c = 0;
        if (cpu->halfc || (!cpu->sub && (a & 0xf) > 9)) {
          c |= 0x6;
        }
        if (cpu->carry || (!cpu->sub && a > 0x99)) {
          c |= 0x60;
          cpu->carry = true;
        }
        a += cpu->sub ? -c : c;
        x.set(a);
        cpu->zero = a == 0;
        cpu->halfc = 0;
      }
    };

    struct PREFIX {
      template<typename Reg>void RLC(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 0x80;
        l = (l << 1) | (l >> 7);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void RRC(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 1;
        l = (l << 7) | (l >> 1);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void RL(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        uint8_t lsb = cpu->carry ? 1 : 0;
        cpu->carry = l & 0x80;
        l = (l << 1) | lsb;
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void RR(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        uint8_t msb = cpu->carry ? 0x80 : 0;
        cpu->carry = l & 1;
        l = msb | (l >> 1);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void SLA(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 0x80;
        l = (l << 1);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void SRA(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 1;
        l = (l >> 1) | (l & 0x80);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void SWAP(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        l = (l << 4) | (l >> 4);
        cpu->zero = l == 0;
        cpu->carry = cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<typename Reg>void SRL(CPU* cpu) {
        Reg x(cpu);
        uint8_t l = x.get();
        cpu->carry = l & 1;
        l = (l >> 1);
        cpu->zero = l == 0;
        cpu->halfc = cpu->sub = false;
        x.set(l);
      }
      template<int i, typename Reg>void BIT(CPU* cpu) {
        Reg p(cpu);
        cpu->zero = !(p.get() & (1 << i));
        cpu->sub = false;
        cpu->halfc = true;
      }
      template<int i, typename Reg>void RES(CPU* cpu) {
        Reg p(cpu);
        p.set(p.get() & ~(1 << i));
      }
      template<int i, typename Reg>void SET(CPU* cpu) {
        Reg p(cpu);
        p.set(p.get() | (1 << i));
      }
      template<typename Reg>void R(CPU* cpu, uint8_t op) {
        switch (op >> 3) {
        case 0x00:RLC   <Reg>(cpu); break;
        case 0x01:RRC   <Reg>(cpu); break;
        case 0x02:RL    <Reg>(cpu); break;
        case 0x03:RR    <Reg>(cpu); break;
        case 0x04:SLA   <Reg>(cpu); break;
        case 0x05:SRA   <Reg>(cpu); break;
        case 0x06:SWAP  <Reg>(cpu); break;
        case 0x07:SRL   <Reg>(cpu); break;
        case 0x08:BIT<0, Reg>(cpu); break;
        case 0x09:BIT<1, Reg>(cpu); break;
        case 0x0A:BIT<2, Reg>(cpu); break;
        case 0x0B:BIT<3, Reg>(cpu); break;
        case 0x0C:BIT<4, Reg>(cpu); break;
        case 0x0D:BIT<5, Reg>(cpu); break;
        case 0x0E:BIT<6, Reg>(cpu); break;
        case 0x0F:BIT<7, Reg>(cpu); break;
        case 0x10:RES<0, Reg>(cpu); break;
        case 0x11:RES<1, Reg>(cpu); break;
        case 0x12:RES<2, Reg>(cpu); break;
        case 0x13:RES<3, Reg>(cpu); break;
        case 0x14:RES<4, Reg>(cpu); break;
        case 0x15:RES<5, Reg>(cpu); break;
        case 0x16:RES<6, Reg>(cpu); break;
        case 0x17:RES<7, Reg>(cpu); break;
        case 0x18:SET<0, Reg>(cpu); break;
        case 0x19:SET<1, Reg>(cpu); break;
        case 0x1A:SET<2, Reg>(cpu); break;
        case 0x1B:SET<3, Reg>(cpu); break;
        case 0x1C:SET<4, Reg>(cpu); break;
        case 0x1D:SET<5, Reg>(cpu); break;
        case 0x1E:SET<6, Reg>(cpu); break;
        case 0x1F:SET<7, Reg>(cpu); break;
        }
      }

      PREFIX(CPU* cpu) {
        uint8_t op = cpu->fetch();
        switch (op & 7) {
        case 0x00:R<r<B> >(cpu, op); break;
        case 0x01:R<r<C> >(cpu, op); break;
        case 0x02:R<r<D> >(cpu, op); break;
        case 0x03:R<r<E> >(cpu, op); break;
        case 0x04:R<r<H> >(cpu, op); break;
        case 0x05:R<r<L> >(cpu, op); break;
        case 0x06:R<p<HL>>(cpu, op); break;
        case 0x07:R<r<A> >(cpu, op); break;
        }
      }
    };

    int now_pad;
    void updateJoypad(int pad_state) {
      now_pad = pad_state;
      mem->pad = pad_state;
    }

    int last_pad;
    void pad() {
      if (last_pad ^ now_pad) {
        mem->write(0xff0f, mem->read(0xff0f) | INT_JOYPAD);
      }
      last_pad = now_pad;
    }

    void timer() {
      uint8_t tac = mem->read(0xff07);
      if (0x04 & tac) {
        static const int thresthold[4] = { 1024,16,64,256 };
        if (timer_cycles >= thresthold[tac & 3]) {
          uint8_t count = mem->read(0xff05);
          if (count == 0xff) {
            count = mem->read(0xff06);
            mem->write(0xff0f, mem->read(0xff0f) | INT_TIMER);
          }
          else {
            count++;
          }
          mem->write(0xff05, count);
          timer_cycles -= thresthold[tac & 3];
        }
        timer_cycles += last_cycles;
      }
    }
    template <typename Opc, typename Op1, typename Op2>
    void exec(int cycles) {
      //int offs = (int)strlen(typeid(CPU).name()) + 2;
      //printf("%04X: %s %s %s\n", cpu->pc - 1, typeid(Opc).name() + offs, typeid(Op1).name() + offs, typeid(Op2).name() + offs);

      Op1 p1(this);
      Op2 p2(this);
      Opc code(this, p1, p2);
      exec_cycles += cycles;
    }
    template <typename Opc, typename Op1>
    void exec(int cycles) {
      //int offs = (int)strlen(typeid(CPU).name()) + 2;
      //printf("%04X: %s %s\n", cpu->pc - 1, typeid(Opc).name() + offs, typeid(Op1).name() + offs);

      Op1 p1(this);
      Opc code(this, p1);
      exec_cycles += cycles;
    }
    template <typename Opc>
    void exec(int cycles) {
      //int offs = (int)strlen(typeid(CPU).name()) + 2;
      //printf("%04X: %s\n", cpu->pc - 1, typeid(Opc).name() + offs);

      Opc code(this);
      exec_cycles += cycles;
    }

    void interrupts() {
      uint8_t enable = mem->read(0xffff);
      uint8_t requests = mem->read(0xff0f);
      if (halt && enable & requests) {
        halt = false;
      }
      if (!ime)return;
      if (INT_VBLANK & enable & requests) {
        exec < INT, i8<0x40>>(16);
        exec < DI>(4);
        requests &= ~INT_VBLANK;
      }
      else if (INT_LCD_STAT & enable & requests) {
        exec < INT, i8<0x48>>(16);
        exec < DI>(4);
        requests &= ~INT_LCD_STAT;
      }
      else if (INT_TIMER & enable & requests) {
        exec < INT, i8<0x50>>(16);
        exec < DI>(4);
        requests &= ~INT_TIMER;
      }
      else if (INT_SERIAL & enable & requests) {
        exec < INT, i8<0x58>>(16);
        exec < DI>(4);
        requests &= ~INT_SERIAL;
      }
      else if (INT_JOYPAD & enable & requests) {
        exec < INT, i8<0x60>>(16);
        exec < DI>(4);
        requests &= ~INT_JOYPAD;
      }
      mem->write(0xff0f, requests);
    }

    void decode(int step) {
      pad();
      while (step--) {
        last_cycles = exec_cycles;
        exec_cycles = 0;
        gpu->Run(last_cycles);
        timer();
        interrupts();
        if (halt) {
          exec_cycles += 4;
          continue;
        }
        uint8_t op = fetch();
        switch (op) {
        case 0x00:exec<NOP>(4); break;
        case 0x01:exec<LD, r<BC>, d16>(12); break;
        case 0x02:exec<LD, p<BC>, r<A>>(8); break;
        case 0x03:exec<INC, r<BC>>(8); break;
        case 0x04:exec<INC, r<B>>(4); break;
        case 0x05:exec<DEC, r<B>>(4); break;
        case 0x06:exec<LD, r<B>, d8>(8); break;
        case 0x07:exec<RLCA>(4); break;
        case 0x08:exec<LD, pa16, r<SP>>(20); break; //
        case 0x09:exec<ADD, r<HL>, r<BC>>(8); break;
        case 0x0A:exec<LD, r<A>, p<BC>>(8); break;
        case 0x0B:exec<DEC, r<BC>>(8); break;
        case 0x0C:exec<INC, r<C>>(4); break;
        case 0x0D:exec<DEC, r<C>>(4); break;
        case 0x0E:exec<LD, r<C>, d8>(8); break;
        case 0x0F:exec<RRCA>(4); break;
        case 0x10:exec<STOP, d8>(4); break;
        case 0x11:exec<LD, r<DE>, d16>(12); break;
        case 0x12:exec<LD, p<DE>, r<A>>(8); break;
        case 0x13:exec<INC, r<DE>>(8); break;
        case 0x14:exec<INC, r<D>>(4); break;
        case 0x15:exec<DEC, r<D>>(4); break;
        case 0x16:exec<LD, r<D>, d8>(8); break;
        case 0x17:exec<RLA>(4); break;
        case 0x18:exec<JR, s8>(8); break;
        case 0x19:exec<ADD, r<HL>, r<DE>>(8); break;
        case 0x1A:exec<LD, r<A>, p<DE>>(8); break;
        case 0x1B:exec<DEC, r<DE>>(8); break;
        case 0x1C:exec<INC, r<E>>(4); break;
        case 0x1D:exec<DEC, r<E>>(4); break;
        case 0x1E:exec<LD, r<E>, d8>(8); break;
        case 0x1F:exec<RRA>(4); break;
        case 0x20:exec<JR, NZ, s8>(8); break;
        case 0x21:exec<LD, r<HL>, d16>(12); break;
        case 0x22:exec<LD, p<HLI>, r<A>>(8); break; //
        case 0x23:exec<INC, r<HL>>(8); break;
        case 0x24:exec<INC, r<H>>(4); break;
        case 0x25:exec<DEC, r<H>>(4); break;
        case 0x26:exec<LD, r<H>, d8>(8); break;
        case 0x27:exec<DAA>(4); break;
        case 0x28:exec<JR, Z, s8>(8); break;
        case 0x29:exec<ADD, r<HL>, r<HL>>(8); break;
        case 0x2A:exec<LD, r<A>, p<HLI>>(8); break; //
        case 0x2B:exec<DEC, r<HL>>(8); break;
        case 0x2C:exec<INC, r<L>>(4); break;
        case 0x2D:exec<DEC, r<L>>(4); break;
        case 0x2E:exec<LD, r<L>, d8>(8); break;
        case 0x2F:exec<CPL>(4); break;
        case 0x30:exec<JR, NC, s8>(8); break;
        case 0x31:exec<LD, r<SP>, d16>(12); break;
        case 0x32:exec<LD, p<HLD>, r<A>>(8); break; //
        case 0x33:exec<INC, r<SP>>(8); break;
        case 0x34:exec<INC, p<HL>>(12); break;
        case 0x35:exec<DEC, p<HL>>(12); break;
        case 0x36:exec<LD, p<HL>, d8>(12); break;
        case 0x37:exec<SCF>(4); break;
        case 0x38:exec<JR, r<C>, s8>(8); break;
        case 0x39:exec<ADD, r<HL>, r<SP>>(8); break;
        case 0x3A:exec<LD, r<A>, p<HLD>>(8); break; //
        case 0x3B:exec<DEC, r<SP>>(8); break;
        case 0x3C:exec<INC, r<A>>(4); break;
        case 0x3D:exec<DEC, r<A>>(4); break;
        case 0x3E:exec<LD, r<A>, d8>(8); break;
        case 0x3F:exec<CCF>(4); break;
        case 0x40:exec<LD, r<B>, r<B>>(4); break;
        case 0x41:exec<LD, r<B>, r<C>>(4); break;
        case 0x42:exec<LD, r<B>, r<D>>(4); break;
        case 0x43:exec<LD, r<B>, r<E>>(4); break;
        case 0x44:exec<LD, r<B>, r<H>>(4); break;
        case 0x45:exec<LD, r<B>, r<L>>(4); break;
        case 0x46:exec<LD, r<B>, p<HL>>(8); break;
        case 0x47:exec<LD, r<B>, r<A>>(4); break;
        case 0x48:exec<LD, r<C>, r<B>>(4); break;
        case 0x49:exec<LD, r<C>, r<C>>(4); break;
        case 0x4A:exec<LD, r<C>, r<D>>(4); break;
        case 0x4B:exec<LD, r<C>, r<E>>(4); break;
        case 0x4C:exec<LD, r<C>, r<H>>(4); break;
        case 0x4D:exec<LD, r<C>, r<L>>(4); break;
        case 0x4E:exec<LD, r<C>, p<HL>>(8); break;
        case 0x4F:exec<LD, r<C>, r<A>>(4); break;
        case 0x50:exec<LD, r<D>, r<B>>(4); break;
        case 0x51:exec<LD, r<D>, r<C>>(4); break;
        case 0x52:exec<LD, r<D>, r<D>>(4); break;
        case 0x53:exec<LD, r<D>, r<E>>(4); break;
        case 0x54:exec<LD, r<D>, r<H>>(4); break;
        case 0x55:exec<LD, r<D>, r<L>>(4); break;
        case 0x56:exec<LD, r<D>, p<HL>>(8); break;
        case 0x57:exec<LD, r<D>, r<A>>(4); break;
        case 0x58:exec<LD, r<E>, r<B>>(4); break;
        case 0x59:exec<LD, r<E>, r<C>>(4); break;
        case 0x5A:exec<LD, r<E>, r<D>>(4); break;
        case 0x5B:exec<LD, r<E>, r<E>>(4); break;
        case 0x5C:exec<LD, r<E>, r<H>>(4); break;
        case 0x5D:exec<LD, r<E>, r<L>>(4); break;
        case 0x5E:exec<LD, r<E>, p<HL>>(8); break;
        case 0x5F:exec<LD, r<E>, r<A>>(4); break;
        case 0x60:exec<LD, r<H>, r<B>>(4); break;
        case 0x61:exec<LD, r<H>, r<C>>(4); break;
        case 0x62:exec<LD, r<H>, r<D>>(4); break;
        case 0x63:exec<LD, r<H>, r<E>>(4); break;
        case 0x64:exec<LD, r<H>, r<H>>(4); break;
        case 0x65:exec<LD, r<H>, r<L>>(4); break;
        case 0x66:exec<LD, r<H>, p<HL>>(8); break;
        case 0x67:exec<LD, r<H>, r<A>>(4); break;
        case 0x68:exec<LD, r<L>, r<B>>(4); break;
        case 0x69:exec<LD, r<L>, r<C>>(4); break;
        case 0x6A:exec<LD, r<L>, r<D>>(4); break;
        case 0x6B:exec<LD, r<L>, r<E>>(4); break;
        case 0x6C:exec<LD, r<L>, r<H>>(4); break;
        case 0x6D:exec<LD, r<L>, r<L>>(4); break;
        case 0x6E:exec<LD, r<L>, p<HL>>(8); break;
        case 0x6F:exec<LD, r<L>, r<A>>(4); break;
        case 0x70:exec<LD, p<HL>, r<B>>(8); break;
        case 0x71:exec<LD, p<HL>, r<C>>(8); break;
        case 0x72:exec<LD, p<HL>, r<D>>(8); break;
        case 0x73:exec<LD, p<HL>, r<E>>(8); break;
        case 0x74:exec<LD, p<HL>, r<H>>(8); break;
        case 0x75:exec<LD, p<HL>, r<L>>(8); break;
        case 0x76:exec<HALT>(4); break;
        case 0x77:exec<LD, p<HL>, r<A>>(8); break;
        case 0x78:exec<LD, r<A>, r<B>>(4); break;
        case 0x79:exec<LD, r<A>, r<C>>(4); break;
        case 0x7A:exec<LD, r<A>, r<D>>(4); break;
        case 0x7B:exec<LD, r<A>, r<E>>(4); break;
        case 0x7C:exec<LD, r<A>, r<H>>(4); break;
        case 0x7D:exec<LD, r<A>, r<L>>(4); break;
        case 0x7E:exec<LD, r<A>, p<HL>>(8); break;
        case 0x7F:exec<LD, r<A>, r<A>>(4); break;
        case 0x80:exec<ADD, r<A>, r<B>>(4); break;
        case 0x81:exec<ADD, r<A>, r<C>>(4); break;
        case 0x82:exec<ADD, r<A>, r<D>>(4); break;
        case 0x83:exec<ADD, r<A>, r<E>>(4); break;
        case 0x84:exec<ADD, r<A>, r<H>>(4); break;
        case 0x85:exec<ADD, r<A>, r<L>>(4); break;
        case 0x86:exec<ADD, r<A>, p<HL>>(8); break;
        case 0x87:exec<ADD, r<A>, r<A>>(4); break;
        case 0x88:exec<ADC, r<A>, r<B>>(4); break;
        case 0x89:exec<ADC, r<A>, r<C>>(4); break;
        case 0x8A:exec<ADC, r<A>, r<D>>(4); break;
        case 0x8B:exec<ADC, r<A>, r<E>>(4); break;
        case 0x8C:exec<ADC, r<A>, r<H>>(4); break;
        case 0x8D:exec<ADC, r<A>, r<L>>(4); break;
        case 0x8E:exec<ADC, r<A>, p<HL>>(8); break;
        case 0x8F:exec<ADC, r<A>, r<A>>(4); break;
        case 0x90:exec<SUB, r<B>>(4); break;
        case 0x91:exec<SUB, r<C>>(4); break;
        case 0x92:exec<SUB, r<D>>(4); break;
        case 0x93:exec<SUB, r<E>>(4); break;
        case 0x94:exec<SUB, r<H>>(4); break;
        case 0x95:exec<SUB, r<L>>(4); break;
        case 0x96:exec<SUB, p<HL>>(8); break;
        case 0x97:exec<SUB, r<A>>(4); break;
        case 0x98:exec<SBC, r<A>, r<B>>(4); break;
        case 0x99:exec<SBC, r<A>, r<C>>(4); break;
        case 0x9A:exec<SBC, r<A>, r<D>>(4); break;
        case 0x9B:exec<SBC, r<A>, r<E>>(4); break;
        case 0x9C:exec<SBC, r<A>, r<H>>(4); break;
        case 0x9D:exec<SBC, r<A>, r<L>>(4); break;
        case 0x9E:exec<SBC, r<A>, p<HL>>(8); break;
        case 0x9F:exec<SBC, r<A>, r<A>>(4); break;
        case 0xA0:exec<AND, r<B>>(4); break;
        case 0xA1:exec<AND, r<C>>(4); break;
        case 0xA2:exec<AND, r<D>>(4); break;
        case 0xA3:exec<AND, r<E>>(4); break;
        case 0xA4:exec<AND, r<H>>(4); break;
        case 0xA5:exec<AND, r<L>>(4); break;
        case 0xA6:exec<AND, p<HL>>(8); break;
        case 0xA7:exec<AND, r<A>>(4); break;
        case 0xA8:exec<XOR, r<B>>(4); break;
        case 0xA9:exec<XOR, r<C>>(4); break;
        case 0xAA:exec<XOR, r<D>>(4); break;
        case 0xAB:exec<XOR, r<E>>(4); break;
        case 0xAC:exec<XOR, r<H>>(4); break;
        case 0xAD:exec<XOR, r<L>>(4); break;
        case 0xAE:exec<XOR, p<HL>>(8); break;
        case 0xAF:exec<XOR, r<A>>(4); break;
        case 0xB0:exec<OR, r<B>>(4); break;
        case 0xB1:exec<OR, r<C>>(4); break;
        case 0xB2:exec<OR, r<D>>(4); break;
        case 0xB3:exec<OR, r<E>>(4); break;
        case 0xB4:exec<OR, r<H>>(4); break;
        case 0xB5:exec<OR, r<L>>(4); break;
        case 0xB6:exec<OR, p<HL>>(8); break;
        case 0xB7:exec<OR, r<A>>(4); break;
        case 0xB8:exec<CP, r<B>>(4); break;
        case 0xB9:exec<CP, r<C>>(4); break;
        case 0xBA:exec<CP, r<D>>(4); break;
        case 0xBB:exec<CP, r<E>>(4); break;
        case 0xBC:exec<CP, r<H>>(4); break;
        case 0xBD:exec<CP, r<L>>(4); break;
        case 0xBE:exec<CP, p<HL>>(8); break;
        case 0xBF:exec<CP, r<A>>(4); break;
        case 0xC0:exec<RET, NZ>(8); break;
        case 0xC1:exec<POP, r<BC>>(12); break;
        case 0xC2:exec<JP, NZ, a16>(12); break;
        case 0xC3:exec<JP, a16>(12); break;
        case 0xC4:exec<CALL, NZ, a16>(12); break;
        case 0xC5:exec<PUSH, r<BC>>(16); break;
        case 0xC6:exec<ADD, r<A>, d8>(8); break;
        case 0xC7:exec<RST, i8<0x00>>(16); break;
        case 0xC8:exec<RET, Z>(8); break;
        case 0xC9:exec<RET>(4); break;
        case 0xCA:exec<JP, Z, a16>(12); break;
        case 0xCB:exec<PREFIX>(4); break;
        case 0xCC:exec<CALL, Z, a16>(12); break;
        case 0xCD:exec<CALL, a16>(12); break;
        case 0xCE:exec<ADC, r<A>, d8>(8); break;
        case 0xCF:exec<RST, i8<0x08>>(16); break;
        case 0xD0:exec<RET, NC>(8); break;
        case 0xD1:exec<POP, r<DE>>(12); break;
        case 0xD2:exec<JP, NC, a16>(12); break;
        case 0xD4:exec<CALL, NC, a16>(12); break;
        case 0xD5:exec<PUSH, r<DE>>(16); break;
        case 0xD6:exec<SUB, d8>(8); break;
        case 0xD7:exec<RST, i8<0x10>>(16); break;
        case 0xD8:exec<RET, r<C>>(8); break;
        case 0xD9:exec<RETI>(16); break;
        case 0xDA:exec<JP, r<C>, a16>(12); break;
        case 0xDC:exec<CALL, r<C>, a16>(12); break;
        case 0xDE:exec<SBC, r<A>, d8>(8); break;
        case 0xDF:exec<RST, i8<0x18>>(16); break;
        case 0xE0:exec<LDH, a8, r<A>>(12); break; //
        case 0xE1:exec<POP, r<HL>>(12); break;
        case 0xE2:exec<LD, p<C>, r<A>>(8); break;
        case 0xE5:exec<PUSH, r<HL>>(16); break;
        case 0xE6:exec<AND, d8>(8); break;
        case 0xE7:exec<RST, i8<0x20>>(16); break;
        case 0xE8:exec<ADD, r<SP>, s8>(16); break;
        case 0xE9:exec<JP, r<HL>>(0); break;
        case 0xEA:exec<LD, pa8, r<A>>(16); break; //
        case 0xEE:exec<XOR, d8>(8); break;
        case 0xEF:exec<RST, i8<0x28>>(16); break;
        case 0xF0:exec<LDH, r<A>, a8>(12); break; //
        case 0xF1:exec<POP, r<AF>>(12); break;
        case 0xF2:exec<LD, r<A>, p<C>>(8); break;
        case 0xF3:exec<DI>(4); break;
        case 0xF5:exec<PUSH, r<AF>>(16); break;
        case 0xF6:exec<OR, d8>(8); break;
        case 0xF7:exec<RST, i8<0x30>>(16); break;
        case 0xF8:exec<LDHL, s8>(12); break; //
        case 0xF9:exec<LD, r<SP>, r<HL>>(8); break;
        case 0xFA:exec<LD, r<A>, pa8>(16); break; //
        case 0xFB:exec<EI>(4); break;
        case 0xFE:exec<CP, d8>(8); break;
        case 0xFF:exec<RST, i8<0x38>>(16); break;
        default:
          break;
        }
        steps++;
        total_cycles += last_cycles;
      }
    }
    void Reset(Mem* m, GPU* g) {
      m->Reset();
      g->mem = m;
      g->Reset();
      this->mem = m;
      this->gpu = g;
      af = 0x0100;
      bc = 0xff13;
      de = 0x00c1;
      hl = 0x8403;
      pc = 0x0100;
      sp = 0xfffe;
      zero = false;
      halfc = false;
      carry = false;
      steps = 0;
      timer_cycles = 0;
      last_cycles = 0;
      exec_cycles = 0;
      total_cycles = 0;
      halt = false;
      ime = false;
      last_pad = now_pad = 0;
    }
  };

  struct MachineNormal :public Machine {

    CPU cpu;
    GPU gpu;
    Mem mem;

    void reset() {
      cpu.Reset(&mem, &gpu);
    }
    MachineNormal() {
      reset();
    }

    virtual void setCartridge(const uint8_t* rom, size_t size) {
      mem.rom = rom;
      mem.rom_size = size;
    }

    virtual void updatePadState(int pad) {
      cpu.updateJoypad(pad);
    }

    virtual void updateNextFrame() {
      int start_frame = gpu.frame_count;
      while (start_frame == gpu.frame_count) {
        cpu.decode(800);
      }
    }
    virtual const uint8_t* getLcdBuffer() {
      return gpu.lcd;
    }
    virtual uint8_t read(int address) {
      return mem.read(address);
    }

  };

  Machine* CreateMachine()
  {
    return new MachineNormal;
  }

}
