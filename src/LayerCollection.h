#ifndef __LAYER_COLLECTION_H__
#define __LAYER_COLLECTION_H__

#include <pebble.h>

void* init_layer_collection();
void destroy_layer_collection(void* layer_collection);

Layer* get_current_layer(void* layer_collection);
Layer* get_next_layer(void* layer_collection);
Layer* get_previous_layer(void* layer_collection);

int set_current_layer(void* layer_collection, int index);

int add_layer(void* layer_collection, Layer* layer);
int add_layer_at(void* layer_collection, Layer* layer, int index);
int remove_layer(void* layer_collection, Layer* layer);
int remove_layer_at(void* layer_collection, int index);

int current_layer_index(void* layer_collection);
int get_next_index(void* layer_collection);
int get_previous_index(void* layer_collection);
int is_index_in_range(void* layer_collection, int index);

int find_layer(void* layer_collection, Layer* layer);
int layer_count(void* layer_collection);


#endif