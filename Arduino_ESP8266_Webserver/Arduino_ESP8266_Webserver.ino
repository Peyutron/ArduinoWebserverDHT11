/*Control de Temperatura Arduino con ESP8266, DHT11 y
 * sensor de humedad en tierra.
 * Este pequeño programa muestra una pagina web con los datos
 * recibidos desde el sensor DHT11 y transmitidos con el 
 * modulo WiFi ESP8266.
 * para un correcto funcionamiento del programa se aconseja
 * actualizar el Firmware del modulo ESP8266 a la version
 * de SDK: 1.3.0
*/

#include <SoftwareSerial.h>
#include "DHT.h"
#define DEBUG true
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE,11);

SoftwareSerial esp8266(3,2); 
// El TX del módulo al terminal 3 del Arduino.
// El RX del módulo al terminal 2 del Arduino.

String json,solicitud;
String peticionHTTP;
int humvalor = 0;
float humidity, tempC;  // Variables para el sensor DHT11
unsigned long previousMillis = 0; // will store last temp was read
const long interval = 2000;   
String pagWeb;

void setup()
{

dht.begin();           // Inicializa el sensor DHT11
Serial.begin(9600);
esp8266.begin(19200); // Importante la velocidad del módulo.
initConfEsp();
}

void loop()
{
if(esp8266.available()) // Consulta si el módulo está enviando algún mensaje
{
if(esp8266.find("+IPD,"))
{
delay(1000);
int connectionId = esp8266.read()-48; 

// Aquí las construcción de la página con dos formularios.
         // Enviamos al cliente una respuesta HTTP
pagWeb =("HTTP/1.1 200 OK");
pagWeb =("Content-Type: text/html");

//String pagWeb = "<head><meta http-equiv=\"refresh\" content=\"4\"></head>";
pagWeb = "<head><meta http-equiv=\"refresh\" content=\"4\"></head>";
// String pagWeb = "<head></head>";
pagWeb += "<h1>Infotronikblog.com</h1>";
gettemperature();   // Adquiere los datos del sensor DHT11
pagWeb += "Temperatura: ";
alertaTemp();
pagWeb +=("<br />");   
pagWeb += "Humedad: ";
pagWeb += humidity;
alertaHum();
pagWeb +=("<br />");
pagWeb += "Humedad en tierra: ";
grdSensor();
pagWeb +=("<br />");
/*getJsonString();
pagWeb += (json);
Serial.print(json);*/


String cipSend = "AT+CIPSEND=";
cipSend += connectionId;
cipSend += ",";
cipSend +=pagWeb.length();
cipSend +="\r\n";

sendConf(cipSend,1000,DEBUG);
sendConf(pagWeb,1000,DEBUG);

// Cierra la conexión
String closeCommand = "AT+CIPCLOSE="; 
closeCommand+=connectionId;
closeCommand+="\r\n";

sendConf(closeCommand,3000,DEBUG);
}
}
}

// Función para Enviar datos al Servidor.
String sendConf(String command, const int timeout, boolean debug)
{
String response = "";
esp8266.print(command); // Envía la información de command al servidor
long int time = millis();

while( (time+timeout) > millis())
{
while(esp8266.available())
{
// A response van los datos que regresan al servidor.
char c = esp8266.read(); // Va leyendo caracter a caracter.
response+=c; 
} 
}

if(debug)
{
String variable;
Serial.print(response);
} 
return response;
}

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Lee la humedad (porcentaje))
    tempC = dht.readTemperature(false);     // Lee la temperatura en Celsiun
        // Comprueba si las lecturas han fallado y sale 
    if (isnan(humidity) || isnan(tempC)) {
      Serial.println("Fallo al leer el sensor DHT!");
      return;
    }
  }
}
void getJsonString()
{
json = "{\"";
json += "temperatura\": \"" + (String)tempC + "\", ";
json += "uptime""\": \"" + (String)millis() + "\" ";
json += "}\n";
  
}
void grdSensor()
{
  humvalor = analogRead(0);
  //pagWeb += (String (humvalor));
    int percentValue = map(humvalor, 0, 700, 0, 100);
  //   Serial.println (humvalor);
  pagWeb += percentValue;
  if (humvalor <= 300)
    pagWeb += ("% Seco, necesitas regar ");
  if ((humvalor > 300) and (humvalor <= 700))
    pagWeb += ("% Humedo, no regar ");
  if (humvalor > 700)
    pagWeb += ("% Encharcado ");
  delay(100);
}

void alertaTemp()
{
  pagWeb += (tempC);
  if (tempC <= 18)
    pagWeb += ("C Temperatura baja ");
  if ((tempC > 18) and (tempC <= 28)) {
    pagWeb += ("C Temperatura optima ");
  }
  if (tempC > 28) {
    pagWeb += ("C Temperatura alta,se enciende extraccion ");
  }
  delay(100);
}
void alertaHum()
{
  //pagWeb += (String (humidity));
  if (humidity <= 10)
    pagWeb += ("% Humedad baja ");
  if ((humidity > 10) and (humidity <= 70))
    pagWeb += ("% Humedad optima ");
  if (humidity > 70)
    pagWeb += ("% Humedad alta ");
  delay(100);
}

void initConfEsp()
// Configuracion del modulo ESP8266. 
// SSID='nombredeturouter' cambia esto por el nombre de tu router.
// Pass='contraseñadelrouter' Cambia esto por la contraseña de tu router.
{
//sendConf("AT+RST\r\n",2000,DEBUG); // Borra la configuración que tenía el módulo
//sendConf("AT+CWJAP=\"nombredeturouter\",\"contraseñadelrouter\"\r\n", 2000, DEBUG);
delay(2000); // Espera un poco que conecte con el Router.
sendConf("AT+GMR\r\n",1000,DEBUG); // Muestra la informacion del firmware.
sendConf("AT+CWMODE=3\r\n",1000,DEBUG); // Modo de cliente y servidor.
sendConf("AT+CIFSR\r\n",1000,DEBUG); // En el Serial Monitor aparece la IP de cliente y servidor.
sendConf("AT+CIPMUX=1\r\n",1000,DEBUG); // Multiples conexiones.
sendConf("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // El Puerto web es el 80.
}

