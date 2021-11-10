#include "cpio.h"
#include "stdio.h"
#include "string.h"

void *cpio_addr;

size_t atol_n(char *s, size_t len, size_t base) {
    if (base == 16) {
        if (strncmp(s, "0x", 2) == 0) {
            s += 2;
        }
    }

    size_t num = 0;
    while (len && *s) {
        size_t n = 0;
        char c = *s;
        if (c >= '0' && c <= '9') {
            n = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            n = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            n = c - 'A' + 10;
        }
        num = num * base + n;
        len--;
        s++;
    }
    return num;
}

void cpio_list() {
    struct cpio_newc_header *cpio_itr = (struct cpio_newc_header *)cpio_addr;
    while (strncmp(cpio_itr->c_magic, "070701", 6) == 0) {
        unsigned long mode = atol_n(cpio_itr->c_mode, 8, 16);
        if (mode == 0 && strcmp(cpio_itr->data, "TRAILER!!!") == 0) {
            return;
        }
        printf("%s\n", cpio_itr->data);
        unsigned long filesize = atol_n(cpio_itr->c_filesize, 8, 16);
        unsigned long namesize = atol_n(cpio_itr->c_namesize, 8, 16);
        cpio_itr = (struct cpio_newc_header *)&(
            cpio_itr->data[2 + pad(namesize - 2, 4) + pad(filesize, 4)]);
    }
}

void *get_cpio_file(char *name) {
    struct cpio_newc_header *cpio_itr = (struct cpio_newc_header *)cpio_addr;
    while (strncmp(cpio_itr->c_magic, "070701", 6) == 0) {
        unsigned long mode = atol_n(cpio_itr->c_mode, 8, 16);
        if (mode == 0 && strcmp(cpio_itr->data, "TRAILER!!!") == 0) {
            return NULL;
        }
        if (strcmp(name, cpio_itr->data) == 0) {
            return (void *)cpio_itr;
        }
        unsigned long filesize = atol_n(cpio_itr->c_filesize, 8, 16);
        unsigned long namesize = atol_n(cpio_itr->c_namesize, 8, 16);
        cpio_itr = (struct cpio_newc_header *)&(
            cpio_itr->data[2 + pad(namesize - 2, 4) + pad(filesize, 4)]);
    }
    return NULL;
}

unsigned long get_file_size(const void *cpio_file) {
    return atol_n(((struct cpio_newc_header *)cpio_file)->c_filesize, 8, 16);
}

void *get_file_data(const void *cpio_file) {
    unsigned long namesize =
        atol_n(((struct cpio_newc_header *)cpio_file)->c_namesize, 8, 16);
    return &(
        ((struct cpio_newc_header *)cpio_file)->data[2 + pad(namesize - 2, 4)]);
}
