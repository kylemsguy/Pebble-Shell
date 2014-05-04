/*
    Pebble Terminal: A Pebble face that simulates a linux terminal running the date command.
	Copyright (C) 2014  David Tschida

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *time_layer; 
static TextLayer *dprompt_layer; 
static TextLayer *prompt_layer; 
static TextLayer *date_layer; 

static AppTimer *timer; 

GFont start_font;// = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_START_12) );
GFont font_large; 

static char _24hourmin[] = "~$date +%H:%M";
static char _12hourmin[] = "~$date +%I:%M";
static char *hourmin;

static char _24hourformat[] = "%H:%M";
static char _12hourformat[] = "%I:%M";
static char *timeFormat;

static char timecmd[] = "             "; 
static char monthday[] = "~$date +%h\\ %d";
static char datecmd[] =  "              ";

static void animateDatePrompt();
static void animateTimePrompt();

static struct tm* lastTime;

static int TYPING_TICK = 150; 

static bool promptVisible = false; 

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	start_font = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_16) );	
	font_large = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_30) );

	strcpy(timecmd, hourmin); 
	strcpy(datecmd, monthday); 

	text_layer = text_layer_create((GRect) { .origin = {5,5}, .size = { bounds.size.w, 20 } });
	text_layer_set_text(text_layer, timecmd);
	text_layer_set_text_color(text_layer, GColorClear);
	text_layer_set_background_color(text_layer, GColorClear); 
	text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
	text_layer_set_font(text_layer, start_font); 

	time_layer = text_layer_create((GRect) { .origin = {5, 20}, .size = { bounds.size.w, 30 } });

	text_layer_set_text_color(time_layer, GColorClear);
	text_layer_set_background_color(time_layer, GColorClear); 
	text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
	text_layer_set_font(time_layer, font_large); 

	dprompt_layer = text_layer_create((GRect) { .origin = {5,55}, .size = {bounds.size.w, 20}});
	text_layer_set_text(dprompt_layer, monthday);
	text_layer_set_text_color(dprompt_layer, GColorClear);
	text_layer_set_background_color(dprompt_layer, GColorClear); 
	text_layer_set_text_alignment(dprompt_layer, GTextAlignmentLeft);
	text_layer_set_font(dprompt_layer, start_font); 

	date_layer = text_layer_create((GRect) { .origin = {5, 70}, .size = { bounds.size.w, 39 } });

	text_layer_set_text_color(date_layer, GColorClear);
	text_layer_set_background_color(date_layer, GColorClear); 
	text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
	text_layer_set_font(date_layer, font_large); 

	prompt_layer = text_layer_create((GRect) { .origin = {5,80+25}, .size = {bounds.size.w, 20} });
	//text_layer_set_text(prompt_layer, "~$_");
	text_layer_set_text_color(prompt_layer, GColorClear);
	text_layer_set_background_color(prompt_layer, GColorClear); 
	text_layer_set_text_alignment(prompt_layer, GTextAlignmentLeft);
	text_layer_set_font(prompt_layer, start_font); 

	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(dprompt_layer));
	layer_add_child(window_layer, text_layer_get_layer(prompt_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	text_layer_destroy(dprompt_layer);
	text_layer_destroy(prompt_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);

	fonts_unload_custom_font(start_font); 
	fonts_unload_custom_font(font_large); 
}




static void handleMinuteTick(struct tm* now, TimeUnits units_changed)
{
	text_layer_set_text_color(time_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorClear); 
	text_layer_set_text_color(dprompt_layer, GColorClear);
	text_layer_set_text_color(prompt_layer, GColorClear);

	lastTime = now; 

	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 52, "---Minute tick %d", now->tm_min); 

	unsigned int v = 0; 
	for(v = 0; v < strlen(hourmin); v++) timecmd[v] = ' ';
	for(v = 0; v < strlen(datecmd); v++) datecmd[v] = ' ';

	text_layer_set_text(time_layer, "");
	text_layer_set_text(date_layer, "");
	text_layer_set_text(dprompt_layer, "");
	text_layer_set_text(prompt_layer, "");

	promptVisible=false; 
	
	timer = app_timer_register(200, animateTimePrompt, 0);	
}


static void animateTimePrompt()
{
	static unsigned int i = 2; 

	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 97, "i: %d", i); 
	strncpy(timecmd, hourmin, i++);
	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 60, "timecmd: \"%s\"", timecmd); 
	text_layer_set_text(text_layer, timecmd);
	if( i > strlen(hourmin))
	{
		app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 97, "Typed word!!: %d", i); 
		i = 2;
		text_layer_set_text_color(time_layer, GColorWhite);	
		text_layer_set_text_color(dprompt_layer, GColorWhite);	
		
		static char time[] = "00:00"; 
		strftime(time, sizeof(time),timeFormat, lastTime); 
		text_layer_set_text(time_layer, time);

		//app_timer_cancel(timer); 
		timer = app_timer_register(TYPING_TICK, animateDatePrompt, 0); 
	}
	else
		timer = app_timer_register(TYPING_TICK, animateTimePrompt, 0); 
}

static void animateDatePrompt()
{
	static int i = 2; 

	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 97, "i: %d", i); 
	strncpy(datecmd, monthday, i++);
	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 60, "datacmd: \"%s\"", datecmd); 
	text_layer_set_text(dprompt_layer, datecmd);
	if((unsigned int) i > strlen(monthday))
	{
		app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 97, "Typed word!!: %d", i); 
		i = 2;
		text_layer_set_text_color(date_layer, GColorWhite);	
		text_layer_set_text_color(prompt_layer, GColorWhite); 
		//timer = app_timer_register(TYPE_TIME, animateDatePrompt, 0); 
		//app_timer_cancel(timer);
		
		static char date[] = "      ";
		strftime(date, sizeof(date), "%h %d", lastTime); 
		text_layer_set_text(date_layer, date); 

		promptVisible = true; 
	}
	else
		timer = app_timer_register(TYPING_TICK, animateDatePrompt, 0); 
}

static void handleSecondTick(struct tm* now, TimeUnits units_changed)
{
	if(promptVisible)
	{
		app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 60, "Second tick %d", now->tm_sec); 
		static char prompt[] = "~$      ";
		static bool cursor = true; 
		static int cursor_loc = 2; 

		if(now->tm_sec < 57) 
		{
			if( prompt[3] != ' ')
			{
				app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 67, "clearing prompt..."); 
				cursor_loc = 2; 
				strcpy(prompt+2, "      ");
			}
		}
		else if(prompt[3] != 'l')
		{	
			cursor_loc = 7; 
			strcpy(prompt+2, "clear ");
		}

		prompt[cursor_loc] = (cursor) ? '_' : ' '; 
		cursor = cursor ? false : true ; 
		text_layer_set_text(prompt_layer, prompt); 
	}
}
static void handleDayTick(struct tm* now, TimeUnits units_changed)
{
	static char date[] = "      ";

	strftime(date, sizeof(date), "%h %d", now); 
	text_layer_set_text(date_layer, date); 
}
static void handleTicks(struct tm* now, TimeUnits units_changed)
{
	if ((units_changed & SECOND_UNIT) != 0)
		handleSecondTick(now, units_changed); 
	if ((units_changed & MINUTE_UNIT) != 0)
		handleMinuteTick(now, units_changed);
//	if ((units_changed & DAY_UNIT) != 0)
//		handleDayTick(now, units_changed); 
}

static void init(void) {
	window = window_create();
	window_set_background_color(window, GColorBlack); 
	window_set_window_handlers(window, (WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
			});
	const bool animated = true;
	if(clock_is_24h_style())
	{
		
		APP_LOG(APP_LOG_LEVEL_DEBUG, "In 24 hour mode: %s", _24hourmin);
		hourmin = _24hourmin; 
		timeFormat = _24hourformat; 
	}
	else 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "In 12 hour mode: %s", _12hourmin);
		hourmin = _12hourmin; 
		timeFormat = _12hourformat; 

	}

	window_stack_push(window, animated);

	time_t now = time(NULL); 
	struct tm *currentTime = localtime(&now); 
	handleTicks(currentTime, MINUTE_UNIT | SECOND_UNIT | DAY_UNIT);
	tick_timer_service_subscribe(SECOND_UNIT, &handleTicks); 
}

static void deinit(void) {
	text_layer_destroy(text_layer);
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
