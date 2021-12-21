/*
 * Created by esp32io.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://esp32io.com/tutorials/esp32-keypad
 */

#include <Keypad.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Servo.h>
// Wifi network station credentials
#define WIFI_SSID "Abhi"
#define WIFI_PASSWORD "abhianee"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "2042853395:AAG44QZRhKkFkPchfvGI7V1Bmp55GHHhgtM"
const char* resource = "/trigger/door_open_close/with/key/c8Eh4w7huS10ee9Jv612eYDT9sWKy_iraeA1MycG1tU";
const char* server = "maker.ifttt.com";
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

const int ledPin = 0;
int ledStatus = 0;
int buzz = 25;
const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //three columns

Servo myservo;
int pos = 0;
int corr = 0;
int started = 0;
void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/open" && corr == 1)
    {
      digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      myservo.write(90);
      bot.sendMessage(chat_id, "Door is Open", "");
      makeIFTTTRequest();
    }

    if (text == "/close" && corr == 1)
    {
      ledStatus = 0;
      myservo.write(0);
      digitalWrite(ledPin, LOW); // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Door is Closed", "");
      makeIFTTTRequest2();
    }

    if (text == "/status" && corr == 1)
    {
      if (ledStatus)
      {
        bot.sendMessage(chat_id, "Door is Opened", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Door is Closed", "");
      }
    }

    if (text == "/start")
    {
      started = 1;
      if(corr == 1){
        String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
        welcome += "This is Flash Led Bot example.\n\n";
        welcome += "/open : to open the door\n";
        welcome += "/close : to close the door\n";
        welcome += "/status : Returns current status of Door\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      }else{
        bot.sendMessage(chat_id,"Enter Password");
        
      }
      
    }

    if(text == "1234"){
          corr = 1;
          bot.sendMessage(chat_id,"Your password is Correct");
          String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
        welcome += "This is Flash Led Bot example.\n\n";
        welcome += "/open : to open the door\n";
        welcome += "/close : to close the door\n";
        welcome += "/status : Returns current status of Door\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
     }
      if(started == 1 && text !="1234" && text !="/open" && text != "/start" && text != "/close" && text != "/status"){
        bot.sendMessage(chat_id,"Your password is in correct. Type again!");
        corr = 0;
        digitalWrite(buzz, HIGH);
        delay(3000);
        digitalWrite(buzz, LOW);
      }
     

    
  }
}

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pin_rows[ROW_NUM] = {18, 5, 17, 16}; // GIOP18, GIOP5, GIOP17, GIOP16 connect to the row pins
byte pin_column[COLUMN_NUM] = {4, 0, 2,15};  // GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const String password = "1234"; // change your password here
String input_password;

void setup(){
  Serial.begin(9600);
  input_password.reserve(32); // maximum input characters is 33, change if needed
  //Serial.begin(115200);
  Serial.println();

  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ledPin, LOW); // initialize pin as off (active LOW)
  pinMode(buzz, OUTPUT);
  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  myservo.attach(13);
}

void loop(){

  
  
  char key = keypad.getKey();

  if (key){
    Serial.println(key);

    if(key == '*') {
      input_password = ""; // clear input password
    } else if(key == '#') {
      if(password == input_password) {
        Serial.println("password is correct");
        // DO YOUR WORK HERE
        digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      myservo.write(90);
      //bot.sendMessage(chat_id, "Led is ON", "");
        
      } else {
        
        Serial.println("password is incorrect, try again");
        digitalWrite(buzz, HIGH);
        delay(3000);
        digitalWrite(buzz, LOW);
      }

      input_password = ""; // clear input password
    } else {
      input_password += key; // append new character to input password string
    }
  }else{
    if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  }
}

void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

  // Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + "open" + "\"}";
                      
  // Comment the previous line and uncomment the next line to publish temperature readings in Fahrenheit                    
  /*String jsonObject = String("{\"value1\":\"") + (1.8 * bme.readTemperature() + 32) + "\",\"value2\":\"" 
                      + (bme.readPressure()/100.0F) + "\",\"value3\":\"" + bme.readHumidity() + "\"}";*/
                      
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
}


void makeIFTTTRequest2() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

  // Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + "close" + "\"}";
                      
  // Comment the previous line and uncomment the next line to publish temperature readings in Fahrenheit                    
  /*String jsonObject = String("{\"value1\":\"") + (1.8 * bme.readTemperature() + 32) + "\",\"value2\":\"" 
                      + (bme.readPressure()/100.0F) + "\",\"value3\":\"" + bme.readHumidity() + "\"}";*/
                      
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
}
