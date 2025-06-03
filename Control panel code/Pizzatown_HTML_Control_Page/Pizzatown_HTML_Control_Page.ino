#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// WiFi AP credentials
const char* ssid = "PizzatownNet";
const char* password = "Pizza1234";

AsyncWebServer server(80);

// Buffer values
int Buf_Route = 0;
int Buf_1e_Station = 0;
int Buf_2e_Station = 0;
int Buf_3e_Station = 0;

// Route connector logic
void ConnectRoute() {
  if (Buf_1e_Station == 1 && Buf_2e_Station == 0 && Buf_3e_Station == 0)
    Buf_Route = 8;
  else if (Buf_1e_Station == 1 && Buf_2e_Station == 2 && Buf_3e_Station == 0)
    Buf_Route = 1;
  else if (Buf_1e_Station == 1 && Buf_2e_Station == 2 && Buf_3e_Station == 3)
    Buf_Route = 1;
  else if (Buf_1e_Station == 1 && Buf_2e_Station == 3 && Buf_3e_Station == 0)
    Buf_Route = 1;
  else if (Buf_1e_Station == 1 && Buf_2e_Station == 6 && Buf_3e_Station == 0)
    Buf_Route = 7;
  else if (Buf_1e_Station == 1 && Buf_2e_Station == 6 && Buf_3e_Station == 3)
    Buf_Route = 7;
  else if (Buf_1e_Station == 2 && Buf_2e_Station == 0 && Buf_3e_Station == 0)
    Buf_Route = 4;
  else if (Buf_1e_Station == 2 && Buf_2e_Station == 3 && Buf_3e_Station == 0)
    Buf_Route = 4;
  else if (Buf_1e_Station == 3 && Buf_2e_Station == 0 && Buf_3e_Station == 0)
    Buf_Route = 2;
  else if (Buf_1e_Station == 6 && Buf_2e_Station == 0 && Buf_3e_Station == 0)
    Buf_Route = 2;
  else if (Buf_1e_Station == 6 && Buf_2e_Station == 3 && Buf_3e_Station == 0)
    Buf_Route = 2;
}

// HTML page with dropdowns
String getHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Pizzatown Route Planner</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
</head>
<body>
  <h2>Pizzatown Bestemming Planner</h2>
  <form action='/submit' method='get'>
    <label for='station1'>Eerste station:</label>
    <select id='station1' name='station1' onchange='updateStation2()'>
      <option value='0'>-- Kies --</option>
      <option value='1'>Station 1</option>
      <option value='2'>Station 2</option>
      <option value='3'>Station 3</option>
      <option value='6'>Station 4</option>
    </select><br><br>

    <label for='station2'>Tweede station:</label>
    <select id='station2' name='station2' onchange='updateStation3()'>
      <option value='0'>-- Kies --</option>
    </select><br><br>

    <label for='station3'>Derde station:</label>
    <select id='station3' name='station3'>
      <option value='0'>-- Kies --</option>
    </select><br><br>

    <input type='submit' value='Verzend Route'>
  </form>

<script>
const routes = {
  "1": {
    "0": ["0"],
    "2": ["0", "3"],
    "3": ["0"],
    "6": ["0", "3"]
  },
  "2": {
    "0": ["0"],
    "3": ["0"]
  },
  "3": {
    "0": ["0"]
  },
  "6": {
    "0": ["0"],
    "3": ["0"]
  }
};

function updateStation2() {
  const s1 = document.getElementById("station1").value;
  const station2 = document.getElementById("station2");
  const station3 = document.getElementById("station3");
  station2.innerHTML = "<option value='0'>-- Kies --</option>";
  station3.innerHTML = "<option value='0'>-- Kies --</option>";
  if (routes[s1]) {
    for (let s2 in routes[s1]) {
      if (s2 !== "0") station2.innerHTML += `<option value='${s2}'>Station ${s2}</option>`;
    }
  }
}

function updateStation3() {
  const s1 = document.getElementById("station1").value;
  const s2 = document.getElementById("station2").value;
  const station3 = document.getElementById("station3");
  station3.innerHTML = "<option value='0'>-- Kies --</option>";
  if (routes[s1] && routes[s1][s2]) {
    routes[s1][s2].forEach(s3 => {
      if (s3 !== "0") station3.innerHTML += `<option value='${s3}'>Station ${s3}</option>`;
    });
  }
}
</script>

</body>
</html>
)rawliteral";
  return html;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.print("ESP32 AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Serve HTML form
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getHTML());
  });

  // Handle form submission
  server.on("/submit", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("station1"))
      Buf_1e_Station = request->getParam("station1")->value().toInt();
    if (request->hasParam("station2"))
      Buf_2e_Station = request->getParam("station2")->value().toInt();
    if (request->hasParam("station3"))
      Buf_3e_Station = request->getParam("station3")->value().toInt();

    ConnectRoute();

    Serial.println("-------------Nieuwe Route Ontvangen-------------");
    Serial.print("Eerste station: "); Serial.println(Buf_1e_Station);
    Serial.print("Tweede station: "); Serial.println(Buf_2e_Station);
    Serial.print("Derde station: "); Serial.println(Buf_3e_Station);
    Serial.print("Route ID: "); Serial.println(Buf_Route);
    Serial.println("------------------------------------------------");

    request->send(200, "text/html", "<html><body><h3>Route ontvangen!</h3><a href='/'>Terug</a></body></html>");
  });

  server.begin();
}

void loop() {
  // Nothing needed here
}