#include "pebble_os.h"
#include "pebble_app.h"
#include "sync.h"

static void intToString(int num, char* res) {
	if(num == 0){
		res[0] = '0';
		res[1] = '\0';
		return;
	}
	
	int len = 0;
	for(; num > 0; ++len)
	{
		res[len] = num%10+'0';
		num/=10;
	}
	res[len] = 0; //null-terminating
	for(int i = 0; i < len/2; ++i)
	{
		char c = res[i]; 
		res[i] = res[len-i-1];
		res[len-i-1] = c;
	}
}

static void put_state(char* state){
	text_layer_set_text(&info_layer, state);
}

static void update_counters()
{
	static char* state = "00/00";
	const char* separator = "/";
	char* missed = "0c";
	char* sms = "0m";

	intToString(s_data.missedCalls, missed); 
	intToString(s_data.unreadSms, sms);
	
	uint missedLength = ARRAY_LENGTH(missed);
	uint sepLength = ARRAY_LENGTH(separator);
	uint smsLength = ARRAY_LENGTH(sms);
	
	for(uint i = 0; i < STATE_LENGTH; i++) {
		if(i<missedLength) {
			if(missed[i] != 0) state[i] = missed[i];
			else missedLength = i;		
		}
		if((i>=missedLength) & (i<sepLength+missedLength)) {
			if(separator[i-missedLength] != 0) state[i] = separator[i-missedLength];
			else sepLength = i-missedLength;
		}	
		if((i>=missedLength+sepLength) & (i<smsLength+missedLength+sepLength)) {
			if(sms[i-missedLength-sepLength] != 0) state[i] = sms[i-missedLength-sepLength];
			else smsLength = i-missedLength-sepLength;
		}
		if(i>=missedLength+sepLength+smsLength)		
		{
			state[i] = 0;
		}
	}
	put_state(state);		
}

 
static void set_reason(int reason) {
	switch(reason){
		case APP_MSG_NOT_CONNECTED: 
			// phone is disconnected	
			put_state("?");
			break;
/*
		case APP_MSG_SEND_TIMEOUT: put_state("time");
			break;
		case APP_MSG_SEND_REJECTED: put_state("rjct");
			break;
		case APP_MSG_APP_NOT_RUNNING: put_state("nrun");
			break;
		case APP_MSG_OK: put_state("ok");
			break;
		case APP_MSG_INVALID_ARGS: put_state("inva");
			break;
		case APP_MSG_BUSY: put_state("busy");
			break;
		case APP_MSG_BUFFER_OVERFLOW: put_state("over");
			break;
		case APP_MSG_ALREADY_RELEASED: put_state("rlsd");
			break;
		case APP_MSG_CALLBACK_ALREADY_REGISTERED: put_state("cllb");
			break;
		case APP_MSG_CALLBACK_NOT_REGISTERED: put_state("clnt");
			break;
		default: put_state("unkn");
			break;
*/
     }
}	
	
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	(void) old_tuple;
	switch (key) {
	  case TUPLE_MISSED_CALLS:
		s_data.missedCalls = new_tuple->value->uint8;
		break;
	  case TUPLE_UNREAD_SMS:
		s_data.unreadSms = new_tuple->value->uint8;
		break;		
	  case TUPLE_STATE:		
		break;
  }
	// draw only if value changed
	if(new_tuple->value!=old_tuple->value) update_counters();
}	


void init_sync() {	
	Tuplet initial_values[] = {
		      	TupletCString(TUPLE_STATE, "       "), 	  
				TupletInteger(TUPLE_MISSED_CALLS, (uint8_t) 0),
		  		TupletInteger(TUPLE_UNREAD_SMS, (uint8_t) 0),  
	};  
	
	app_sync_init
		(&s_data.sync, 					
		 s_data.sync_buffer, 			
		 sizeof(s_data.sync_buffer), 
		 initial_values, 
		 ARRAY_LENGTH(initial_values),
		 sync_tuple_changed_callback, 
		 sync_error_callback,
		 NULL);	
	
	register_callbacks();
	requestPhoneData(); 
}


void requestPhoneData(){
  Tuplet value = TupletInteger(REQUEST_DATA_KEY, REQUEST_DATA);
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL) return;
  	
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}
	

static bool register_callbacks() {
	if (callbacks_registered) {
		if (app_message_deregister_callbacks(&app_callbacks) == APP_MSG_OK)
			callbacks_registered = false;
	}
	if (!callbacks_registered) {
		app_callbacks = (AppMessageCallbacksNode){
			.callbacks = {
				.out_failed = app_send_failed,
        .in_received = app_received_msg
			},
			.context = NULL
		};
		if (app_message_register_callbacks(&app_callbacks) == APP_MSG_OK) {
      callbacks_registered = true;
      }
	}
	return callbacks_registered;
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  (void) dict_error;
  (void) app_message_error;
  (void) context;
	
	set_reason(app_message_error);
}

static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context) {
	put_state("fail!");
}


static void app_received_msg(DictionaryIterator* received, void* context) {	
	// light_enable_interaction();
}

 void app_deinit(AppContextRef c) {
    app_sync_deinit(&s_data.sync);
 }
