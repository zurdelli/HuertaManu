AsyncWebServer server(80);

void InitServer()
{
	server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

	
    // server.on("/LED", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, setData);

	server.on("/save", HTTP_POST,[](AsyncWebServerRequest * request){}, NULL, setData);
	server.on("/rele", HTTP_POST, toggleBombaManual);
	server.on("/info", HTTP_POST, getData);

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(400, "text/plain", "Not found");
	});

	server.begin();
  	Serial.println("HTTP server started");
}
