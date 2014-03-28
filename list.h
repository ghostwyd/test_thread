#ifndef __LIST_H__
#define __LIST_H__


struct list_head {
    struct list_head     *prev;
    struct list_head     *next;
};

typedef struct list_head list_head_t;

#define INIT_LIST_HEAD(head)  \
do {\
    (head)->prev= (head); \
    (head)->next = (head);\
}while(0);\

#define LIST_ENTRY(elem, type, member) \
    (type*)((char*)(elem) - offsetof(type, member)) \

#define LIST_FOR_EACH_ENTRY(curr, head)   for ((curr) = (head)->next; (curr) != (head); (curr) = (curr)->next)

static inline void __list_add(list_head_t *new_list, list_head_t *prev, list_head_t *next)
{
    next->prev = new_list;
    new_list->next = next;
    prev->next = new_list;
    new_list->prev = prev;
}

static inline void __list_del(list_head_t *prev, list_head_t *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_add_tail(list_head_t *new_list, list_head_t *head)
{
    __list_del(new_list->prev, new_list->next); 
    INIT_LIST_HEAD(new_list);
    __list_add(new_list, head->prev, head);
}
static inline void list_add_head(list_head_t *new_list, list_head_t *head) 
{
    __list_del(new_list->prev, new_list->next);
    INIT_LIST_HEAD(new_list);
    __list_add(new_list, head, head->next);
}
static inline void list_del(list_head_t *del_list)
{
    __list_del(del_list->prev, del_list->next);
    INIT_LIST_HEAD(del_list);
}

#endif
