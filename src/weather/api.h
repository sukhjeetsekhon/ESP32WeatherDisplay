#ifndef OPEN_METEO_API_H
#define OPEN_METEO_API_H

#define OPEN_METEO_API "https://api.open-meteo.com/v1/forecast?latitude=36.7477&longitude=-119.7724&current=temperature_2m,relative_humidity_2m,weather_code&wind_speed_unit=mph&temperature_unit=fahrenheit&precipitation_unit=inch" // add current temperature (F) and current humidity URL

// default weather data consts
inline float IMPOSSIBLE_TEMPERATURE = 200; // impossible value to check as a default value
inline float IMPOSSIBLE_HUMIDITY = 101; // impossible value to check as a default value
inline int IMPOSSIBLE_WEATHER_CODE = -1; // 1 is an invalid number for WMO code

inline int CLEAR_SKY_CODE = 0;
inline int MAINLY_CLEAR_CODE = 1;
inline int PARTLY_CLOUDY_CODE = 2;
inline int OVERCAST_CODE = 3;
inline int FOG_CODE = 45;
inline int DEPOSITING_RIME_FOG_CODE = 48;
inline int LIGHT_DRIZZLE_CODE = 51;
inline int MODERATE_DRIZZLE_CODE = 53;
inline int DENSE_DRIZZLE_CODE = 55;
inline int LIGHT_FREEZING_DRIZZLE_CODE = 56;
inline int DENSE_FREEZING_DRIZZLE_CODE = 57;
inline int SLIGHT_RAIN_CODE = 61;
inline int MODERATE_RAIN_CODE = 63;
inline int HEAVY_RAIN_CODE = 65;
inline int LIGHT_FREEZING_RAIN_CODE = 66;
inline int HEAVY_FREEZING_RAIN_CODE = 67;
inline int SLIGHT_SNOW_FALL_CODE = 71;
inline int MODERATE_SNOW_FALL_CODE = 73;
inline int HEAVY_SNOW_FALL_CODE = 75;
inline int SNOW_GRAINS_CODE = 77;
inline int SLIGHT_RAIN_SHOWERS_CODE = 85;
inline int HEAVY_RAIN_SHOWERS_CODE = 86;
inline int THUNDERSTORM_CODE = 95;
inline int THUNDERSTORM_WITH_SLIGHT_HAIL_CODE = 96;
inline int THUNDERSTORM_WITH_HEAVY_HAIL_CODE = 99;

// checks if the weather is only rain
inline bool isRain(int weatherCode) {
   return LIGHT_DRIZZLE_CODE == weatherCode ||
   MODERATE_DRIZZLE_CODE == weatherCode ||
   DENSE_DRIZZLE_CODE == weatherCode ||
   LIGHT_FREEZING_DRIZZLE_CODE == weatherCode ||
   DENSE_FREEZING_DRIZZLE_CODE == weatherCode ||
   SLIGHT_RAIN_CODE == weatherCode ||
   MODERATE_RAIN_CODE == weatherCode ||
   HEAVY_RAIN_CODE == weatherCode ||
   LIGHT_FREEZING_RAIN_CODE == weatherCode ||
   HEAVY_FREEZING_RAIN_CODE == weatherCode ||
   SLIGHT_RAIN_SHOWERS_CODE == weatherCode ||
   HEAVY_RAIN_SHOWERS_CODE == weatherCode;
}

// checks if the weather is only thunderstorm
inline bool isThunder(int weatherCode) {
   return THUNDERSTORM_CODE == weatherCode ||
   THUNDERSTORM_WITH_SLIGHT_HAIL_CODE == weatherCode ||
   THUNDERSTORM_WITH_HEAVY_HAIL_CODE == weatherCode;
}

// checks if the weather is overcast
inline bool isCloudy(const int weatherCode) {
   return OVERCAST_CODE == weatherCode;
}

// returns if weather is partly cloudy
inline bool isCloudySun(const int weatherCode) {
   return PARTLY_CLOUDY_CODE == weatherCode;
}

// returns if weather is mostly clear or clear
inline bool isSunny(const int weatherCode) {
   return CLEAR_SKY_CODE == weatherCode ||
   MAINLY_CLEAR_CODE == weatherCode;
}

// returns if snow fall or snow grains
inline bool isSnowy(const int weatherCode) {
   return SLIGHT_SNOW_FALL_CODE == weatherCode ||
   MODERATE_SNOW_FALL_CODE == weatherCode ||
   HEAVY_SNOW_FALL_CODE == weatherCode ||
   SNOW_GRAINS_CODE == weatherCode;
}

// Open-Meteo weather variables
extern float currentTemperature = IMPOSSIBLE_TEMPERATURE;
extern float currentRelativeHumidity = IMPOSSIBLE_HUMIDITY;
extern int weatherCode = IMPOSSIBLE_WEATHER_CODE;

#endif /* OPEN_METEO_API_H */