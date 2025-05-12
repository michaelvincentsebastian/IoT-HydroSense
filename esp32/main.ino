// Blynk IoT Template Info
#define BLYNK_TEMPLATE_NAME "Smart Water Monitor"
#define BLYNK_TEMPLATE_ID "TMPL6Ja84eLVE"
#define BLYNK_AUTH_TOKEN "yxaEKUJImvxZ9m_xCsGAPW4h7ljuZUb1"

// Enable debug print
#define BLYNK_PRINT Serial

// Include libraries
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "BAHAGIA";
char pass[] = "sehat5758";

// Timer instance
BlynkTimer timer;

// Namespace for hardware pins
namespace pin {
    const byte tds_sensor = 34;
}

// Namespace for device settings
namespace device {
    float aref = 3.3; // ESP32 voltage reference (3.3V)
}

// Namespace for sensor data
namespace sensor {
    float ec = 0;
    unsigned int tds = 0;
    float ecCalibration = 1.0;
}

// Fungsi pembacaan data TDS dan EC
void readTdsQuick() {
    float rawEc = analogRead(pin::tds_sensor) * device::aref / 4095.0;

    Serial.print(F("Raw Analog: "));
    Serial.println(rawEc);

    float offset = 0.14;
    sensor::ec = (rawEc * sensor::ecCalibration) - offset;
    if (sensor::ec < 0) sensor::ec = 0;

    sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * pow(sensor::ec, 2) + 857.39 * sensor::ec) * 0.5;

    Serial.print(F("TDS: "));
    Serial.println(sensor::tds);
    Serial.print(F("EC: "));
    Serial.println(sensor::ec, 2);

    // Kirim ke Blynk Virtual Pins
    Blynk.virtualWrite(V0, sensor::tds);
    Blynk.virtualWrite(V1, sensor::ec);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Koneksi WiFi manual
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    int wifiTimeout = 10000; // 10 detik timeout
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
        // Bisa reboot atau masuk mode fallback
        return;
    }

    // Koneksi ke Blynk
    Blynk.config(BLYNK_AUTH_TOKEN);
    if (!Blynk.connect(5000)) {
        Serial.println("Blynk connection failed.");
        // Tambahkan retry atau fallback logic jika perlu
    }

    // Jadwalkan pembacaan sensor setiap 2 detik
    timer.setInterval(2000L, readTdsQuick);
}

void loop() {
    Blynk.run();
    timer.run();
}
