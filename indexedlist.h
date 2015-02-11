#ifndef INDEXEDLIST_H
#define INDEXEDLIST_H

/* oh, look, completely opaque structures. */
typedef void *list_t;

#define new_list() malloc(sizeof(list_t))

/* oh look, functions */
unsigned long long list_first_index (list_t list);
list_t list_insert (list_t list, void *data, unsigned index);
void *list_pop (list_t list);
void list_destroy (list_t list);

#endif /* INDEXEDLIST_H */
