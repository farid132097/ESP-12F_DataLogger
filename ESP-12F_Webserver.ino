
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient   espClient;
PubSubClient client(espClient);

typedef struct wifi_target_t{
  uint8_t Found;
  String  SSID;
  int     RSSI;
  uint8_t Connected;
  uint8_t ClientConnected;
}wifi_target_t;

typedef struct wifi_list_t{
  int     DevicesFound;
  String  SSID[30];
  int     RSSI[30];
  int     StrongestRSSI;
  int     StrongestRSSIIndex;
}wifi_list_t;


typedef struct wifi_device_t{
  wifi_list_t   All;
  wifi_list_t   Shortlisted;
  wifi_target_t Target;
}wifi_device_t;

wifi_device_t WIFI_type;
wifi_device_t *WIFI;

void WIFI_Init(void){
  WIFI=&WIFI_type;
  WIFI->All.DevicesFound=0;
  WIFI->All.StrongestRSSI=0;
  WIFI->All.StrongestRSSIIndex=0;
  WIFI->Shortlisted.DevicesFound=0;
  WIFI->Shortlisted.StrongestRSSI=0;
  WIFI->Shortlisted.StrongestRSSIIndex=0;
  WIFI->Target.Found=0;
  WIFI->Target.SSID="";
  WIFI->Target.RSSI=0;
  WIFI->Target.Connected=0;
  WIFI->Target.ClientConnected=0;
}

void WIFI_Scan(void){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WIFI->All.StrongestRSSI=-500;
  WIFI->All.DevicesFound=WiFi.scanNetworks();
  for(int i=0;i<WIFI->All.DevicesFound;i++){
    WIFI->All.SSID[i]=WiFi.SSID(i);
    WIFI->All.RSSI[i]=WiFi.RSSI(i);
    if(WIFI->All.RSSI[i]>WIFI->All.StrongestRSSI){
      WIFI->All.StrongestRSSI=WIFI->All.RSSI[i];
      WIFI->All.StrongestRSSIIndex=i;
    }
  }
}

void WIFI_Shortlist(){
  WIFI->Shortlisted.DevicesFound=0;
  for(int i=0;i<WIFI->All.DevicesFound;i++){
    if( (strstr(WIFI->All.SSID[i].c_str(), "MFI"))  ||  (strstr(WIFI->All.SSID[i].c_str(), "Far")) ){
      WIFI->Shortlisted.SSID[WIFI->Shortlisted.DevicesFound]=WIFI->All.SSID[i];
      WIFI->Shortlisted.RSSI[WIFI->Shortlisted.DevicesFound]=WIFI->All.RSSI[i];
      WIFI->Shortlisted.DevicesFound++;
    }
  }
}

void WIFI_Select_Target(void){
  WIFI->Target.Found=0;
  WIFI->Shortlisted.StrongestRSSI=-500;
  for(int i=0;i<WIFI->Shortlisted.DevicesFound;i++){
    if(WIFI->Shortlisted.RSSI[i]>WIFI->Shortlisted.StrongestRSSI){
      WIFI->Shortlisted.StrongestRSSI=WIFI->Shortlisted.RSSI[i];
      WIFI->Shortlisted.StrongestRSSIIndex=i;
    }
  }
  if(WIFI->Shortlisted.StrongestRSSI!=-500){
    WIFI->Target.RSSI=WIFI->Shortlisted.RSSI[WIFI->Shortlisted.StrongestRSSIIndex];
    WIFI->Target.SSID=WIFI->Shortlisted.SSID[WIFI->Shortlisted.StrongestRSSIIndex];
    WIFI->Target.Found=1;
  }
}

void WIFI_Connect_To_Target(void){
  int counter=0;
  WIFI->Target.Connected=0;
  if(WIFI->Target.Found==1){
    WiFi.begin(WIFI->Target.SSID.c_str(), "abcddcba");
    Serial.print("[");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      counter++;
      if(counter>50){
        break;
      }
    }
    Serial.println("]");
    if(counter<50){
      Serial.print("Time Taken: ");
      Serial.print(counter/2);
      Serial.println("s");
      WIFI->Target.Connected=1;
    }
  }
}

void WIFI_Connect_To_Client(void){
  while (!client.connected()){
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    //if (client.connect(clientId.c_str()))
    if (client.connect("EhsZOxQzMTENMjEaFyk8IwU"))
    {
      WIFI->Target.ClientConnected=1;
    }
    else
    { 
      WIFI->Target.ClientConnected=0;
      if (WiFi.status() != WL_CONNECTED){
        WIFI_Connect_To_Target();
      }
      delay(5000);
    }
  }
}

void Print_All_WiFi_Devices(void){
  Serial.print("ALL DEVICES:\r\n");
  for(int i=0;i<WIFI->All.DevicesFound;i++){
    Serial.print("[");
    Serial.print(WIFI->All.RSSI[i]);
    Serial.print("] ");
    Serial.print(WIFI->All.SSID[i]);
    Serial.print("\r\n");
  }
  Serial.print("\r\n");
}

void Print_Shortlisted_WiFi_Devices(void){
  Serial.print("SHORTLISTED DEVICES:\r\n");
  for(int i=0;i<WIFI->Shortlisted.DevicesFound;i++){
    Serial.print("[");
    Serial.print(WIFI->Shortlisted.RSSI[i]);
    Serial.print("] ");
    Serial.print(WIFI->Shortlisted.SSID[i]);
    Serial.print("\r\n");
  }
  Serial.print("\r\n");
}

void Print_Target_WiFi_Device(void){
  Serial.print("TARGET DEVICE:\r\n");
  Serial.print("[");
  Serial.print(WIFI->Target.RSSI);
  Serial.print("] ");
  Serial.print(WIFI->Target.SSID);
  Serial.print("\r\n");
  Serial.print("\r\n");
}

void setup() {
  WIFI_Init();
  Serial.begin(38400);
  client.setServer("test.mosca.io", 80);
  client.setBufferSize(2000);
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  delay(8000);
}

void loop() {

  WIFI_Scan();
  WIFI_Shortlist();
  WIFI_Select_Target();

  Print_All_WiFi_Devices();
  Print_Shortlisted_WiFi_Devices();
  Print_Target_WiFi_Device();

  WIFI_Connect_To_Target();
  if(WIFI->Target.Connected){
    Serial.println("Connected to WiFi");
  }
  WIFI_Connect_To_Client();
  if(WIFI->Target.ClientConnected){
    Serial.println("Connected to Client");
    digitalWrite(2,LOW);
    while(1);
  }else{
    Serial.println("Cannot connect to Client");
    delay(1000);
  }

  
}
