// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "codec.h"

#include <cassert>
#include <cstdio>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Frames are of 70 bits encoded with 9 bytes so the two MSB are not valid (9 bytes * 8 bits/byte - 2 bits = 70 bits).

  bit 69              64 63               40 39                              8 7                            0
  +------------------------------------------------------------------------------------------------------+
  | OPCODE (6 bits) | ADDRESS (24 bits) | DATA (32 bits)                  | CRC (8 bits)                 |
  +------------------------------------------------------------------------------------------------------+

*/
int fpga_link1::Encoder(Frame& f, SerializedFrame* s) {

      // Address space is only 24 bits, so do not assign the upper (MSB) 8 bits of address.
      assert((f.address & 0xff000000) == 0);
      
      switch (f.type) {          
      case kWrite32:
            s->data[0] = f.type;
            s->data[1] = (f.address & 0x00ff0000) >> 16;
            s->data[2] = (f.address & 0x0000ff00) >> 8;
            s->data[3] = (f.address & 0x000000ff) >> 0;
            s->data[4] = (f.data32  & 0xff000000) >> 24;
            s->data[5] = (f.data32  & 0x00ff0000) >> 16;
            s->data[6] = (f.data32  & 0x0000ff00) >> 8;
            s->data[7] = (f.data32  & 0x000000ff) >> 0;
            break;
      case kRead32:
            s->data[0] = f.type;
            s->data[1] = (f.address & 0x00ff0000) >> 16;
            s->data[2] = (f.address & 0x0000ff00) >> 8;
            s->data[3] = (f.address & 0x000000ff) >> 0;
            s->data[4] = 0x00000000;
            s->data[5] = 0x00000000;
            s->data[6] = 0x00000000;
            s->data[7] = 0x00000000;
            break;
      default:
            assert(false);
      }
  
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fpga_link1::Decoder(Frame* cmd, SerializedFrame& s) {

      switch (s.data[0]) {
      case kRead32:
      case kWrite32:
            cmd->type    = static_cast<FrameType>(s.data[0]);
            cmd->address = s.data[1] << 16 | s.data[2] <<  8 | s.data[3] << 0;
            cmd->data32  = s.data[4] << 24 | s.data[5] << 16 | s.data[6] << 8 | s.data[7] << 0;
            break;
      case kPing:
      case kPingAck:
            cmd->type    = static_cast<FrameType>(s.data[0]);
            cmd->address = s.data[1] << 16 | s.data[2] <<  8 | s.data[3] << 0;
            cmd->data32  = s.data[4] << 24 | s.data[5] << 16 | s.data[6] << 8 | s.data[7] << 0;
            break;
      default:
            assert(false);
      }

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
