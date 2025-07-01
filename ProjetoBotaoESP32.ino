#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>

// Wi-Fi
#define WIFI_SSID "your_wifi"
#define WIFI_PASSWORD "your_wifipassword"

// CallMeBot (WhatsApp)
const char* whatsapp_api_key = "your_apikey";
const char* whatsapp_phone = "receive_phone";
const char* whatsapp_msg = "EMERGENCIA!+Ajuda+necessaria!";

// Twilio (Chamada)
const char* account_sid = "your_sid";
const char* auth_token = "your_authtoken";
const char* from_number = "caller_phone";
const char* to_number = "calling_phone";
const char* twiml_url = "your_url.com";

// GPIOs
const int botaoPin = 27;  // Botão NF (HIGH quando pressionado)
const int ledPin = 26;    // LED
const int buzzerPin = 25; // Buzzer (controle de base do transistor)

bool acionado = false;

void setup() {
  Serial.begin(115200);

  pinMode(botaoPin, INPUT_PULLUP);  // Configura o botão com pull-up interno
  pinMode(ledPin, OUTPUT);          // Configura o pino do LED como saída
  pinMode(buzzerPin, OUTPUT);       // Configura o pino do buzzer como saída

  digitalWrite(ledPin, LOW);       // Inicia o LED apagado
  digitalWrite(buzzerPin, LOW);    // Inicia o buzzer desligado

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado!");
}

void loop() {
  int estadoBotao = digitalRead(botaoPin);

  // Se o botão for pressionado (HIGH) e ainda não foi acionado
  if (estadoBotao == HIGH && !acionado) {
    acionado = true;  // Marca que a emergência foi acionada
    Serial.println("🔔 Emergência detectada!");

    // Liga LED e buzzer por 3 segundos (piscando)
     for (int i = 0; i < 6; i++) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(250);  // ligado por 250 ms

      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, LOW);
      delay(250);  // desligado por 250 ms
    }
    // Envia a mensagem via WhatsApp e faz a chamada
    enviarMensagemWhatsApp();
    fazerChamada();
  }
  // Se o botão for solto (LOW), reseta a flag para permitir nova execução
  if (estadoBotao == LOW && acionado) {
    acionado = false;  // Reseta a flag de acionamento
  }

  delay(10);  // Debounce do botão para evitar múltiplas leituras
}

void enviarMensagemWhatsApp() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.callmebot.com/whatsapp.php?phone=";
    url += whatsapp_phone;
    url += "&text=" + String(whatsapp_msg);
    url += "&apikey=" + String(whatsapp_api_key);

    Serial.println("📨 Enviando mensagem via CallMeBot...");
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) {
      Serial.println("✅ Mensagem enviada com sucesso via WhatsApp!");
    } else {
      Serial.printf("❌ Erro ao enviar WhatsApp. Código HTTP: %d\n", httpCode);
      Serial.println(http.getString());
    }

    http.end();
  } else {
    Serial.println("❌ WiFi não conectado.");
  }
}

void fazerChamada() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(account_sid) + "/Calls.json";
    String postData = "To=" + String(to_number) + "&From=" + String(from_number) + "&Url=" + String(twiml_url);
    String auth = String(account_sid) + ":" + String(auth_token);
    String encodedAuth = base64::encode(auth);

    Serial.println("📞 Fazendo chamada via Twilio...");
    http.begin(url);
    http.addHeader("Authorization", "Basic " + encodedAuth);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);

    if (httpCode == 201) {
      Serial.println("✅ Chamada iniciada com sucesso via Twilio!");
    } else {
      Serial.printf("❌ Erro ao iniciar chamada. Código HTTP: %d\n", httpCode);
      Serial.println(http.getString());
    }

    http.end();
  } else {
    Serial.println("❌ WiFi não conectado.");
  }
}
