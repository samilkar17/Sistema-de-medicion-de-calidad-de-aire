#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#define D4 2//led
#define D3 0// buzzer




//---- variables para conexion a red-------
const char *ssid = "-------";
const char *password = "------";


unsigned long previousMillis = 0;


char host[48];
String strhost = "---------"; // cambiar por una ip asignada en nuestro pc
String strurl = "-------------"; // Ruta para el envio de los datos al servidor local
String nodo = "";

//-------- variables del sensor ultrasonico--------

const int EchoPin = 4;
const int TriggerPin = 5;

//----- funcion para calcular distancia----------

int ping(int TriggerPin, int EchoPin) {
  long duration, distanceCm;
  
  digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  
  duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
  
  distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
  return distanceCm;


}

//-------Función para Enviar Datos a la Base de Datos SQL--------

String enviardatos(String datos) {
  String linea = "error";
  WiFiClient client;
  strhost.toCharArray(host, 49);
  if (!client.connect(host, 80)) {
    Serial.println("Fallo de conexion");
    return linea;
  }

  client.print(String("POST ") + strurl + " HTTP/1.1" + "\r\n" + 
               "Host: " + strhost + "\r\n" +
               "Accept: */*" + "*\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
               "\r\n" + datos);           
  delay(10);             
  
  Serial.print("Enviando datos a SQL...");
  
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Cliente fuera de tiempo!");
      client.stop();
      return linea;
    }
  }
  // Lee todas las lineas que recibe del servidor y las imprime por la terminal serial
  while(client.available()){
    linea = client.readStringUntil('\r');
  }  
  Serial.println(linea);
  return linea;
}


void setup() {

  // Inicia Serial
  Serial.begin(115200);
  pinMode(D4, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  Serial.println(""); 

  Serial.print("Nodo:");
  nodo = String(ESP.getChipId());
  Serial.println(nodo);

  
  // Conexión WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//--------------------------LOOP--------------------------------
void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 3000) { //envia el aire cada 3 segundos
    previousMillis = currentMillis;
    int aire = analogRead(A0);
    int cm = ping(TriggerPin, EchoPin);
    
    if(aire > 400 || cm <10){
  digitalWrite(D4,HIGH);
  digitalWrite(D3,HIGH);
   
  }
 else{
  digitalWrite(D4,LOW);
  digitalWrite(D3,LOW);
 
 }
 
    Serial.print("Distancia: ");    
    Serial.println(cm);
    Serial.print("Calidad del aire: ");
    Serial.println(aire);
    enviardatos("nodo=" + nodo + "&aire=" + String(aire));
  }
}
