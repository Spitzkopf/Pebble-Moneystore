#include "LinkedList.h"

#include <stdlib.h>

typedef struct LinkedListNode
{
	void* item;
	struct LinkedListNode* next_node;
} LinkedListNode;

typedef struct
{
	LinkedListNode* head;
	int current_index;
	int count;	
} LinkedList;

int ll_is_index_in_range(void* linked_list, int index)
{
	LinkedList* lc = (LinkedList*)linked_list;
	int count = lc->count;
	
	if (index + 1 >= count)
	{
		return 0;
	}
	if (index < 0)
	{
		return 0;
	}
	return 1;
}

void* ll_init_linked_list()
{
	LinkedList* lc = malloc(sizeof(LinkedList));
	lc->head = malloc(sizeof(LinkedListNode));
	lc->head->item = NULL;
	lc->head->next_node = NULL;
	lc->current_index = -1;
	lc->count = 0;
	
	return (void*)lc;
}

void ll_destroy_linked_list(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	LinkedListNode* temp;
	
	while (NULL != current->next_node)
	{
		temp = current;
		current = current->next_node;
		free(temp);
	}
	
	free(current);
}

int ll_add_item(void* linked_list, void* item)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
		
	if ((lc->count) == 0)
	{
		current->item = item;
		++lc->count;
		return 1;
	}
	
	while (NULL != current->next_node)
	{
		current = current->next_node;
	}
	
	current->next_node = malloc(sizeof(LinkedListNode));
	
	if (NULL == current->next_node)
	{
		return 0;
	}
	
	current->next_node->item = item;
	current->next_node->next_node = NULL;
	++lc->count;
	return 1;
}

int ll_add_item_at(void* linked_list, void* item, int index)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	LinkedListNode* temp;
	int current_index = 0;
	
	if (index > (lc->count - 1))
	{
		return 0;
	}
	
	if (index == (lc->count - 1))
	{
		return ll_add_item(linked_list, item);
	}
	
	while (current_index != index)
	{
		current = current->next_node;
	}
	
	temp = current->next_node;
	current->next_node = malloc(sizeof(LinkedListNode));
	if (NULL == current->next_node)
	{
		return 0;
	}
	
	current->next_node->item = item;
	current->next_node->next_node = temp;
	return 1;
}

int ll_remove_item(void* linked_list, void* item)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	LinkedListNode* temp;
	int index = 0;
	
	if (lc->count == 0)
	{
		return 0;
	}
	
	while (current->next_node != NULL)
	{
		if (current->next_node->item == item)
		{
			if (!current->next_node->next_node)
			{
				free(current->next_node);
				current->next_node = NULL;
				--lc->count;
				
				if (lc->count == 0)
				{
					lc->current_index = -1;
				}
				
				else if (lc->current_index == index)
				{
					lc->current_index = ll_get_previous_index(linked_list);
				}
				
				return 1;
			}
			else
			{
				temp = current->next_node;
				current->next_node = temp->next_node;
				
				free(temp);
				
				if (lc->current_index == index)
				{
					lc->current_index = ll_get_previous_index(linked_list);
				}
				
				return 1;
			}
		}	
		++index;
	}
	
	return 0;
}

int ll_remove_item_at(void* linked_list, int index)
{
	void* item = NULL;
	int i = 0;
	
	if (!ll_is_index_in_range(linked_list, index))
	{
		return 0;
	}
	
	for(;i < index; ++i)
	{
		item = ll_get_next_item(linked_list);
	}
	
	return ll_remove_item(linked_list, item);
}

int ll_get_next_index(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	if ((lc->current_index + 1) == lc->count)
	{
		return 0;
	}
	return lc->current_index + 1;
}

int ll_get_previous_index(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	if ((lc->current_index - 1) == 0)
	{
		return lc->count - 1;
	}
	return lc->current_index - 1;
}

int ll_current_item_index(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	
	if (((lc->current_index) == -1) && (lc->count > 0))
	{
		lc->current_index = 0;
		return 0;
	}
	
	return lc->current_index;
}

int ll_item_count(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	return lc->count;
}

void* ll_get_current_item(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	int i = 0;
	
	if ((lc->current_index) == -1)
	{
		lc->current_index = 0;
		return current->item;
	}
	
	for(;i < lc->current_index; ++i)
	{
		current = current->next_node;
	}
	
	return current->item;
}

void* ll_get_next_item(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	int i = 0;
	
	if ((lc->count) == 0)
	{
		return NULL;
	}
	
	if ((lc->current_index) == -1)
	{
		lc->current_index = 0;
		return current->item;
	}
	
	if ((lc->current_index + 1) == lc->count)
	{
		lc->current_index = 0;
		return current->item;
	}
	
	++lc->current_index;
	
	for(;i < lc->current_index; ++i)
	{
		current = current->next_node;
	}
	
	return current->item;
}

void* ll_get_previous_item(void* linked_list)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	int i = lc->count - 1;
	
	if (lc->current_index == 0)
	{
		lc->current_index = lc->count - 1;
		while (NULL != current->next_node)
		{
			current = current->next_node;
		}
		return current->item;
	}
	
	--lc->current_index;
	
	for(;i > lc->current_index; --i)
	{
		current = current->next_node;
	}
	
	return current->item;
}

int ll_set_current_item(void* linked_list, int index)
{
	LinkedList* lc = (LinkedList*)linked_list;
	if (!ll_is_index_in_range(linked_list, index))
	{
		return 0;
	}
	
	lc->current_index = index;
	return 1;
}

int ll_find_item(void* linked_list, void* item)
{
	LinkedList* lc = (LinkedList*)linked_list;
	LinkedListNode* current = lc->head;
	int index = 0;
	
	while (current->next_node != NULL)
	{
		if (current->item == item)
		{
			return index;
		}
		current = current->next_node;
		++index;
	}
	
	if (current->item == item)
	{
		return index;
	}

	return -1;
}