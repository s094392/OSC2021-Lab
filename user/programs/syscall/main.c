#include "img.h"
#include "mbox.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

void read_string(char *cmd) {
  char now;
  cmd[0] = 0;
  int now_cur = 0;
  while ((now = getchar()) != '\n') {
    if ((int)now == 240) {
      continue;
    } else if (now == 127) {
      now_cur -= 1;
      if (now_cur >= 0) {
        printf("\b \b");
        now_cur -= 1;
      }
    } else {
      cmd[now_cur] = now;
      printf("%c", now);
    }
    now_cur++;
  }
  printf("\n");
  cmd[now_cur] = 0;
}

int strcmp(const char *s1, const char *s2) {
  int value;

  s1--, s2--;
  do {
    s1++, s2++;
    if (*s1 == *s2) {
      value = 0;
    } else if (*s1 < *s2) {
      value = -1;
      break;
    } else {
      value = 1;
      break;
    }
  } while (*s1 != 0 && *s2 != 0);
  return value;
}

long _strlen(char *s) {
  long n = 0;

  while (*s++)
    n++;
  return n;
}

void showimage() {
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
    puts("Unable to set screen resolution to 1024x768x32\n");
  }

  int x, y;

  while (1) {

    for (int frame_id = 0; frame_id < img_frames; frame_id++) {
      unsigned char *ptr = lfb;

      char *data = header_data + frame_id * img_width * img_height * 4,
           pixel[4];

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
      for (int i = 0; i < 8000000; i++)
        asm volatile("nop");
    }
  }
}

void shell() {
  printf("# ");
  char cmd[256];
  read_string(cmd);
  if (!strcmp(cmd, "help")) {
    printf("This is user process\n"
           "exec:\t Exec syscall.img.\n"
           "pid:\t Show pid of current process.\n"
           "fork:\t Fork a child process that plays awesome video.\n"
           "exit:\t Quit.\n");
  } else if (!strcmp(cmd, "pid")) {
    int pid = getpid();
    printf("Pid is: %d\n", pid);
  } else if (!strcmp(cmd, "exec")) {
    exec("syscall.img", 0);
  } else if (!strcmp(cmd, "fork")) {
    int child_pid = fork();
    if (child_pid == 0) {
      showimage(0);
    } else {
      printf("Child pid: %d\n", child_pid);
    }
  } else if (!strcmp(cmd, "exit")) {
    exit(0);
  } else if (!strcmp(cmd, "check")) {
    int el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    printf("Current: %d\n", el >> 2);
  } else {
    printf("Not a vaild command!\n");
  }
}

int main() {
  int pid = getpid();
  printf("Pid is: %d\n", pid);

  while (1)
    shell();
}
