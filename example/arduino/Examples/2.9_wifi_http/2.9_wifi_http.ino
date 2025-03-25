#include <WiFi.h>              // Include WiFi library for connecting to WiFi networks
#include <Arduino.h>          // Include Arduino library for basic functionalities
#include "EPD.h"              // Include EPD library for controlling the electronic ink screen (E-Paper Display)

#include <ArduinoJson.h>      // Include ArduinoJson library for parsing JSON data
#include <HTTPClient.h>       // Include HTTPClient library for sending HTTP requests
#include "pic.h"              // Include header file containing image resources

const char * ID = "yanfa_software";        // SSID of the WiFi network
const char * PASSWORD = "yanfa-123456";    // Password of the WiFi network

// Weather-related parameters
String API = "ShpLFA64JQmtRVReV";        // Secret key for the weather API
String WeatherURL = "";                  // Store the generated weather API URL
String CITY = "深圳";                    // City to query weather for
String url_xinzhi = "";                 // Store the complete URL
String Weather = "0";                   // Store the retrieved weather information

long sum = 0;                           // Record the cumulative value, currently unused

// Define a black and white image array as a buffer for the e-paper display
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// Create an HTTPClient instance for sending HTTP requests
HTTPClient http;

// Generate the weather API URL based on the API key and city
String GitURL(String api, String city)
{
  url_xinzhi =  "https://api.seniverse.com/v3/weather/now.json?key=";
  url_xinzhi += api;                      // Add API key
  url_xinzhi += "&location=";
  url_xinzhi += city;                    // Add city name
  url_xinzhi += "&language=zh-Hans&unit=c"; // Add language and unit parameters
  return url_xinzhi;                     // Return the generated URL
}

// Parse weather data
void ParseWeather(String url)
{
  DynamicJsonDocument doc(1024);        // Allocate 1024 bytes of memory to store JSON data
  http.begin(url);                      // Initialize HTTP request, specify the URL

  int httpGet = http.GET();             // Send GET request and get the HTTP status code
  if (httpGet > 0)                      // Check if the request was successful
  {
    Serial.printf("HTTPGET is %d", httpGet);  // Print the HTTP status code

    if (httpGet == HTTP_CODE_OK)        // Check if the HTTP status code is 200 (OK)
    {
      String json = http.getString();   // Get the JSON string from the response
      Serial.println(json);             // Print the JSON string

      deserializeJson(doc, json);       // Parse the JSON string into doc

      Weather = doc["results"][0]["now"]["text"].as<String>(); // Extract the weather description
    }
    else
    {
      Serial.printf("ERROR1!!");        // Print error message
    }
  }
  else
  {
    Serial.printf("ERROR2!!");          // Print error message
  }
  http.end();                          // End the HTTP request
}

// Create a character array for displaying information
char buffer[40];

void setup()
{
  Serial.begin(115200);                // Initialize serial communication at a baud rate of 115200

  //==================WiFi connection==================

  Serial.println("WiFi:");              // Print WiFi information
  Serial.println(ID);                  // Print the SSID of the WiFi
  Serial.println("PASSWORD:");        // Print the password of the WiFi
  Serial.println(PASSWORD);

  WiFi.begin(ID, PASSWORD);           // Connect to the WiFi network

  // Wait for the WiFi connection to be successful
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);                        // Delay for 500 milliseconds
    Serial.println("Connecting...");   // Print connection progress
  }

  Serial.println("Connected!");         // Print success information after connection

  //==================WiFi connection==================

  WeatherURL = GitURL(API, CITY);     // Generate the weather API URL
  pinMode(7, OUTPUT);                 // Set the screen power pin to output mode
  digitalWrite(7, HIGH);              // Set the power pin to high level to turn on the power

  EPD_GPIOInit();                     // Initialize the e-paper display

  Serial.println("Connected!");         // Print success information after connection

}

void loop()
{
  ParseWeather(WeatherURL);           // Parse weather data

  PW(WeatherURL);                     // Call the PW function to update the screen display content
  delay(1000*60*60); // Main loop delay for 1 hour
}

// Function to clear the screen display
void clear_all() {
  EPD_Init();                          // Initialize the e-paper display
  EPD_Clear(0, 0, 296, 128, WHITE);    // Clear the display area to white
  EPD_ALL_Fill(WHITE);                 // Fill the entire screen with white
  EPD_Update();                        // Update the display
  EPD_Clear_R26H();                    // Clear the R26H area on the display (specific purpose)
}

void PW(String url)
{
  // Create a dynamic JSON document, size 1024 bytes
  DynamicJsonDocument doc(1024); 

  // Initialize HTTP request
  http.begin(url);

  // Send HTTP GET request
  int httpGet = http.GET();
  if (httpGet > 0)
  {
    Serial.printf("HTTPGET is %d\n", httpGet);

    // Check if the HTTP response status code is 200 (success)
    if (httpGet == HTTP_CODE_OK)
    {
      // Get the JSON string from the response
      String json = http.getString();
      Serial.println(json);

      // Parse the JSON string
      deserializeJson(doc, json);

      // Extract various data elements from the JSON
      String location = doc["results"][0]["location"]["name"].as<String>();
      String weatherText = doc["results"][0]["now"]["text"].as<String>();
      String temperature = doc["results"][0]["now"]["temperature"].as<String>();
      String humidity = doc["results"][0]["now"]["humidity"].as<String>();
      String windSpeed = doc["results"][0]["now"]["wind"]["speed"].as<String>();

      String Country = doc["results"][0]["location"]["country"].as<String>();
      String Timezone = doc["results"][0]["location"]["timezone"].as<String>();
      String last_update = doc["results"][0]["last_update"].as<String>();

      // Create a character array for display
      char buffer[40];

      // Clear the display area and initialize the e-ink screen
      clear_all();
      EPD_HW_RESET();

      // Display the home icon
      EPD_ShowPicture(0, 0, 16, 16, home_small_standard, BLACK);

      // Display the update time
      EPD_ShowPicture(0, 100, 16, 16, weather, BLACK);
      EPD_ShowString(20, 100, "Last Time :", BLACK, 12);

      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "%s ", last_update.c_str());
      EPD_ShowString(0, 117, buffer, BLACK, 12);

      // Display the city name
      EPD_ShowString(20, 25, "City :", BLACK, 12);
      EPD_ShowPicture(0, 25, 16, 16, city_small1, BLACK);

      memset(buffer, 0, sizeof(buffer));
      if (strcmp(location.c_str(), "深圳") == 0)
      {
        snprintf(buffer, sizeof(buffer), " %s", "Sheng Zhen");
      } else
      {
        snprintf(buffer, sizeof(buffer), "%s", "Null");
      }
      EPD_ShowString(10, 40, buffer, BLACK, 12);

      // Display the timezone information
      EPD_ShowString(242, 25, "time zone :", BLACK, 12);
      EPD_ShowPicture(222, 25, 16, 16, time_zone2, BLACK);

      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "%s ", Timezone.c_str());
      EPD_ShowString(220, 45, buffer, BLACK, 12);

      // Display the temperature information
      EPD_ShowString(250, 90, "Temp :", BLACK, 12);
      EPD_ShowPicture(230, 90, 16, 16, temp_small2, BLACK);

      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "%s C", temperature.c_str());
      EPD_ShowString(240, 110, buffer, BLACK, 16);

      // Display the weather information
      memset(buffer, 0, sizeof(buffer));
      if (strcmp(weatherText.c_str(), "大雨") == 0)
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "heavy rain");
        EPD_ShowPicture(130, 25, 40, 40, heavy_rain_small, BLACK);
      } else if (strcmp(weatherText.c_str(), "多云") == 0)
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "Cloudy");
        EPD_ShowPicture(130, 25, 40, 40, cloudy_small, BLACK);
      }
      else if (strcmp(weatherText.c_str(), "小雨") == 0)
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "small rain");
        EPD_ShowPicture(130, 25, 40, 40, small_rain_small, BLACK);
      } else if (strcmp(weatherText.c_str(), "晴") == 0)
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "clear day");
        EPD_ShowPicture(130, 25, 40, 40, clear_day_small, BLACK);
      }

      EPD_ShowString(100, 70, buffer, BLACK, 12);

      // Draw partition lines
      EPD_DrawLine(0, 18, 296, 18, BLACK);
      EPD_DrawLine(0, 86, 296, 86, BLACK);
      EPD_DrawLine(90, 18, 90, 86, BLACK);
      EPD_DrawLine(210, 18, 210, 86, BLACK);

      // Update the display content
      EPD_DisplayImage(ImageBW);      // Display the black and white image
      EPD_FastUpdate();               // Quickly update the display
      EPD_Sleep();                    // Put the display into sleep mode

      // Update the Weather variable
      Weather = weatherText;
    }
    else
    {
      Serial.println("ERROR1!!");
    }
  }
  else
  {
    Serial.println("ERROR2!!");
  }
  // End the HTTP request
  http.end();
}