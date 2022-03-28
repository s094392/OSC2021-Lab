#include "stdio.h"
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

void shell() {
  printf("# ");
  char cmd[256];
  read_string(cmd);
  int pid = getpid();
  if (!strcmp(cmd, "pid")) {
  } else if (!strcmp(cmd, "help")) {
    printf("This is user process\n");
  }
}

int main() {
  int pid = getpid();
  printf("Pid is: %d\n", pid);

  while (1)
    shell();
}
