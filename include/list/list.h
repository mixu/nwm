
typedef struct List_T {
  struct List_T *next;
  void *data;
} List;

extern List* List_push(List **list, void *data);
extern int List_remove(List **list, List *node);

extern int List_length(List *node);

extern void List_free(List *list);

#define List_for_each(pos, list) \
 for((pos) = (list); pos; (pos) = (pos)->next)

#define List_search(list, pos, value) \
 List_for_each(pos, list) {           \
   if((pos)->data == (value)) break;  \
 }

