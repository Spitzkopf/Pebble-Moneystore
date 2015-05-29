#include <pebble.h>

#include "LayerCollection.h"

#define PERSISTENT_SETTINGS_KEY 0xDEADBEEF

#define ANIM_DURATION 400
#define ANIM_DELAY 500

#define TIME_LAYER_RETUNR_TIME 2 * 1000

enum AppMessageCodes {
  KEY_TEMPERATURE = 0,
  KEY_CONDITIONS = 1
};

static Window *s_main_window;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;

static Layer* s_currently_showing_layer = NULL;

static void* layer_collection = NULL;

static AppTimer* s_time_return_timer = NULL;

typedef struct {
  int celsius;
  int bt_vibe;
  int hour_vibe;
} __attribute__((__packed__)) WatchSettings;

enum SettingsKeys {
  SETTINGS_CELSIUS = 10,
  SETTINGS_BTVIBE = 11,
  SETTINGS_HOURVIBE = 12,
  SETTINGS_NUM
};

WatchSettings settings = {
  .celsius = 1,
  .bt_vibe = 1,
  .hour_vibe = 1
};

static void tap_handler(AccelAxisType axis, int32_t direction);
static void time_layer_timeout_handler(void *data);

static void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay, AnimationStartedHandler on_started, AnimationStoppedHandler on_stopped, void* context) {
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
    
    AnimationHandlers handlers = {
        .started = (AnimationStartedHandler) on_started,
        .stopped = (AnimationStoppedHandler) on_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, context);
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

static void layer_to_show_end_callback(Animation *anim, bool finished, void *layer) {
    property_animation_destroy((PropertyAnimation*) anim);

    if (s_currently_showing_layer == text_layer_get_layer(s_time_layer)) {
      //time is now showing, resubscribe
      accel_tap_service_subscribe(tap_handler);
    }
    else {
      s_time_return_timer = app_timer_register(TIME_LAYER_RETUNR_TIME, (AppTimerCallback)time_layer_timeout_handler, NULL);
    }
}

static void layer_to_hide_start_callback(Animation *animation, void *context) {
  Layer* hidden = (Layer*)context;
  GRect next_layer_start = layer_get_frame(hidden);
  GRect next_layer_end = get_new_rect_for_layer(next_layer_start, 0);
  animate_layer(hidden, &next_layer_start, &next_layer_end, ANIM_DURATION, 0, NULL, layer_to_show_end_callback, NULL);
}

static void layer_to_hide_end_callback(Animation *anim, bool finished, void *layer) {
  property_animation_destroy((PropertyAnimation*) anim);
}

static void swap_layers(Layer* showing, Layer* hidden) {
  GRect current_layer_start = layer_get_frame(showing);
  GRect next_layer_start = layer_get_frame(hidden);
  GRect current_layer_end = get_new_rect_for_layer(current_layer_start, get_direction_for_swap(next_layer_start));
  
  s_currently_showing_layer = hidden;
  
  animate_layer(showing, &current_layer_start, &current_layer_end, ANIM_DURATION, ANIM_DELAY, layer_to_hide_start_callback, layer_to_hide_end_callback, (void*)hidden);
}

static void swap_layers_animated() {
  Layer* next_layer = get_next_layer(layer_collection);
  app_timer_cancel(s_time_return_timer);
  
  if (s_currently_showing_layer == next_layer) {
    next_layer = get_next_layer(layer_collection);
  }
  
  swap_layers(s_currently_showing_layer, next_layer);
}

static void time_layer_timeout_handler(void *data) {
   Layer* time_layer = text_layer_get_layer(s_time_layer);
   Layer* current_layer = get_current_layer(layer_collection);
   
   APP_LOG(APP_LOG_LEVEL_INFO, "Other Layer Timeout");
   
   if (time_layer == s_currently_showing_layer) {
     return;
   }
   
   accel_tap_service_unsubscribe();   

   swap_layers(current_layer, text_layer_get_layer(s_time_layer));
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
    swap_layers_animated();
}

static void update_date() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char buffer[] = "00/00/00";
  strftime(buffer, sizeof(buffer), "%d/%m/%y", tick_time);
  
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

static void update_weather_layer(int weather, char* condition)
{
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[64];
  char weather_char = settings.celsius ? 'C' : 'F';
  
  if (-1 != weather)
  {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d%c°", weather, weather_char);
  }
  
  if (condition)
  {
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", condition);
  }
  
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
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
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  s_date_layer = create_text_layer_by_dx();
  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00/00/00");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  s_weather_layer = create_text_layer_by_dx();
  text_layer_set_background_color(s_weather_layer, GColorBlack);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text(s_weather_layer, "Noided");
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  layer_collection = init_layer_collection();
  add_layer(layer_collection, text_layer_get_layer(s_time_layer));
  add_layer(layer_collection, text_layer_get_layer(s_date_layer));
  add_layer(layer_collection, text_layer_get_layer(s_weather_layer));
  
  s_currently_showing_layer = get_current_layer(layer_collection);
  
  update_time();
  update_date();
}

static void main_window_unload(Window *window) {
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  destroy_layer_collection(layer_collection);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
  
  int temperature = -1;
  char* condition = NULL;
  
  while(t != NULL) {
    switch(t->key) {
    case KEY_TEMPERATURE:
      temperature = (int)t->value->int32;
      break;
    case KEY_CONDITIONS:
      condition = t->value->cstring;
      break;
    case SETTINGS_BTVIBE:
      settings.bt_vibe = (int)t->value->int32;
      break;    
    case SETTINGS_HOURVIBE:
      settings.hour_vibe = (int)t->value->int32;
      break;
    case SETTINGS_CELSIUS:
      settings.celsius = (int)t->value->int32;
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    t = dict_read_next(iterator);
  }
  
  update_weather_layer(temperature, condition);
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

static void load_settings() {
  persist_read_data(PERSISTENT_SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
  persist_write_data(PERSISTENT_SETTINGS_KEY, &settings, sizeof(settings));
}

static void init() {
  load_settings();
  
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
  save_settings();
  window_destroy(s_main_window);
  accel_tap_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}