#define BLYNK_TEMPLATE_ID "TMPL6zxYPaA2_"
#define BLYNK_TEMPLATE_NAME "IoT HydroSense"
#define BLYNK_AUTH_TOKEN "KIwLz4rKtJsNyS7YcgXD7f8ruRh7tnIV"

// Enable debug print
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "WIFI SSID";
const char* pass = "WIFI PASSWORD";

BlynkTimer timer;

// Pin configuration
namespace pin {
  const byte tds_sensor = 34;
  const byte relay = 26;
}

// reference constant
namespace device {
  float aref = 3.3;
}

// sensor data
namespace sensor {
  float ec = 0;
  unsigned int tds = 0;
  float ecCalibration = 1.0;
}

// pump control threshold
const float TDS_THRESHOLD = 400.0;
const float EC_THRESHOLD = 1.0;

// sensor reading and pump control functions
void readTdsAndControlPump() {
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 4095.0;

  float offset = 0.14;
  sensor::ec = (rawEc * sensor::ecCalibration) - offset;
  if (sensor::ec < 0) sensor::ec = 0;

  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * pow(sensor::ec, 2) + 857.39 * sensor::ec) * 0.5;

  Serial.print("TDS: ");
  Serial.println(sensor::tds);
  Serial.print("EC: ");
  Serial.println(sensor::ec, 2);

  // Kirim ke Blynk
  Blynk.virtualWrite(V0, sensor::tds);
  Blynk.virtualWrite(V1, sensor::ec);

  // Kontrol pompa (relay aktif LOW)
  if (sensor::tds > TDS_THRESHOLD || sensor::ec > EC_THRESHOLD) {
    digitalWrite(pin::relay, LOW); // relay active
    Serial.println("Pompa AKTIF - Menurunkan kadar TDS/EC");
  } else {
    digitalWrite(pin::relay, HIGH); // relay off
    Serial.println("Pompa NONAKTIF - Air dalam kondisi aman");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TDS:");
  lcd.print(sensor::tds);
  lcd.setCursor(0, 1);
  lcd.print("EC:");
  lcd.print(sensor::ec, 2);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai sistem...");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("HydroSense Init");
  delay(1500);
  lcd.clear();

  // Siapkan pin relay dan matikan dulu (HIGH = OFF karena relay aktif LOW)
  pinMode(pin::relay, OUTPUT);
  digitalWrite(pin::relay, HIGH); // Hindari nyala spontan saat boot
  
  // Scan WiFi (opsional debugging)
  Serial.println("Scan jaringan WiFi:");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");
  }

  // Debug koneksi manual
  WiFi.begin(ssid, pass);
  Serial.print("Menghubungkan ke WiFi");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Tersambung!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal tersambung ke WiFi!");
  }

  // Mulai koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  // Timer baca sensor tiap 2 detik
  timer.setInterval(2000L, readTdsAndControlPump);
}

void loop() {
  Blynk.run();
  timer.run();
}
