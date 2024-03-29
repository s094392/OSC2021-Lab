#include "list.h"
void INIT_LIST_HEAD(struct list_head *list) {
  list->next = list;
  list->prev = list;
}

int list_empty(const struct list_head *head) { return head->next == head; }
void __list_add(struct list_head *new_lst, struct list_head *prev,
                struct list_head *next) {
  next->prev = new_lst;
  new_lst->next = next;
  new_lst->prev = prev;
  prev->next = new_lst;
}
void list_add(struct list_head *new_lst, struct list_head *head) {
  __list_add(new_lst, head, head->next);
}

void list_add_tail(struct list_head *new_lst, struct list_head *head) {
  __list_add(new_lst, head->prev, head);
}

void __list_del(struct list_head *prev, struct list_head *next) {
  next->prev = prev;
  prev->next = next;
}
