/*Biblioteca do Arduino*/
#include <Arduino.h>

/*Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
TaskHandle_t taskContHandle = NULL;

/*mapeamento de pinos*/
#define botao1 23  //Botões de interrupção
#define botao2 22
#define LED_1 4
#define LED_2 2
#define LED_TIME_1 500
#define LED_TIME_2 300

int contadores[4] = { 0, 0, 0, 0 };  // Vetor para armazenar contadores

int lastButtonState1 = LOW;
int buttonState1;
unsigned long lastDebounceTime1 = 0;
unsigned long debounceDelay1 = 10;

int lastButtonState2 = LOW;
int buttonState2;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 10;

/*
contador[0] e [1] = contadores das tarefas
contador[2] = contador de cliques do botão 1
contador[3] = contador de cliques do botão 2
*/

// Tarefa do contador geral
void TASK_Cont(void *pvParameters) {
  int count = 0;
  while (1) {

    if (count == 19) {
      Serial.println("Contador Geral: " + String(contadores[0]));
      contadores[0]++;
      contadores[1]++;
      count = 0;
      //conta até completar 1 segundo
    }
    // Verifica o botão 1
    int leitura1 = digitalRead(botao1);
    if (leitura1 != lastButtonState1) {

      lastDebounceTime1 = millis();
    }
    if ((millis() - lastDebounceTime1) > debounceDelay1) {

      if (leitura1 != buttonState1) {


        buttonState1 = leitura1;

        if (buttonState1 == LOW) {
          contadores[2]++;
          Serial.println("Botão 1 pressionado: " + String(contadores[2]) + " vezes");
        }
      }
    }

    lastButtonState1 = leitura1;

    if (contadores[2] == 10) {
      Serial.println("Suspender a Task do LED Verde (Task 1)...");
      vTaskSuspend(task1Handle);
      vTaskDelay(pdMS_TO_TICKS(20000));  // Espera 20 segundos
      Serial.println("Reiniciar a Task do LED Verde (Task 1)...");
      vTaskResume(task1Handle);
      contadores[2] = 0;  // Reinicia o contador de cliques do botão 1
    }
    // Verifica o botão 2

    int leitura2 = digitalRead(botao2);
    if (leitura2 != lastButtonState2) {

      lastDebounceTime2 = millis();
    }
    if ((millis() - lastDebounceTime2) > debounceDelay2) {

      if (leitura2 != buttonState2) {


        buttonState2 = leitura2;

        if (buttonState2 == LOW) {
          contadores[3]++;
          Serial.println("Botão 2 pressionado: " + String(contadores[3]) + " vezes");
        }
      }
    }

    lastButtonState2 = leitura2;
    if (contadores[3] == 10) {
      Serial.println("Suspender a Task do LED Vermelho (Task 2)...");
      vTaskSuspend(task2Handle);
      vTaskDelay(pdMS_TO_TICKS(20000));  // Espera 20 segundos
      contadores[0] = 0;  // Reinicia o contador geral
      contadores[3] = 0;
      count = 0;
      Serial.println("Reiniciar Contador Geral...");
      vTaskResume(task2Handle);
    }

    vTaskDelay(pdMS_TO_TICKS(50));  // Delay de 1/20 segundo
    count++;
  }
}

// Tarefa para controlar o LED 1 (LED Verde)
void vTask1(void *pvParameters) {
  pinMode(LED_1, OUTPUT);
  int TIME = (int)pvParameters;
  while (1) {

    digitalWrite(LED_1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(TIME));  // Pisca a cada 500ms
    digitalWrite(LED_1, LOW);
    vTaskDelay(pdMS_TO_TICKS(TIME));
  }
}

// Tarefa para controlar o LED 2 (LED Vermelho)
void vTask2(void *pvParameters) {

  pinMode(LED_2, OUTPUT);
  int TIME = (int)pvParameters;
  while (1) {
    digitalWrite(LED_2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(TIME));  // Pisca a cada 300ms
    digitalWrite(LED_2, LOW);
    vTaskDelay(pdMS_TO_TICKS(TIME));
  }
}
void setup() {
  Serial.begin(9600);

  pinMode(botao1, INPUT_PULLUP);  // Configura os botões como entradas com pull-up
  pinMode(botao2, INPUT_PULLUP);

  // Criação das tarefas
  xTaskCreatePinnedToCore(vTask1, "Task LED 1", configMINIMAL_STACK_SIZE, (void*)LED_TIME_1, 1, &task1Handle, 0);  // Tarefa LED Verde
  xTaskCreatePinnedToCore(vTask2, "Task LED 2", configMINIMAL_STACK_SIZE, (void*)LED_TIME_2, 1, &task2Handle, 0);  // Tarefa LED Vermelho
  xTaskCreatePinnedToCore(TASK_Cont, "Task Contador", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &taskContHandle, 1);    // Tarefa do Contador
}

void loop() {
}
