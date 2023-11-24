// --- includes ----------------------------------------------------------------
#include "timestamp.h"
#include "date_time.h"

// --- defines -----------------------------------------------------------------
#define UPDATE_TIMESTAMP_RETRIES 3
// --- static variables definitions --------------------------------------------
static int64_t timestamp = 0;

// --- structs -----------------------------------------------------------------
/**
 * @brief function to update timestamp
 * 
 */
void update_timestamp(void)
{
    int err = 0;
    uint8_t retries = 0;
    // Get calendar time
    while(retries < UPDATE_TIMESTAMP_RETRIES)
    {
        err = date_time_now(&timestamp);
        if(err == 0)
        {
            break;
        }
        retries++;
    }
    // Add 2 hours to Unix time to match our timezone offset
    // TODO: Summer time is adjusted in database_tools.py
    timestamp += 7200000;
}

/**
 * @brief Get the timestamp object
 * 
 * @return int64_t 
 */
int64_t get_timestamp(void)
{
    return timestamp;
}