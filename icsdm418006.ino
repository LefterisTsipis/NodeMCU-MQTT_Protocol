// Define libraries
//-----------------------------
#include <WiFi.h>
#include <DHTesp.h>  // Library to be used to read the DHT22 sensor
#include <PubSubClient.h>
#include <stdio.h>

typedef unsigned char uint8_t;
/* gloabal variable  */

int sendDaratoServer;
char sendDaratoMQTTServer[16];
int flag=0;

int TempLimit=0;
int humLimit=0;
int SamplingLimit=0;
/** define DHT */

#define DHTPIN 23     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 
DHTesp  dht;

// MQTT connection parameters
const char* MQTT_USER = "user8"; 
const char* MQTT_CLIENT = "user8";  
const char* MQTT_TOPIC_ESP32 = "dataCrc";
const char* MQTT_TOPIC_SERVER = "data";
const char* MQTT_TOPIC_BOTH = "control";
const char* MQTT_ADDRESS = "esp-32.zapto.org";

// WiFi connection parameters 

const char* WIFI_SSID     = "lefis";
const char* WIFI_PASSWORD = "LTicsd12189";



/*   TOPICS parameters  */

char TOPIC_ESP32[150], TOPIC_SERVER[150],TOPIC_BOTH[150];


/*   MQTTparameters  */

void callback(char*, byte*, unsigned int);
WiFiClient wifiClient;
PubSubClient client(MQTT_ADDRESS, 1883, callback, wifiClient);

/* callback function  after interrupt this function is enabled  */
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  String topic_str(topic);

  // Print received messages and topics
  //------
  Serial.println("");
  Serial.println("Callback function");
  Serial.println("Topic: ");
  Serial.println(topic);

  Serial.println("Message: ");
  Serial.println(message);
  //------
  //------

  // Check received messages and topics
  //------
  
  if (topic_str == TOPIC_BOTH) {
    if (message == "startMeasurements")
    {
      Serial.println("start Measurements ");
           flag=1;//when flag is 1 the startMeasurements() function is starting 
    }
    }
  if (topic_str == TOPIC_SERVER) {
   CrcCode(message.toInt());//if we receive 6 number from server we pass it to the CrcCode
   //sendDaratoServer=sendDaratoServer;// we receive the result of crc function at the global variable sendDaratoServer
   sprintf( sendDaratoMQTTServer, "%09d", sendDaratoServer );// we convert sendDaratoServer variable to char 
  // itoa(sendDaratoServer, sendDaratoMQTTServer,10);
        delay(1000);
    if (client.connected()) { // if MQTT server connected
      client.publish(TOPIC_ESP32, sendDaratoMQTTServer);//we send the encoding number to the server
      Serial.println("");
      Serial.println("**************************");
      Serial.println("Message published");
  
    }
  }

  if (topic_str == TOPIC_BOTH) {
    if (message == "updateConfig")
    {
      Serial.println("updateConfig ");
       //ESP.restart();
       flag=0;//id flag is 0 then the startMeasurements() function is stoping 
       init();//the the program starts from the beginning by the init() function
    }
    }
  //------
  //------
  
}
/** this function make the first connection  with the mqtt broker  and makes reconnection after a connection failure*/
void mqttReconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT, "samos", "karlovasi33#")) {
      Serial.println("MQTT connected");
      topicSubscribe();
    } 
    
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
   delay(5000);
  /*  after troubleshooting of  connection failure we start or restart  the process*/
flag=0;
  init();
}
/*Subscribe to MQTT topics*/
 void topicSubscribe() {
  if(client.connected()) {

    Serial.println("Subscribe to MQTT topics: ");
    Serial.println(TOPIC_ESP32);
    Serial.println(TOPIC_SERVER);
    Serial.println(TOPIC_BOTH);
    client.subscribe(TOPIC_ESP32);
    client.subscribe(TOPIC_SERVER);
     client.subscribe(TOPIC_BOTH);
    client.loop();
  }  
}
/*this function performs the process of   temperature measuring*/
void startMeasurements()
{
   
 TempAndHumidity newValues = dht.getTempAndHumidity();//get the temperature and humidity from DHT sensor 
  Serial.print(F(" Temperature: "));
  Serial.print(newValues.temperature);//display temperature
 
  Serial.print(F("  Humidity: "));
  Serial.print(newValues.humidity);//display humidity 
  Serial.println(); 
  Serial.print(F("  Temperature threshold: "));//display Temperature threshold
   Serial.print(TempLimit);
    Serial.print(F("  Humidity threshold: "));//display Humidity threshold
    Serial.print(humLimit);
      Serial.print(F("  Sampling time: "));//display Sampling threshold
        Serial.print(SamplingLimit);
          Serial.println(); 
        /* alarm display*/
  Serial.println(F(" ********************************************************* "));
        if(newValues.temperature>TempLimit){
             Serial.print(F("Temperature High" ));
          }
           Serial.println(); 
          if(newValues.humidity>humLimit){
             Serial.print(F("Humidity High"));
             
            }
                Serial.println(); 
   Serial.println(F(" ********************************************************* "));
  delay(SamplingLimit*1000);
      Serial.println(); 
 }
void setup() {
   // Connect to WiFi and establish serial connection
  
  Serial.begin(115200);
  delay(1000);//1 sec delay

  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Wait for WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //------
  //------

  // Connect to MQTT broker and subscribe to topics
  //------
  client.setCallback(callback);

  // Define MQTT topic names
  sprintf(TOPIC_ESP32, "%s/%s", MQTT_USER, MQTT_TOPIC_ESP32);
  sprintf(TOPIC_SERVER, "%s/%s", MQTT_USER, MQTT_TOPIC_SERVER);
  sprintf(TOPIC_BOTH, "%s/%s", MQTT_USER, MQTT_TOPIC_BOTH);
//connection with mqtt server (broker)
  Serial.print("Wait for MQTT broker...");
 mqttReconnect();
 
  /* DHT22 */
  dht.setup(DHTPIN, DHTesp::DHT22);




}

void loop() {
 Serial.print(" ");
 delay(2000);
 if (flag==1){
 startMeasurements();
 }

 if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnecting to WiFi...\n");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //------
  //------

  // Reconnect to MQTT broker if not connected
  //------
  if (!client.connected()) {
    mqttReconnect();
  }
  //------
  //------

  
  client.loop();
}

void init(){
  //int innerflag=0;

    if (client.connected()) { // if MQTT server coinnected
      client.publish(TOPIC_BOTH, "sendConfig");
      Serial.println("");
      Serial.println("**************************");
      Serial.println("Message published");
  
    }
  }


  uint8_t Compute_CRC8(uint8_t data)
{

    uint8_t crc = 0;
  const uint8_t generator = 0x8;
  uint8_t b[2]={data, 0x00};
int i,j=0;
  for ( j = 0; j<2; j++)
  {
      for ( i = 7; i >= 0; i--)
      {
          /* check if MSB is set */
            if ((crc & 0x80) != 0)
            {   /* MSB set, shift it out of the register */
                crc = (uint8_t)(crc << 1);
                /* shift in next bit of input stream:
                 * If it's 1, set LSB of crc to 1.
                 * If it's 0, set LSB of crc to 0. */
                crc = ((uint8_t)(b[j] & (1 << i)) != 0) ? (uint8_t)(crc | 0x01) : (uint8_t)(crc & 0xFE);
                /* Perform the 'division' by XORing the crc register with the generator polynomial */
                crc = (uint8_t)(crc ^ generator);
            }
            else
            {   /* MSB not set, shift it out and shift in next bit of input stream. Same as above, just no division */
                crc = (uint8_t)(crc << 1);
                crc = ((uint8_t)(b[j] & (1 << i)) != 0) ? (uint8_t)(crc | 0x01) : (uint8_t)(crc & 0xFE);
            }



      }


  }

    return crc;
}

void CrcCode(int d)
{
uint8_t crc;
int insertdata =d;//her we take the numbert of the server
int num1[3]={0,0,0};
int num2[3]={0,0,0};
int i=0;
/* at the egining we have six consecutive numbers (πχ606060) after this while we will take three number which consisting of two digits

60
60
60

*/
while(insertdata!=0)
  {
      num1[i]=insertdata%100;
    insertdata=insertdata/100;

    i++;
  }
int j=0;
//here we dispaly the three new number number
for(i=2;i>=0;i--)
    {  printf("%d\n",num1[i]);}
printf("******************************************\n");
 TempLimit=num1[1];
 humLimit=num1[0];
 SamplingLimit=num1[2];

    for(i=2;i>=0;i--){
      num2[j++]=000+Compute_CRC8(num1[i]);//here we  perform the Compute_CRC8 function for the three new number
            }

     int x;
     Serial.println("  crc process\n");
  for(i=0;i<3;i++)
    {//here we display three encoded numbers that the Compute_CRC8 function return
       printf("%03d\n",num2[i]);

    }

 sendDaratoServer=((num2[2]+(num2[1]*1000)+(num2[0]*1000000)));// the three different numbers are appropriately joined
    /**  APOTELESMA */
printf("the crc encoded  data that will be sent to the server is \n");
printf("%09d \n",sendDaratoServer);

}
