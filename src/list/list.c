#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

List* List_push(List **list, void *data) {
  List *newnode;

  if(!(newnode = malloc(sizeof(newnode)))) {
    return NULL;
  }

  newnode->data = data;
  newnode->next = *list;
  *list = newnode;

  fprintf(stdout, "list item data pointer %p.\n", data);
  return newnode;
}

/*
 * Note that this function only calls free() for the node, not whatever
 * is contained in the data
 */
int List_remove(List **list, List *node){
  List *current = *list;

  if(current == node) {
    // need to delete head item
    *list = node->next;
    free(node);
    return 0;
  }

  while(current->next && current->next != node) {
    current = current->next;
  }

  if(!current) {
    // not found e.g. reached end of list
    return -1;
  }

  // deleting other item
  current->next = node->next;
  free(node);
  return 0;
}

int List_length(List *node) {
  int i = 0;
  for ( ; node; node = node->next) {
   // fprintf(stdout, "%i (%p) ", (int) node->data, node->data);
    i++;
  }
  if(i > 0) {
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "list length %d.\n", i);
  return i;
}

void List_free(List *list) {
  List *next;

  for( ; list; list = next) {
    next = list->next;
    free(list);
  }
}

int List_map(List *list, int apply(void *item, void *ctxt), void *context) {
  int result;
  for ( ; list; list = list->next) {
//    fprintf(stdout, "apply list item data pointer %p.\n", (void *)list->data);
    result = apply(list->data, context);
    if(result != 0) {
      return result;
    }
  }
  return 0;
}
