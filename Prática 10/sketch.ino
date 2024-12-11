#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

// Definição dos pinos
const int btnPin = 15;     // Pino do botão
const int potPin = 34;     // Pino do potenciômetro
const int led1Primary = 2; // LED motor principal conjunto 1
const int led1Secondary = 4; // LED motor secundário conjunto 1
const int led2Primary = 5; // LED motor principal conjunto 2
const int led2Secondary = 18; // LED motor secundário conjunto 2

#define MOTOR_RUNNING_BIT  (1 << 0) // Bit para indicar que um motor está ligado
#define MOTOR_IDLE_BIT     (1 << 1) // Bit para indicar que os motores estão desligados

QueueHandle_t potentiometerQueue;
SemaphoreHandle_t buttonSemaphore;
EventGroupHandle_t motorEventGroup;

bool systemOn = false; // Estado do sistema
int activeSet = 1;     // Conjunto ativo (1 ou 2)
unsigned long lastSwitchTime = 0; // Para controlar a alternância de 10 segundos

void setup() {
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(led1Primary, OUTPUT);
  pinMode(led1Secondary, OUTPUT);
  pinMode(led2Primary, OUTPUT);
  pinMode(led2Secondary, OUTPUT);

  Serial.begin(115200);
  Serial.println("Inicializando sistema...");

  buttonSemaphore = xSemaphoreCreateBinary();
  if (buttonSemaphore == NULL) {
    Serial.println("Erro ao criar o semáforo.");
    while (true);
  }

  motorEventGroup = xEventGroupCreate();
  if (motorEventGroup == NULL) {
    Serial.println("Erro ao criar o grupo de eventos.");
    while (true);
  }

  xTaskCreate(monitorButtonTask, "MonitorButtonTask", 2048, NULL, 1, NULL);
  xTaskCreate(motorControlTask, "MotorControlTask", 2048, NULL, 1, NULL);
}

void loop() {
}

// Task para monitorar o botão
void monitorButtonTask(void *pvParameters) {
  bool lastButtonState = HIGH;
  while (true) {
    bool currentButtonState = digitalRead(btnPin);
    if (currentButtonState == LOW && lastButtonState == HIGH) {
      Serial.println("Botão pressionado!");
      xSemaphoreGive(buttonSemaphore);
    }
    lastButtonState = currentButtonState;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// Task para controlar os motores
void motorControlTask(void *pvParameters) {
  int level = 0;
  while (true) {

    // Se o botão foi pressionado
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY)) {
      systemOn = !systemOn;
      Serial.println(systemOn ? "Sistema ligado" : "Sistema desligado");
      if (!systemOn) {
        digitalWrite(led1Primary, LOW);
        digitalWrite(led1Secondary, LOW);
        digitalWrite(led2Primary, LOW);
        digitalWrite(led2Secondary, LOW);
        xEventGroupClearBits(motorEventGroup, MOTOR_RUNNING_BIT);
        continue;
      }
    }

    // Se o sistema está ligado
    if (systemOn) {
      // Leitura contínua do nível do reservatório
      level = analogRead(potPin);
      level = map(level, 0, 4095, 0, 100); // Mapear valor para 0-100%
      Serial.print("Nível do reservatório: ");
      Serial.print(level);
      Serial.println("%");

      unsigned long currentTime = millis();
      
      if (level < 50) {
        Serial.println("Nível abaixo de 50%. Acionando ambos os conjuntos com 5 segundos de diferença.");

        // Acionar o conjunto 1
        if ((xEventGroupGetBits(motorEventGroup) & MOTOR_RUNNING_BIT) == 0) {
          Serial.println("Ativando conjunto 1.");
          digitalWrite(led1Primary, HIGH);
          digitalWrite(led1Secondary, HIGH);
          xEventGroupSetBits(motorEventGroup, MOTOR_RUNNING_BIT);
          vTaskDelay(pdMS_TO_TICKS(5000));  // Delay de 5 segundos antes de acionar o próximo conjunto

          // Acionar o conjunto 2
          Serial.println("Ativando conjunto 2.");
          digitalWrite(led2Primary, HIGH);
          digitalWrite(led2Secondary, HIGH);
        }
      } else {
        Serial.println("Nível acima de 50%. Acionando conjuntos alternadamente a cada 10 segundos.");

        if (currentTime - lastSwitchTime >= 10000) {
          // Alternar o conjunto
          if (activeSet == 1) {
            Serial.println("Ativando conjunto 1.");
            digitalWrite(led1Primary, HIGH);
            digitalWrite(led1Secondary, HIGH);
            digitalWrite(led2Primary, LOW);
            digitalWrite(led2Secondary, LOW);
            activeSet = 2;
          } else {
            Serial.println("Ativando conjunto 2.");
            digitalWrite(led2Primary, HIGH);
            digitalWrite(led2Secondary, HIGH);
            digitalWrite(led1Primary, LOW);
            digitalWrite(led1Secondary, LOW);
            activeSet = 1;
          }
          lastSwitchTime = currentTime; // Atualiza o tempo da última alternância
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
