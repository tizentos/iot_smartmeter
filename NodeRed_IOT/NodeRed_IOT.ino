#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Change the credentials below, so your ESP8266 connects to your router
#define WIFI_AP "OFFICE"
#define WIFI_PASSWORD "facecafe11"

//#define WIFI_AP "Tosin Router"
//#define WIFI_PASSWORD ""


// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
//const char* mqtt_server = "192.168.43.186";
const char* mqtt_server = "192.168.1.18";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
int status = WL_IDLE_STATUS;

PubSubClient client(espClient);



#define GPIO0 0
#define GPIO2 2

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  String top=String(topic);

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(top.equals("room/lamp")){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){
        digitalWrite(GPIO2, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(GPIO2, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
     // client.subscribe("lamp");
      Serial.println("Subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(GPIO2, OUTPUT);
  Serial.begin(115200);
  delay(10);
  InitWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 10000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    int light=analogRead(GPIO0);
  //  String data=String(light);

    // Check if any reads failed and exit early (to try again).
    if (isnan(light)) {
      Serial.println("Failed to read from LDR sensor!");
      return;
    }
    static char data[7];
    dtostrf(light, 6, 2, data);

    // Publishes Temperature and Humidity values
    client.publish("room/lightintensity",data);
   

    Serial.print(" Intensity: ");
    Serial.print(light);
    Serial.println();
  }
} 
