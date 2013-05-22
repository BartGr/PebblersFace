#define STATE_LENGTH 6
#define SYNC_BUFFER_SIZE 32
	
enum {
	TUPLE_STATE = 0x0, // TUPLE_STRING		
	TUPLE_MISSED_CALLS = 0x1, // TUPLE_INT
	TUPLE_UNREAD_SMS = 0x2, // TUPLE_INT
	// TUPLE_BATTERY_LEVEL = 0x2, // TUPLE_INT
	// TUPLE_NETWORK = 0x3, // TUPLE_INT
};

enum {
	REQUEST_DATA_KEY = 0x10, 
};

enum { REQUEST_DATA = 0x0};// TUPLE_INTEGER

TextLayer info_layer;

static struct PhoneStateData {
  uint8_t missedCalls;
  uint8_t unreadSms;
  AppSync sync;
  uint8_t sync_buffer[SYNC_BUFFER_SIZE]; 
} s_data;


static bool callbacks_registered;
static AppMessageCallbacksNode app_callbacks;

void init_sync();
void app_deinit(AppContextRef c);
void requestPhoneData();

static bool register_callbacks();
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context);
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context);
static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context);
static void app_received_msg(DictionaryIterator* received, void* context);
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context);
static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context);
static void app_received_msg(DictionaryIterator* received, void* context);
