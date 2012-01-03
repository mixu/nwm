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

int List_remove(List *list, List *node){

  while(list->next && list->next!= node) {
    list = list->next;
  }
  if(list->next) {
    list->next = node->next;
    free(node);
    return 0;
  } else {
    return -1;
  }
}

int List_length(List *node) {
  int i = 0;
  for ( ; node; node = node->next) {
    i++;
  }
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

int List_filter(List *list, int filter(void *item, void *ctxt), void *context) {
  int result;
  int num_removed = 0;
  List *prev = NULL;

  while(list) {
    result = filter(list->data, context);
    if(result != 0 && prev) {
      prev->next = list->next;
      free(list);
      num_removed++;
      // we do not advance the prev pointer since we can't do that until we move forward
      list = prev->next;
    } else {
      prev = list;
      list = list->next;
    }
  }
  return num_removed;
}
