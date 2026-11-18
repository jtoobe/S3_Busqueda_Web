// ===============================================================
// VERSIÓN 2: Búsqueda Web con Serper.dev (Educativa y Simplificada)
// Objetivo: Entender el uso de Headers (Encabezados) en HTTP 
//           y el parseo de resultados orgánicos de Google.
// ===============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 1. CONFIGURACIÓN DE RED (Cámbialos por los tuyos)
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD_WIFI";


// 2. CONFIGURACIÓN DE LA API
// Obtén tu clave gratis en https://serper.dev
const char* serperApiKey = "PEGA_AQUI_TU_API_KEY_DE_SERPER";


// ===============================================================
// FUNCIÓN AUXILIAR: Escapar caracteres para JSON
// (Idéntica a la versión 1, sigue siendo necesaria)
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
// FUNCIÓN PRINCIPAL: Buscar en Serper
// Devuelve un String con los resultados o un mensaje de error.
// ===============================================================
String buscarEnSerper(String query) {
  Serial.println("[PASO 1] Verificando conexión Wi-Fi...");
  if (WiFi.status() != WL_CONNECTED) {
    return "Error: No hay conexión Wi-Fi.";
  }

  Serial.println("[PASO 2] Configurando conexión segura (HTTPS)...");
  WiFiClientSecure client;
  client.setInsecure(); // Omite validación de certificados para prototipos
  
  HTTPClient http;
  http.setTimeout(10000); // 10 segundos de tiempo límite
  
  // Endpoint de Serper
  http.begin(client, "https://google.serper.dev/search");
  
  Serial.println("[PASO 3] Configurando Encabezados (Headers)...");
  http.addHeader("Content-Type", "application/json");
  // DIFERENCIA CLAVE: Serper usa un Header especial para la clave, no el cuerpo JSON
  http.addHeader("X-API-KEY", serperApiKey); 

  Serial.println("[PASO 4] Construyendo el cuerpo de la petición JSON...");
  // El cuerpo es mucho más simple que en Tavily, solo necesita la consulta "q"
  String body = "{\"q\":\"" + jsonEscape(query) + "\"}";

  Serial.println("[PASO 5] Enviando petición a Serper...");
  int httpCode = http.POST(body);

  String resultadoFinal = "";

  if (httpCode == 200) { // 200 significa "Éxito"
    Serial.println("[PASO 6] Petición exitosa. Recibiendo datos...");
    String payload = http.getString(); // JSON crudo
    
    DynamicJsonDocument doc(8192); // 8KB de memoria para el JSON
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("Error al leer el JSON: ");
      Serial.println(error.c_str());
      return "Error al procesar la respuesta.";
    }

    Serial.println("[PASO 7] Extrayendo resultados orgánicos del JSON...");
    resultadoFinal += "RESULTADOS DE BÚSQUEDA (Google vía Serper):\n\n";

    // Serper devuelve los resultados principales en un array llamado "organic"
    JsonArray organic = doc["organic"].as<JsonArray>();
    
    // Limitamos a 3 resultados para mantener la salida limpia y ahorrar créditos
    int maxResultados = min(3, (int)organic.size());
    
    for (int i = 0; i < maxResultados; i++) {
      JsonObject r = organic[i];
      String title   = r["title"] | "Sin título";
      String link    = r["link"] | "Sin enlace";
      String snippet = r["snippet"] | "Sin descripción";

      resultadoFinal += " " + title + "\n";
      resultadoFinal += "   " + snippet + "\n";
      resultadoFinal += "   " + link + "\n\n";
    }

  } else {
    Serial.printf("[ERROR] La petición falló. Código HTTP: %d\n", httpCode);
    if (httpCode == 401) {
      resultadoFinal = "Error 401: API Key inválida o no configurada.";
    } else if (httpCode == 403) {
      resultadoFinal = "Error 403: Te quedaste sin créditos gratuitos en Serper.";
    } else {
      resultadoFinal = "Error en la búsqueda. Código: " + String(httpCode);
    }
  }

  http.end(); // Liberamos recursos de red
  return resultadoFinal;
}

// ===============================================================
// CONFIGURACIÓN INICIAL (Se ejecuta una vez)
// ===============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- INICIANDO ESP32-S3 (Versión Serper Educativa) ---");

  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ¡Conectado! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println(" No se pudo conectar al Wi-Fi. Reinicia el dispositivo.");
  }

// Hacemos una sola búsqueda de prueba
  String miConsulta = "ESP32-S3 especificaciones técnicas principales";
  
  Serial.println("\n==================================================");
  Serial.println(" CONSULTA: " + miConsulta);
  Serial.println("==================================================");
  
  String resultado = buscarEnSerper(miConsulta);
  
  // Imprimimos el resultado limpio en el Monitor Serie
  Serial.println(resultado);
  
  Serial.println("==================================================");
  Serial.println("Fin de la prueba. El programa se detiene aquí.");
  

}

void loop() {
  // ===============================================================
// BUCLE PRINCIPAL (tu codigo aqui)
// ===============================================================
 
}
