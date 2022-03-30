#include "mbox.h"

#include "gpio.h"
#include "img.h"
#include "stdio.h"

/* mailbox message buffer */

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */

int mbox_call(unsigned char ch, unsigned int *mbox) {
  unsigned int r = (((unsigned int)((unsigned long)mbox) & ~0xF) |
                    (ch & 0xF)); /* wait until we can write to the mailbox */
  do {
    asm volatile("nop");
  } while (*MBOX_STATUS & MBOX_FULL);
  /* write the address of our message to the mailbox with channel identifier
   */
  *MBOX_WRITE = r;
  /* now wait for the response */
  while (1) {
    /* is there a response? */
    do {
      asm volatile("nop");
    } while (*MBOX_STATUS & MBOX_EMPTY);
    /* is it a response to our message? */
    if (r == *MBOX_READ) /* is it a valid successful response? */
      return mbox[1] == MBOX_RESPONSE;
  }
  return 0;
}

void get_memory() {
  unsigned int __attribute__((aligned(16))) mbox[36];
  mbox[0] = 8 * 4; // buffer size in bytes
  mbox[1] = REQUEST_CODE;
  // tags begin
  mbox[2] = GET_MEMORY; // tag identifier
  mbox[3] = 8; // maximum of request and response value buffer's length.
  mbox[4] = TAG_REQUEST_CODE;
  mbox[5] = 0; // value buffer
  mbox[6] = 0; // value buffer
  // tags end
  mbox[7] = END_TAG;

  mbox_call(MBOX_CH_PROP,
            mbox); // message passing procedure call, you should implement
                   // it following the 6 steps provided above.

  printf("Memory: %x\n", mbox[5]);
  printf("Size: %x\n", mbox[6]);
}

void get_board_revision() {
  unsigned int __attribute__((aligned(16))) mbox[36];
  mbox[0] = 7 * 4; // buffer size in bytes
  mbox[1] = REQUEST_CODE;
  // tags begin
  mbox[2] = GET_BOARD_REVISION; // tag identifier
  mbox[3] = 4; // maximum of request and response value buffer's length.
  mbox[4] = TAG_REQUEST_CODE;
  mbox[5] = 0; // value buffer
  // tags end
  mbox[6] = END_TAG;

  mbox_call(MBOX_CH_PROP,
            mbox); // message passing procedure call, you should implement
                   // it following the 6 steps provided above.

  printf("Board revision: %x\n", mbox[5]);
}

/**
 * Set screen resolution to 1024x768
 */
void lfb() {
  unsigned int __attribute__((aligned(16))) mbox[36];
  unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
  unsigned char *lfb;                       /* raw frame buffer address */

  mbox[0] = 35 * 4;
  mbox[1] = MBOX_REQUEST;

  mbox[2] = 0x48003; // set phy wh
  mbox[3] = 8;
  mbox[4] = 8;
  mbox[5] = 1024; // FrameBufferInfo.width
  mbox[6] = 768;  // FrameBufferInfo.height

  mbox[7] = 0x48004; // set virt wh
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = 1024; // FrameBufferInfo.virtual_width
  mbox[11] = 768;  // FrameBufferInfo.virtual_height

  mbox[12] = 0x48009; // set virt offset
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0; // FrameBufferInfo.x_offset
  mbox[16] = 0; // FrameBufferInfo.y.offset

  mbox[17] = 0x48005; // set depth
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = 32; // FrameBufferInfo.depth

  mbox[21] = 0x48006; // set pixel order
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = 1; // RGB, not BGR preferably

  mbox[25] = 0x40001; // get framebuffer, gets alignment on request
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = 4096; // FrameBufferInfo.pointer
  mbox[29] = 0;    // FrameBufferInfo.size

  mbox[30] = 0x40008; // get pitch
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0; // FrameBufferInfo.pitch

  mbox[34] = MBOX_TAG_LAST;

  // this might not return exactly what we asked for, could be
  // the closest supported resolution instead
  if (mbox_call(MBOX_CH_PROP, mbox) && mbox[20] == 32 && mbox[28] != 0) {
    mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
    width = mbox[5];        // get actual physical width
    height = mbox[6];       // get actual physical height
    pitch = mbox[33];       // get number of bytes per line
    isrgb = mbox[24];       // get the actual channel order
    lfb = (void *)((unsigned long)mbox[28]);
  } else {
    /*uart_puts("Unable to set screen resolution to 1024x768x32\n");*/
  }
  for (int i = 0; i < 64 * 7; i++) {
    printf("%d ", header_data[0][0]);
  }

  int x, y;
  while (1) {
    for (int frame_id = 0; frame_id < img_frames; frame_id++) {
      unsigned char *ptr = lfb;
      char *data = header_data[frame_id], pixel[4];

      ptr += (height - img_height) / 2 * pitch + (width - img_width) * 2;
      for (y = 0; y < img_height; y++) {
        for (x = 0; x < img_width; x++) {
          HEADER_PIXEL(data, pixel);
          // the image is in RGB. So if we have an RGB framebuffer, we
          // can copy the pixels directly, but for BGR we must swap R
          // (pixel[0]) and B (pixel[2]) channels.
          *((unsigned int *)ptr) =
              isrgb ? *((unsigned int *)&pixel)
                    : (unsigned int)(pixel[0] << 16 | pixel[1] << 8 | pixel[2]);
          ptr += 4;
        }
        ptr += pitch - img_width * 4;
      }
      for (int i = 0; i < 150000; i++)
        asm volatile("nop");
    }
  }
}
