#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "EPD.h"               // Include the EPD library for controlling the e-paper display

#include "pic.h"
// Define a black and white image array as the buffer for the e-paper display
extern uint8_t ImageBW[ALLSCREEN_BYTES];  // Define the size according to the resolution of the e-paper display

const char* ssid = "yanfa_software";        // WiFi SSID
const char* password = "yanfa-123456";      // WiFi password

// Replace with your OpenWeatherMap API key
String openWeatherMapApiKey = "3c03d1f4e2dd3e14474a9a3a2f2299ff";
// Example:
// String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";

// Replace with your city and country code
String city = "London";        // City name
String countryCode = "2643743"; // Country code (this is the ID for London)

// Default timer set to 10 seconds (for testing purposes), check API call limits for final application
unsigned long lastTime = 0;    // Last update time
unsigned long timerDelay = 10000; // Timer delay (10 seconds)

String jsonBuffer; // Buffer to store JSON data
int httpResponseCode; // HTTP response code
JSONVar myObject; // JSON object

String weather; // Weather condition
String temperature; // Temperature
String humidity; // Humidity
String sea_level; // Sea level pressure
String wind_speed; // Wind speed
String city_js; // City name
int weather_flag = 0; // Weather flag

// Function to clear the screen display
void clear_all() {
  EPD_Init();                          // Initialize the e-paper display
  EPD_Clear(0, 0, 296, 128, WHITE);    // Clear the display area to white
  EPD_ALL_Fill(WHITE);                 // Fill the entire screen with white
  EPD_Update();                        // Update the display
  EPD_Clear_R26H();                    // Clear the R26H area on the display (specific purpose)
}

// Function to display weather forecast
void UI_weather_forecast() {
  char buffer[40]; // Create a character array to store information

  EPD_GPIOInit();  // Initialize screen GPIO
  clear_all();     // Clear the screen display

  EPD_HW_RESET();  // Hardware reset the e-paper
  EPD_ShowPicture(0, 0, 296, 128, pic, BLACK); // Display background image

  EPD_ShowPicture(9, 4, 176, 80, gImage_clouds, BLACK); // Display cloud image

  // Draw partition lines
  EPD_DrawLine(0, 85, 296, 85, BLACK); // Draw horizontal line
  EPD_DrawLine(191, 0, 191, 85, BLACK); // Draw vertical line
  EPD_DrawLine(191, 45, 296, 45, BLACK); // Draw horizontal line

  // Display city name
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", city_js); // Format city name as string
  EPD_ShowString(237, 29, buffer, BLACK, 12); // Display city name
  
  // Display temperature
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s C", temperature); // Format temperature as string
  EPD_ShowString(123, 112, buffer, BLACK, 12); // Display temperature
  
  // Display humidity
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", humidity); // Format humidity as string
  EPD_ShowString(236, 72, buffer, BLACK, 12); // Display humidity
  
  // Display wind speed
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s m/s", wind_speed); // Format wind speed as string
  EPD_ShowString(43, 112, buffer, BLACK, 12); // Display wind speed
  
  // Display sea level pressure
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", sea_level); // Format sea level pressure as string
  EPD_ShowString(236, 112, buffer, BLACK, 12); // Display sea level pressure

  // Update the e-paper display content
  EPD_DisplayImage(ImageBW);      // Display the black and white image
  EPD_FastUpdate();               // Quickly update the display
  EPD_Sleep();                    // Put the display into sleep mode
}

void setup() {
  Serial.begin(115200); // Initialize serial communication

  WiFi.begin(ssid, password); // Connect to WiFi
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection success
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); // Print local IP address

  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  // Set the screen power pin to output mode and set it to high level to turn on the power
  pinMode(7, OUTPUT);  // Set GPIO 7 to output mode
  digitalWrite(7, HIGH);  // Set GPIO 7 to high level

  // Initialize the e-paper display
  EPD_GPIOInit();  // Initialize the GPIO pins for the e-paper
}

void loop() {
  js_analysis(); // Parse JSON data
  UI_weather_forecast(); // Display weather forecast
  delay(1000 * 60 * 60); // Main loop delay for 1 hour
}

void js_analysis()
{
  if (WiFi.status() == WL_CONNECTED) { // Ensure WiFi is connected
    // Build the API request path, including city, country code, and API key
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";
    while (httpResponseCode != 200) // Loop until a successful HTTP 200 response code is received
    {
      jsonBuffer = httpGETRequest(serverPath.c_str()); // Send HTTP GET request and get the returned JSON data
      Serial.println(jsonBuffer); // Print the received JSON data
      myObject = JSON.parse(jsonBuffer); // Parse the JSON data

      // Check if the JSON data was parsed successfully
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!"); // If parsing fails, print error message
        return; // Exit the function
      }
      delay(2000); // Wait 2 seconds before retrying
    }

    // Extract weather, temperature, humidity, sea level pressure, and wind speed information from the JSON object
    weather = JSON.stringify(myObject["weather"][0]["main"]);
    temperature = JSON.stringify(myObject["main"]["temp"]);
    humidity = JSON.stringify(myObject["main"]["humidity"]);
    sea_level = JSON.stringify(myObject["main"]["sea_level"]);
    wind_speed = JSON.stringify(myObject["wind"]["speed"]);
    city_js = JSON.stringify(myObject["name"]);

    // Print the extracted data
    Serial.print("String weather: ");
    Serial.println(weather);
    Serial.print("String Temperature: ");
    Serial.println(temperature);
    Serial.print("String humidity: ");
    Serial.println(humidity);
    Serial.print("String sea_level: ");
    Serial.println(sea_level);
    Serial.print("String wind_speed: ");
    Serial.println(wind_speed);
    Serial.print("String city_js: ");
    Serial.println(city_js);

    // Set the value of weather_flag based on the weather condition
    if (weather.indexOf("clouds") != -1 || weather.indexOf("Clouds") != -1) {
      weather_flag = 1; // Cloudy
    } else if (weather.indexOf("clear sky") != -1 || weather.indexOf("Clear sky") != -1) {
      weather_flag = 3; // Clear sky
    } else if (weather.indexOf("rain") != -1 || weather.indexOf("Rain") != -1) {
      weather_flag = 5; // Rainy
    } else if (weather.indexOf("thunderstorm") != -1 || weather.indexOf("Thunderstorm") != -1) {
      weather_flag = 2; // Thunderstorm
    } else if (weather.indexOf("snow") != -1 || weather.indexOf("Snow") != -1) {
      weather_flag = 4; // Snowy
    } else if (weather.indexOf("mist") != -1 || weather.indexOf("Mist") != -1) {
      weather_flag = 0; // Misty
    }
  }
  else {
    Serial.println("WiFi Disconnected"); // If WiFi is disconnected, print error message
  }
}

// Send HTTP GET request and return the response data
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Initialize HTTP request with server address
  http.begin(client, serverName);

  // Send HTTP GET request
  httpResponseCode = http.GET();

  String payload = "{}"; // Initialize the returned data as an empty JSON object

  // Handle the returned data based on the response code
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode); // Print HTTP response code
    payload = http.getString(); // Get the string data of the response
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode); // Print error code
  }
  // Release HTTP resources
  http.end();

  return payload; // Return the response data
}