#include "include/library/list.h"
#include "include/kernel/interrupt.h"
#include "include/library/types.h"


/* make list elems points each other*/
void list_init(list* list) 
{
    list->head.prev = NULL;
    list->head.next = &list->tail; 
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

void list_insert_before(list_elem* before, list_elem* elem) { 
    // disable the intr, for safe insert!
    Interrupt_Status old_status = disable_intr();
    before->prev->next = elem; 
    elem->prev = before->prev;
    elem->next = before;
    before->prev = elem;
    set_intr_state(old_status);
}

void list_push(list* plist, list_elem* elem) {
    list_insert_before(plist->head.next, elem); 
}

/* 追加元素到链表队尾,类似队列的先进先出操作 */
void list_append(list* plist, list_elem* elem) {
    list_insert_before(&plist->tail, elem);     // 在队尾的前面插入
}

/* 使元素pelem脱离链表 */
void list_remove(list_elem* pelem) {
    Interrupt_Status old_status = disable_intr();
   
    pelem->prev->next = pelem->next;
    pelem->next->prev = pelem->prev;

    set_intr_state(old_status);
}

/* 将链表第一个元素弹出并返回,类似栈的pop操作 */
list_elem* list_pop(list* plist) {
    list_elem* elem = plist->head.next;
    list_remove(elem);
    return elem;
} 

/* 从链表中查找元素obj_elem,成功时返回true,失败时返回false */
bool elem_find(list* plist, list_elem* obj_elem) {
    list_elem* elem = plist->head.next;
    while (elem != &plist->tail) {
        if (elem == obj_elem) {
	        return true;
        }
        elem = elem->next;
   }
   return false;
}

// iterate the list
list_elem* list_traversal(list* plist, functor func, int arg) {
    list_elem* elem = plist->head.next;

    if (list_empty(plist)) { 
        return NULL;
    }

    while (elem != &plist->tail) {
        if (func(elem, arg)) {		  // func返回ture则认为该元素在回调函数中符合条件,命中,故停止继续遍历
	        return elem;
        }					  // 若回调函数func返回true,则继续遍历
        elem = elem->next;	       
   }
   return NULL;
}


uint32_t list_len(list* plist) {
    list_elem* elem = plist->head.next;
    uint32_t length = 0;
    while (elem != &plist->tail) {
        length++; 
        elem = elem->next;
    }
    return length;
}


bool list_empty(list* plist) {		
    return (plist->head.next == &plist->tail ? true : false);
}
