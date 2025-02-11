#ifndef LIST_H
#define LIST_H
#include "include/library/types.h"

#define offset(struct_type,member) (int)(&((struct_type*)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
	 (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

typedef struct __list_elem {
    struct __list_elem*  prev;
    struct __list_elem*  next;
}list_elem;

typedef struct {
    list_elem   head;
    list_elem   tail;
}list;

typedef bool(*functor)(list_elem*, int);

/* some utils here */
void            list_init(list* _list);
void            list_insert_before(list_elem* before,  list_elem* elem);
void            list_push(list* plist,  list_elem* elem);
void            list_append(list* plist,  list_elem* elem);  
void            list_remove( list_elem* pelem);
list_elem*      list_pop(list* plist);
bool            list_empty(list* plist);
uint32_t        list_len(list* plist);
list_elem*      list_traversal(list* plist, functor func, int arg);
bool            elem_find(list* plist, list_elem* obj_elem);
#endif