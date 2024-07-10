#include "Arduino.h"
#include "DHTesp.h"
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>
#define SPI_SPEED SD_SCK_MHZ(4)
#define SS_PIN 5
#define RST_PIN 9
File root;
DHTesp dhtSensor;
File myFile;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
WiFiClient espClient;
PubSubClient client(espClient);
//RC522-SPI
//SDA-GPIO5
//MISO-GPIO19
//MOSI-GPIO23
//SCK-GPIO18
//RST-EN-Pin9


byte nuidPICC[4];
unsigned long int code1 = 0;

//char filename[] = "/filename.CSV";
//const int chipSelect = 5;
const char *subscription = "t_treshold";
const int DHT_PIN = 15;
const int door = 2;
const int buzzer_pin=4;
const int relay_fan = 27;
const int relay_resistor = 14;
const int pir = 33;
const int alert_pin = 26;
const int lcd_button=16;
const int inc_button=17;
const int dec_button=35;
const int ldr_pin=32;
float temp=0;
float hum=0;
float voltage ;
float resistance;
float lux;
byte *buffer;
byte bufferSize;
int detection_var = 0;
int pirState = LOW;
int rfid_var = LOW;
int display_state = 0;
int display_mode = 0;
int lastMillis = 0;
int lastMillis_2 = 0;
int lastMillis_3 = 0;
int lastMillis_4 = 0;
int lastMillis_5 = 0;
int lastMillis_6= 0;
int lastMillis_7 = 0;
int lastMillis_8= 0;
int lastMillis_9= 0;
int lastMillis_10= 0;
int time_sent=0;
int time_received=0;
int latency=0;
volatile float temp_threshold=25;
float hum_threshold=70;
long sum;
const int freq = 2000;
const int ledChannel = 0;
const int resolution = 8;
const char* ssid = "NOS-1DA6";
const char* password = "2MHPTZ46";
//const char* user ="DTSD";
//const char* pass="Rafaeljff123";
const char* user ="DTSD";
const char* pass="Rafaeljff123";
const char* mqtt_server = "192.168.1.7";
String system_status="OFF";
//char filename[] = "/filename.CSV";
bool validate;
bool detection;
int flag_entrada=1;
String fan_state;


void setup() {
  Serial.begin(115200);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(buzzer_pin, ledChannel);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(pir, INPUT);
  pinMode(ldr_pin, INPUT);
  pinMode(lcd_button, INPUT_PULLUP);
  pinMode(inc_button, INPUT_PULLUP);
  pinMode(dec_button, INPUT_PULLUP);
  pinMode(alert_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(relay_fan, OUTPUT);
  pinMode(door, OUTPUT);
  pinMode(relay_resistor, OUTPUT);
  //Serial.print("Initializing SD card...");
/*
if (SD.begin(5)) {

	Serial.println("SD card pronto para ser usado.");
  }

 if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");

  }*/
  Wire.begin();
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Still not connected to wifi");
      }
    client.setServer(mqtt_server,1883);
  //File_write();

  for(int i =0; i<8; i++) {
  display.begin(0x3C, true); // Address 0x3C default
  delay(250);
  }
  display.display();

  delay(2000);
// Address 0x3C default

   Serial.println("OLED begun");

   display.clearDisplay();
   display.setRotation(1);
   display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
   display.print("Welcome to energy management system!");
   //display_show();
   display.display(); // actually display all of the above
delay(5000);
display.clearDisplay();
client.subscribe(subscription);
client.setCallback(callback);
}

void loop() {

if(millis() - lastMillis_3> 1000){
	pir_sensor();
	display_show();
	security();
	publish_on_mqtt();
	lastMillis_3 = millis();
}}

/*
if(strcmp(subscription,"t_treshold")==0)
	subscription="t_system";
else
	subscription="t_treshold";
}*/
void LDR() {
  int analogValue = analogRead(ldr_pin);
 float voltage = analogValue / 4090. * 3.3;
 float resistance = 2000 * voltage / (1 - voltage / 3.3);
  lux = pow(50 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
  Serial.print("LUX:");
  Serial.println(analogValue);
}

void display_show() {
	change_lcd();
if (display_state==0){

	if(millis() - lastMillis_6> 2000){


display.clearDisplay();
if (display_mode==1){
display.setTextSize(1);
display.setTextColor(SH110X_WHITE);
display.setCursor(0, 0);
display.print("Temperature:");
display.println((String)temp);
display.print("Luminosity:");
display.println(lux);
display.print("Humidity:");
display.println(hum);
display.print("Fan Status:");
display.println(fan_state);

display.display();
}
else
{
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0, 0);
	display.print("User:");
	display.println((String)sum);
	display.print("Treshold temperature:");
	display.println(temp_threshold);
	display.print("Latency of mqtt:");
	display.println(latency);
	display.print("ms");
	display.display();

}



lastMillis_6 = millis();
}

}else if (display_state==1){

if(millis() - lastMillis_6> 2000){
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SH110X_WHITE);
display.setCursor(0, 0);
display.print("Power and door are shutdown, enter a valid id to open the system");
display.display();
lastMillis_6 = millis();
		}
}
}
void pir_sensor() {

  detection_var = digitalRead(pir);  // read input value
  if (detection_var == HIGH) {            // check if the input is HIGH
    digitalWrite(alert_pin, HIGH);  // turn LED ON
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      rc522(rfid.uid.uidByte,rfid.uid.size);//le se o cartao e valido
      pirState = HIGH;

  } else {
    digitalWrite(alert_pin, LOW); // turn LED OFF
    if (pirState == HIGH) {
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }

  }
}


void rc522(byte *buffer, byte bufferSize){


 if ( rfid.PICC_IsNewCardPresent() and rfid.PICC_ReadCardSerial()){
	 int id[bufferSize];

 // Repeat anti collision loop
 //Verifica se existe colisão entre leituras, entra num loop de verificação de anti colosão


 //Serial.print(F("PICC type: "));
 MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
 //Serial.println(rfid.PICC_GetTypeName(piccType));

 // Deteta o tipo de tag que é lida
 //Caso não seja destes tipos não é da marca MIFARE
 if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
     piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
     piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
   Serial.println(F("Not MIFARE Classic tag type."));
   return;
 }

 // Conversão dos bytes lidos para um array
 for (byte i = 0; i < 4; i++) {
   nuidPICC[i] = rfid.uid.uidByte[i];

 }

 code1 = ( buffer[0]) + ( buffer[1] << 8) + ( buffer[2] << 16) + (buffer[3] << 24);

 id[3] = code1 >> 24;
 id[2] = code1 >> 16 & 0x00FF;
 id[1] = code1 >> 8 & 0x0000FF;
 id[0] = code1 & 0x000000FF;

sum= id[0] + id[1] +id[2] + id[3];

 Serial.print("USER IDENTIFIED WITH ID:");
Serial.println(String(sum));
if (sum==637){

	if (flag_entrada==0){
		validate=true;
		flag_entrada=1;
	}
	else{
		validate=false;
		flag_entrada=0;
	}

}

}
}
void buzzer(){

	if (millis() - lastMillis_5 > 250){
		ledcAttachPin(buzzer_pin, ledChannel);
ledcWrite(ledChannel,50);
lastMillis_5=millis();
}}


  void relay_activation(){
	  if (millis() - lastMillis_7 > 250){
  if (temp> temp_threshold){

      digitalWrite(relay_fan, HIGH);  // turn LED ON
      digitalWrite(relay_resistor, LOW);  // turn LED ON
      fan_state="LOW";

  }
  else if(temp< temp_threshold){

    digitalWrite(relay_fan, HIGH);  // turn LED ON
      digitalWrite(relay_resistor, HIGH);  // turn LED ON
      fan_state="HIGH";

  }
  lastMillis_7=millis();
  }
  }

  /*void File_creation() {

    //Check file name exist?
	    myFile = SD.open(filename);
    if (!myFile) {

        myFile = SD.open(filename, FILE_WRITE);
    if (myFile) {
      myFile.println("New file created");

      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }
    else
    File_read();
  }


  void File_read() {
	  myFile = SD.open(filename);
    if (myFile) {
      Serial.println("taglist.txt:");

      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.write(myFile.read());
      }


      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }
  void File_write() {
    String last_reading;
    int file_counter = 0;
    int pos_reg;
    myFile = SD.open(filename);

    if (myFile) {

      while (myFile.available()) {

        last_reading = myFile.readStringUntil(','); // para ler x

        if (last_reading.length() != 0) {
          pos_reg = myFile.position();

        } else
          pos_reg = 0;

        file_counter++;

      }
      myFile.close();

      myFile = SD.open(filename, FILE_WRITE);

      if (myFile) {
        if (myFile.seek(pos_reg)) {
          //myFile.println('\n');
          myFile.println("{");
          myFile.print("temperatura");
          myFile.print(":");
          myFile.print(temp);
          myFile.print(",");
          myFile.print("humidade");
          myFile.print(":");
          myFile.print(hum);
          myFile.print(",");
          myFile.print("luminosidade");
          myFile.print(":");
          myFile.print(lux);
          myFile.print("}");

          myFile.close();

        }
      }

    }
  }
*/
void information_show(){
	 Serial.println("Utilizador: " + String(sum));
	  Serial.println("temperature: " + String(temp, 2) + "°C");
	  Serial.println("humidity: " + String(hum, 1) + "%");
	  Serial.println("luminosiy: " + String(lux, 1) + "Lux");
	  Serial.println("Temperature fan:"+String(fan_state));
Serial.println("Time of delay on data transmission-reception:"+String(latency)+"ms");

	Serial.println("---");
	  //Serial.println("State of Fan: " + (bool)analogRead(relay_fan));
	 // Serial.println("State of Heating resistor: " + (bool)analogRead(relay_resistor));
}

void information_system_close(){
	 Serial.println("Door is close, so the system is off!");
	  Serial.println("Insert your RFID card to enter the door");

}


void security(){

	 if (validate== true){
	 if (detection_var== 1){
		system_start();
		digitalWrite(door, HIGH);
		display_state=0;
	}}

	else if (validate== false){
	if(detection_var== 1){

	digitalWrite(door, LOW);
	information_system_close();
	display_state=1;
	buzzer();
	system_status="OFF";
}}

}

void change_lcd(){
	int mode=digitalRead(lcd_button);
	if(mode==1){
if (millis() - lastMillis_8 > 250){
if(mode==1){
display_mode=1;

}

lastMillis_8=millis();

}}
	else
	display_mode=0;

}

/*
void change_threshold(){

	int dec_t=digitalRead(dec_button);
	int inc_t=digitalRead(inc_button);


if (dec_t==0){
if (temp_threshold>15)
temp_threshold--;
}

if (inc_t==0){
if (temp_threshold<30)
temp_threshold++;
}
}
*/
void system_start(){

TempAndHumidity  data = dhtSensor.getTempAndHumidity();
temp=data.temperature;
hum=data.humidity;
LDR();
relay_activation();
information_show();
ledcDetachPin(buzzer_pin);
//change_threshold();
system_status="ON";
}


void publish_on_mqtt(){
int counter=1;
	reconnect();
	char char_temperature[5];
	char char_humidity[5];
	char char_lux[5];
	char char_rfid[5];
	char door_state[4];
	char c_fan_state[5];
	char latency_packets[5];
	String s_door;
	String S_t;
	String S_h;
	String S_l;
	String S_rfid;
String lat;
	S_rfid=String(sum);
	S_t=String(temp,5);
	S_h=String(hum,5);
	S_l =String(lux,5);
	s_door=system_status;
	lat=String(latency,2);
	S_t.toCharArray(char_temperature,5);
	S_h.toCharArray(char_humidity,5);
	S_l.toCharArray(char_lux,5);
	S_rfid.toCharArray(char_rfid, 5);
	s_door.toCharArray(door_state, 6);
	lat.toCharArray(latency_packets, 5);
	fan_state.toCharArray(c_fan_state,6);
	client.publish("t_temp",char_temperature);
	client.publish("t_hum",char_humidity);
	client.publish("t_lum",char_lux);
	client.publish("t_rfid",char_rfid);
	client.publish("t_door",door_state);
	client.publish("fan_state",c_fan_state);
	client.publish("latency",latency_packets);
	time_sent=millis();
//client.subscribe(subscription);
client.loop(); // Call the loop to maintain connection to the server.

}
void reconnect() {

	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		client.subscribe(subscription);
		if (client.connect("user_esp32",user,pass)) {

			Serial.println("connected");
			client.subscribe(subscription);
		} else {
			client.subscribe(subscription);
			Serial.print("failed, rc=");
			Serial.print(client.state());

		}
	}
}
void callback(char* topic, byte* payload, unsigned int length) {
	Serial.println("*************************************************************************");
Serial.print(topic);

time_received=millis();
latency=time_received-time_sent;
time_received=0;
time_sent=0;
if (strcmp(topic,"t_treshold")==0) {


	String message;
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    message=message+ (char)payload[i];
  }
  char i[6];
  message.toCharArray(i,6);
  Serial.println("eeeeeeeeeeeeeeeeee"+message);

  if((strcmp(i,"power")==0)){
	  Serial.println("ddddddddddddddddddddddddddddddd");
	  if(validate==true)
		  validate=false;
	  else
		  validate=true;
  }else{
  temp_threshold=message.toFloat();
  Serial.println("Temperature treshold changed to:");
  Serial.println(message)
		  ;}}
/*
else if(strcmp(topic,"t_system")==0){

	validate=bool(payload);
/*
	char message[10];
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    message[i]=message[i-1]+ (char)payload[i];
  }
	if(strcmp(message,"on")==0)
	validate=1;
	else
	validate=0;
*/



}
