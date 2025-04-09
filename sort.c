#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sort.h"
#define MAX_RUNS 85

typedef struct Run {
    struct list_head *start;
    int length;
} Run;

static struct list_head *merge_two_sorted_queue(struct list_head *L1,
                                                struct list_head *L2)
{
    if (!L1)
        return L2;
    if (!L2)
        return L1;
    struct list_head *head = NULL;
    struct list_head **ptr = NULL;
    struct list_head **node = NULL;

    head = (strcmp(list_entry(L1, element_t, list)->value,
                   list_entry(L2, element_t, list)->value) <= 0)
               ? L1
               : L2;
    ptr = &head->next;
    node = (head == L1) ? &L1 : &L2;
    *node = (*node)->next;
    // cppcheck-suppress knownConditionTrueFalse
    while (L1 && L2) {
        node = (strcmp(list_entry(L1, element_t, list)->value,
                       list_entry(L2, element_t, list)->value) <= 0)
                   ? &L1
                   : &L2;
        *ptr = *node;
        ptr = &(*ptr)->next;
        if (*node)
            *node = (*node)->next;
    }
    *ptr = (L1) ? L1 : L2;

    return head;
}

static void swap(char **p1, char **p2)
{
    if (p1 == p2 || *p1 == *p2)
        return;
    char *temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

static int selection_sort(struct list_head *head, struct list_head *tail)
{
    int length = 0;

    struct list_head *i = head;
    while (i != tail) {
        struct list_head *min = i;
        struct list_head *j = i->next;

        while (j != tail->next && j != NULL) {
            length++;
            if (strcmp(list_entry(j, element_t, list)->value,
                       list_entry(min, element_t, list)->value) < 0) {
                min = j;
            }
            j = j->next;
        }

        if (min != i) {
            swap(&list_entry(i, element_t, list)->value,
                 &list_entry(min, element_t, list)->value);
        }

        i = i->next;
    }

    tail->next = NULL;

    return length;
}

static bool should_merge(Run x, Run y, Run z)
{
    return (x.length <= y.length + z.length) || (y.length <= z.length);
}

static void rebuild_list_link(struct list_head *head)
{
    if (!head || !head->next)
        return;
    struct list_head *cur = head->next, *prev = head;

    while (cur != NULL) {
        cur->prev = prev;
        prev = cur;
        cur = cur->next;
    }

    prev->next = head;
    head->prev = prev;
}

void tim_sort(struct list_head *head, int minrun)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    Run run_stack[MAX_RUNS];
    int top = -1;

    element_t *curr, *safe;
    element_t *curr_head, *curr_tail;
    int crun = 0, run_len = 0;
    bool increasing = true;
    curr_head = list_entry(head->next, element_t, list);
    list_for_each_entry_safe(curr, safe, head, list) {
        if (&safe->list == head)
            break;
        if (strcmp(curr->value, safe->value) <= 0 && increasing) {
            crun++;
            continue;
        } else {
            increasing = false;
            if (crun >= minrun) {
                curr_tail = curr;
                run_len = selection_sort(&curr_head->list, &curr_tail->list);
                run_stack[++top] = (Run){&curr_head->list, run_len};

                while (top >= 2 &&
                       should_merge(run_stack[top - 2], run_stack[top - 1],
                                    run_stack[top])) {
                    Run z = run_stack[top--];
                    Run y = run_stack[top--];
                    Run x = run_stack[top--];
                    Run merged = (Run){merge_two_sorted_queue(x.start, y.start),
                                       x.length + y.length};
                    run_stack[++top] = merged;
                    run_stack[++top] = z;
                }

                crun = 0;
                curr_head = safe;
                increasing = true;
                continue;
            } else {
                crun++;
                continue;
            }
        }
    }

    if (crun != 0) {
        curr_tail = curr;
        run_len = selection_sort(&curr_head->list, &curr_tail->list);
        run_stack[++top] = (Run){&curr_head->list, run_len};
    }

    while (top > 0) {
        Run b = run_stack[top--];
        Run a = run_stack[top--];
        Run merged = (Run){merge_two_sorted_queue(a.start, b.start),
                           a.length + b.length};
        run_stack[++top] = merged;
    }

    head->next = run_stack[top].start;
    rebuild_list_link(head);
}
/*======lib/list_sort.c======*/
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

typedef int (*list_cmp_func_t)(const struct list_head *a,
                               const struct list_head *b);

static int cmp_strcmp(const struct list_head *a, const struct list_head *b)
{
    const element_t *ea = list_entry(a, element_t, list);
    const element_t *eb = list_entry(b, element_t, list);
    return strcmp(ea->value, eb->value);
}

static struct list_head *merge(list_cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void merge_final(list_cmp_func_t cmp,
                        struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    struct list_head *tail = head;
    uint8_t count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    tail->next = b;
    do {
        if (unlikely(!++count))
            cmp(b, b);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

void list_sort(struct list_head *head, list_cmp_func_t cmp)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    head->prev->next = NULL;

    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(cmp, b, a);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(cmp, pending, list);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(cmp, head, pending, list);
}

void linux_sort(struct list_head *head)
{
    list_sort(head, cmp_strcmp);
}
