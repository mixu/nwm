

/* The linked list should provide:
 add(item)
 some(callback) --> for writing a find function
 remove(index)
 */


typedef struct List_T {
  struct List_T *next;
  void *data;
} List;

extern List *List_list(void *data);
extern List *List_push(List *list, void *x);
extern int List_remove(List *list, List *node);

extern int List_length(List *node);

extern void List_free(List *list);
