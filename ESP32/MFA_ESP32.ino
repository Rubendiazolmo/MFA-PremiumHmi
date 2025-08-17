#include <WiFi.h>
#include <time.h>
#include <TOTP.h>
#include <ModbusIP_ESP8266.h>  // Librería para Modbus TCP en ESP32/ESP8266

#define LED_BUILTIN 2   // En la mayoría de ESP32 el LED azul está en GPIO 2

// Clave TOTP decodificada desde "KVQFQJWK33OQBIJE3ZE3XKZLW4DYVZRZ"
uint8_t hmacKey[] = { 
  0x55, 0x60, 0x58, 0x26, 0xCA, 0xDE, 0xDD, 0x00, 
  0xA1, 0x24, 0xDE, 0x49, 0xBB, 0xAB, 0x2B, 0xB7, 
  0x07, 0x8A, 0xE6, 0x39
};
int keyLength = sizeof(hmacKey);

TOTP totp = TOTP(hmacKey, keyLength);

// Datos de conexión Wi-Fi
const char* ssid     = "";
const char* password = "";

// Configuración de red estática
IPAddress local_IP(192, 168, 1, 240);   // IP fija que quieres asignar
IPAddress gateway(192, 168, 1, 1);      // Puerta de enlace
IPAddress subnet(255, 255, 255, 0);     // Máscara de subred
IPAddress primaryDNS(8, 8, 8, 8);       // DNS primario (Google)
IPAddress secondaryDNS(8, 8, 4, 4);     // DNS secundario (Google)

// Instancia del servidor Modbus
ModbusIP mb;  

// Dirección de holding register (ejemplo)
const int REG_HOLDING_ALIVE   = 0;  // Registro de sistema en marcha (0 = Sin comunicación con HMI, 1 = Comunicación establecida)
const int REG_HOLDING_STEP    = 1;  // Registro del paso actual

const int REG_HOLDING_TRIGGER = 10; // Registro de trigger               (0 = En espera,       1 = Valida código)
const int REG_HOLDING_RSP     = 11; // Registro de respuesta             (0 = No ejectuado,    1 = Código aceptado, 2 = Código incorrecto)
const int REG_HOLDING_CODE    = 12; // Registro de entrada del código    (Escrito externamente)

const int DELAY_MS            = 500; // Delay en ms que define ciclo de scan
const int DELAY_COM_DOWN_SEC  = 10;  // Delay en segundos para definir fallo de comunicación
const int ITER_FALLO_COM      = (DELAY_COM_DOWN_SEC * (1000 / DELAY_MS)); // Iteraciones para fallo comunicación

int CntFallosCom = 0;

void ParpadeaLedFalloCom(){

    // Hago parapadear el LED mientras está esperando comunicación con la HMI
    digitalWrite(LED_BUILTIN, HIGH);  // Enciende el LED
    delay(500);                       // Espera 500 ms
    digitalWrite(LED_BUILTIN, LOW);   // Apaga el LED
    delay(500);                       // Espera 500 ms

}

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Conectando a WiFi...");
  // Configurar IP fija antes de iniciar la conexión
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("⚠️ Error al configurar la IP estática");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Iniciar servidor Modbus TCP
  mb.server();

  // Crear holding registe
  for (int i = 0; i < 21; i++) {
    mb.addHreg(i, 0);   // Crea registros 0...20
  }

  mb.Hreg(REG_HOLDING_ALIVE, 0);

  pinMode(LED_BUILTIN, OUTPUT);  // Configura el pin como salida

  // Espero a que haya comunicación con la HMI
  while (mb.Hreg(REG_HOLDING_ALIVE) == 0) {

    mb.task();

    uint16_t dummy = mb.Hreg(REG_HOLDING_ALIVE);

    // Hago parapadear el LED mientras está esperando comunicación con la HMI
    ParpadeaLedFalloCom();
  }
}

void loop() {

  // Actualizar servidor Modbus
  mb.task();

  // *******************************************
  // Fallo Comunicación con HMI
  // *******************************************

  if (mb.Hreg(REG_HOLDING_ALIVE) == 0) {
    CntFallosCom++;
  } else {
    CntFallosCom = 0;
  }

  bool FalloCom = CntFallosCom >= ITER_FALLO_COM;

  if (FalloCom) {
    ParpadeaLedFalloCom();
  }

  // *******************************************
  // Reinicializar en cada iteración
  // *******************************************
  mb.Hreg(REG_HOLDING_ALIVE, 0);                // Reseteo bit de vida

  // Obtener steps desde Modbus, lo hago así para no tener que estar sincronizando relojes, doy por hecho que la HMI tiene la hora bien.
  uint32_t inputRegStep = (mb.Hreg(REG_HOLDING_STEP) << 16) | mb.Hreg(REG_HOLDING_STEP + 1);
  uint32_t steps          = inputRegStep;
  
  String code       = totp.getCodeFromSteps(steps);       // Generar código TOTP
  String code_last  = totp.getCodeFromSteps(steps - 1);   // Generar código TOTP anterior
  String code_next  = totp.getCodeFromSteps(steps + 1);   // Generar código TOTP siguiente

  uint16_t trigger = mb.Hreg(REG_HOLDING_TRIGGER);

  if (trigger == 1) {

    Serial.println("Validando código...");

    uint32_t inputRegCode = (mb.Hreg(REG_HOLDING_CODE) << 16) | mb.Hreg(REG_HOLDING_CODE + 1);
    String inputCode = String(inputRegCode);

    mb.Hreg(REG_HOLDING_RSP, 0);

    // Comparar con el código esperado
    if (inputCode.equals(code) || inputCode.equals(code_last) || inputCode.equals(code_next)) {
      mb.Hreg(REG_HOLDING_RSP, 1);
      Serial.println("Código correcto");
    } else {
      mb.Hreg(REG_HOLDING_RSP, 2);
      Serial.println("Código incorrecto");
    }

    mb.Hreg(REG_HOLDING_TRIGGER, 0);   // Reseteo trigger

    mb.task();
 
  }

  delay(DELAY_MS);
}
