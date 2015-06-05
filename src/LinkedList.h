#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

void* ll_init_linked_list();
void ll_destroy_linked_list(void* linked_list);

void* ll_get_current_item(void* linked_list);
void* ll_get_next_item(void* linked_list);
void* ll_get_previous_item(void* linked_list);

int ll_set_current_item(void* linked_list, int index);

int ll_add_item(void* linked_list, void* item);
int ll_add_item_at(void* linked_list, void* item, int index);
int ll_remove_item(void* linked_list, void* item);
int ll_remove_item_at(void* linked_list, int index);

int ll_current_item_index(void* linked_list);
int ll_get_next_index(void* linked_list);
int ll_get_previous_index(void* linked_list);
int ll_is_index_in_range(void* linked_list, int index);

int ll_find_item(void* linked_list, void* item);
int ll_item_count(void* linked_list);

#endif