#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// DHT11 Sensor Settings
#define DHTPIN 4         // Pin connected to DHT11 DATA pin
#define DHTTYPE DHT11    // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// LED Pins
#define GREEN_LED 25     // GPIO pin for the green LED
#define RED_LED 26       // GPIO pin for the red LED

// Hydration thresholds
#define LOW_HYDRATION_THRESHOLD 60  // Relative humidity (%) below this indicates low hydration
#define HYSTERESIS 5                // To avoid flickering, add a margin

// Wi-Fi Credentials
const char* ssid = "Umair";       // Replace with your Wi-Fi SSID
const char* password = "umair69123";  // Replace with your Wi-Fi Password

// Web server on port 80
WebServer server(80);

// Variables to store hydration levels
String hydrationStatus = "Normal";
String gptResponse = "";

// Array to store humidity values
#define NUM_READINGS 10
float humidityReadings[NUM_READINGS] = {0};
int currentReadingIndex = 0;
bool isArrayFull = false;

// Function to simulate a GPT-style response
String generateGPTResponse(float averageHumidity) {
  if (averageHumidity < LOW_HYDRATION_THRESHOLD) {
    return "Your hydration level is low! Drink water and take care of yourself.";
  } else if (averageHumidity >= LOW_HYDRATION_THRESHOLD + HYSTERESIS) {
    return "Your hydration level is normal! Keep up the good work staying hydrated.";
  } else {
    return "Hydration status is fluctuating. Monitor closely.";
  }
}

void handleRoot() {
  // Calculate the average of the stored readings
  float sum = 0;
  int count = isArrayFull ? NUM_READINGS : currentReadingIndex;
  for (int i = 0; i < count; i++) {
    sum += humidityReadings[i];
  }
  float averageHumidity = sum / count;

  // Prepare styled HTML output
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Hydration Monitoring System</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; color: #333; }";
  html += ".container { margin: 20px auto; padding: 20px; border-radius: 10px; background-color: #fff; max-width: 400px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #007BFF; }";
  html += ".status { font-size: 1.5em; margin: 10px 0; }";
  html += ".message { font-style: italic; color: #555; }";
  html += ".value { font-size: 2em; color: #28a745; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Hydration Monitoring System</h1>";
  html += "<p class='status'>Hydration Status: <strong>" + hydrationStatus + "</strong></p>";
  html += "<p class='value'>Average Humidity: " + String(averageHumidity, 1) + "%</p>";
  html += "<p class='message'>" + gptResponse + "</p>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize DHT Sensor
  dht.begin();

  // Initialize LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // Read humidity from the DHT11 sensor
  float humidity = dht.readHumidity();

  // Check if reading is valid
  if (isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Store the reading in the array
  humidityReadings[currentReadingIndex] = humidity;
  currentReadingIndex = (currentReadingIndex + 1) % NUM_READINGS;
  if (currentReadingIndex == 0) isArrayFull = true;

  // Calculate the average of the stored readings
  float sum = 0;
  int count = isArrayFull ? NUM_READINGS : currentReadingIndex;
  for (int i = 0; i < count; i++) {
    sum += humidityReadings[i];
  }
  float averageHumidity = sum / count;

  // Display average humidity in Serial Monitor
  Serial.print("Average Humidity: ");
  Serial.print(averageHumidity);
  Serial.println(" %");

  // Determine hydration status and control LEDs
  if (averageHumidity < LOW_HYDRATION_THRESHOLD) {
    // Low hydration: Turn on red LED, turn off green LED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    hydrationStatus = "Low";
  } else if (averageHumidity >= LOW_HYDRATION_THRESHOLD + HYSTERESIS) {
    // Normal hydration: Turn on green LED, turn off red LED
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    hydrationStatus = "Normal";
  }

  // Generate custom GPT response
  gptResponse = generateGPTResponse(averageHumidity);

  // Handle web client requests
  server.handleClient();

  delay(2000);
}
