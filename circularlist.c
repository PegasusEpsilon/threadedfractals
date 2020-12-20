/* circularlist.c, from threadedfractals
 * by "Pegasus Epsilon" <pegasus@pimpninjas.org>
 * Distribute Unmodified - http://pegasus.pimpninjas.org/license
 */

#include <stdlib.h> 	/* malloc(), free() */
#include <stdbool.h>	/* bool, true, false */

#define new(x) calloc(1, sizeof(x));

struct node {
	void *data;
	// everything below here is opaque
	struct node *next;
	struct node *previous;
	bool busy;
	bool ready;
};

struct list {
	struct node *read;
	struct node *write;
	long long unsigned length;
	long long unsigned used;
};

long long unsigned list_used (struct list *restrict const this) {
	return this->used;
}

long long unsigned list_length (struct list *restrict const this) {
	return this->length;
}

struct node *new_node (void) {
	struct node *new = new(struct node);
	(*new) = (struct node){
		.data = NULL,
		.next = NULL,
		.previous = NULL,
		.busy = false,
		.ready = false
	};
	return new;
}

// immediately before read pointer
__attribute__((always_inline)) static inline
struct node *insert_new (struct list *restrict const this) {
	// create
	struct node *new = new_node();
	// bind
	new->next = this->read;
	new->previous = new->next->previous;
	// insert
	new->previous->next = new;
	new->next->previous = new;
	// update count
	this->length++;
	return new;
}

// yay tristates
// there is no such thing as busy && ready
#define is_busy(x) (x->busy)
#define is_ready(x) (x->ready)
#define is_idle(x) (!(x->ready || x->busy))

void list_mark_idle (struct node *restrict const this) {
	this->ready = false;
	this->busy = false;
}

void list_mark_ready (struct node *restrict const this) {
	this->ready = true;
	this->busy = false;
}

void list_mark_busy (struct node *restrict const this) {
	this->ready = false;
	this->busy = true;
}

struct node *list_read (struct list *restrict const this) {
	if (!is_ready(this->read)) return NULL;
	is_ready(this->read) = false;
	this->used--;
	list_mark_idle(this->read);
	this->read = this->read->next;
	return this->read->previous;
}

// remember to check .data vs NULL and allocate if needed
struct node *list_get_write_ptr (struct list *restrict const this) {
	this->used++;
	if (is_idle(this->write)) {
		is_busy(this->write) = true;
		this->write = this->write->next;
		return this->write->previous;
	}
	struct node *new = insert_new(this);
	is_busy(new) = true;
	return new;
}

struct list *new_list (long long unsigned length) {
	// allocate a new list
	struct list *this = new(struct list);
	// create the initial loop
	struct node *new = new_node();
	new->next = new;
	new->previous = new;
	// point the list to the loop
	(*this) = (struct list){
		.read = new,
		.write = new,
		.length = 1
	};
	// grow the loop to match the requested length
	while (this->length < length) insert_new(this);
	// reset the write pointer
	this->write = this->read;
	// return the new list
	return this;
}

/*
void destroy_node (struct list *restrict const this) {
	struct node *current = this->write;
	struct node *next = current->next;
	struct node *previous = current->previous;
	// disconnect node from loop
	previous->next = next;
	next->previous = previous;
	// disconnect node from list
	if (this->read == current) this->read = next;
	this->write = next;
	// node disconnected, free data
	free(current->data);
	// free node
	free(current);
}
*/

void delete_list (struct list *restrict const this) {
	struct node *first = this->write;
	while (this->write != NULL) {
		this->read = this->write->next;
		free(this->write->data);
		free(this->write);
		this->write = this->read;
		if (this->write == first) break;
	}
	free(this);
}
