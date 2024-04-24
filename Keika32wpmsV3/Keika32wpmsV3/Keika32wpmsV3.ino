 /*
   ESP32 AJAX Demo
   Updates and Gets data from webpage without page refresh
   https://circuits4you.com
Se considera la version 0.5 porque estan funcionando todos los sensores y los plugs
W por mantener la conectividad wifi ya se ha generado codigo para guardar datos enteros en la microsd
manteniendo el formato entregado por carolina trujillo y el servidor de C
Codigo implementado por Alejandro Rebolledo 2021
ultima modificacion 12- 04- 2022
 * ESTE CODIGO ES DEFINITORIO PARA LOS PINES
 * SERVIRA DE BASE PARA ARMAR TODO EL CIRCUITO KEICA, DE BASE SE USO UN 
 * LOLIN ESP32 LITE
 * PARA LA SD:
 * 19 MISO (SPI)
 * 23  MOSI (SPI)
 * 18  SCK (SPI)
 * 5 CS (SPI)
 * links de referenciaspara estructurar la forma de guardar los datos
 * https://randomnerdtutorials.com/esp32-data-logging-temperature-to-microsd-card/
 * https://techtutorialsx.com/2018/10/11/esp32-arduino-fat-file-system-checking-if-file-exists/
 * 
 * /////////// links de referenica para el uso de la web SD
 * http://kio4.com/arduino/9130f.htm
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer/examples/SDWebServer
 * 
 * links al mac adrres
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ChipID/GetChipID/GetChipID.ino
 * 
 * links a las librerias necesarias
 * pantalla es la U8g2lib
 * https://github.com/olikraus/u8g2
 * para generar graficos
 * https://p3dt.net/post/2019/09/17/simple-ui-elements.html
 * fonts usadas en este programa sacados de la libreria
 * u8g2.setFont(u8g2_font_6x10_tf); //font de 7 px
 * u8g2.setFont(u8g2_font_10x20_tf); // 13 px
 * u8g2.setFont( u8g2_font_crox4hb_tr);  //font grande
 * la sonda externa de temperatura es DS18b20 es la primera en la busqueda que trae dallas temperature
 * https://github.com/milesburton/Arduino-Temperature-Control-Library
 * la base usa un BME280 hay que considerar que hay muchos batchs malos del sensor se uso el tradiciona de adafruit por el soporte a sacar la altura al nivel del mar de forma nativa
 * https://github.com/adafruit/Adafruit_BME280_Library
 * plantower no usa una libreria en particular si no usa el segundo serial de hardware segun la referecia de 
 * https://wiki.dfrobot.com/Air_Quality_Monitor__PM_2.5,_Formaldehyde,_Temperature_&_Humidity_Sensor__SKU__SEN0233
 * https://www.dfrobot.com/product-1272.html //sobre los sensores en si al parecer hay discrepancias con los sensores sus librerias y codigos
 * si bien el ultimo producto en la libreria a descargar sale otro modelo, al parecer hay variedad de sensores con las mismas caracteristicas.
 * habra que revisar y considerar que sensores son los mejores y cuales bloquean menos el codigo, al parecer tambien es necesario aumentar el baud para que el procesador funcione 
 * mas rapido aun asi existira la posiblidad de necesitar un micro en el plantawer, pero primero hay que definir que clase de sensor es.
 * Efectivamente los dispositivos chinos usan otra libreria, sin embargo funcionan mejor que los anteriores por lo tanto se ha usado la libreria PMserial
 * https://github.com/avaldebe/PMserial
 * 
*/
/////////////////////libreria tone32 para el buzz depreciada a partir de la version 2.0.2 no se usa
//#include <Tone32.h>

#define BUZZER_PIN 14
//#define BUZZER_CHANNEL 0
//////////////////////////////
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
///////////////////////websd
#include <ESPmDNS.h>
///////////////////// SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"
//////////////////////////////////
//===============================================================
// Sleep
//===============================================================
/////////////////////////////////
// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// Sleep for 1 hour = 3600 seconds // 1 minuto 60 segundos
RTC_DATA_ATTR int TIME_TO_SLEEP = 10; //el deep sleep se puede guardar como variable para guardar en el reloj asi se puede modificar pero hasta que no se resetea no se pierde
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean recordSleep = false;
RTC_DATA_ATTR boolean firstRecordSleep = true;
//////////variables de nombres para guardar con reloj interno
RTC_DATA_ATTR char filenameCSV[25]=""; //para construir el archivo csv base cada vez que se prenda y guarde

/////////////////////////// SD variables y funciones
boolean sd = false;
boolean record = false;
boolean recordP = false;
boolean firstRecord = true;
void recordNewData();
void storeDataToSDCard(fs::FS &fs, const char * path, const char * message);
////////////////datos para guardarlos 
String nombreCHIP;
String versionP = "Beta V0.94Px";//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// version del programa
String dato1; //dispositivo o chipID
String dato2; //hora
String dato3; //temp interna
String dato4; //press
String dato5; //hum
String dato6; //alt
String dato7; //bat dispositivo
String dato8; //analogo RAW
String dato9; //analogo Volt
String dato10; //de min 0 a max 100
String dato11; //DS18b20
String dato12; //ppm1
String dato13; //pp2.5
String dato14; //ppm10
String dato15; //segundos durmiendo
String dato16; //veces dormido
float tempInt ;
float humInt;
float pressInt;
float altitud;

////////////////////////////////////////// DS18b20
////////// DS18b20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 12 // data de temperatura en el pin 12
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature ds18(&oneWire); //sensor sonda de temperatura
float tempC ;

///////////////////////////////// i2c
///////// i2c librarys
#include <Wire.h>
#define SCL 22
#define SDA 13
///bme 
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1016.25)
Adafruit_BME280 bme; // I2C
/////////////////////////////////pantalla
#include <U8g2lib.h>
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/////////////////////////INCLUDE PESTAÑA////////////////////////////////////
#include "keicalogo.h"    //grafica
#include "index.h"        //Web page header file
#include "conectividad.h" //incluir conectividad
#include "tonos.h"        //sonidos


//==========================//plantower=================================================
//Plantawer para pms5003ST probado con datos segun codigo hecho por pablo
//=======================================================================
#include <PMserial.h> // Arduino library for PM sensors with serial interface
const uint8_t PMS_RX = 16, PMS_TX = 17;
SerialPM pms(PMS5003, Serial2); // PMSx003, UART

String PM01Value;          //define PM1.0 value of the air detector module
String PM2_5Value;         //define PM2.5 value of the air detector module
String PM10Value;         //define PM10 value of the air detector module

//===============================================================
// Setup
//===============================================================
//////////////PINES
//33  Analog Menu BTN 
//32  Analog 0  rutina de calibracion?
//35  Analog BAT  en pin
//12   ONEWIRE DS18B20  trae resistencia 4.7Kohm
//15  jack digital
//2 jack analogo
//4 jack serial

#define Abat 35   //analogo bateria
#define anBTN 33  //analogor botones
#define Asen 32   //analogo general, sensor de 3.5 Jack
#define jDS 15    //ONEWIRE DS18B20 jack 
#define jA   2    //jack analogo
#define jS   4    //jack Serial
#define jDSV 25   //ONEWIRE DS18B20 jack voltaje 
#define jAV   26  //jack analogo  voltaje
#define jSV   27  //jack Serial voltaje NO SE USA PODRIA SACARSE UN PIN MAS
//#define buz 14    // buzzer extra

////////////// boleanos para control
boolean JC18 = true; // estructura para romper el loop
boolean medirJC = false; //si se conecta el jack medir el sensor sonda
boolean JA = true; // estructura para romper el loop en elsensor analogo
boolean medirJA = false; //si se conecta el jack medir el sensor analogo
boolean JS = true; // estructura para romper el loop en el sensor serial
boolean medirJS = false; //si se conecta el jack medir el sensor serial


////////////variables para sensores
int raw ;    //dato raw del pin analogo
float senV; // voltaje del sensor analogo
int anP;// esta variable sera el porcetaje para la funcion calibracion analoga
int minA = 0; //minimo del sensor
int maxA = 4095; //maximo para la base
int anPV; //solo para generar visual
int error;
///////////////////// bateria
float battv;
float batA = 3.30; //voltaje para generar alamra

String ip;
int menA;
int J18;   //jack a gnd del sensor ds18b
int Jan;   //jack a gnd del sensor analogo
int Jserial;
///=================================
// variables ventanas
////////// para el menu y ventanas
//==================================================================================
boolean menuDeb = true; //para mostar menu de botones
//==================================================================================
boolean menu = false;    //se genera un bool para activar o no pantalla menu
int pantallas = 1;
boolean subDeepsleep = false; //menu para configurar el deepsleep
boolean analCal = false;      //menu para calibrar el analogo
boolean ok = false;
boolean todoM = false;
int btnAmin =1060;
int btnAmax =1190;
int btnBmin =750;
int btnBmax = 850;
int btnCmin =1800;
int btnCmax =1980;
int btnACmin = 2160; //los botones de los extremos se bajo el minimo
int btnACmax = 2400; //el rango maximo para los dos botones de los extremos
int btnABC =  2450;
/////// rec
unsigned long lastmillisP2 = 0;
unsigned long previousMillisP2 = 0;       
const long intervalP2 = 1000;           // interval  (milliseconds)
/////////////////////
unsigned long lastmillisP = 0;
unsigned long previousMillisP = 0;       
const long intervalP = 2000;           // interval  (milliseconds)

int okrecord; //contador para entrar mas lento
/////////////////////// tiempo
String tiempo;
unsigned long currentMillis;
unsigned long lastmillis = 0;
unsigned long previousMillis = 0;       
const long interval = 1000;           // interval  (milliseconds)
int seg = 0; //contador para desaparecer menu
int minut = 0;
int hor = 0;
int dia = 0;

////////////////////////////////////////// web SD

const char* host = "keica"; //nombre para el .local
static bool hasSD = false;
File uploadFile;

void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile)
    return false;

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    if(SD.exists((char *)upload.filename.c_str())) SD.remove((char *)upload.filename.c_str());
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    Serial.print("Upload: START, filename: "); Serial.println(upload.filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    Serial.print("Upload: WRITE, Bytes: "); Serial.println(upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(uploadFile) uploadFile.close();
    Serial.print("Upload: END, Size: "); Serial.println(upload.totalSize);
  }
}

void deleteRecursive(String path){
  File file = SD.open((char *)path.c_str());
  if(!file.isDirectory()){
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while(true) {
    File entry = file.openNextFile();
    if (!entry) break;
    String entryPath = path + "/" +entry.name();
    if(entry.isDirectory()){
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void handleCreate(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }

  if(path.indexOf('.') > 0){
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if(file){
// TODO Create file with 0 bytes???
      file.write(NULL, 0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void printDirectory() {
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" && !SD.exists((char *)path.c_str())) return returnFail("BAD PATH");
  File dir = SD.open((char *)path.c_str());
  path = String();
  if(!dir.isDirectory()){
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();

  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry)
    break;
    String output;
    if (cnt > 0)
    output = ',';
    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    // Ignore '/' prefix
    output += entry.name()+1;
    output += "\"";
    output += "}";
    server.sendContent(output);
    entry.close();
 }
 server.sendContent("]");
 // Send zero length chunk to terminate the HTTP body
 server.sendContent("");
 dir.close();
}

void handleNotFound(){
  if(hasSD && loadFromSdCard(server.uri())) return;
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
  message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.print(message);
}
//===========================================================================================================================================================================
// Setup
//===========================================================================================================================================================================

void setup() { 

/////////////////////DS18 sonda temperatura
  ds18.begin();
  ////////////////// I2C
  Wire.begin(SDA,SCL); //esp32 lolin lite wemos 
  u8g2.begin();
  
  bool status;
 // default settings
   status = bme.begin(0x76);    //76
   if (!status) {
   Serial.println("Could not find a valid BME280 sensor, check wiring!");
   //alarmas
tone(BUZZER_PIN, NOTE_D6, 250);
delay(80);
tone(BUZZER_PIN, NOTE_F5, 150);
delay(80);
tone(BUZZER_PIN, NOTE_D5, 150); // beep( -PIN OF SPEAKER-, -THE NOTE WANTING TO BE PLAYED-, -DURATION OF THE NOTE IN MILISECONDS- )
   delay(500);
   }else{
   Serial.println("valid BME280 sensor, OK");
   tempInt   = bme.readTemperature();
   humInt    = bme.readHumidity();
   pressInt  = bme.readPressure() / 100.0F;
   altitud   = bme.readAltitude(SEALEVELPRESSURE_HPA);  
    } 
  ////////////////// plantower segundo serial
 Serial2.begin(9600);
  pms.init();                   // config serial port
/////////////////////
  Serial.begin(115200);
  Serial.println(); 
  Serial.println("Booting Keica32...");
/////////////// seteo entradas y saldoas
pinMode(jDS,INPUT_PULLUP);    //ONEWIRE DS18B20 jack  sensa si se conecto o no 
pinMode(jA,INPUT_PULLUP); //jack analogo sensa si se conecto o no
pinMode(jS,INPUT_PULLUP); //jack Serial sensa si se conecto o no 
pinMode(jDSV,OUTPUT); //jack de alimentacion entrega 3V despues de conectado
pinMode(jAV,OUTPUT); //jack de alimentacion entrega 3V despues de conectado
pinMode(jSV,OUTPUT); //jack de alimentacion entrega 3V despues de conectado
////////////////// nombre de dispositivo sacado desde el chip id
uint32_t theChipId = 0;
for(int i=0; i<17; i=i+8) {
    theChipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
Serial.printf(" ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
Serial.printf(" This chip has %d cores\n", ESP.getChipCores());
Serial.print(" ChipID: ");
Serial.print(theChipId);
// Define 
nombreCHIP = "KEICA";
nombreCHIP += String(theChipId);
Serial.print(" nuevo: ");
Serial.print(nombreCHIP);
 
 if (recordSleep == false ){
 u8g2.clearBuffer();          // clear the internal memory
 u8g2.setFont( u8g2_font_crox4hb_tr);  // choose a suitable font grande
 for (int pos = -36; pos <= 0; pos += 1) { // for para animacion de keica
   // u8g2.clearBuffer();          // clear the internal memory
    u8g2.drawXBMP(0,pos+2, 128, 35, keica_bits); //dibujo de logo
    u8g2.sendBuffer();          // transfer internal memory to the display
  }

 u8g2.setCursor(0,55);///////// versiones
 u8g2.print(versionP); ///////////////// version
 u8g2.setFont(u8g2_font_micro_tr);  // font muy chica 5px de alto
 u8g2.drawStr(0, 64, "multiples sensores con wifi");  // write something to the internal memory
 u8g2.sendBuffer();          // transfer internal memory to the display
 partida();
 delay(1000);
//================ wifi   
//este es la forma generar el wifi
// Length (with one extra character for the null terminator) este es la forma de pasar un string a char array
//para este caso es el nombre del wifi  
int str_len = nombreCHIP.length() + 1;  // esta funcion es para asignarle el valor de casillas al array
// Prepare the character array (the buffer) 
char char_array[str_len]; //este es el char array
// Copy it over 
nombreCHIP.toCharArray(char_array, str_len);
  WiFi.mode(WIFI_AP); //Access Point mode
  WiFi.softAP(char_array); //sin passs WiFi.softAP(ssid, password); 
  Serial.print(" Connecting to ");
  Serial.print(char_array);
  IPAddress IP = WiFi.softAPIP();
  IPS = String(IP);
  Serial.print(" AP IP address: ");  
  Serial.println(IP);
   Serial.println(__DATE__);
  //----------------------------------------------------------------
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(host);
    Serial.println(".local");
  }

  server.on("/list", HTTP_GET, printDirectory);
  server.on("/edit", HTTP_DELETE, handleDelete);
  server.on("/edit", HTTP_PUT, handleCreate);
  server.on("/edit", HTTP_POST, [](){ returnOK(); }, handleFileUpload);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

 }
 if(!SD.begin()){
        Serial.println("Card Mount Failed");
        delay(1000);
        return;
    }
if(SD.begin()){
        Serial.println("SD OK");
        sd = true;
        hasSD = true;
        u8g2.drawDisc(124, 60, 3, U8G2_DRAW_ALL); //dibujo de punto cuando esta gravando    
        u8g2.sendBuffer();          // transfer internal memory to the display
        delay(2000);
 ///////////////funcion crear csvs distintos segun si hay o no archivo       
 if (recordSleep == false){
 int n = 0;
 
String NP = "/";
NP = NP+nombreCHIP+"N%03d.csv"; 
Serial.println(NP)  ;
int str_lenNP = NP.length() + 1;  // esta funcion es para asignarle el valor de casillas al array
char npC[str_lenNP]; //este es el char array  
NP.toCharArray(npC, str_lenNP);  
Serial.println(npC);  
snprintf(filenameCSV, sizeof(npC), npC, n); // includes a three-digit sequence number in the file name 
Serial.println(filenameCSV);
 while(SD.exists(filenameCSV)) {
 n++;
 snprintf(filenameCSV, sizeof(npC), npC, n); // includes a
  }
 
  File file = SD.open(filenameCSV,FILE_READ); //crea el archivo
  Serial.println(n);
  Serial.println(filenameCSV);
  file.close();      
  return;
    } 
 }
 if (recordSleep == true){
 WiFi.mode(WIFI_OFF); 
 bootCount = bootCount+1; 
 Serial.println("veces booteando: ");
 Serial.println(bootCount);
 }
}

//=======================================================================================================
// Void Loop
//=======================================================================================================
void loop() {
  u8g2.setPowerSave(0);
  currentMillis = millis();
  segundos();
  u8g2.clearBuffer();          // clear the internal memory
  server.handleClient();
  menA = analogRead(anBTN); //analogo de botones escalera de resistencias
  J18 = digitalRead(jDS);   //jack a gnd del sensor ds18b
  Jan = digitalRead(jA);   //jack a gnd del sensor analogo
  Jserial = digitalRead(jS);   //jack a gnd del sensor serial
  
  battv = ((float)analogRead(Abat)/4095 * 3.3)*2*1.04; //voltaje relativo de la bateria
  int batbar =map(battv, batA, 4.2, 0,10);
  if ( battv <= batA){
  lowbat(); 
    }
 if(battv < 3){
  TIME_TO_SLEEP = 36000;
  sleepDeep();
 }
////////////////analogo
if(Jan == 1 && JA == true){
  u8g2.clearBuffer();          // clear the internal memory
  if ( recordSleep == false){
  entrada();

   u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font grande
  for (int pos = 64; pos >= 10; pos -= 1) { // for para animacion de keica
   // u8g2.clearBuffer();          // clear the internal memory
  u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
  u8g2.drawStr(20, 10, "ANALOGO");  // write something to the internal memory 
  u8g2.drawStr(20, 20, "CONECTADO");
  u8g2.sendBuffer();          // transfer internal memory to the display
    }
     }
  Serial.println("Conectado el jack analogo");
  digitalWrite(jAV,HIGH);
  JA = false;
  medirJA = true;   
  } 
  
  if(Jan == 0 && JA == false){
   salida();
   digitalWrite(jAV,LOW);
   JA = true;
   u8g2.clearBuffer();          // clear the internal memory
   Serial.println("Desconectado el jack analogo");
   for (int pos = 10; pos <= 60; pos += 1) { // for para animacion de keica
   u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font grande
   u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
   u8g2.drawStr(20, 10, "ANALOGO");  // write something to the internal memory 
   u8g2.drawStr(20, 20, "DESCONECTADO");
   u8g2.sendBuffer(); 
   medirJA = false;
   raw= 0;
   senV= 0;
    }
  } 
 ////////////////////// sonda externa jack
 if(J18 == 1 && JC18 == true){
  u8g2.clearBuffer();          // clear the internal memory
  if ( recordSleep == false){
  entrada();

  u8g2.setFont(u8g2_font_t0_11_tf);  //font grande
  for (int pos = 64; pos >= 10; pos -= 1) { // for para animacion de keica
   // u8g2.clearBuffer();          // clear the internal memory
    u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
    u8g2.drawStr(20, 10, "DS18B20");  // write something to the internal memory 
    u8g2.drawStr(20, 20, "CONECTADO");
    u8g2.sendBuffer();          // transfer internal memory to the display
      }
        }
     Serial.println("Conectado el jack de la sonda: ds18b");
   digitalWrite(jDSV,HIGH);
  JC18 = false;
   medirJC = true;   
  } 
  
  if(J18 == 0 && JC18 == false){
    salida();
    digitalWrite(jDSV,LOW);
    JC18 = true;
     u8g2.clearBuffer();          // clear the internal memory
   Serial.println("Desconectado el jack de la sonda: ds18b");
     for (int pos = 10; pos <= 60; pos += 1) { // for para animacion de keica
    u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font media
    u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
    u8g2.drawStr(20, 10, "DS18B20");  // write something to the internal memory 
    u8g2.drawStr(20, 20, "DESCONECTADO");
    u8g2.sendBuffer(); 
   medirJC = false;
   tempC = 0;
    }
  } 
//////////////// Serial
if(Jserial == 1 && JS == true){
  u8g2.clearBuffer();          // clear the internal memory
   if ( recordSleep == false){
  entrada();
   
   u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font grande
  for (int pos = 64; pos >= 10; pos -= 1) { // for para animacion de keica
  u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
  u8g2.drawStr(20, 10, "SERIAL");  // write something to the internal memory 
  u8g2.drawStr(20, 20, "CONECTADO");
  u8g2.sendBuffer();          // transfer internal memory to the display
        }
    }
     Serial.println("Conectado el jack Serial");
     digitalWrite(jSV,HIGH);
     JS = false;
     medirJS = true;
  } 
  
  if(Jserial == 0 && JS == false){
    salida();
    digitalWrite(jSV,LOW);
    JS = true;
   u8g2.clearBuffer();          // clear the internal memory
   Serial.println("Desconectado el jack analogo");
   for (int pos = 10; pos <= 60; pos += 1) { // for para animacion de keica
   u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font grande
   u8g2.drawXBMP(0,pos, 12, 50, jack3_bits); //dibujo de logo
   u8g2.drawStr(20, 10, "SERIAL");  // write something to the internal memory 
   u8g2.drawStr(20, 20, "DESCONECTADO");
   u8g2.sendBuffer(); 
   medirJS = false;
   PM01Value="0";          //define PM1.0 value of the air detector module
   PM2_5Value="0";         //define PM2.5 value of the air detector module
   PM10Value="0";         //define PM10 value of the air detector module
    }
  }   
   
  if ( medirJA == true){
  raw = analogRead(Asen);
  senV = ((float)raw/4095) * 3.3; //voltaje aproximado del sensor
  anP = map(raw, minA, maxA, 0, 100); //take the value of x, compared it to the scale of the potentiometer pMin to pMax, and translate that value to the scale of 0 to 100
  anP = constrain(anP, 0, 100);
  anPV= map(anP, 0, 100, 0, 128); //take the value of x, compared it to the scale of the potentiometer pMin to pMax, and translate that value to the scale of 0 to 100 
  Serial.print("Raw analogo: ");
  Serial.print(raw);
  Serial.print("\t");
  Serial.print("voltaje de referencia");
  Serial.print(senV);
  Serial.print("\t");
  Serial.print("Segun porcentaje sin calibrar: ");
  Serial.print(anP);
  Serial.print("\t");
  
    }    
//////////////////funciones de medicion  
  if ( medirJC == true){
      DS18();
    } 
     
   if ( medirJS == true){  
      plantower(); 
    }   
  /////////// PARA MENU CON BOTON ANALOGO  
 if( menuDeb == true){ 
  Serial.print(" btnA: ");
  Serial.print(menA);
  Serial.print(" Variables menu: ");
  Serial.print(menu); 
  Serial.print(" pantalla: ");
  Serial.print(pantallas); 
  Serial.print(" submenuSleep: ");
  Serial.print(subDeepsleep); 
  Serial.print(" submenuCAlanal: ");
  Serial.print(analCal); 
  Serial.print(" menuTodo: ");
  Serial.println(todoM); 
 } else{
//////////////////////////////////resto de datos
  Serial.print(" Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.print(" *C ");
  Serial.print(" Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.print(" hPa ");
  Serial.print(" Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.print(" m ");
  Serial.print(" Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.print("% ");
  Serial.print("\t");
 }
//////////7para las pantallas  
 static uint32_t TsensorINT;
 if (millis() - TsensorINT >= 3000){
  TsensorINT = millis();
   tempInt   = bme.readTemperature();
   humInt    = bme.readHumidity();
   pressInt  = bme.readPressure() / 100.0F;
   altitud   = bme.readAltitude(SEALEVELPRESSURE_HPA);
 }
  
////// ================================= deep sleep activado
 if (recordSleep == true){
 static uint32_t deepS;
 if (millis() - deepS >= 15000){ 
  deepS = millis();
  sleepDeep();
  } 
 } 
 if (recordSleep == false){
////// =================================== funcionamiento normal despues de medir todo 
  if (menu == false){  
  if (menA >  btnAmin && menA < btnAmax){ //Izquierdo
  static unsigned long btnI=millis();        
    if (millis() - btnI >=100) 
    {
      btnI=millis(); 
      // boton izquierdo
  pantallas = pantallas-1;
  
  if (pantallas < 1) pantallas = 5;  
    }
  }   
 if (menA >  btnCmin && menA < btnCmax){ //boton derecho
      static unsigned long btnI=millis();       //every 0.5s update the temperature and humidity from DHT11 sensor
    if (millis() - btnI >=100) 
    {
      btnI=millis();
  pantallas = pantallas+1;
  if (pantallas > 5) pantallas = 1;
    } 
    }
if (menA >  btnBmin && menA < btnBmax){//central
    
   if (currentMillis - previousMillisP >= intervalP) {
   previousMillisP = currentMillis;
    okrecord = okrecord+1;
   }
   if (okrecord == 3 && record == false && sd == true){
   // clear the internal memory
   Serial.println("SD");
   for (int pos = 64; pos >= 25; pos -= 1) { // for para animacion de keica
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_10x20_tf); 
    u8g2.drawStr(10, pos, "GUARDANDO");  // write something to the internal memory 
    u8g2.drawStr(10, pos+20, "DATOS");
    u8g2.setCursor(0, pos+35);
    u8g2.print(String(filenameCSV));
    u8g2.sendBuffer();
        }
   }
   
   if ( sd == false){
   Serial.println("SD");
   for (int pos = 64; pos >= 12; pos -= 1) { // for para animacion de keica
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_10x20_tf); // 13 px
    u8g2.drawStr(0, pos, "SIN SD ");  // write something to the internal memory 
    u8g2.drawStr(0, pos+20, "IMPOSIBLE");
    u8g2.drawStr(0, pos+40, "GUARDAR");
    u8g2.sendBuffer();
        }
   }
    if (okrecord == 3 && record == true && sd == true){
            // clear the internal memory
   Serial.println("SD");
   for (int pos = 64; pos >= 25; pos -= 1) { // for para animacion de keica
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_10x20_tf); 
    u8g2.drawStr(10, pos, "SIN GUARDAR DATOS");  // write something to the internal memory 
    u8g2.drawStr(20, pos+20, "SD LIBRE");
    u8g2.sendBuffer();
        }    
    }
    if(okrecord == 3){
    record = !record;
    okrecord = 0; 
    }
  }
 

  if (menA > btnABC && sd == true){ //los 3 botones
  recordSleep = true;
  }
  
  if (menA > btnABC && sd == false){
    for (int pos = 64; pos >= 12; pos -= 1) { // for para animacion de texto
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_10x20_tf); 
    u8g2.drawStr(0, pos, "SIN SD ");  
    u8g2.drawStr(0, pos+20, "IMPOSIBLE");
    u8g2.drawStr(0, pos+40, "GUARDAR");
    u8g2.sendBuffer();
        }
   }
       
  if (menA >btnACmin && menA< btnACmax){ 
   menu = true;
   delay(500);
   menA = 0;
   pantallas = 1;
  }
  
 if (pantallas == 1) temphum(); 
 if (pantallas == 2) presalt();
 if (pantallas == 3) sensorA(); 
 if (pantallas == 4) ds18ext();
 if (pantallas == 5) plantawer();  
 } 
   
if(menu == true){
  if (menA >  btnAmin && menA < btnAmax){ //Izquierdo  
      static unsigned long btnIm=millis();       //every 0.5s update the temperature and humidity from DHT11 sensor
    if (millis() - btnIm >=100) 
    {
      btnIm=millis();
    if (pantallas == 13) pantallas = 2;  
    if (pantallas == 14) pantallas = 2;
    if (pantallas == 15) pantallas = 2;
    if (pantallas == 16) pantallas = 17;
    pantallas = pantallas-1;
    if (pantallas < 1 && subDeepsleep == false && analCal == false) pantallas = 6;
    if (analCal == true){
    if (pantallas < 1) pantallas = 4; 
    }
    if (subDeepsleep == true){
    if (pantallas < 1) pantallas = 3; 
    }
      
    } 
  }   
  if (menA >  btnBmin && menA < btnBmax){ //boton central
      if (subDeepsleep == false && analCal == false){
 pantallas = pantallas+10;
 if(pantallas == 23) pantallas = 1;
 if(pantallas == 24) pantallas = 1;
 if(pantallas == 25) pantallas = 1; 
 if(pantallas == 26){ 
  pantallas = 16;  
 menuDeb = !menuDeb;
 }
 delay(500);
      }
////////////////////////////////////funcion guardar deepsleep      
   if (subDeepsleep == true && pantallas == 1){
   TIME_TO_SLEEP = 3600;
   okenter();
   subDeepsleep = false;
   pantallas = 1; 
    }  
     if (subDeepsleep == true && pantallas == 2){
   TIME_TO_SLEEP = 1800;
   okenter();
   subDeepsleep = false;
   pantallas = 1; 
    } 
    if (subDeepsleep == true && pantallas == 3){
   TIME_TO_SLEEP = 900; 
   okenter();
   subDeepsleep = false;
   pantallas = 1;
    } 
 if (analCal == true && pantallas == 1){
  minA = raw; 
   okenter();   
 }
   if (analCal == true && pantallas == 3){
   maxA = raw; 
   okenter();   
 }

  
  }
if (menA >  btnCmin && menA < btnCmax){ //boton derecho
      static unsigned long btnDm=millis();       //every 0.5s update the temperature and humidity from DHT11 sensor
    if (millis() - btnDm >=100) 
    {
      btnDm=millis();
    if (todoM == false){  
    if (pantallas == 16) pantallas = 15;
    pantallas = pantallas+1;
    if (subDeepsleep == false && analCal == false){
       if (pantallas > 6) pantallas = 1;
      }
    if (analCal == true){
    if (pantallas > 4) pantallas = 1; 
      }
    if (subDeepsleep == true){
       if (pantallas > 3) pantallas = 1;
      }
     }
   }  
}

if (menA >btnACmin && menA< btnACmax){ //los dos laterales
 static unsigned long btnMm=millis();       //every 0.5s update the temperature and humidity from DHT11 sensor
    if (millis() - btnMm >=300) 
  {
  if(todoM ==false){  
  if (subDeepsleep == true){
  subDeepsleep = false;
  }
  if (analCal == true){
  analCal = false ;
  }   
 
   if(subDeepsleep == false && analCal == false && pantallas <10 && todoM == false){
   menu = false;
   btnMm=millis();  
   }
 pantallas = 1;
 delay(200); 
  }
}
 
  }  
if (menA > btnABC){ //los 3 botones
 todoM= false;
 pantallas = 1;
 u8g2.clearBuffer(); 
 u8g2.setFont(u8g2_font_10x20_tf); 
 u8g2.drawStr(0, 30, "Saliendo del menu");  
 u8g2.sendBuffer();
 delay(1000); 
 menu = false;
 }

if (subDeepsleep == false){
  
if (pantallas == 1) men1();
if (pantallas == 2) men2();
if (pantallas == 3) men3(); 
if (pantallas == 4) men4();
if (pantallas == 5) men5(); 
if (pantallas == 6) men6();

if (pantallas == 11) {
  analCal = true;
  pantallas = 1;
}
if (pantallas == 12) {
  subDeepsleep = true;
  pantallas = 1;
}
if (pantallas == 13) ayuda(); 
if (pantallas == 14) menInfo();
if (pantallas == 15) nombres(); 
if (pantallas == 16){
  todo();
  todoM = true;
 }

} 

if (analCal == true){ /////////////////// menu calibracion
if( pantallas == 1)menCMIN();
if (pantallas == 2)menCMIN1();
if (pantallas == 3)menCMAX();
if (pantallas == 4)menCMAX1();  
  }
if( subDeepsleep == true){/////////////mwenu deepsleep
if( pantallas == 1)  menDS1();
if (pantallas == 2)  menDS2();
if (pantallas == 3)  menDS3();   
 } 
} 

//===================== punto de grabando
  if ( record == true && sd == true) {
   static uint32_t ToSD; //timetofill
 if (millis() - ToSD >= 15000) {
    recordNewData(); ///////////// llama a guardar cada 5 segundos como minimo
    ToSD = millis(); 
 }  
    
  if (currentMillis - previousMillisP2 >= intervalP2) {
   previousMillisP2 = currentMillis;
   recordP = !recordP;
  }
  if(recordP == true){
  u8g2.drawDisc(124, 54, 3, U8G2_DRAW_ALL); //dibujo de punto cuando esta gravando 
    }
  }
//===========================  
  ////////////////////////////////////// FOOT/HEADER
  if(menu == false){
  u8g2.setFont(u8g2_font_u8glib_4_tf); // tipo chico
  u8g2.setCursor(0, 64); ;
  u8g2.print("T:"+String(hor)+":"+String(minut)+":"+String(seg)); //display.print("millis: "+String(millis())+" time: "+String(hor)+":"+String(minut)+":"+String(seg)+"D"+String(dia))
  u8g2.setCursor(32, 64);
  u8g2.print("IP: "+ip);
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(103, 58);
  u8g2.print("Rec"); 
}
  u8g2.setFont(u8g2_font_u8glib_4_tf); // tipo chico
  u8g2.setCursor(81, 64);
  u8g2.print("Bat: "+String(battv));
  u8g2.drawFrame(116,59,11,5);
   u8g2.drawLine(127, 60, 127, 62);
  u8g2.drawBox(117,60,batbar-1,3 ); //batbar
  u8g2.sendBuffer(); // transfer internal memory to the display 
  if(recordSleep == false){
  Serial.print(" AP IP address: ");
  ip = WiFi.softAPIP().toString(); // .toString(); la forma mas facil de guardar la ip estructurado
  Serial.println(ip);
  }
 }
}
//===============================================================
// Toma de datos
//===============================================================


 void plantower()
 { 
  static uint32_t tmP;
 if (millis() - tmP >= 5000) {
    ds18.requestTemperatures(); // Send the command to get temperatures
    tmP = millis();   
 // read the PM sensor
  pms.read();
  if (pms)
  { // successfull read
 error = 0;
    Serial.print(F("PM1.0 "));
    Serial.print(pms.pm01);
   PM01Value = String(pms.pm01);
    Serial.print(F(", "));
    Serial.print(F("PM2.5 "));
    Serial.print(pms.pm25);
    PM2_5Value =String(pms.pm25);
    Serial.print(F(", "));
    Serial.print(F("PM10 "));
    Serial.print(pms.pm10);
    PM10Value = String(pms.pm10);
    Serial.println(F(" [ug/m3]"));
  }
  else
  { // something went wrong
    switch (pms.status)
    {
    case pms.OK: // should never come here
      break;     // included to compile without warnings
    case pms.ERROR_TIMEOUT:
      Serial.println(F(PMS_ERROR_TIMEOUT));
      error = 1;
      break;
    case pms.ERROR_MSG_UNKNOWN:
      Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
      error = 1;
      break;
    case pms.ERROR_MSG_HEADER:
      Serial.println(F(PMS_ERROR_MSG_HEADER));
      error = 1;
      break;
    case pms.ERROR_MSG_BODY:
      Serial.println(F(PMS_ERROR_MSG_BODY));
      error = 1;
      break;
    case pms.ERROR_MSG_START:
      Serial.println(F(PMS_ERROR_MSG_START));
      error = 1;
      break;
    case pms.ERROR_MSG_LENGTH:
      Serial.println(F(PMS_ERROR_MSG_LENGTH));
      error = 1;
      break;
    case pms.ERROR_MSG_CKSUM:
      Serial.println(F(PMS_ERROR_MSG_CKSUM));
      error = 1;
      break;
    case pms.ERROR_PMS_TYPE:
      Serial.println(F(PMS_ERROR_PMS_TYPE));
      error = 1;
      break;
    }
  }

 }
  yield(); 
 }
 

void DS18()
{   
 static uint32_t tmr;
 if (millis() - tmr >= 3000) {
    ds18.requestTemperatures(); // Send the command to get temperatures
    tmr = millis();
    tempC = ds18.getTempCByIndex(0); 
      if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print(" sonda ext: ");
    Serial.println(tempC);
  } 
  else
  {
    Serial.print (" Error sonda Ext ");
  }
  }
  Serial.print(" ds18:");
  Serial.print("\t");
  Serial.print(tempC);
  Serial.println("\t"); 
}

//===============================================================
// enbloqueado de datos para guardar el csv
// se consideran generar instancias segun que sensores esten conectados
//===============================================================
void recordNewData(){
 String message;
 ////////////////////////////////////////////////////////////////primer loop para generar header
 if (firstRecord == true && firstRecordSleep  == true  ){
 dato1 = "NOMBRE"; //dispositivo
 dato2 = "TIEMPO PRENDIDO"; //hora relativa al uso del dispositivo
 dato3 = "VOLTAJE BATERIA"; //hora relativa al uso del dispositivo 
 dato4 = "TEMPERATURA INT"; //temp interna
 dato5 ="HUMEDAD INT";     //hum interna
 dato6 = "PRESION INT";     //pression interna
 dato7 = "ALTURA";          //alt
 //////////////////////// AQUI ES BUENO GENERAR UNOS IF SEGUN LO QUE ESTE CONECTADO
 dato8 = "ANALOGO RAW";                 //analogo RAW
 dato9 = "VOLTAJE";                     //analogo Volt
 dato10 = "ANALOGO MAPEADO";            //de min 0 a max 100
 dato11 = "SONDA TEMPERATURA EXTERNA";  //DS18b20
 dato12= "MATERIAL PARTICULADO PPM1";   //ppm1
 dato13= "MATERIAL PARTICULADO PPM2.5";  //pp2.5
 dato14= "MATERIAL PARTICULADO PPM10";  //ppm10 
 dato15= "SEG. DURMIENDO";               //tiempo seño
 dato16= "VECES DORMIDO";               //veces booteado

  message = String(dato1) //nombre del dispositvo
  + "," 
  + String(dato2)  //tiempo
  + "," 
  + String(dato3)  //VOLTAJE
  + "," 
  + String(dato4)  //TEMP INT
  + "," 
  + String(dato5)  //HUM INT
  + "," 
  + String(dato6)  // PRESS INT
  + "," 
  + String(dato7)  // ALTURA
  + "," 
  + String(dato8)  //"ANALOGO RAW"
  + "," 
  + String(dato9)  //analogo Volt
  + "," 
  + String(dato10)  //de min 0 a max 100
  + "," 
  + String(dato11)  //DS18b20
  + "," 
  + String(dato12)  //ppm1
  + "," 
  + String(dato13)  //ppm10
  + "," 
  + String(dato14); //ppm2.5
  if ( recordSleep == true){
   message += "," 
  + String(dato15)  //TIEMPO DORMIDO
  + "," 
  + String(dato16); //VECES REBOOT
  }
   if( recordSleep == true && firstRecordSleep  == true){  
  firstRecordSleep  = false; 
  }
 firstRecord = false; 
 }else{

 dato1 = nombreCHIP ; //dispositivo
 dato2 = tiempo; //hora relativa al uso del dispositivo
 dato3 = String(battv); //bat dispositivo
 dato4 = bme.readTemperature(); //temp interna
 dato5 = bme.readHumidity(); //hum interna
 dato6 = bme.readPressure() / 100.0F;//pression interna
 dato7 = bme.readAltitude(SEALEVELPRESSURE_HPA); //altura en metros
 dato8 = raw ; //analogo RAW
 dato9 = senV ; //analogo Volt
 dato10 = anP; //de min 0 a max 100
 dato11 = tempC; //DS18b20
 dato12 = PM01Value; //ppm1
 dato13 = PM2_5Value; //pp2.5
 dato14 = PM10Value; //ppm10
  if (  recordSleep == true){
 dato15 = TIME_TO_SLEEP; //TIEMPO DORMIDO
 dato16 = bootCount;     //VECES REBOOT
  }
  message = String(dato1) //nombre del dispositvo
  + "," 
  + String(dato2) //tiempo
  + "," 
  + String(dato3) //VOLTAJE
  + "," 
  + String(dato4) //TEMPINT
  + "," 
  + String(dato5) //HUMINT
  + "," 
  + String(dato6) //PRESSINT
  + "," 
  + String(dato7) //ALTURA
  + "," 
  + String(dato8)  //"ANALOGO RAW"
  + "," 
  + String(dato9)  //analogo Volt
  + "," 
  + String(dato10)  //de min 0 a max 100
  + "," 
  + String(dato11)  //DS18b20
  + "," 
  + String(dato12)  //ppm1
  + "," 
  + String(dato13)  //ppm10
  + "," 
  + String(dato14); //ppm2.5
   if ( recordSleep == true){
  message += "," 
  + String(dato15) //TIEMPO DORMIDO
  + "," 
  + String(dato16); //VECES REBOOT
   } 

 }
  Serial.println(message);
  storeDataToSDCard(SD, filenameCSV, message.c_str());

}
//===============================================================
// guardando SD
//===============================================================
void storeDataToSDCard(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending data to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.println(message)) {
    Serial.println("Data appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
//===============================================================
// pantallas
//===============================================================
void todo(){   
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(0, 8);
  u8g2.print(String(bme.readTemperature()) + " \xb0""C"+" EXT: " +String(tempC));
  u8g2.setCursor(0, 18);
  u8g2.print(String(bme.readHumidity()) + " %H "+"SD: "+sd+" Rec "+String(record));
  u8g2.drawStr(0, 28, "P: ");
  u8g2.setCursor(15, 28);
  u8g2.print(String(bme.readPressure())); 
  u8g2.setCursor(70, 28);
  u8g2.print(String(bme.readAltitude(SEALEVELPRESSURE_HPA))+"MT");
  u8g2.setCursor(0, 38); 
  u8g2.print("A: "+String(raw)+" V: "+String(senV));
  u8g2.setCursor(0, 48); 
  u8g2.print("btnesXA: "+String(menA));
  u8g2.setCursor(0, 58);
  u8g2.print("P.1:"+String(PM01Value)+"P2:"+String(PM2_5Value)+"P1:"+String(PM10Value));
  u8g2.setFont(u8g2_font_u8glib_4_tf); // tipo chico
  u8g2.setCursor(0, 64); //display.print("millis: "+String(millis())+" time: "+String(hor)+":"+String(minut)+":"+String(seg)+"D"+String(dia));
  u8g2.print(String(hor)+":"+String(minut)+":"+String(seg));
  u8g2.setCursor(32, 64);
  u8g2.print("IP: "+ip);  
  }
//temperatura humedad  
void nombres(){
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_t0_11_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(40,10);  u8g2.print ("NOMBRES");
u8g2.setCursor(0, 24);
u8g2.print("NOMBRE: "+String(nombreCHIP));
u8g2.setCursor(0, 36);
u8g2.print("TIEMPO DORMIDO:" +String(TIME_TO_SLEEP) +"S" );
u8g2.setCursor(0, 48);
u8g2.print("A:" + String(filenameCSV));
u8g2.setCursor(0, 60);
u8g2.print("IP: "+ip);
}  
 
void menCMIN()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(30,10);  u8g2.print ("CAL MINIMO");
u8g2.setCursor(0,22);  u8g2.print ("ANALOGO RAW: "+String(raw));
u8g2.setCursor(0,32);  u8g2.print ("ANALOGO MINIMO: "+String(minA));
u8g2.setCursor(0,42);  u8g2.print ("ANALOGO Volt: "+String(senV));
u8g2.drawButtonUTF8(0, 52, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">GUARDAR DE NUEVO" );
u8g2.setCursor(0,64);  u8g2.print(" SIGUIENTE"); 
//minA, maxA, 
}
void menCMIN1()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(30,10);  u8g2.print ("CAL MINIMO");
u8g2.setCursor(0,22);  u8g2.print ("ANALOGO RAW: "+String(raw));
u8g2.setCursor(0,32);  u8g2.print ("ANALOGO MINIMO: "+String(minA));
u8g2.setCursor(0,42);  u8g2.print ("ANALOGO Volt: "+String(senV));
u8g2.setCursor(0,52);  u8g2.print (" GUARDAR DE NUEVO");
u8g2.drawButtonUTF8(0, 62, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">SIGUIENTE" );
}
void menCMAX()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(30,10);  u8g2.print ("CAL MAX");
u8g2.setCursor(0,22);  u8g2.print ("ANALOGO RAW: "+String(raw));
u8g2.setCursor(0,32);  u8g2.print ("ANALOGO MAX: "+String(maxA));
u8g2.setCursor(0,42);  u8g2.print ("ANALOGO Volt: "+String(senV));
u8g2.drawButtonUTF8(0, 52, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">GUARDAR DE NUEVO" );
u8g2.setCursor(0,64);  u8g2.print(" SIGUIENTE");
}
void menCMAX1()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(30,10);  u8g2.print ("CAL MAX");
u8g2.setCursor(0,22);  u8g2.print ("ANALOGO RAW: "+String(raw));
u8g2.setCursor(0,32);  u8g2.print ("ANALOGO MAX: "+String(maxA));
u8g2.setCursor(0,42);  u8g2.print ("ANALOGO Volt: "+String(senV));
u8g2.setCursor(0,52);  u8g2.print (" GUARDAR DE NUEVO");
u8g2.drawButtonUTF8(0, 62, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">SIGUIENTE" );
}

 
void menDS1()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(20,10);  u8g2.print ("TIEMPO DORMIDO");
u8g2.drawButtonUTF8(0, 22, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">1 HORA = 3600s" );
u8g2.setCursor(0,34);  u8g2.print (" 1/2 HORA = 1800s");
u8g2.setCursor(0,44);  u8g2.print (" 15 MIN = 900s");
u8g2.setCursor(0,54);  u8g2.print ("BTN CENTRAL PARA");
u8g2.setCursor(0,64);  u8g2.print("GUARDAR SELECCION"); 
}

void menDS2()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_6x10_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(20,10);  u8g2.print ("TIEMPO DORMIDO");
u8g2.setCursor(0,22);  u8g2.print (" 1 HORA = 3600s");
u8g2.drawButtonUTF8(0, 32, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">1/2 HORA = 1800s" );
u8g2.setCursor(0,44);  u8g2.print (" 15 MIN = 900s");
u8g2.setCursor(0,54);  u8g2.print ("BTN CENTRAL PARA");
u8g2.setCursor(0,64);  u8g2.print("GUARDAR SELECCION");
}

void menDS3()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(20,10);  u8g2.print ("TIEMPO DORMIDO");
  u8g2.setCursor(0,22);  u8g2.print (" 1 HORA = 3600s");
  u8g2.setCursor(0,32);  u8g2.print (" 1/2 HORA");
  u8g2.drawButtonUTF8(0, 42, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">15 MIN = 900s" );
  u8g2.setCursor(0,54);  u8g2.print ("BTN CENTRAL PARA");
  u8g2.setCursor(0,64);  u8g2.print("GUARDAR SELECCION"); 
}


void men1()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_t0_11_tf);
//u8g2.drawButtonUTF8(60, 9,U8G2_BTN_HCENTER| U8G2_BTN_INV, 128,  1,  1, "MENU" );
 u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
 u8g2.setCursor(50,10);  u8g2.print ("MENU");
 u8g2.drawButtonUTF8(0, 24, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">CALIBRACION" );
 u8g2.setCursor(0,38);  u8g2.print (" DEEP SLEEP");
 u8g2.setCursor(0,48);  u8g2.print (" AYUDA");
 u8g2.setCursor(0,58);  u8g2.print (" PROJECT INFO");
 
}

void men2()
{
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_t0_11_tf);
u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
u8g2.setCursor(50,10);  u8g2.print ("MENU");
u8g2.setCursor(0,24);  u8g2.print (" CALIBRACION");
u8g2.drawButtonUTF8(0, 36, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">DEEP SLEEP" );
u8g2.setCursor(0,48);  u8g2.print (" AYUDA");
u8g2.setCursor(0,58);  u8g2.print (" PROJECT INFO");
}

void men3()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(50,10);  u8g2.print ("MENU");
  u8g2.setCursor(0,24);  u8g2.print (" DEEP SLEEP");
  u8g2.drawButtonUTF8(0, 36, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">AYUDA" );
  u8g2.setCursor(0,48);  u8g2.print (" PROJECT INFO");
  u8g2.setCursor(0,58);  u8g2.print (" NOMBRES");
}

void men4()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(50,10);  u8g2.print ("MENU");
  u8g2.setCursor(0,24);  u8g2.print (" AYUDA");
  u8g2.drawButtonUTF8(0, 36, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">PROJECT INFO" );
  u8g2.setCursor(0,48);  u8g2.print (" NOMBRES");
  u8g2.setCursor(0,58);  u8g2.print (" DEBUG");
}

void men5()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(50,10);  u8g2.print ("MENU");
  u8g2.setCursor(0,24);  u8g2.print (" AYUDA");
  u8g2.setCursor(0,34);  u8g2.print (" PROJECT INFO");
  u8g2.drawButtonUTF8(0, 46, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">NOMBRES" );
  u8g2.setCursor(0,58);  u8g2.print (" DEBUG");
 // u8g2.drawButtonUTF8(0, 56, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">DEBUG" );
}

void men6()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(50,10);  u8g2.print ("MENU");
  u8g2.setCursor(0,24);  u8g2.print (" AYUDA");
  u8g2.setCursor(0,34);  u8g2.print (" PROJECT INFO");
  u8g2.setCursor(0,44);  u8g2.print (" NOMBRES");
  u8g2.drawButtonUTF8(0, 56, U8G2_BTN_INV, u8g2.getDisplayWidth()-5*2,  10,  1, ">DEBUG" );
}

void ayuda()
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(50,10);  u8g2.print ("AYUDA");
  u8g2.setCursor(0,22);  u8g2.print ("Visite la web:");
  u8g2.setCursor(0,32);  u8g2.print ("https://www.keica.cl/");
  u8g2.setCursor(0,43);  u8g2.print ("192.168.4.1/test.pdf");
  u8g2.setCursor(0,52);  u8g2.print("revise manual en SD");
  u8g2.setCursor(0,62);  u8g2.print("arebolledo@udd.cl");
}

void menInfo() ////////// SOBRE EL PROGRAMA
{
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),12); //drawFrame 
  u8g2.setCursor(30,10);  u8g2.print ("INFO: KEICA");
  u8g2.setCursor(0,22);  u8g2.print ("Fecha: "+String(__DATE__));
  u8g2.setCursor(0,32);  u8g2.print ("Equipo C+");
  u8g2.setCursor(0,42);  u8g2.print ("Firmware por: A.R.");
  u8g2.setCursor(0,52);  u8g2.print("Ver: "+versionP);
  u8g2.setCursor(0,62);  u8g2.print("HECHO EN CHILE");
 static uint32_t tmrINFO;
 if (millis() - tmrINFO >= 30000) {
  tmrINFO = millis(); 
  for (int pos = 20; pos >= -130; pos -= 1) { // for para animacion de keica
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_10x20_tf); 
    u8g2.drawStr(10, pos, "Equipo C+");  // write something to the internal memory 
    u8g2.setFont(u8g2_font_t0_11_tf);
    u8g2.drawStr(0, pos+15, "Gonzalo Anais");
    u8g2.setCursor(0, pos+30);
    u8g2.print("Andres Cepeda");
    u8g2.setCursor(0, pos+45);
    u8g2.print("Francisco Fuentes");
    u8g2.setCursor(0, pos+60);
    u8g2.print("Martin Gaete");
    u8g2.setCursor(0, pos+75);
    u8g2.print("Alejandro Rebolledo");
    u8g2.setCursor(0, pos+90);
    u8g2.print("Felipe Roa");
    u8g2.setFont(u8g2_font_10x20_tf); 
    u8g2.setCursor(20, pos+130);
    u8g2.print("GRACIAS!");
    u8g2.sendBuffer();
        }
       
   }
}



//temperatura humedad  
void temphum(){
   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_10x20_tf);  
   u8g2.setCursor(0, 14);
   u8g2.print("TEMP/HUM");
   u8g2.setCursor(0, 30);
   u8g2.print(String(tempInt) + " \xb0""C");
   u8g2.setCursor(0, 46);
   u8g2.print(String(humInt) + " %H ");
}
//presion altura
void presalt(){
u8g2.clearBuffer();          // clear the internal memory
u8g2.setFont(u8g2_font_10x20_tf);  
u8g2.setCursor(0, 14);
u8g2.print("PRES/ALT");
u8g2.setCursor(0, 30);
u8g2.print(String(pressInt) + "Pasc");
u8g2.setCursor(0, 46);
u8g2.print(String(altitud)+" MT");
}
//sonda externa temperatura
void ds18ext(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_10x20_tf); 
  u8g2.setCursor(0, 14);
  u8g2.print("SONDA TEMP"); 
  if( medirJC == false){
  u8g2.drawButtonUTF8(0, 38, U8G2_BTN_INV, u8g2.getDisplayWidth(),  2,  2, "NO CONECTADA" );  
    }else{
  u8g2.setCursor(20, 35);
  u8g2.print(String(tempC) +" \xb0""C"); 
    }
}
//Sensor Analogo
void sensorA(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_10x20_tf); 
  u8g2.setCursor(0, 13);
  u8g2.print("SONDA Analoga:"); 
  if( medirJA == false){
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawButtonUTF8(0, 38, U8G2_BTN_INV, u8g2.getDisplayWidth(),  2,  2, "NO CONECTADA" );  
    }else{
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(0, 28);
  u8g2.print("Raw: "+String(raw)+" Volts: "+String(senV));
  u8g2.drawFrame(0,32,u8g2.getDisplayWidth(),13 );
  u8g2.drawBox(1,33,anPV,11 );
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_10x20_tf); 
  u8g2.drawStr(3, 45, "-"); 
  u8g2.drawStr(115, 45, "+");
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_t0_11_tf);
  u8g2.setCursor(0, 56);
  u8g2.print(+"%: "+ String(anP));
    }
}
//material particulado
void plantawer(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_10x20_tf); 
  u8g2.setCursor(0, 13);
  u8g2.print("PPM: ");
   if( medirJS == false){
    error =0;
   u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawButtonUTF8(0, 38, U8G2_BTN_INV, u8g2.getDisplayWidth(),  2,  2, "NO CONECTADA" );  
    }
    else if( error == 1){
   u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawButtonUTF8(0, 38, U8G2_BTN_INV, u8g2.getDisplayWidth(),  2,  2, "ERROR" );  
    }else{
  u8g2.setCursor(0, 28);
  u8g2.print("PPM1: "+String(PM01Value));
  u8g2.setCursor(0, 43);
  u8g2.print("PM2.5: "+String(PM2_5Value));
  u8g2.setCursor(0, 58);
  u8g2.print("PM10 :"+String(PM10Value));
    }
}

//===============================================================
// alrma bateria baja
//===============================================================
void lowbat(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_17b_tf);
  u8g2.setCursor(0, 13);
  u8g2.print("ADVERTENCIA");
  u8g2.setCursor(0, 28);
  u8g2.print("BATERIA BAJA");
  u8g2.setCursor(0, 43);
  u8g2.print("CARGUE PARA");
  u8g2.setCursor(0, 56);
  u8g2.print("EVITAR FALLA"); 
  //alarmas
tone(BUZZER_PIN, NOTE_D6, 250);
delay(80);
tone(BUZZER_PIN, NOTE_F5, 150);
delay(80);
tone(BUZZER_PIN, NOTE_D5, 150); // beep( -PIN OF SPEAKER-, -THE NOTE WANTING TO BE PLAYED-, -DURATION OF THE NOTE IN MILISECONDS- )
u8g2.sendBuffer();
delay(500);
}

//===============================================================
// tiempo en millis
//===============================================================
void segundos(){
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    seg++;
    if(seg == 60){
      seg = 0;
      minut++;
    }
    if(minut == 60){
      minut = 0;
      hor++;
    }
    if(hor == 12){
      hor = 0;
      dia++;
    }   
  }  
 tiempo = String(dia) +":" + String(hor) +":" +String(minut) +":" +String(seg);
 //Serial.println(tiempo); 
  }

//===============================================================
// DEEP SLEEP
//===============================================================
void sleepDeep(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  u8g2.clearBuffer();  
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds"); 
  u8g2.setPowerSave(0);
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),u8g2.getDisplayHeight() );
  u8g2.setFont(u8g2_font_10x20_tf); 
  u8g2.drawStr(5, 18, "SUSPENCION");
  u8g2.setCursor(5,38);
  u8g2.print(String(TIME_TO_SLEEP)+" SEG."); 
  u8g2.setCursor(5,58);
  u8g2.print("BOOTEADO: "+String(bootCount));
  Serial.println("Going to sleep now");
  u8g2.sendBuffer();          // transfer internal memory to the display
  recordNewData();
  delay(5000); //Take some time 
  u8g2.setPowerSave(1);
  Serial.flush(); 
  esp_deep_sleep_start();   
  }
