#include <Arduino.h>
#include <WiFi.h>
#include <esp32_smartdisplay.h>
#include <lvgl.h>

const char* ssid = "Livebox6-1461";             //Variables del SSID y contraseña de mi Wifi
const char* password = "726fQ6z3HXnP";

lv_obj_t *btn_mensaje, *btn_qr, *qr_code, *btn_volver;          //Objetos de los botones
unsigned long lv_last_tick = 0;

void enviar_hola_mundo(lv_event_t * e);             //Creacion de funciones
void mostrar_qr(lv_event_t * e);
void ocultar_qr(lv_event_t * e);
void mostrar_botones();

void conectar_wifi() {
    Serial.println("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 15) {                    //Comprobamos que conecta al wifi con los métodos que trae Wifi.h
        delay(1000);
        Serial.print(".");
        intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConexión establecida.");                  //Si conecta lo indica por el puerto Serial
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nNo se pudo conectar a WiFi.");
    }
}

void enviar_hola_mundo(lv_event_t * e) {
    Serial.println("Hola Mundo");           //Imprime hola mundo en el puerto Serial
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());         //Indica la IP
}

void mostrar_qr(lv_event_t * e) {               // pantalla de mostrar QR
    lv_obj_del(btn_mensaje);                    // borro los botones para dejar limpia la interfaz
    lv_obj_del(btn_qr);                         // Y asi puedo ver el QR y el boton de volver     || Basicamente trampeo el codigo para parecer que paso a una nueva pantalla
    qr_code = lv_qrcode_create(lv_scr_act());              //Creacion de QR
    lv_qrcode_set_size(qr_code, 120);                      //Tamaño de QR
    lv_qrcode_set_dark_color(qr_code, lv_color_hex(0x000000));                                  // color negro || Colores de QR para que sea en blanco y negro, los colores son RGB a hexadecimal, por eso el 0x
    lv_qrcode_set_light_color(qr_code, lv_color_hex(0xFFFFFF));                                // color blanco
    String mac = WiFi.macAddress();                                                             // Obtiene la MAC como String
    lv_qrcode_update(qr_code, mac.c_str(), mac.length());                                       // Convierte a const char* y actualiza el QR

    lv_obj_align(qr_code, LV_ALIGN_CENTER, 0, -20);                                     // Pongo el qr en el centro de la pantalla, hay que tener en cuenta que las dimensiones del reloj son 240*240 y la pantalla es redonda y LVGL pinta en un cuadrado
    
    btn_volver = lv_btn_create(lv_scr_act());                                           //Boton de volver
    lv_obj_align(btn_volver, LV_ALIGN_CENTER, 0, 90);                                   //Lo pongo a la par que el QR en el centro , por obvias razones se situa debajo del QR
    lv_obj_t *label = lv_label_create(btn_volver);  
    lv_label_set_text(label, "Volver");                                             //texto
    lv_obj_add_event_cb(btn_volver, ocultar_qr, LV_EVENT_CLICKED, NULL);                // metodo para volver
}

void ocultar_qr(lv_event_t * e) {                   //Metodo para borrar QR y boton de volver, para estar en la "pantalla incial"
    lv_obj_del(qr_code);
    lv_obj_del(btn_volver);
    mostrar_botones();                          // Muestra los botones iniciales nuevamente
}

void mostrar_botones() {                                //BOTONES INCIALES
    btn_mensaje = lv_btn_create(lv_scr_act());      
    lv_obj_align(btn_mensaje, LV_ALIGN_CENTER, 0, -30);                                     // el -30 desplaza el boton  
    lv_obj_t *label1 = lv_label_create(btn_mensaje);
    lv_label_set_text(label1, "Mensaje");                                               //texto
    lv_obj_add_event_cb(btn_mensaje, enviar_hola_mundo, LV_EVENT_CLICKED, NULL);                //Al pulsar el boton "Mensaje", llama a la funcion enviar_hola_mundo
    
    btn_qr = lv_btn_create(lv_scr_act());
    lv_obj_align(btn_qr, LV_ALIGN_CENTER, 0, 30);                                 // el 30 desplaza el boton  
    lv_obj_t *label2 = lv_label_create(btn_qr);
    lv_label_set_text(label2, "Mostrar QR");                                        //texto
    lv_obj_add_event_cb(btn_qr, mostrar_qr, LV_EVENT_CLICKED, NULL);                //Al pulsar el boton "Mostrar QR", llama a la funcion mostrar_qr y trampea la pantalla
}

void setup() {
    Serial.begin(115200);                   //inicia la comunicacion del Serial con baudios
    smartdisplay_init();                    // enciende la pantalla del esp23
    lv_init();                              // inicia lvgl
    conectar_wifi();                             // inicia wifi
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xADD8E6), LV_PART_MAIN);              //Color de fondo de la pantalla, un azul clarito
    mostrar_botones();                  //Muestra los botones inciales de " Mensaje " y " Mostrar Qr"
}

void loop() {
    unsigned long now = millis();           // tiempo (hora) en milisegundos
    lv_tick_inc(now - lv_last_tick);
    lv_last_tick = now;
    lv_timer_handler();                     //Actualiza la  interfaz con lvgl 
}   