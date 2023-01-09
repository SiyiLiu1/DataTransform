/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ics.h"
#include "emalloc.h"
#include "listy.h"

/*
 * Function: new_node
 *
 * Creat a new node_t with event_t val as value
 */
node_t *new_node(event_t val) {
    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->val = val;
    temp->next = NULL;

    return temp;
}

/*
 * Function: add_front
 *
 * add the new node_t new to the front of the list
 */
node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}

/*
 * Function: add_end
 *
 * add the new node_t new to the end of the list
 */
node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}

/*
 * Function: add_inorder
 *
 * add the new node_t new to the list,
 * and the list will keep the same order as before
 */
node_t *add_inorder(node_t * list, node_t *new) {
    node_t *prev = NULL;
    node_t *curr = NULL;

    if (list == NULL) 
        return add_front(list, new);

    for (curr = list; curr != NULL; curr = curr->next) {
        
        if (strcmp(curr->val.dtstart, new->val.dtstart) <= 0) {
            prev = curr;
        } else {
            break;
        }
    }

     new->next = curr;

    if (prev == NULL) {
        return (new);
    } else {
        prev->next = new;
        return list;
    }
}

/*
 * Function: peek_front
 *
 * return the node_t in the front of the list.
 */
node_t *peek_front(node_t *list) {
    return list;
}

/*
 * Function: remove_front
 *
 * remove the node_t from the front of the list
 * and return this node_t
 */
node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }

    return list->next;
}


/*
 * Function: apply
 *
 * Go through each node_t from the list one by one,
 * and do something inside the function fn.
 */
void apply(node_t *list,
           void (*fn)(node_t *list, void *),
           void *arg)
{
    for ( ; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}
