#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

char buf[50];
int idx = 0;
int tam = 0;

char maiusculas[50];
char minusculas[50];
char numeros[50];

SemaphoreHandle_t mutex;
volatile bool flagImprimir = false;

void leituraTask(void *arg) {
  while (1) {
    while (Serial.available() > 0) {  // Verifica se há dados na serial
      char c = Serial.read();         // Lê o caractere da serial
      buf[idx] = c;                   // Adiciona o caractere ao buffer
      idx++;                          // Incrementa o índice

      if (c == '\n') {                // Quando pressionar Enter
        buf[idx - 1] = '\0';          // Insere o terminador de string
        tam = idx;                    // Armazena o tamanho dos dados
        idx = 0;                      // Reseta o índice para a próxima entrada
        flagImprimir = true;          // Define a flag para impressão
        Serial.println("Dados recebidos para processamento.");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void separarTask(void *arg) {
  while (1) {
    if (flagImprimir && xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {  // Aguarda o mutex
      // Limpa as listas para novo processamento
      memset(maiusculas, 0, sizeof(maiusculas));
      memset(minusculas, 0, sizeof(minusculas));
      memset(numeros, 0, sizeof(numeros));

      int mIdx = 0, minIdx = 0, numIdx = 0;

      // Separa os caracteres
      for (int i = 0; i < tam; i++) {
        if (buf[i] >= 'A' && buf[i] <= 'Z') {
          maiusculas[mIdx++] = buf[i];
        } else if (buf[i] >= 'a' && buf[i] <= 'z') {
          minusculas[minIdx++] = buf[i];
        } else if (buf[i] >= '0' && buf[i] <= '9') {
          numeros[numIdx++] = buf[i];
        }
      }
      xSemaphoreGive(mutex);  // Libera o mutex
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Atraso para evitar sobrecarga
  }
}

void ordenarTask(void *arg) {
  while (1) {
    if (flagImprimir && xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {  // Aguarda o mutex
      // Ordena maiúsculas
      for (int i = 0; i < strlen(maiusculas) - 1; i++) {
        for (int j = i + 1; j < strlen(maiusculas); j++) {
          if (maiusculas[i] > maiusculas[j]) {
            char temp = maiusculas[i];
            maiusculas[i] = maiusculas[j];
            maiusculas[j] = temp;
          }
        }
      }
      // Ordena minúsculas
      for (int i = 0; i < strlen(minusculas) - 1; i++) {
        for (int j = i + 1; j < strlen(minusculas); j++) {
          if (minusculas[i] > minusculas[j]) {
            char temp = minusculas[i];
            minusculas[i] = minusculas[j];
            minusculas[j] = temp;
          }
        }
      }
      // Ordena números
      for (int i = 0; i < strlen(numeros) - 1; i++) {
        for (int j = i + 1; j < strlen(numeros); j++) {
          if (numeros[i] > numeros[j]) {
            char temp = numeros[i];
            numeros[i] = numeros[j];
            numeros[j] = temp;
          }
        }
      }
      xSemaphoreGive(mutex);  // Libera o mutex após ordenar
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void imprimirTask(void *arg) {
  while (1) {
    if (flagImprimir && xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {  // Aguarda o mutex
      Serial.println("Listas ordenadas:");
      Serial.print("Maiúsculas: ");
      Serial.println(maiusculas);
      Serial.print("Minúsculas: ");
      Serial.println(minusculas);
      Serial.print("Números: ");
      Serial.println(numeros);
      flagImprimir = false;  // Reseta a flag após a impressão
      xSemaphoreGive(mutex);  // Libera o mutex após imprimir
    }
    vTaskDelay(pdMS_TO_TICKS(500));  // Atraso para evitar sobrecarga
  }
}

void setup() {
  Serial.begin(115200);

  // Cria o mutex
  mutex = xSemaphoreCreateMutex();

  // Criação das tarefas
  xTaskCreate(leituraTask, "Ler serial", 2048, NULL, 1, NULL);
  xTaskCreate(separarTask, "Separar caracteres", 2048, NULL, 1, NULL);
  xTaskCreate(ordenarTask, "Ordenar caracteres", 2048, NULL, 1, NULL);
  xTaskCreate(imprimirTask, "Imprimir listas", 2048, NULL, 1, NULL);
}

void loop() {
}
