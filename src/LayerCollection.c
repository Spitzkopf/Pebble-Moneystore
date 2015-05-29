#include "LayerCollection.h"

typedef struct LayerCollectionNode
{
	Layer* layer;
	struct LayerCollectionNode* next_layer;
} LayerCollectionNode;

typedef struct
{
	LayerCollectionNode* head;
	int current_index;
	int count;	
} LayerCollection;

void* init_layer_collection()
{
	LayerCollection* lc = malloc(sizeof(LayerCollection));
	lc->head = malloc(sizeof(LayerCollectionNode));
	lc->head->layer = NULL;
	lc->head->next_layer = NULL;
	lc->current_index = -1;
	lc->count = 0;
	
	return (void*)lc;
}

void destroy_layer_collection(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
	LayerCollectionNode* temp;
	
	while (NULL != current->next_layer)
	{
		temp = current;
		current = current->next_layer;
		free(temp);
	}
}

int add_layer(void* layer_collection, Layer* layer)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
		
	if ((lc->count) == 0)
	{
		current->layer = layer;
		++lc->count;
		return 1;
	}
	
	while (NULL != current->next_layer)
	{
		current = current->next_layer;
	}
	
	current->next_layer = malloc(sizeof(LayerCollectionNode));
	
	if (NULL == current->next_layer)
	{
		return 0;
	}
	
	current->next_layer->layer = layer;
	current->next_layer->next_layer = NULL;
	++lc->count;
	return 1;
}

int add_layer_at(void* layer_collection, Layer* layer, int index)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
	LayerCollectionNode* temp;
	int current_index = 0;
	
	if (index > (lc->count - 1))
	{
		return 0;
	}
	
	if (index == (lc->count - 1))
	{
		return add_layer(layer_collection, layer);
	}
	
	while (current_index != index)
	{
		current = current->next_layer;
	}
	
	temp = current->next_layer;
	current->next_layer = malloc(sizeof(LayerCollectionNode));
	if (NULL == current->next_layer)
	{
		return 0;
	}
	
	current->next_layer->layer = layer;
	current->next_layer->next_layer = temp;
	return 1;
}

int current_layer_index(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	return lc->current_index;
}

int layer_count(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	return lc->count;
}

Layer* get_current_layer(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
	int i = 0;
	
	for(;i < lc->current_index; ++i)
	{
		current = current->next_layer;
	}
	
	return current->layer;
}

Layer* get_next_layer(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
	int i = 0;
	
	if ((lc->count) == 0)
	{
		return NULL;
	}
	
	if ((lc->current_index) == -1)
	{
		lc->current_index = 0;
		return current->layer;
	}
	
	if ((lc->current_index + 1) == lc->count)
	{
		lc->current_index = 0;
		return current->layer;
	}
	
	++lc->current_index;
	
	for(;i < lc->current_index; ++i)
	{
		current = current->next_layer;
	}
	
	return current->layer;
}

Layer* get_previous_layer(void* layer_collection)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	LayerCollectionNode* current = lc->head;
	int i = lc->count - 1;
	
	if (lc->current_index == 0)
	{
		lc->current_index = lc->count - 1;
		while (NULL != current->next_layer)
		{
			current = current->next_layer;
		}
		return current->layer;
	}
	
	--lc->current_index;
	
	for(;i > lc->current_index; --i)
	{
		current = current->next_layer;
	}
	
	return current->layer;
}

int set_current_layer(void* layer_collection, int index)
{
	LayerCollection* lc = (LayerCollection*)layer_collection;
	if (index > (lc->count - 1))
	{
		return 0;
	}
	
	lc->current_index = index;
	return 1;
}