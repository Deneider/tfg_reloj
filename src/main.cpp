#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp32_smartdisplay.h>
#include <lvgl.h>

// Declaración de funciones antes de usarlas
void mostrar_botones();
void ocultar_qr(lv_event_t * e);
void consultar_cliente();
void mostrar_mensaje(const String& mensaje);
void mostrar_qr(lv_event_t * e);
void conectar_wifi();

// Variables globales
//Movistar_7E3E
//cS4rE2LoxWUnJZ274jn8
//const char* ssid = "Livebox6-1461";
//const char* password = "726fQ6z3HXnP";

const char* ssid = "GalaxyA12CE0B";
const char* password = "gazpaxonvtl2023";

const char* api_url_base = "http://obkserver.duckdns.org:8000/apiDesubicados/obtener_cliente_por_mac/";  // Sustituye TU_IP_LOCAL por la IP real del backend

lv_obj_t *btn_qr, *qr_code, *btn_volver, *label_info;

unsigned long lv_last_tick = 0;
bool cliente_vinculado = false;
String nombre_anterior = "";
int puntos_anterior = -1;  // Valor inicial que nunca sería válido

void conectar_wifi() {
    Serial.println("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 15) {
        delay(1000);
        Serial.print(".");
        intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConexión establecida.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nNo se pudo conectar a WiFi.");
    }
}

void mostrar_mensaje(const String& mensaje) {
    if (label_info) {
        lv_label_set_text(label_info, mensaje.c_str());
    } else {
        label_info = lv_label_create(lv_scr_act());
        lv_obj_set_width(label_info, 200);  // Limita el ancho del texto
        lv_label_set_long_mode(label_info, LV_LABEL_LONG_WRAP);  // Permite el ajuste de línea si es necesario
        lv_label_set_text(label_info, mensaje.c_str());
        
        // Alineación centrada tanto en X como en Y
        lv_obj_align(label_info, LV_ALIGN_CENTER, 30, -20);  // Centra el texto
    }
}

void consultar_cliente() {
    if (WiFi.status() != WL_CONNECTED) {
        mostrar_mensaje("WiFi no conectado");
        return;
    }

    String mac = WiFi.macAddress();  // La MAC viene con dos puntos
    
    String url = api_url_base + mac;
    Serial.print("Consultando API: ");
    Serial.println(url);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) {
        String respuesta = http.getString();
        Serial.println("Respuesta: " + respuesta);

        // Extraer nombre y puntos de la respuesta
        String nombre = respuesta.substring(respuesta.indexOf("nombre") + 9);
        nombre = nombre.substring(0, nombre.indexOf("\""));
        String puntos = respuesta.substring(respuesta.indexOf("puntos") + 8);
        puntos = puntos.substring(0, puntos.indexOf("}"));
        
        // Convertir puntos a entero
        int puntos_actuales = puntos.toInt();

        // Verificar si los puntos han bajado
        String mensaje;
        if (nombre != "" && puntos_actuales >= 0) {
            if (nombre != nombre_anterior) {
                mensaje = "Nombre: " + nombre + "\nPuntos: " + puntos;
                nombre_anterior = nombre;
                puntos_anterior = puntos_actuales;
            } else if (puntos_actuales < puntos_anterior) {
                mensaje = "Nombre: " + nombre + "\nPuntos: " + String(puntos_actuales);
            } else {
                mensaje = "Nombre: " + nombre + "\nPuntos: " + String(puntos_actuales);
            }
            cliente_vinculado = true;
        } else {
            mensaje = "Reloj no vinculado";
            cliente_vinculado = false;
        }
        
        mostrar_mensaje(mensaje);
    } else {
        Serial.println("Cliente no vinculado aún. HTTP code: " + String(httpCode));
        mostrar_mensaje("Reloj no vinculado");
    }

    http.end();
}

void mostrar_qr(lv_event_t * e) {               // pantalla de mostrar QR
    lv_obj_del(btn_qr);                         // borro el botón de mostrar QR para evitar duplicados
    qr_code = lv_qrcode_create(lv_scr_act());   // Creación de QR
    lv_qrcode_set_size(qr_code, 120);           // Tamaño de QR
    lv_qrcode_set_dark_color(qr_code, lv_color_hex(0x000000)); // color negro
    lv_qrcode_set_light_color(qr_code, lv_color_hex(0xFFFFFF)); // color blanco
    String mac = WiFi.macAddress();             // Obtiene la MAC como String
    lv_qrcode_update(qr_code, mac.c_str(), mac.length()); // Actualiza el QR

    lv_obj_align(qr_code, LV_ALIGN_CENTER, 0, -20); // Pone el QR en el centro de la pantalla
    
    btn_volver = lv_btn_create(lv_scr_act());  // Botón de volver
    lv_obj_align(btn_volver, LV_ALIGN_CENTER, 0, 90); // Lo coloca debajo del QR
    lv_obj_t *label = lv_label_create(btn_volver);  
    lv_label_set_text(label, "Volver");  // Texto
    lv_obj_add_event_cb(btn_volver, ocultar_qr, LV_EVENT_CLICKED, NULL); // Método para volver
    
    // Consultar el cliente solo cuando se muestre el QR
    consultar_cliente();
}

void ocultar_qr(lv_event_t * e) {
    lv_obj_del(qr_code);  // Elimina el QR
    lv_obj_del(btn_volver); // Elimina el botón de "Volver"
    mostrar_botones();  // Vuelve a mostrar los botones iniciales
}

void mostrar_botones() {  // Botones iniciales
    btn_qr = lv_btn_create(lv_scr_act());
    lv_obj_align(btn_qr, LV_ALIGN_CENTER, 0, 30);  // Alinea el botón
    lv_obj_t *label2 = lv_label_create(btn_qr);
    lv_label_set_text(label2, "Mostrar QR");  // Texto
    lv_obj_add_event_cb(btn_qr, mostrar_qr, LV_EVENT_CLICKED, NULL);  // Llama a mostrar_qr cuando se presiona
}

void setup() {
    Serial.begin(115200);
    smartdisplay_init();
    lv_init();
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xADD8E6), LV_PART_MAIN);
    conectar_wifi();
    mostrar_mensaje("Reloj no vinculado");  // Muestra mensaje inicial centrado
    mostrar_botones();  // Mostrar el botón "Mostrar QR"
}

void loop() {
    unsigned long ahora = millis();
    lv_tick_inc(ahora - lv_last_tick);
    lv_last_tick = ahora;
    lv_timer_handler();
}
