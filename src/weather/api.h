#ifndef OPEN_METEO_API_H
#define OPEN_METEO_API_H

#define OPEN_METEO_API "https://api.open-meteo.com/v1/forecast?latitude=36.7477&longitude=-119.7724&current=temperature_2m,relative_humidity_2m&wind_speed_unit=mph&temperature_unit=fahrenheit&precipitation_unit=inch" // add current temperature (F) and current humidity URL

// default weather data consts
inline float IMPOSSIBLE_TEMPERATURE = 200; // impossible value to check as a default value
inline float IMPOSSIBLE_HUMIDITY = 101; // impossible value to check as a default value

// Open-Meteo weather variables
extern float currentTemperature = IMPOSSIBLE_TEMPERATURE;
extern float currentRelativeHumidity = IMPOSSIBLE_HUMIDITY;


#endif /* OPEN_METEO_API_H */