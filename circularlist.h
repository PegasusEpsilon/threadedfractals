#ifndef CIRCULAR_LIST_H
#define CIRCULAR_LIST_H

#define new(x) calloc(1, sizeof(x))

typedef void *list;
typedef void **list_buffer;

list new_list (long long unsigned length);
/* remember to check turned buffer vs NULL and allocate if needed */
list_buffer list_get_write_ptr (list);
void list_mark_ready (list_buffer);
list_buffer list_read (list);
void delete_list (list);

long long unsigned list_used (list);
long long unsigned list_length (list);

#endif /* CIRCULAR_LIST_H */
