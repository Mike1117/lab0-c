#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occurr. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);

    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *curr, *safe;
    list_for_each_entry_safe (curr, safe, head, list)
        q_release_element(curr);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_first_entry(head, element_t, list);
    list_del(&element->list);
    if (sp && bufsize) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = list_last_entry(head, element_t, list);
    list_del(&element->list);
    if (sp && bufsize) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    struct list_head *curr;
    int size = 0;
    list_for_each (curr, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    int index = (q_size(head) + 1) / 2;
    int i = 0;
    for (i = 0; i < index; i++) {
        head = head->next;
    }
    element_t *entry = list_entry(head, element_t, list);
    list_del(head);
    q_release_element(entry);
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    element_t *curr, *safe;
    bool dup = false;
    list_for_each_entry_safe (curr, safe, head, list) {
        if (curr->list.next != head && !strcmp(curr->value, safe->value)) {
            dup = true;
            list_del(&curr->list);
            q_release_element(curr);
        } else {
            if (dup) {
                list_del(&curr->list);
                q_release_element(curr);
                dup = false;
            }
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
static void swap(char **p1, char **p2)
{
    if (p1 == p2 || *p1 == *p2)
        return;
    char *temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    element_t *curr, *safe;
    list_for_each_entry_safe (curr, safe, head, list) {
        if (&safe->list == head) {
            break;
        } else {
            swap(&curr->value, &safe->value);
            curr = safe;
            safe = list_entry(safe->list.next, element_t, list);
        }
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *curr = head;
    struct list_head *tmp;
    do {
        tmp = curr->next;
        curr->next = curr->prev;
        curr->prev = tmp;
        curr = tmp;
    } while (curr != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || head->next->next == head || k <= 1)
        return;
    struct list_head *tmp_h = head;
    struct list_head *o_prev;
    struct list_head *curr_h = NULL;
    struct list_head *curr = NULL, *safe = NULL;
    while (safe != head) {
        curr_h = tmp_h->next;
        int i = 0;
        list_for_each_safe (curr, safe, tmp_h) {
            i++;
            if (i == k) {
                curr->next = tmp_h;
                o_prev = tmp_h->prev;
                tmp_h->prev = curr;
                q_reverse(tmp_h);
                curr_h->next = safe;
                safe->prev = curr_h;
                tmp_h->prev = o_prev;
                tmp_h = curr_h;
                break;
            }
            if (safe == head)
                break;
        }
    }
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    struct list_head *curr = head->prev;
    element_t *to_del;
    while (curr->prev != head) {
        if (strcmp(list_entry(curr, element_t, list)->value,
                   list_entry(curr->prev, element_t, list)->value) > 0) {
            to_del = list_entry(curr->prev, element_t, list);
            list_del(&to_del->list);
            q_release_element(to_del);
        } else {
            curr = curr->prev;
        }
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    struct list_head *curr = head->next;
    element_t *to_del;
    while (curr->next != head) {
        if (strcmp(list_entry(curr, element_t, list)->value,
                   list_entry(curr->next, element_t, list)->value) < 0) {
            to_del = list_entry(curr->next, element_t, list);
            list_del(&to_del->list);
            q_release_element(to_del);
        } else {
            curr = curr->next;
        }
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
struct list_head *merge_two_sorted_queue(struct list_head *L1,
                                         struct list_head *L2,
                                         bool descend)
{
    struct list_head *head = L1, **ptr = &head->next, **node;
    const struct list_head *L2_head = L2;
    struct list_head *prev = head;

    if (!descend) {  // ascend
        for (node = NULL; L1 != NULL && L2 != NULL; *node = (*node)->next) {
            node = (strcmp(list_entry(L1, element_t, list)->value,
                           list_entry(L2, element_t, list)->value) < 0)
                       ? &L1
                       : &L2;
            *ptr = *node;
            (*ptr)->prev = prev;
            prev = *ptr;
            ptr = &(*ptr)->next;
        }
        *ptr = (L1) ? L1 : L2;
        (*ptr)->prev = prev;

        while (*ptr && (*ptr) != head && (*ptr) != L2_head) {
            ptr = &(*ptr)->next;
        }

        *ptr = head;
        head->prev = list_entry(ptr, struct list_head, next);
    } else {  // descend
        for (node = NULL; L1 != NULL && L2 != NULL;) {
            node = (strcmp(list_entry(L1, element_t, list)->value,
                           list_entry(L2, element_t, list)->value) > 0)
                       ? &L1
                       : &L2;
            *ptr = *node;
            *node = (*node)->prev;
            (*ptr)->prev = prev;
            prev = *ptr;
            ptr = &(*ptr)->next;
        }

        *node = (L1) ? L1 : L2;

        while (*node && (*node)->prev && (*node) != head &&
               (*node) != L2_head) {
            *ptr = *node;
            *node = (*node)->prev;
            (*ptr)->prev = prev;
            prev = *ptr;
            ptr = &(*ptr)->next;
        }

        *ptr = head;
        head->prev = list_entry(ptr, struct list_head, next);
    }
    return head;
}
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    struct list_head *ptr_first = head->next;
    struct list_head *ptr_i = head->next->next;
    struct list_head *ptr_j = head->prev;
    while (ptr_i != ptr_j) {
        while (ptr_i != ptr_j && ptr_j->next != ptr_i) {
            list_entry(ptr_i, queue_contex_t, chain)->q =
                merge_two_sorted_queue(
                    list_entry(ptr_i, queue_contex_t, chain)->q,
                    list_entry(ptr_j, queue_contex_t, chain)->q, 0);
            ptr_i = ptr_i->next;
            ptr_j = ptr_j->prev;
        }
        ptr_i = head->next->next;
    }
    list_entry(ptr_first, queue_contex_t, chain)->q = merge_two_sorted_queue(
        list_entry(ptr_first, queue_contex_t, chain)->q,
        list_entry(ptr_i, queue_contex_t, chain)->q, descend);
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return q_size(list_entry(ptr_first, queue_contex_t, chain)->q);
}
