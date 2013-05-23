/*
	Pebbler's face
*/

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <string.h>
#include <stdlib.h>
#include "sync.h"

#define MY_UUID {0x64, 0x8c, 0xcb, 0x50, 0xb6, 0x0c, 0x42, 0xea, 0x98, 0x3d, 0xae, 0x13, 0x01, 0x49, 0x2b, 0x08}

PBL_APP_INFO(MY_UUID, "Pebblers Face", "Bartosz Grabowski", 3, 2, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define DATE_TOP_PADDING 12
#define PADDING 10
#define LINE_HEIGHT 50

Window window;

PblTm display_time;

TextLayer date_layer;
TextLayer time_layer;
Layer lineLayer;
		
void update_time_text(PblTm *current_time) {
	static char time_text[] = "00:00";
	if(clock_is_24h_style())
		string_format_time(time_text, sizeof(time_text), "%R", current_time);
	else
		string_format_time(time_text, sizeof(time_text), "%I:%M", current_time);
	text_layer_set_text(&time_layer, time_text);
	display_time.tm_min = current_time->tm_min;
}

void update_date_text(PblTm *current_time) {
  static char date_text[] = "Xxxxxxxxx 00";
  if(current_time->tm_wday == 3)
    // "Wednesday" doesn't fit on the screen so just show "Wed"
    string_format_time(date_text, sizeof(date_text), "%a %e", current_time);
  else
    string_format_time(date_text, sizeof(date_text), "%A %e", current_time);
  text_layer_set_text(&date_layer, date_text);
  display_time.tm_mday = current_time->tm_mday;
}

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_draw_line(ctx, GPoint(PADDING, DATE_TOP_PADDING + LINE_HEIGHT - 3), GPoint(SCREEN_WIDTH - PADDING, DATE_TOP_PADDING + LINE_HEIGHT - 3));
  graphics_draw_line(ctx, GPoint(PADDING, DATE_TOP_PADDING + LINE_HEIGHT - 4), GPoint(SCREEN_WIDTH - PADDING, DATE_TOP_PADDING + LINE_HEIGHT - 4));
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)t;
  (void)ctx;

  PblTm current_time;

  if(t)
    current_time = *t->tick_time;
  else
    get_time(&current_time);

  if(current_time.tm_min != display_time.tm_min)
    update_time_text(&current_time);

  if(current_time.tm_mday != display_time.tm_mday)
    update_date_text(&current_time);
	
	// additional update each 5 minutes, in case phone app missed some event (like reading sms in external app) 
   if((current_time.tm_min %5) == 0) if(current_time.tm_sec == 0) requestPhoneData();
}

void handle_init_app(AppContextRef app_ctx) {
	window_init(&window, "Pebbler");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);
	
	text_layer_init(&date_layer, GRect(PADDING*1.5, PADDING + DATE_TOP_PADDING, SCREEN_WIDTH-PADDING, LINE_HEIGHT));
	text_layer_set_text_color(&date_layer, GColorWhite);
	text_layer_set_background_color(&date_layer, GColorClear);
	text_layer_set_font(&date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	layer_add_child(&window.layer, &date_layer.layer);
	
	text_layer_init(&time_layer, GRect(0, PADDING + LINE_HEIGHT*1, SCREEN_WIDTH, LINE_HEIGHT));
	text_layer_set_text_color(&time_layer, GColorWhite);
	text_layer_set_text_alignment(&time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(&time_layer, GColorClear);
	text_layer_set_font(&time_layer, fonts_get_system_font(FONT_KEY_GOTHAM_42_MEDIUM_NUMBERS));
	layer_add_child(&window.layer, &time_layer.layer);
	
	text_layer_init(&info_layer, GRect(0, LINE_HEIGHT*2, SCREEN_WIDTH, LINE_HEIGHT));
	text_layer_set_text_color(&info_layer, GColorWhite);
	text_layer_set_text_alignment(&info_layer, GTextAlignmentCenter);
	text_layer_set_background_color(&info_layer, GColorClear);
	text_layer_set_font(&info_layer, fonts_get_system_font(FONT_KEY_GOTHAM_42_LIGHT));
	layer_add_child(&window.layer, &info_layer.layer);
	
	layer_init(&lineLayer, window.layer.frame);
	lineLayer.update_proc = &line_layer_update_callback;
	layer_add_child(&window.layer, &lineLayer);
	
	
	// request data refresh on window appear (for example after notification)
	WindowHandlers handlers = { .appear = &requestPhoneData };
	window_set_window_handlers(&window, handlers);
	// init AppSync subsystem
	init_sync();	
	
	handle_second_tick(app_ctx, NULL);
}


void pbl_main(void *params) {
   PebbleAppHandlers handlers = {
    .init_handler = &handle_init_app,
    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT  // for this watchface it would be enaought to use MINUTE tick, seconds are used for easier modifciations
	},

	.deinit_handler = &app_deinit,
	.messaging_info = {
      .buffer_sizes = {
        .inbound = 32, 
        .outbound = 16
	  },
	}
   };
  app_event_loop(params, &handlers);
}