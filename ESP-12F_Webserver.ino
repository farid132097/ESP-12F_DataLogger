
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient   espClient;
PubSubClient client(espClient);

typedef struct wifi_target_t{
  String SSID;
  int    RSSI;
}wifi_target_t;

typedef struct wifi_list_t{
  int    DevicesFound;
  String SSID[30];
  int    RSSI[30];
  int    StrongestRSSI;
  int    StrongestRSSIIndex;
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
  WIFI->Shortlisted.StrongestRSSI=-500;
  for(int i=0;i<WIFI->All.DevicesFound;i++){
    if(strstr(WIFI->All.SSID[i].c_str(), "MFI")){
      WIFI->Shortlisted.SSID[WIFI->Shortlisted.DevicesFound]=WIFI->All.SSID[i];
      WIFI->Shortlisted.RSSI[WIFI->Shortlisted.DevicesFound]=WIFI->All.RSSI[i];
      if(WIFI->Shortlisted.RSSI[WIFI->Shortlisted.DevicesFound]>WIFI->Shortlisted.StrongestRSSI){
        WIFI->Shortlisted.StrongestRSSI=WIFI->Shortlisted.RSSI[WIFI->Shortlisted.DevicesFound];
      }
    }
  }
}


void setup() {
  WIFI_Init();
  Serial.begin(38400);
  //setup_wifi();
  //client.setServer(mqtt_server, 1883);
  //client.setBufferSize(2000);

  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
}

void loop() {

  WIFI_Scan();
  for(int i=0;i<WIFI->All.DevicesFound;i++){
    Serial.print("[");
    Serial.print(WIFI->All.RSSI[i]);
    Serial.print("] ");
    Serial.print(WIFI->All.SSID[i]);
    Serial.print("\r\n");
  }
  Serial.print("[STRONGEST] ");
  Serial.print("[");
  Serial.print(WIFI->All.RSSI[WIFI->All.StrongestRSSIIndex]);
  Serial.print("] ");
  Serial.print(WIFI->All.SSID[WIFI->All.StrongestRSSIIndex]);
  Serial.println("\r\n");

  delay(1000);
}
