#ifndef CPIO_H
#define CPIO_H
#define pad(x, y) ((((x) + (y)-1) / (y)) * (y))

extern void *cpio_addr;

struct cpio_newc_header {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
  char data[2];
};

void cpio_list();
struct cpio_newc_header *get_cpio_file(const char *name);
unsigned long get_file_size(const struct cpio_newc_header *cpio_file);
void *get_file_data(const struct cpio_newc_header *cpio_file);

#endif
