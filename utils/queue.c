/* 
 * queue.c --- 
 * 
 * Author: Insecticide
 * Created: 10-12-2025
 * Version: 1.0
 * 
 * Description: This file implements the queue.h interface 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

typedef struct qnode{
  void *data;
  struct qnode *next;
}qnode_t;

typedef struct queuei{
  qnode_t *front;
  qnode_t *back;
}queuei_t;

queue_t *qopen(void) {
    queuei_t *q = malloc(sizeof(queuei_t));
    if (q == NULL) return NULL;
    q->front = NULL;
    q->back  = NULL;
    return (queue_t*)q;
}

void qclose(queue_t *qp) {
    if (qp == NULL) return;
    queuei_t *q = (queuei_t*)qp;
    qnode_t *curr = q->front;
    while (curr != NULL) {
        qnode_t *next = curr->next;
        free(curr);
        curr = next;
    }
    free(q);
}

int32_t qput(queue_t *qp, void *elementp) {
    if (qp == NULL || elementp == NULL) return 1;

    queuei_t *q = (queuei_t*)qp;
    qnode_t *node = malloc(sizeof(qnode_t));
    if (node == NULL) return 1;

    node->data = elementp;
    node->next = NULL;

    if (q->back == NULL) {
        q->front = q->back = node;
    } else {
        q->back->next = node;
        q->back = node;
    }
    return 0;
}

void *qget(queue_t *qp) {
    if (qp == NULL) return NULL;
    queuei_t *q = (queuei_t*)qp;
    if (q->front == NULL) return NULL;

    qnode_t *first = q->front;
    void *data = first->data;

    q->front = first->next;
    if (q->front == NULL)
        q->back = NULL;

    free(first);
    return data;
}

void qapply(queue_t *qp, void (*fn)(void *elementp)) {
    if (qp == NULL || fn == NULL) return;
    queuei_t *q = (queuei_t*)qp;
    for (qnode_t *curr = q->front; curr != NULL; curr = curr->next)
        fn(curr->data);
}

void* qsearch(queue_t *qp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp) {
    if (qp == NULL || searchfn == NULL) return NULL;
    queuei_t *q = (queuei_t*)qp;
    const void* keyp = skeyp;
    for (qnode_t *curr = q->front; curr != NULL; curr = curr->next) {
      if (searchfn(curr->data, keyp)){
        return curr->data;
      }
    }
    return NULL;
}

void* qremove(queue_t *qp, bool (*searchfn)(void* elementp, const void*keyp), const void* skeyp) {
    if (qp == NULL || searchfn == NULL) return NULL;
    queuei_t *q = (queuei_t*)qp;
    qnode_t *prev = NULL;
    for (qnode_t *curr = q->front; curr != NULL; prev = curr, curr = curr->next) {
        if (searchfn(curr->data, skeyp)) {
            if (prev == NULL)
                q->front = curr->next;
            else
                prev->next = curr->next;
            if (curr == q->back)
                q->back = prev;
            void *data = curr->data;
            free(curr);
            return data;
        }
    }
    return NULL;
}

void qconcat(queue_t *q1p, queue_t *q2p) {
	  if (q1p == q2p) return; 
	  if (q1p == NULL || q2p == NULL) return;
    queuei_t *q1 = (queuei_t*)q1p;
    queuei_t *q2 = (queuei_t*)q2p;

    if (q2->front == NULL) {
        free(q2);
        return;
    }

    if (q1->back == NULL) {
        q1->front = q2->front;
        q1->back  = q2->back;
    } else {
        q1->back->next = q2->front;
        q1->back = q2->back;
    }
    free(q2);
}
