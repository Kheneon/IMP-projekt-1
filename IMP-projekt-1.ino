/************** IMP PROJECT ****************
 *
 * Assignment: Display + gestures + MQTT
 *
 * Author: Michal Zapletal
 * Email:  xzaple41@stud.fit.vutbr.cz
 *
 *
 *
 *******************************************/


#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_APDS9960.h>
#include <Wire.h>
#include <MQTT.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI  23
#define OLED_CLK   18
#define OLED_DC    27
#define OLED_CS    5
#define OLED_RESET 17

#define WHITE_COLOR 1
#define BLACK_COLOR 0

#define PIXELS_PER_UNIT_STS 10 // In function setTextSize means each unit 10 pixles, in range 1-5 (10-50 pixels)

#define DELAY_AFTER_GESTURE 500

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Adafruit_APDS9960 apds;

WiFiClient net;
MQTTClient client;

char client_name[] = "d";
char client_login[] = "";
char client_pwd[] = "";

char WiFi_SSID [] = "";
char WiFi_pwd [] = "";

/********************************************/
/*!
 @brief Connect ESP32 to wifi and MQTT cleint
*/
/********************************************/
void connectToWifiAndClient() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Wifi connected");

  while (!client.connect(client_name, client_login, client_pwd)) {
    delay(1000);
  }
  Serial.println("Connected to MQTT client");
}

/********************************************/
/*!
 @brief Prints on debug output recieved message
*/
/********************************************/
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

/********************************************/
/*!
 @brief Int to string
 @param str * where to store int as string
 @param val value to convert
 @param size how big is str
*/
/********************************************/
void itoa(char * str, int val, int size){
  int value = val;
  for(int i = size-1; i >= 0; i--){
    if(value <= 0){
      str[i] = '0';
    }else{
      str[i] = value % 10 + 48;
      value = value / 10;
    }
  }
}

/********************************************/
/*!
 @brief Sends value to client
 @param topic * topic to send
 @param payload * payload to send
*/
/********************************************/
void sendValueToClientMQTT(char * topic, char * payload){
  display.clearDisplay();
  drawTextOnScreen(0,24,2,"Sending");
  display.display();
  connectToWifiAndClient();
  if(client.publish(topic,payload)){
    Serial.println("Message successfully sent");
  }else{
    Serial.println("Message was not sent");
  }
  if(client.disconnect()){
    Serial.println("Disconnected from MQTT");
  }else{
    Serial.println("Unable to disconnect MQTT");
  }
}

/********************************************/
/*!
 @brief Wait until user makes gesture and returns it
 @return gesture
*/
/********************************************/
uint8_t getGesture(){
  uint8_t gesture;
  while(true){
    gesture = apds.readGesture();
    if(gesture == APDS9960_UP || gesture == APDS9960_DOWN || gesture == APDS9960_LEFT || gesture == APDS9960_RIGHT){
      break;
    }
  }
  return gesture;
}

/********************************************/
/*!
 @brief draws text on screen
 @param x X coord to draw
 @param y Y coord to draw
 @param text_size size of text * 10
 @param text text to draw
*/
/********************************************/
void drawTextOnScreen(int16_t x, int16_t y, int16_t text_size, char * text){
  display.setCursor(x, y);
  display.setTextSize(text_size); // each value means 10 pixels
  display.println(text);
}

/***************************************/
/*!
 @brief Draws Frame
 @param x0 left coord
 @param y0 top coord
 @param x1 right coord
 @param y1 bottom coord
 @param width width of frame
 @param color color of frame (WHITE/BLACK)
*/
/***************************************/
void drawFrame(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t width, uint16_t color){
  for(int i = 0; i < width; i++){
    // LEFT TOP -> RIGHT TOP
    display.drawLine(x0+i,y0+i,x1-i,y0+i,color);

    // RIGHT TOP -> RIGHT BOT
    display.drawLine(x1-i,y0+i,x1-i,y1-i,color);

    // LEFT TOP -> LEFT DOWN
    display.drawLine(x0+i,y0+i,x0+i,y1-i,color);

    // LEFT DOWN -> RIGHT DOWN
    display.drawLine(x0+i,y1-i,x1-i,y1-i,color);
  }
}

/********************************************/
/*!
 @brief draws start screen
*/
/********************************************/
void startScreen(){
  display.clearDisplay();
  drawFrame(0,0,127,63,4,WHITE_COLOR);
  drawTextOnScreen(10,(SCREEN_HEIGHT/2)-PIXELS_PER_UNIT_STS*2, 2, "Gesture");
  drawTextOnScreen(10,(SCREEN_HEIGHT/2), 2, "Display");
  display.display();
}

/********************************************/
/*!
 @brief Shows 2 options on display
 @param opt_text_0 * name of option in top
 @param opt_text_1 * name of option in bottom
 @param selected whcih option is already hovered
*/
/********************************************/
void showOptions(char * opt_text_0, char * opt_text_1, uint8_t selected){
  int16_t padding = 10;
  display.clearDisplay();
  drawTextOnScreen(padding,(SCREEN_HEIGHT/4 - PIXELS_PER_UNIT_STS),2,opt_text_0);
  drawFrame(0,0,127,31,1,WHITE_COLOR);
  drawTextOnScreen(padding,((SCREEN_HEIGHT/4) * 3 - PIXELS_PER_UNIT_STS),2,opt_text_1);
  drawFrame(0,32,127,63,1,WHITE_COLOR);
  if(selected == 0){
    drawFrame(3,3,124,28,1,WHITE_COLOR);
  }else{
    drawFrame(3,35,124,60,1,WHITE_COLOR);
  }
  display.display();
}

/********************************************/
/*!
 @brief Shows whole menu
 @param options ** array with all options
 @param num_of_options number of options
*/
/********************************************/
int showMenu(char ** options, uint8_t num_of_options){
  uint8_t options_index = 0;
  uint8_t selected = 0;
  showOptions(options[options_index],options[options_index+1],selected);
  while(true){
    uint8_t gesture = getGesture();
    switch(gesture){
      case APDS9960_UP:
        if(selected == 1){
          selected = 0;
        }else{
          if(options_index != 0){
            options_index--;
          }
        }
        break;

      case APDS9960_DOWN:
        if(selected == 0){
          selected = 1;
        }else{
          if(options_index + 2 < num_of_options){
            options_index++;
          }
        }
        break;

      case APDS9960_RIGHT:
        return options_index + selected;
        break;

      case APDS9960_LEFT:
        return -1;
        break;
    }
    showOptions(options[options_index],options[options_index+1],selected);
    delay(DELAY_AFTER_GESTURE);
  }
}

/********************************************/
/*!
 @brief Converts time to string
 @param str * place to save value
 @param hour hour to convert
 @param min minute to convert
*/
/********************************************/
void timeToChar(char * str, int hour, int min){
  itoa(str,hour,2);
  str[2] = ':';
  itoa(str+3,min,2);
  str[5] = '\0';
}

/********************************************/
/*!
 @brief Converts date to string 
 @param str * wherer to store date
 @param day day to convert
 @param month month to convert
 @param year year to convert
*/
/********************************************/
void dateToChar(char * str, int day, int month, int year){
  itoa(str,day,2);
  itoa(str+3,month,2);
  itoa(str+6,year,2);
  str[2] = '.';
  str[5] = '.';
  str[8] = '\0';
}

/********************************************/
/*!
 @brief Edit time by client
 @param hour * save edited hour value here
 @param min * save edited minute value here
*/
/********************************************/
void editTime(int * hour, int * min){
  char time[6];
  int buffer_hour = *hour, buffer_min = *min;
  int index_of_change = 0;
  while(true){
    display.clearDisplay();
    timeToChar(time,buffer_hour,buffer_min);
    drawTextOnScreen(30,SCREEN_HEIGHT/2-PIXELS_PER_UNIT_STS,2,time);
    if(index_of_change == 0){
      display.drawLine(30,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,50,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,WHITE_COLOR);
    }else{
      display.drawLine(66,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,86,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,WHITE_COLOR);
    }
    display.display();
    delay(DELAY_AFTER_GESTURE);
    switch (getGesture())
    {
    case APDS9960_LEFT:
      if(index_of_change <= 0){
        return;
      }
      index_of_change--;
      break;

    case APDS9960_RIGHT:
      if(index_of_change >= 1){
        sendValueToClientMQTT("Time",time);
        *hour = buffer_hour;
        *min = buffer_min;
        return;
      }
      index_of_change++;
      break;
    
    case APDS9960_UP:
      if(index_of_change == 0){
        buffer_hour+=1;
      }else{
        buffer_min+=1;
      }
      break;

    case APDS9960_DOWN:
      if(index_of_change == 0){
        buffer_hour-=1;
      }else{
        buffer_min-=1;
      }
      break;

    default:
      break;
    }

    // Check if values are in range
    if(buffer_hour < 0){
      buffer_hour = 24;
    }else if(buffer_min > 24){
      buffer_hour = 0;
    }
    if(buffer_min < 0){
      buffer_min = 60;
    }else if(buffer_min > 60){
      buffer_min = 0;
    }
  }
}

/********************************************/
/*!
 @brief Edit date by user 
 @param day * edited day value save here
 @param month * edited month value save here
 @param year * edited year value save here
*/
/********************************************/
void editDate(int * day, int * month, int * year){
  char date[9];
  int buffer_day = *day, buffer_month = *month, buffer_year = *year;
  int index_of_change = 0;
  while(true){
    display.clearDisplay();
    dateToChar(date,buffer_day,buffer_month,buffer_year);
    drawTextOnScreen(20,SCREEN_HEIGHT/2-PIXELS_PER_UNIT_STS,2,date);
    if(index_of_change == 0){
      display.drawLine(20,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,40,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,WHITE_COLOR);
    }else if(index_of_change == 1){
      display.drawLine(56,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,76,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,WHITE_COLOR);
    }else{
      display.drawLine(92,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,112,SCREEN_HEIGHT/2+PIXELS_PER_UNIT_STS+5,WHITE_COLOR);
    }
    display.display();
    delay(DELAY_AFTER_GESTURE);
    switch (getGesture())
    {
    case APDS9960_LEFT:
      if(index_of_change <= 0){
        return;
      }
      index_of_change--;
      break;

    case APDS9960_RIGHT:
      if(index_of_change >= 2){
        sendValueToClientMQTT("Date",date);
        *year = buffer_year;
        *month = buffer_month;
        *day = buffer_day;
        return;
      }
      index_of_change++;
      break;
    
    case APDS9960_UP:
      if(index_of_change == 0){
        buffer_day+=1;
      }else if(index_of_change == 1){
        buffer_month+=1;
      }else{
        buffer_year+=1;
      }
      break;

    case APDS9960_DOWN:
      if(index_of_change == 0){
        buffer_day-=1;
      }else if(index_of_change == 1){
        buffer_month-=1;
      }else{
        buffer_year-=1;
      }
      break;

    default:
      break;
    }
    if(buffer_day < 1){
      buffer_day = 31;
    }else if(buffer_day > 31){
      buffer_day = 1;
    }
    if(buffer_month < 1){
      buffer_month = 12;
    }else if(buffer_month > 12){
      buffer_month = 1;
    }
    if(buffer_year < 0){
      buffer_year = 99;
    }else if(buffer_year > 99){
      buffer_year = 0;
    }
    if(buffer_month == 2 && buffer_day > 28){
      buffer_day = 1;
    }else if((buffer_month == 4 || buffer_month == 6 || buffer_month == 9 || buffer_month == 11) && buffer_day > 30){
      buffer_day = 1;
    }
  }
}

/********************************************/
/*!
 @brief Shows time on display
*/
/********************************************/
void showTime(){
  static int time_min = 0;
  static int time_hour = 12;
  static int time_day = 1;
  static int time_month = 1;
  static int time_year = 23;
  while(true){
    char time[6];
    timeToChar(time,time_hour,time_min);
    char date[9];
    dateToChar(date,time_day,time_month,time_year);
    char *options[] = {time, date};
    int selected_opt = showMenu(options,2);
    switch(selected_opt){
      case -1:
        return;

      case 0:
        Serial.println("Set time");
        editTime(&time_hour,&time_min);
        break;

      case 1:
        editDate(&time_day,&time_month,&time_year);
        Serial.println("Set Date");
        break;
    }
  }
}

/********************************************/
/*!
 @brief Show data about user on display
*/
/********************************************/
void showPerson(){
  int padding = 5;
  display.clearDisplay();
  drawTextOnScreen(padding,padding,1,"FIRST NAME:");
  drawTextOnScreen(padding,padding*2+PIXELS_PER_UNIT_STS,1,"Michal");
  drawTextOnScreen(padding,padding*3+PIXELS_PER_UNIT_STS*2,1,"SURNAME:");
  drawTextOnScreen(padding,padding*4+PIXELS_PER_UNIT_STS*3,1,"Zapletal");
  display.display();
  delay(DELAY_AFTER_GESTURE);
  while (true){
    switch (getGesture())
    {
      case APDS9960_LEFT:
        return;
      
      default:
        break;
    }
  }
}

/********************************************/
/*!
 @brief Show changable value of height to client
*/
/********************************************/
void showHeight(){
  char no_data[] = "No data";
  char higher[] = "180 or /\\";
  char lower[] = "180 or \\/";
  char *selection[] = {no_data,higher,lower};
  static int selected = 0;
  uint8_t num_of_options = 2;
  while(true){
    char *options[] = {selection[selected],"Edit"};
    int selected_opt = showMenu(options,num_of_options);
    switch (showMenu(options,num_of_options))
    {
    case -1:
      return;
    
    case 0:
      break;

    case 1:
      display.clearDisplay();
      drawTextOnScreen(0,0,1,"Are you higher than  180cm ?");
      drawTextOnScreen(20,32,1,"<-- YES/NO -->");
      display.display();
      delay(DELAY_AFTER_GESTURE);
      switch (getGesture())
      {
      case APDS9960_LEFT:
        selected = 1;
        break;

      case APDS9960_RIGHT:
        selected = 2;
        break;
      
      default:
        break;
      }
      break;

    default:
      break;
    }
  }
}

/********************************************/
/*!
 @brief Shows user main menu
*/
/********************************************/
void mainMenu(){
  while(true){
    char *options[] = {"Time","Person","Height"};
    uint8_t num_of_options = 3;
    int selected_opt = showMenu(options,num_of_options);
    Serial.println((String)"Selected option:"+selected_opt);
    switch(selected_opt){
      case 0:
        showTime();
        break;

      case 1:
        showPerson();
        break;

      case 2:
        showHeight();
        break;

      default:
        break;
    }
  }
}

/********************************************/
/*!
 @brief Setup of ESP32
*/
/********************************************/
void setup() {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display.clearDisplay();

  display.setTextSize(2);

  display.setTextColor(WHITE_COLOR);

  if(apds.begin()){
    Serial.println("Gesture module initialized");
  }else{
    Serial.println("Gesture module failure");
  }
  apds.enableProximity(true);
  apds.enableGesture(true);

  WiFi.begin(WiFi_SSID,WiFi_pwd);

  client.begin("node02.myqtthub.com", net);
  client.onMessage(messageReceived);
}

/********************************************/
/*!
 @brief Loop  
*/
/********************************************/
void loop(){
  // Show start screen and wait until user makes any gesture
  startScreen();
  uint8_t gesture = getGesture();

  // Show main menu
  mainMenu();
}