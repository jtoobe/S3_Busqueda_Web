// ===============================================================
// VERSIÓN 1: Búsqueda Web con Tavily (Educativa y Simplificada)
// Objetivo: Entender cómo pedir datos a una API y leer el JSON.
//   https://www.tavily.com/
// ===============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 1. CONFIGURACIÓN DE RED (Cámbialos por los tuyos)
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD_WIFI";


// 2. CONFIGURACIÓN DE LA API
const char *tavilyApiKey = "Tu clave API de Tavily";

// ===============================================================
// FUNCIÓN AUXILIAR: Escapar caracteres para JSON
// Evita que comillas o saltos de línea rompan el formato JSON.
// ===============================================================
String jsonEscape(String str) {
  str.replace("\\", "\\\\");
  str.replace("\"", "\\\"");
  str.replace("\n", "\\n");
  str.replace("\r", "\\r");
  str.replace("\t", "\\t");
  return str;
}

// ===============================================================
// FUNCIÓN PRINCIPAL: Buscar en Tavily
// Devuelve un String con la respuesta o un mensaje de error.
// ===============================================================
String buscarEnTavily(String query) {
  Serial.println("[PASO 1] Verificando conexión Wi-Fi...");
  if (WiFi.status() != WL_CONNECTED) {
    return "Error: No hay conexión Wi-Fi.";
  }

  Serial.println("[PASO 2] Configurando conexión segura (HTTPS)...");
  WiFiClientSecure client;
  client.setInsecure();  // Omite validación de certificados (ideal para prototipos educativos)

  HTTPClient http;
  http.setTimeout(10000);  // 10 segundos de tiempo límite
  http.begin(client, "https://api.tavily.com/search");
  http.addHeader("Content-Type", "application/json");

  Serial.println("[PASO 3] Construyendo el cuerpo de la petición JSON...");
  String body = "{\"api_key\":\"" + String(tavilyApiKey) + "\","
                                                           "\"query\":\""
                + jsonEscape(query) + "\","
                                      "\"search_depth\":\"basic\","
                                      "\"max_results\":3,"         // Solo 3 resultados para ahorrar datos
                                      "\"include_answer\":true}";  // Pedimos que la IA de Tavily resuma la respuesta

  Serial.println("[PASO 4] Enviando petición a Tavily...");
  int httpCode = http.POST(body);

  String resultadoFinal = "";

  if (httpCode == 200) {  // 200 significa "Éxito"
    Serial.println("[PASO 5] Petición exitosa. Recibiendo datos...");
    String payload = http.getString();  // Aquí está el JSON crudo

    // Reservamos 8KB de memoria para el documento JSON (el ESP32-S3 tiene de sobra)
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("Error al leer el JSON: ");
      Serial.println(error.c_str());
      return "Error al procesar la respuesta.";
    }

    Serial.println("[PASO 6] Extrayendo información del JSON...");
    // Extraemos el resumen general que genera Tavily
    String answer = doc["answer"] | "Sin resumen automático.";
    resultadoFinal += "RESPUESTA DIRECTA:\n" + answer + "\n\n";
    resultadoFinal += "FUENTES:\n";

    // Recorremos el array de "results"
    JsonArray results = doc["results"].as<JsonArray>();
    for (JsonObject r : results) {
      String title = r["title"] | "Sin título";
      String url = r["url"] | "Sin URL";
      String content = r["content"] | "Sin contenido";

      resultadoFinal += "• " + title + "\n";
      resultadoFinal += "  " + content.substring(0, 150) + "...\n";  // Solo los primeros 150 caracteres
      resultadoFinal += "  " + url + "\n\n";
    }

  } else {
    Serial.printf("[ERROR] La petición falló. Código HTTP: %d\n", httpCode);
    resultadoFinal = "Error en la búsqueda. Código: " + String(httpCode);
  }

  http.end();  // Liberamos recursos de red
  return resultadoFinal;
}

// ===============================================================
// CONFIGURACIÓN INICIAL (Se ejecuta una vez)
// ===============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- INICIANDO ESP32-S3 (Versión Educativa) ---");

  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Esperamos hasta conectar (máximo 10 segundos)
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("¡Conectado! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("No se pudo conectar al Wi-Fi. Reinicia el dispositivo.");
  }

  // Hacemos una sola búsqueda de prueba
  String miConsulta = "¿Qué es un ESP32-S3 y cuáles son sus ventajas?";

  Serial.println("\n==================================================");
  Serial.println("🔍 CONSULTA: " + miConsulta);
  Serial.println("==================================================");

  String resultado = buscarEnTavily(miConsulta);

  // Imprimimos el resultado limpio en el Monitor Serie
  Serial.println(resultado);

  Serial.println("==================================================");
  Serial.println("Fin");
}


void loop() {
// ===============================================================
// BUCLE PRINCIPAL Tu codigo aqui
// ===============================================================

}
