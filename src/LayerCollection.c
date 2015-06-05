#include "LayerCollection.h"
#include "LinkedList.h"

int is_index_in_range(void* layer_collection, int index)
{
	return ll_is_index_in_range(layer_collection, index);
}

void* init_layer_collection()
{	
	return (void*)ll_init_linked_list();
}

void destroy_layer_collection(void* layer_collection)
{
	ll_destroy_linked_list(layer_collection);
}

int add_layer(void* layer_collection, Layer* layer)
{
	return ll_add_item(layer_collection, (void*)layer);
}

int add_layer_at(void* layer_collection, Layer* layer, int index)
{
	return ll_add_item_at(layer_collection, (void*)layer, index);
}

int remove_layer(void* layer_collection, Layer* layer)
{
	return ll_remove_item(layer_collection, (void*)layer);
}

int remove_layer_at(void* layer_collection, int index)
{
	return ll_remove_item_at(layer_collection, index);
}

int get_next_index(void* layer_collection)
{
	return ll_get_next_index(layer_collection);
}

int get_previous_index(void* layer_collection)
{
	return ll_get_previous_index(layer_collection);
}

int current_layer_index(void* layer_collection)
{
	return ll_current_item_index(layer_collection);
}

int layer_count(void* layer_collection)
{
	return ll_item_count(layer_collection);
}

Layer* get_current_layer(void* layer_collection)
{
	return (Layer*)ll_get_current_item(layer_collection);
}

Layer* get_next_layer(void* layer_collection)
{
	return (Layer*)ll_get_next_item(layer_collection);
}

Layer* get_previous_layer(void* layer_collection)
{
	return (Layer*)ll_get_previous_item(layer_collection);
}

int set_current_layer(void* layer_collection, int index)
{
	return ll_set_current_item(layer_collection, index);
}

int find_layer(void* layer_collection, Layer* layer)
{
	return ll_find_item(layer_collection, (void*)layer);
}