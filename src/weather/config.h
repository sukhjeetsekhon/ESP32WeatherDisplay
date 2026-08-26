#ifndef CONFIG_H
#define CONFIG_H

typedef uint16_t DHTSizeType;

#define USE_FAHRENHEIT true // must be a define to use #if

// Task Timing config
inline unsigned int API_CALL_DELAY = 60000;
inline unsigned int DISPLAY_REFRESH_DELAY = 1000;
inline unsigned int DHT_UPDATE_DELAY = 1000; // 1000ms is the minimum delay between readings

// DHT11 config
inline DHTSizeType DHT_QUEUE_SIZE = 100; // a larger size will change the average values more slowly and stabilize output measurements


#endif /* CONFIG_H */