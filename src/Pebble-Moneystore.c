#include <pebble.h>

#define ANIM_DURATION 300
#define ANIM_DELAY 500

enum AppMessageCodes {
  KEY_TEMPERATURE = 0,
  KEY_CONDITIONS = 1
};

typedef struct {
  int celsius;
  int bt_vibe;
  int hour_vibe;
} __attribute__((__packed__)) WatchSettings;

static Window *s_main_window;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;

static Layer** s_layer_collection[] = { (Layer**)&s_date_layer, (Layer**)&s_weather_layer };
static Layer* s_current_layer = NULL;
static int s_next_layer = 0;
static int s_dx = 1;
static const int s_layer_collection_max = sizeof(s_layer_collection) / sizeof(Layer**);

static AppTimer* s_time_return_timer;
static const int s_return_time = 1000 * 2;
static bool s_should_set_return_timer = true;
static bool s_is_time_returing = false;

WatchSettings settings = {
  .celsius = 1,
  .bt_vibe = 1,
  .hour_vibe = 1
};

static void time_layer_timeout_handler(void *data);

static void on_animation_stopped(Animation *anim, bool finished, void *context) {
    property_animation_destroy((PropertyAnimation*) anim);
    if (s_should_set_return_timer) {
      s_time_return_timer = app_timer_register(s_return_time, (AppTimerCallback)time_layer_timeout_handler, NULL);
      s_should_set_return_timer = false;
    }
    if ((Layer*)context == text_layer_get_layer(s_time_layer)) {
      s_is_time_returing = false;
    }
}
 
static void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay) {
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
    
    AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, layer);
    animation_schedule((Animation*) anim);
}

static GRect get_new_rect_for_layer(GRect to_swap, int direction) {
  if (to_swap.origin.x >= 144) {
    return GRect(0, 87, 144, 38);
  } else if (to_swap.origin.x <= -144) {
    return GRect(0, 87, 144, 38);
  }
   return GRect(144 * direction, 87, 144, 38);
}

static int get_direction_for_swap(GRect layer_to_show) {
  if (layer_to_show.origin.x >= 144) {
    return -1;
  } else if (layer_to_show.origin.x <= -144) {
    return 1;
  }
  return 1;
}

static void swap_layers(Layer* showing, Layer* hidden) {
  GRect current_layer_start = layer_get_frame(showing);
  
  GRect next_layer_start = layer_get_frame(hidden);
  GRect next_layer_end = get_new_rect_for_layer(next_layer_start, 0);
  
  GRect current_layer_end = get_new_rect_for_layer(current_layer_start, get_direction_for_swap(next_layer_start));
  
  animate_layer(showing, &current_layer_start, &current_layer_end, ANIM_DURATION, ANIM_DELAY);
  animate_layer(hidden, &next_layer_start, &next_layer_end, ANIM_DURATION, ANIM_DELAY);
}

static void swap_layers_animated() {
  app_timer_cancel(s_time_return_timer);
  
  s_should_set_return_timer = true;
  
  swap_layers(s_current_layer, *s_layer_collection[s_next_layer]);
  s_current_layer = *s_layer_collection[s_next_layer];
  
  s_next_layer = (s_next_layer + 1) == s_layer_collection_max ? 0 : s_next_layer + 1;
}

static void time_layer_timeout_handler(void *data) {
   Layer* time_layer = text_layer_get_layer(s_time_layer);
   
   APP_LOG(APP_LOG_LEVEL_INFO, "Other Layer Timeout");
   
   if (time_layer == s_current_layer) {
     return;
   }
   s_should_set_return_timer = false;
   s_is_time_returing = true;   

   swap_layers(s_current_layer, text_layer_get_layer(s_time_layer));    
   s_current_layer =  text_layer_get_layer(s_time_layer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (!s_is_time_returing) {
    swap_layers_animated();
  }
}

static void update_date() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char buffer[] = "00/00/00";
  strftime(buffer, sizeof(buffer), "%D", tick_time);
  
  text_layer_set_text(s_date_layer, buffer);
}

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char buffer[] = "00:00";

  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  if (settings.hour_vibe && (0 == tick_time->tm_min) && (0 == tick_time->tm_sec)) {
    vibes_short_pulse();
  }

  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  
  if(tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    dict_write_uint8(iter, 0, 0);
    
    app_message_outbox_send();
  }
}

static TextLayer* create_text_layer_by_dx() {
  TextLayer* created_layer;
  static int dx_counter = 0;
  
  switch(dx_counter) {
    case 1:
      created_layer = text_layer_create(GRect(144, 87, 144, 38));
      dx_counter = -1;
      break;
    case -1:
      created_layer = text_layer_create(GRect(-144, 87, 144, 38));
      dx_counter = -1;
      break;
    default:
      created_layer = text_layer_create(GRect(0, 87, 144, 38));
      dx_counter = 1;
      break;
  }
  
  return created_layer;
}

static void main_window_load(Window *window) {
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  s_time_layer = create_text_layer_by_dx();
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  s_current_layer = text_layer_get_layer(s_time_layer);
  
  s_date_layer = create_text_layer_by_dx();
  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00/00/00");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  s_weather_layer = create_text_layer_by_dx();
  text_layer_set_background_color(s_weather_layer, GColorBlack);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text(s_weather_layer, "Noided");
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  update_time();
  update_date();
}

static void main_window_unload(Window *window) {
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  text_layer_destroy(s_time_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
  
  while(t != NULL) {
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    t = dict_read_next(iterator);
  }
  
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  s_main_window = window_create();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}