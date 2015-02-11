#include <stdlib.h> 	/* malloc(), free() */
#include <stdbool.h>	/* bool */

#define _hot __attribute__((hot))	/* called often */
#define new(x) malloc(sizeof(x))
#define init_new(x, y, ...) x *y = malloc(sizeof(x)); *y = (x)__VA_ARGS__

struct node { void *data; unsigned long long index; struct node *next; };
typedef struct node **list_t;
#define new_list() new(list_t)

void *list_pop (list_t this) {
	struct node *old = *this;
	void *temp = old->data;
	*this = old->next;
	free(old);
	return temp;
}

unsigned long long list_first_index (list_t this) {
	return this && (*this) ? (*this)->index : (unsigned long long)-1;
}

struct node *list_insert (list_t this, void *data, unsigned long long index) {
	/* create a list if not handed one */
	if (!this) this = new_list();

	/* create a new node object with the passed-in data */
	init_new(struct node, temp, { .data = data, .index = index, .next = NULL});

	/* if we're the only node, we're done */
	if (!*this) return *this = temp;

	/* if there is only one node, and we belong before it... */
	if (!(*this)->next && (*this)->index > temp->index) {
		temp->next = *this;
		return (*this) = temp;
	}

	/* otherwise, figure out where we belong */
	while ((*this)->index < temp->index && (*this)->next)
		this = &(*this)->next;

	if ((*this)->index < temp->index)
		/* there is no next */
		return (*this)->next = temp;

	temp->next = *this;
	return (*this) = temp;
}

/* technically not needed, but whatever */
void list_destroy (list_t this) {
	if (!this) return;
	if (*this) {
		struct node *next, *current = *this;
		while (current) {
			if (current->next) next = current->next;
			free(current->data);
			free(current);
			current = next;
		}
	}
	free(this);
}
