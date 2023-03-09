      case 0x00:exec<NOP> (4); break;
      case 0x01:exec<LD,r<BC>,d16> (12); break;
      case 0x02:exec<LD,p<BC>,r<A>> (8); break;
      case 0x03:exec<INC,r<BC>> (8); break;
      case 0x04:exec<INC,r<B>> (4); break;
      case 0x05:exec<DEC,r<B>> (4); break;
      case 0x06:exec<LD,r<B>,d8> (8); break;
      case 0x07:exec<RLCA> (4); break;
      case 0x08:exec<LD,p<a16>,r<SP>> (20); break;
      case 0x09:exec<ADD,r<HL>,r<BC>> (8); break;
      case 0x0A:exec<LD,r<A>,p<BC>> (8); break;
      case 0x0B:exec<DEC,r<BC>> (8); break;
      case 0x0C:exec<INC,r<C>> (4); break;
      case 0x0D:exec<DEC,r<C>> (4); break;
      case 0x0E:exec<LD,r<C>,d8> (8); break;
      case 0x0F:exec<RRCA> (4); break;
      case 0x10:exec<STOP,d8> (4); break;
      case 0x11:exec<LD,r<DE>,d16> (12); break;
      case 0x12:exec<LD,p<DE>,r<A>> (8); break;
      case 0x13:exec<INC,r<DE>> (8); break;
      case 0x14:exec<INC,r<D>> (4); break;
      case 0x15:exec<DEC,r<D>> (4); break;
      case 0x16:exec<LD,r<D>,d8> (8); break;
      case 0x17:exec<RLA> (4); break;
      case 0x18:exec<JR,s8> (12); break;
      case 0x19:exec<ADD,r<HL>,r<DE>> (8); break;
      case 0x1A:exec<LD,r<A>,p<DE>> (8); break;
      case 0x1B:exec<DEC,r<DE>> (8); break;
      case 0x1C:exec<INC,r<E>> (4); break;
      case 0x1D:exec<DEC,r<E>> (4); break;
      case 0x1E:exec<LD,r<E>,d8> (8); break;
      case 0x1F:exec<RRA> (4); break;
      case 0x20:exec<JR,NZ,s8> (12); break;// 8
      case 0x21:exec<LD,r<HL>,d16> (12); break;
      case 0x22:exec<LD,p<HL>,r<A>> (8); break;
      case 0x23:exec<INC,r<HL>> (8); break;
      case 0x24:exec<INC,r<H>> (4); break;
      case 0x25:exec<DEC,r<H>> (4); break;
      case 0x26:exec<LD,r<H>,d8> (8); break;
      case 0x27:exec<DAA> (4); break;
      case 0x28:exec<JR,Z,s8> (12); break;// 8
      case 0x29:exec<ADD,r<HL>,r<HL>> (8); break;
      case 0x2A:exec<LD,r<A>,p<HL>> (8); break;
      case 0x2B:exec<DEC,r<HL>> (8); break;
      case 0x2C:exec<INC,r<L>> (4); break;
      case 0x2D:exec<DEC,r<L>> (4); break;
      case 0x2E:exec<LD,r<L>,d8> (8); break;
      case 0x2F:exec<CPL> (4); break;
      case 0x30:exec<JR,NC,s8> (12); break;// 8
      case 0x31:exec<LD,r<SP>,d16> (12); break;
      case 0x32:exec<LD,p<HL>,r<A>> (8); break;
      case 0x33:exec<INC,r<SP>> (8); break;
      case 0x34:exec<INC,p<HL>> (12); break;
      case 0x35:exec<DEC,p<HL>> (12); break;
      case 0x36:exec<LD,p<HL>,d8> (12); break;
      case 0x37:exec<SCF> (4); break;
      case 0x38:exec<JR,r<C>,s8> (12); break;// 8
      case 0x39:exec<ADD,r<HL>,r<SP>> (8); break;
      case 0x3A:exec<LD,r<A>,p<HL>> (8); break;
      case 0x3B:exec<DEC,r<SP>> (8); break;
      case 0x3C:exec<INC,r<A>> (4); break;
      case 0x3D:exec<DEC,r<A>> (4); break;
      case 0x3E:exec<LD,r<A>,d8> (8); break;
      case 0x3F:exec<CCF> (4); break;
      case 0x40:exec<LD,r<B>,r<B>> (4); break;
      case 0x41:exec<LD,r<B>,r<C>> (4); break;
      case 0x42:exec<LD,r<B>,r<D>> (4); break;
      case 0x43:exec<LD,r<B>,r<E>> (4); break;
      case 0x44:exec<LD,r<B>,r<H>> (4); break;
      case 0x45:exec<LD,r<B>,r<L>> (4); break;
      case 0x46:exec<LD,r<B>,p<HL>> (8); break;
      case 0x47:exec<LD,r<B>,r<A>> (4); break;
      case 0x48:exec<LD,r<C>,r<B>> (4); break;
      case 0x49:exec<LD,r<C>,r<C>> (4); break;
      case 0x4A:exec<LD,r<C>,r<D>> (4); break;
      case 0x4B:exec<LD,r<C>,r<E>> (4); break;
      case 0x4C:exec<LD,r<C>,r<H>> (4); break;
      case 0x4D:exec<LD,r<C>,r<L>> (4); break;
      case 0x4E:exec<LD,r<C>,p<HL>> (8); break;
      case 0x4F:exec<LD,r<C>,r<A>> (4); break;
      case 0x50:exec<LD,r<D>,r<B>> (4); break;
      case 0x51:exec<LD,r<D>,r<C>> (4); break;
      case 0x52:exec<LD,r<D>,r<D>> (4); break;
      case 0x53:exec<LD,r<D>,r<E>> (4); break;
      case 0x54:exec<LD,r<D>,r<H>> (4); break;
      case 0x55:exec<LD,r<D>,r<L>> (4); break;
      case 0x56:exec<LD,r<D>,p<HL>> (8); break;
      case 0x57:exec<LD,r<D>,r<A>> (4); break;
      case 0x58:exec<LD,r<E>,r<B>> (4); break;
      case 0x59:exec<LD,r<E>,r<C>> (4); break;
      case 0x5A:exec<LD,r<E>,r<D>> (4); break;
      case 0x5B:exec<LD,r<E>,r<E>> (4); break;
      case 0x5C:exec<LD,r<E>,r<H>> (4); break;
      case 0x5D:exec<LD,r<E>,r<L>> (4); break;
      case 0x5E:exec<LD,r<E>,p<HL>> (8); break;
      case 0x5F:exec<LD,r<E>,r<A>> (4); break;
      case 0x60:exec<LD,r<H>,r<B>> (4); break;
      case 0x61:exec<LD,r<H>,r<C>> (4); break;
      case 0x62:exec<LD,r<H>,r<D>> (4); break;
      case 0x63:exec<LD,r<H>,r<E>> (4); break;
      case 0x64:exec<LD,r<H>,r<H>> (4); break;
      case 0x65:exec<LD,r<H>,r<L>> (4); break;
      case 0x66:exec<LD,r<H>,p<HL>> (8); break;
      case 0x67:exec<LD,r<H>,r<A>> (4); break;
      case 0x68:exec<LD,r<L>,r<B>> (4); break;
      case 0x69:exec<LD,r<L>,r<C>> (4); break;
      case 0x6A:exec<LD,r<L>,r<D>> (4); break;
      case 0x6B:exec<LD,r<L>,r<E>> (4); break;
      case 0x6C:exec<LD,r<L>,r<H>> (4); break;
      case 0x6D:exec<LD,r<L>,r<L>> (4); break;
      case 0x6E:exec<LD,r<L>,p<HL>> (8); break;
      case 0x6F:exec<LD,r<L>,r<A>> (4); break;
      case 0x70:exec<LD,p<HL>,r<B>> (8); break;
      case 0x71:exec<LD,p<HL>,r<C>> (8); break;
      case 0x72:exec<LD,p<HL>,r<D>> (8); break;
      case 0x73:exec<LD,p<HL>,r<E>> (8); break;
      case 0x74:exec<LD,p<HL>,r<H>> (8); break;
      case 0x75:exec<LD,p<HL>,r<L>> (8); break;
      case 0x76:exec<HALT> (4); break;
      case 0x77:exec<LD,p<HL>,r<A>> (8); break;
      case 0x78:exec<LD,r<A>,r<B>> (4); break;
      case 0x79:exec<LD,r<A>,r<C>> (4); break;
      case 0x7A:exec<LD,r<A>,r<D>> (4); break;
      case 0x7B:exec<LD,r<A>,r<E>> (4); break;
      case 0x7C:exec<LD,r<A>,r<H>> (4); break;
      case 0x7D:exec<LD,r<A>,r<L>> (4); break;
      case 0x7E:exec<LD,r<A>,p<HL>> (8); break;
      case 0x7F:exec<LD,r<A>,r<A>> (4); break;
      case 0x80:exec<ADD,r<A>,r<B>> (4); break;
      case 0x81:exec<ADD,r<A>,r<C>> (4); break;
      case 0x82:exec<ADD,r<A>,r<D>> (4); break;
      case 0x83:exec<ADD,r<A>,r<E>> (4); break;
      case 0x84:exec<ADD,r<A>,r<H>> (4); break;
      case 0x85:exec<ADD,r<A>,r<L>> (4); break;
      case 0x86:exec<ADD,r<A>,p<HL>> (8); break;
      case 0x87:exec<ADD,r<A>,r<A>> (4); break;
      case 0x88:exec<ADC,r<A>,r<B>> (4); break;
      case 0x89:exec<ADC,r<A>,r<C>> (4); break;
      case 0x8A:exec<ADC,r<A>,r<D>> (4); break;
      case 0x8B:exec<ADC,r<A>,r<E>> (4); break;
      case 0x8C:exec<ADC,r<A>,r<H>> (4); break;
      case 0x8D:exec<ADC,r<A>,r<L>> (4); break;
      case 0x8E:exec<ADC,r<A>,p<HL>> (8); break;
      case 0x8F:exec<ADC,r<A>,r<A>> (4); break;
      case 0x90:exec<SUB,r<B>> (4); break;
      case 0x91:exec<SUB,r<C>> (4); break;
      case 0x92:exec<SUB,r<D>> (4); break;
      case 0x93:exec<SUB,r<E>> (4); break;
      case 0x94:exec<SUB,r<H>> (4); break;
      case 0x95:exec<SUB,r<L>> (4); break;
      case 0x96:exec<SUB,p<HL>> (8); break;
      case 0x97:exec<SUB,r<A>> (4); break;
      case 0x98:exec<SBC,r<A>,r<B>> (4); break;
      case 0x99:exec<SBC,r<A>,r<C>> (4); break;
      case 0x9A:exec<SBC,r<A>,r<D>> (4); break;
      case 0x9B:exec<SBC,r<A>,r<E>> (4); break;
      case 0x9C:exec<SBC,r<A>,r<H>> (4); break;
      case 0x9D:exec<SBC,r<A>,r<L>> (4); break;
      case 0x9E:exec<SBC,r<A>,p<HL>> (8); break;
      case 0x9F:exec<SBC,r<A>,r<A>> (4); break;
      case 0xA0:exec<AND,r<B>> (4); break;
      case 0xA1:exec<AND,r<C>> (4); break;
      case 0xA2:exec<AND,r<D>> (4); break;
      case 0xA3:exec<AND,r<E>> (4); break;
      case 0xA4:exec<AND,r<H>> (4); break;
      case 0xA5:exec<AND,r<L>> (4); break;
      case 0xA6:exec<AND,p<HL>> (8); break;
      case 0xA7:exec<AND,r<A>> (4); break;
      case 0xA8:exec<XOR,r<B>> (4); break;
      case 0xA9:exec<XOR,r<C>> (4); break;
      case 0xAA:exec<XOR,r<D>> (4); break;
      case 0xAB:exec<XOR,r<E>> (4); break;
      case 0xAC:exec<XOR,r<H>> (4); break;
      case 0xAD:exec<XOR,r<L>> (4); break;
      case 0xAE:exec<XOR,p<HL>> (8); break;
      case 0xAF:exec<XOR,r<A>> (4); break;
      case 0xB0:exec<OR,r<B>> (4); break;
      case 0xB1:exec<OR,r<C>> (4); break;
      case 0xB2:exec<OR,r<D>> (4); break;
      case 0xB3:exec<OR,r<E>> (4); break;
      case 0xB4:exec<OR,r<H>> (4); break;
      case 0xB5:exec<OR,r<L>> (4); break;
      case 0xB6:exec<OR,p<HL>> (8); break;
      case 0xB7:exec<OR,r<A>> (4); break;
      case 0xB8:exec<CP,r<B>> (4); break;
      case 0xB9:exec<CP,r<C>> (4); break;
      case 0xBA:exec<CP,r<D>> (4); break;
      case 0xBB:exec<CP,r<E>> (4); break;
      case 0xBC:exec<CP,r<H>> (4); break;
      case 0xBD:exec<CP,r<L>> (4); break;
      case 0xBE:exec<CP,p<HL>> (8); break;
      case 0xBF:exec<CP,r<A>> (4); break;
      case 0xC0:exec<RET,NZ> (20); break;// 8
      case 0xC1:exec<POP,r<BC>> (12); break;
      case 0xC2:exec<JP,NZ,a16> (16); break;// 12
      case 0xC3:exec<JP,a16> (16); break;
      case 0xC4:exec<CALL,NZ,a16> (24); break;// 12
      case 0xC5:exec<PUSH,r<BC>> (16); break;
      case 0xC6:exec<ADD,r<A>,d8> (8); break;
      case 0xC7:exec<RST,i8<0x00>> (16); break;
      case 0xC8:exec<RET,Z> (20); break;// 8
      case 0xC9:exec<RET> (16); break;
      case 0xCA:exec<JP,Z,a16> (16); break;// 12
      case 0xCB:exec<PREFIX> (4); break;
      case 0xCC:exec<CALL,Z,a16> (24); break;// 12
      case 0xCD:exec<CALL,a16> (24); break;
      case 0xCE:exec<ADC,r<A>,d8> (8); break;
      case 0xCF:exec<RST,i8<0x08>> (16); break;
      case 0xD0:exec<RET,NC> (20); break;// 8
      case 0xD1:exec<POP,r<DE>> (12); break;
      case 0xD2:exec<JP,NC,a16> (16); break;// 12
      case 0xD4:exec<CALL,NC,a16> (24); break;// 12
      case 0xD5:exec<PUSH,r<DE>> (16); break;
      case 0xD6:exec<SUB,d8> (8); break;
      case 0xD7:exec<RST,i8<0x10>> (16); break;
      case 0xD8:exec<RET,r<C>> (20); break;// 8
      case 0xD9:exec<RETI> (16); break;
      case 0xDA:exec<JP,r<C>,a16> (16); break;// 12
      case 0xDC:exec<CALL,r<C>,a16> (24); break;// 12
      case 0xDE:exec<SBC,r<A>,d8> (8); break;
      case 0xDF:exec<RST,i8<0x18>> (16); break;
      case 0xE0:exec<LDH,p<a8>,r<A>> (12); break;
      case 0xE1:exec<POP,r<HL>> (12); break;
      case 0xE2:exec<LD,p<C>,r<A>> (8); break;
      case 0xE5:exec<PUSH,r<HL>> (16); break;
      case 0xE6:exec<AND,d8> (8); break;
      case 0xE7:exec<RST,i8<0x20>> (16); break;
      case 0xE8:exec<ADD,r<SP>,s8> (16); break;
      case 0xE9:exec<JP,r<HL>> (4); break;
      case 0xEA:exec<LD,p<a16>,r<A>> (16); break;
      case 0xEE:exec<XOR,d8> (8); break;
      case 0xEF:exec<RST,i8<0x28>> (16); break;
      case 0xF0:exec<LDH,r<A>,p<a8>> (12); break;
      case 0xF1:exec<POP,r<AF>> (12); break;
      case 0xF2:exec<LD,r<A>,p<C>> (8); break;
      case 0xF3:exec<DI> (4); break;
      case 0xF5:exec<PUSH,r<AF>> (16); break;
      case 0xF6:exec<OR,d8> (8); break;
      case 0xF7:exec<RST,i8<0x30>> (16); break;
      case 0xF8:exec<LD,r<HL>,r<SP>> (12); break;
      case 0xF9:exec<LD,r<SP>,r<HL>> (8); break;
      case 0xFA:exec<LD,r<A>,p<a16>> (16); break;
      case 0xFB:exec<EI> (4); break;
      case 0xFE:exec<CP,d8> (8); break;
      case 0xFF:exec<RST,i8<0x38>> (16); break;