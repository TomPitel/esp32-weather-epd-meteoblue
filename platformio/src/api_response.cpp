/* API response deserialization for esp32-weather-epd.
 * Copyright (C) 2022-2024  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <vector>
#include <ArduinoJson.h>
#include "api_response.h"
#include "config.h"

#include "timestamp_utils.h"

DeserializationError deserializeOneCall(WiFiClient &json,
                                        owm_resp_onecall_t &r)
{
  int i;

  JsonDocument filter;
  filter["current"]  = true;
  filter["minutely"] = false;
  filter["hourly"]   = true;
  filter["daily"]    = true;
#if !DISPLAY_ALERTS
  filter["alerts"]   = false;
#else
  // description can be very long so they are filtered out to save on memory
  // along with sender_name
  for (int i = 0; i < OWM_NUM_ALERTS; ++i)
  {
    filter["alerts"][i]["sender_name"] = false;
    filter["alerts"][i]["event"]       = true;
    filter["alerts"][i]["start"]       = true;
    filter["alerts"][i]["end"]         = true;
    filter["alerts"][i]["description"] = false;
    filter["alerts"][i]["tags"]        = true;
  }
#endif

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, json,
                                         DeserializationOption::Filter(filter));
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : "
                 + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  r.lat             = doc["lat"]            .as<float>();
  r.lon             = doc["lon"]            .as<float>();
  r.timezone        = doc["timezone"]       .as<const char *>();
  r.timezone_offset = doc["timezone_offset"].as<int>();

  JsonObject current = doc["current"];
  r.current.dt         = current["dt"]        .as<int64_t>();
  r.current.sunrise    = current["sunrise"]   .as<int64_t>();
  r.current.sunset     = current["sunset"]    .as<int64_t>();
  r.current.temp       = current["temp"]      .as<float>();
  r.current.feels_like = current["feels_like"].as<float>();
  r.current.pressure   = current["pressure"]  .as<int>();
  r.current.humidity   = current["humidity"]  .as<int>();
  r.current.dew_point  = current["dew_point"] .as<float>();
  r.current.clouds     = current["clouds"]    .as<int>();
  r.current.uvi        = current["uvi"]       .as<float>();
  r.current.visibility = current["visibility"].as<int>();
  r.current.wind_speed = current["wind_speed"].as<float>();
  r.current.wind_gust  = current["wind_gust"] .as<float>();
  r.current.wind_deg   = current["wind_deg"]  .as<int>();
  r.current.rain_1h    = current["rain"]["1h"].as<float>();
  r.current.snow_1h    = current["snow"]["1h"].as<float>();
  JsonObject current_weather = current["weather"][0];
  r.current.weather.id          = current_weather["id"]         .as<int>();
  r.current.weather.main        = current_weather["main"]       .as<const char *>();
  r.current.weather.description = current_weather["description"].as<const char *>();
  r.current.weather.icon        = current_weather["icon"]       .as<const char *>();

  // minutely forecast is currently unused
  // i = 0;
  // for (JsonObject minutely : doc["minutely"].as<JsonArray>())
  // {
  //   r.minutely[i].dt            = minutely["dt"]           .as<int64_t>();
  //   r.minutely[i].precipitation = minutely["precipitation"].as<float>();

  //   if (i == OWM_NUM_MINUTELY - 1)
  //   {
  //     break;
  //   }
  //   ++i;
  // }

  i = 0;
  for (JsonObject hourly : doc["hourly"].as<JsonArray>())
  {
    r.hourly[i].dt         = hourly["dt"]        .as<int64_t>();
    r.hourly[i].temp       = hourly["temp"]      .as<float>();
    r.hourly[i].feels_like = hourly["feels_like"].as<float>();
    r.hourly[i].pressure   = hourly["pressure"]  .as<int>();
    r.hourly[i].humidity   = hourly["humidity"]  .as<int>();
    r.hourly[i].dew_point  = hourly["dew_point"] .as<float>();
    r.hourly[i].clouds     = hourly["clouds"]    .as<int>();
    r.hourly[i].uvi        = hourly["uvi"]       .as<float>();
    r.hourly[i].visibility = hourly["visibility"].as<int>();
    r.hourly[i].wind_speed = hourly["wind_speed"].as<float>();
    r.hourly[i].wind_gust  = hourly["wind_gust"] .as<float>();
    r.hourly[i].wind_deg   = hourly["wind_deg"]  .as<int>();
    r.hourly[i].pop        = hourly["pop"]       .as<float>();
    r.hourly[i].rain_1h    = hourly["rain"]["1h"].as<float>();
    r.hourly[i].snow_1h    = hourly["snow"]["1h"].as<float>();
    JsonObject hourly_weather = hourly["weather"][0];
    r.hourly[i].weather.id          = hourly_weather["id"]         .as<int>();
    r.hourly[i].weather.main        = hourly_weather["main"]       .as<const char *>();
    r.hourly[i].weather.description = hourly_weather["description"].as<const char *>();
    r.hourly[i].weather.icon        = hourly_weather["icon"]       .as<const char *>();

    if (i == OWM_NUM_HOURLY - 1)
    {
      break;
    }
    ++i;
  }

  i = 0;
  for (JsonObject daily : doc["daily"].as<JsonArray>())
  {
    r.daily[i].dt         = daily["dt"]        .as<int64_t>();
    r.daily[i].sunrise    = daily["sunrise"]   .as<int64_t>();
    r.daily[i].sunset     = daily["sunset"]    .as<int64_t>();
    r.daily[i].moonrise   = daily["moonrise"]  .as<int64_t>();
    r.daily[i].moonset    = daily["moonset"]   .as<int64_t>();
    r.daily[i].moon_phase = daily["moon_phase"].as<float>();
    JsonObject daily_temp = daily["temp"];
    r.daily[i].temp.morn  = daily_temp["morn"] .as<float>();
    r.daily[i].temp.day   = daily_temp["day"]  .as<float>();
    r.daily[i].temp.eve   = daily_temp["eve"]  .as<float>();
    r.daily[i].temp.night = daily_temp["night"].as<float>();
    r.daily[i].temp.min   = daily_temp["min"]  .as<float>();
    r.daily[i].temp.max   = daily_temp["max"]  .as<float>();
    JsonObject daily_feels_like = daily["feels_like"];
    r.daily[i].feels_like.morn  = daily_feels_like["morn"] .as<float>();
    r.daily[i].feels_like.day   = daily_feels_like["day"]  .as<float>();
    r.daily[i].feels_like.eve   = daily_feels_like["eve"]  .as<float>();
    r.daily[i].feels_like.night = daily_feels_like["night"].as<float>();
    r.daily[i].pressure   = daily["pressure"]  .as<int>();
    r.daily[i].humidity   = daily["humidity"]  .as<int>();
    r.daily[i].dew_point  = daily["dew_point"] .as<float>();
    r.daily[i].clouds     = daily["clouds"]    .as<int>();
    r.daily[i].uvi        = daily["uvi"]       .as<float>();
    r.daily[i].visibility = daily["visibility"].as<int>();
    r.daily[i].wind_speed = daily["wind_speed"].as<float>();
    r.daily[i].wind_gust  = daily["wind_gust"] .as<float>();
    r.daily[i].wind_deg   = daily["wind_deg"]  .as<int>();
    r.daily[i].pop        = daily["pop"]       .as<float>();
    r.daily[i].rain       = daily["rain"]      .as<float>();
    r.daily[i].snow       = daily["snow"]      .as<float>();
    JsonObject daily_weather = daily["weather"][0];
    r.daily[i].weather.id          = daily_weather["id"]         .as<int>();
    r.daily[i].weather.main        = daily_weather["main"]       .as<const char *>();
    r.daily[i].weather.description = daily_weather["description"].as<const char *>();
    r.daily[i].weather.icon        = daily_weather["icon"]       .as<const char *>();

    if (i == OWM_NUM_DAILY - 1)
    {
      break;
    }
    ++i;
  }

#if DISPLAY_ALERTS
  i = 0;
  for (JsonObject alerts : doc["alerts"].as<JsonArray>())
  {
    owm_alerts_t new_alert = {};
    // new_alert.sender_name = alerts["sender_name"].as<const char *>();
    new_alert.event       = alerts["event"]      .as<const char *>();
    new_alert.start       = alerts["start"]      .as<int64_t>();
    new_alert.end         = alerts["end"]        .as<int64_t>();
    // new_alert.description = alerts["description"].as<const char *>();
    new_alert.tags        = alerts["tags"][0]    .as<const char *>();
    r.alerts.push_back(new_alert);

    if (i == OWM_NUM_ALERTS - 1)
    {
      break;
    }
    ++i;
  }
#endif

  return error;
} // end deserializeOneCall

// Deserialize meteoblue JSON into owm_resp_onecall_t
DeserializationError deserializeMeteoBlue(WiFiClient &json, owm_resp_onecall_t &r)
{
  int i;
  JsonDocument doc;
 
  Serial.println("Receiving JSON data...");
  // from the server, read them and print them:
  char c = ' ';
  while (json.available() && c != '\n') {
    c = json.read();
    Serial.print(c);
  }
  Serial.println("\nStopping JSON data read.");

  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  // Set coordinates and timezone info
  r.lat = doc["metadata"]["latitude"].as<float>();
  r.lon = doc["metadata"]["longitude"].as<float>();
  r.timezone = doc["metadata"]["timezone_abbrevation"].as<const char *>();
  r.timezone_offset = doc["metadata"]["utc_timeoffset"].as<int>() * 3600;

  // --- CURRENT ---
  JsonObject current = doc["data_current"];
  const char* current_time = current["time"].as<const char *>();
  r.current.dt = timestampToUnix(current_time);
  // For sunrise/sunset, get from data_day if available
  JsonObject day = doc["data_day"];
  int todayIdx = 0;
  if (day["time"]) {
    JsonArray day_times = day["time"].as<JsonArray>();
    for (size_t d = 0; d < day_times.size(); ++d) {
      if (current_time && strncmp(current_time, day_times[d], 10) == 0) {
        todayIdx = d;
        break;
      }
    }
  }
  // Compose base date string for HH:MM fields
  char baseDate[11] = "";
  if (current_time) {
    strncpy(baseDate, current_time, 10);
    baseDate[10] = '\0';
  }
  // Sunrise/sunset
  if (day["sunrise"]) {
    JsonArray sunriseArr = day["sunrise"].as<JsonArray>();
    r.current.sunrise = timestampToUnix(sunriseArr[todayIdx], baseDate);
  } else {
    r.current.sunrise = 0;
  }
  if (day["sunset"]) {
    JsonArray sunsetArr = day["sunset"].as<JsonArray>();
    r.current.sunset = timestampToUnix(sunsetArr[todayIdx], baseDate);
  } else {
    r.current.sunset = 0;
  }
  // Find nearest hour index in data_1h
  JsonObject data1h = doc["data_1h"];
  int nearestIdx = 0;
  if (data1h["time"]) {
    JsonArray times = data1h["time"].as<JsonArray>();
    int64_t minDiff = INT64_MAX;
    for (size_t j = 0; j < times.size(); ++j) {
      int64_t t = timestampToUnix(times[j]);
      int64_t diff = abs((int64_t)r.current.dt - t);
      if (diff < minDiff) {
        minDiff = diff;
        nearestIdx = j;
      }
    }
  }

  //Serial.println("Nearest hour index for current weather: " + String(nearestIdx));
  //Serial.println("Current time: " + String(r.current.dt) + ", matched time: " + String(timestampToUnix(data1h["time"][nearestIdx])));

  // Fill current weather fields
  r.current.temp = current["temperature"].as<float>() + 273.15f; // Convert C to K
  r.current.feels_like = data1h["felttemperature"][nearestIdx].as<float>() + 273.15f;
  r.current.pressure = data1h["sealevelpressure"][nearestIdx].as<int>();
  r.current.humidity = data1h["relativehumidity"][nearestIdx].as<int>();
  r.current.dew_point = 0; // Not available
  r.current.clouds = data1h["totalcloudcover"][nearestIdx].as<int>();
  r.current.uvi = data1h["uvindex"][nearestIdx].as<float>();
  r.current.visibility = data1h["visibility"][nearestIdx].as<int>();
  r.current.wind_speed = data1h["windspeed"][nearestIdx].as<float>();
  r.current.wind_gust = 0; // Not available
  r.current.wind_deg = data1h["winddirection"][nearestIdx].as<int>();
  r.current.rain_1h = data1h["precipitation"][nearestIdx].as<float>();
  r.current.snow_1h = 0; // Not available
  r.current.weather.id = data1h["pictocode"][nearestIdx].as<int>();
  r.current.weather.main = "";
  r.current.weather.description = "";
  if (r.current.dt > r.current.sunrise && r.current.dt < r.current.sunset) {
    // Day icon
    r.current.weather.icon = String(data1h["pictocode"][nearestIdx].as<int>()) + "d";
  } else {
    // Night icon
    r.current.weather.icon = String(data1h["pictocode"][nearestIdx].as<int>()) + "n";
  }

  // --- HOURLY ---
  i = 0;
  JsonArray times = data1h["time"].as<JsonArray>();
  for (size_t j = nearestIdx; j < times.size() && i < OWM_NUM_HOURLY; ++j, ++i) {
    r.hourly[i].dt = timestampToUnix(times[j]);
    r.hourly[i].temp = data1h["temperature"][j].as<float>() + 273.15f;
    r.hourly[i].feels_like = data1h["felttemperature"][j].as<float>() + 273.15f;
    r.hourly[i].pressure = data1h["sealevelpressure"][j].as<int>();
    r.hourly[i].humidity = data1h["relativehumidity"][j].as<int>();
    r.hourly[i].dew_point = 0;
    r.hourly[i].clouds = data1h["totalcloudcover"][j].as<int>();
    r.hourly[i].uvi = data1h["uvindex"][j].as<float>();
    r.hourly[i].visibility = data1h["visibility"][j].as<int>();
    r.hourly[i].wind_speed = data1h["windspeed"][j].as<float>();
    r.hourly[i].wind_gust = 0;
    r.hourly[i].wind_deg = data1h["winddirection"][j].as<int>();
    r.hourly[i].pop = data1h["precipitation_probability"][j].as<float>() / 100.0f;
    r.hourly[i].rain_1h = data1h["precipitation"][j].as<float>();
    r.hourly[i].snow_1h = 0;
    r.hourly[i].weather.id = data1h["pictocode"][j].as<int>();
    r.hourly[i].weather.main = "";
    r.hourly[i].weather.description = "";
    if (r.hourly[i].dt > r.current.sunrise && r.hourly[i].dt < r.current.sunset) {
      // Day icon
      r.hourly[i].weather.icon = String(data1h["pictocode"][j].as<int>()) + "d";
    } else {
      // Night icon
      r.hourly[i].weather.icon = String(data1h["pictocode"][j].as<int>()) + "n";
    }
  }

  // --- DAILY ---
  i = 0;
  JsonArray day_times = day["time"].as<JsonArray>();
  for (size_t d = 0; d < day_times.size() && i < OWM_NUM_DAILY; ++d, ++i) {
    // Compose YYYY-MM-DD for this day
    const char* daystr = day_times[d];
    r.daily[i].dt = timestampToUnix(daystr);
    r.daily[i].sunrise = day["sunrise"] ? timestampToUnix(day["sunrise"][d], daystr) : 0;
    r.daily[i].sunset = day["sunset"] ? timestampToUnix(day["sunset"][d], daystr) : 0;
    r.daily[i].moonrise = day["moonrise"] ? timestampToUnix(day["moonrise"][d], daystr) : 0;
    r.daily[i].moonset = day["moonset"] ? timestampToUnix(day["moonset"][d], daystr) : 0;
    r.daily[i].moon_phase = 0; // Not available
    r.daily[i].temp.morn = 0;
    r.daily[i].temp.day = day["temperature_mean"][d].as<float>() + 273.15f;
    r.daily[i].temp.eve = 0;
    r.daily[i].temp.night = 0;
    r.daily[i].temp.min = day["temperature_min"][d].as<float>() + 273.15f;
    r.daily[i].temp.max = day["temperature_max"][d].as<float>() + 273.15f;
    r.daily[i].feels_like.morn = 0;
    r.daily[i].feels_like.day = day["felttemperature_mean"][d].as<float>() + 273.15f;
    r.daily[i].feels_like.eve = 0;
    r.daily[i].feels_like.night = 0;
    r.daily[i].pressure = day["sealevelpressure_mean"][d].as<int>();
    r.daily[i].humidity = day["relativehumidity_mean"][d].as<int>();
    r.daily[i].dew_point = 0;
    r.daily[i].clouds = day["totalcloudcover_mean"][d].as<int>();
    r.daily[i].uvi = day["uvindex"][d].as<float>();
    r.daily[i].visibility = day["visibility_mean"][d].as<int>();
    r.daily[i].wind_speed = day["windspeed_mean"][d].as<float>();
    r.daily[i].wind_gust = day["windspeed_max"][d].as<float>();
    r.daily[i].wind_deg = day["winddirection"][d].as<int>();
    r.daily[i].pop = day["precipitation_probability"][d].as<float>() / 100.0f;
    r.daily[i].rain = day["precipitation"][d].as<float>();
    r.daily[i].snow = 0;
    r.daily[i].weather.id = day["pictocode"][d].as<int>();
    r.daily[i].weather.main = "";
    r.daily[i].weather.description = "";
    r.daily[i].weather.icon = String(day["pictocode"][d].as<int>()) + "d";
  }

  // Alerts not available in meteoblue
  r.alerts.clear();

  return error;
}

DeserializationError deserializeAirQuality(WiFiClient &json,
                                           owm_resp_air_pollution_t &r)
{
  int i = 0;

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : "
                 + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  r.coord.lat = doc["coord"]["lat"].as<float>();
  r.coord.lon = doc["coord"]["lon"].as<float>();

  for (JsonObject list : doc["list"].as<JsonArray>())
  {

    r.main_aqi[i] = list["main"]["aqi"].as<int>();

    JsonObject list_components = list["components"];
    r.components.co[i]    = list_components["co"].as<float>();
    r.components.no[i]    = list_components["no"].as<float>();
    r.components.no2[i]   = list_components["no2"].as<float>();
    r.components.o3[i]    = list_components["o3"].as<float>();
    r.components.so2[i]   = list_components["so2"].as<float>();
    r.components.pm2_5[i] = list_components["pm2_5"].as<float>();
    r.components.pm10[i]  = list_components["pm10"].as<float>();
    r.components.nh3[i]   = list_components["nh3"].as<float>();

    r.dt[i] = list["dt"].as<int64_t>();

    if (i == OWM_NUM_AIR_POLLUTION - 1)
    {
      break;
    }
    ++i;
  }

  return error;
} // end deserializeAirQuality

