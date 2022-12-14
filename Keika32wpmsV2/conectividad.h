////////////////////////////////
WebServer server(80);
String IPS;
//Enter your SSID and PASSWORD
//const char* ssid = "TestK32";
//const char* password = "20112011";

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleADC() {
  int a = analogRead(32);
  String adcValue = String(a);
  String adcValue2 = String(a);
  String adcValue3 = String(a);
  server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}
