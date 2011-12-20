#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

List* List_list (void *data) {
  List *node;

  if(!(node = malloc(sizeof(node)))) {
    return NULL;
  }
  node->data = data;
  node->next = NULL;
  return node;
}

List* List_push(List *list, void *data){
  List *newnode;

  newnode = List_list(data);
  newnode->data = data;
  newnode->next = list->next;
  list->next = newnode;

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
  while(node) {
    i++;
    node = node->next;
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
