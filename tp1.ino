#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DHT.h>

// Definir el tamaño del display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Crear una instancia del display
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

const int DHT_PIN = GPIO_NUM_33; // Pin de datos del sensor DHT
const int PIN_ADC = GPIO_NUM_32; // Potenciómetro
const int PIN_ENC_PUSH = GPIO_NUM_19; // Pulsador
//const int LED_BUILTIN = 2; // LED Azul interno (integrado en la placa)
const int RELAY_PIN = GPIO_NUM_32; // Pin de control del relé
const int PIN_LED_R = GPIO_NUM_23; // LED Verde

bool estadoLED = false; // Estado del LED Azul
int contadorPulsaciones = 0; // Contador de pulsaciones
volatile int estadoPote;

bool estadoBotonAnterior = HIGH; // Estado previo del pulsador
unsigned long ultimoTiempoPulsacion = 0; // Para el debouncing
const unsigned long tiempoDebounce = 50; // Tiempo de debounce en milisegundos

// Inicializar el sensor DHT22
DHT dht(DHT_PIN, DHT22);

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ADC, INPUT);
  pinMode(PIN_ENC_PUSH, INPUT_PULLUP); // Pullup pone un cero lógico cuando se aprieta el pulsador
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Configura el pin como salida
  digitalWrite(LED_BUILTIN, estadoLED);

  // Inicializar pantalla OLED
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.display(); // Llama a display.display() para mostrar lo que se haya dibujado.

  // Inicializar sensor DHT22
  dht.begin();
}

void loop() {
  // Leer el valor del potenciómetro
  int valorPote = analogRead(PIN_ADC);
  
  // Mapear el valor del potenciómetro (0-4095) al rango de salida del LED (0-255)
  int valorLED = map(valorPote, 0, 4095, 0, 255);
  
  // Calcular el porcentaje para mostrar en el display
  int porcentajePote = map(valorPote, 0, 4095, 0, 100);

  // Activar o desactivar el relé según el valor del potenciómetro
  if (porcentajePote >= 50) {
    digitalWrite(RELAY_PIN, HIGH); // Encender el relé
    digitalWrite(PIN_LED_R, HIGH); // Encender el LED verde
  } else {
    digitalWrite(RELAY_PIN, LOW); // Apagar el relé
    digitalWrite(PIN_LED_R, LOW); // Apagar el LED verde
  }

  // Leer el estado actual del pulsador
  bool estadoBotonActual = digitalRead(PIN_ENC_PUSH);

  // Verificar si se ha presionado el pulsador y si ha pasado suficiente tiempo para evitar rebotes
  if (estadoBotonAnterior == HIGH && estadoBotonActual == LOW && (millis() - ultimoTiempoPulsacion) > tiempoDebounce) {
    // Cambiar el estado del LED Azul
    estadoLED = !estadoLED;
    digitalWrite(LED_BUILTIN, estadoLED); // Cambiar el estado del LED_BUILTIN

    // Incrementar el contador
    contadorPulsaciones++;

    // Imprimir por el puerto serial
    Serial.println("Pulsador presionado");
    Serial.printf("Contador de pulsaciones: %d\n", contadorPulsaciones);

    // Registrar el tiempo de la pulsación
    ultimoTiempoPulsacion = millis();
  }

  // Leer la temperatura y la humedad del sensor DHT22
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // Verificar si la lectura es válida
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer del sensor DHT22");
    temperatura = 0.0;
    humedad = 0.0;
  }

  // Mostrar los datos en el display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Pulsador: %d\n", contadorPulsaciones);
  
  display.print("LED Azul: ");
  display.println(estadoLED ? "Encendido" : "Apagado");
  
  display.printf("Led Verde: %d%%\n", porcentajePote);

  // Mostrar temperatura y humedad
  display.printf("Temp: %.1f C\nHum: %.1f%%", temperatura, humedad);
  display.display();

  // Actualizar el estado del botón anterior
  estadoBotonAnterior = estadoBotonActual;
}
