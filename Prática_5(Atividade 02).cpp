#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <math.h>

struct sensor {
  int deviceId;
  char measurementType;
  float value;
};

#define sensor_luz 34
#define sensor_temperatura 32

QueueHandle_t queue;

void leitura_task(void *arg) {
    int count_id = 0;
    while(1) {
        struct sensor mySensor;

        const float BETA = 3950;
        int analogValue = analogRead(sensor_temperatura);
        float celsius = 1 / (log(1 / (4096. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;

        mySensor.deviceId = count_id;
        mySensor.measurementType = 'T';
        mySensor.value = celsius;

        xQueueSend(queue, &mySensor, (TickType_t)0);

        const float GAMMA = 0.7;
        const float RL10 = 50;
        analogValue = analogRead(sensor_luz);
        float voltage = analogValue / 1024.0 * 5.0;
        float resistance = 2000 * voltage / (1 - voltage / 5);
        float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

        mySensor.deviceId = count_id;
        mySensor.measurementType = 'L';
        mySensor.value = lux;

        xQueueSend(queue, &mySensor, (TickType_t)0);

        count_id++;
        if (count_id == 10) {
            count_id = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda 2 segundos antes de ler novamente
    }
}

void controlhe_task(void *arg) {
    while(1) {
        struct sensor element;
        if (xQueueReceive(queue, &element, portMAX_DELAY) == pdTRUE) {
            Serial.print("Device ID: ");
            Serial.println(element.deviceId);

            Serial.print("Measurement type: ");
            Serial.println(element.measurementType);

            Serial.print("Value: ");
            Serial.println(element.value);

            Serial.println("----------------");
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Criação da fila
    queue = xQueueCreate(10, sizeof(struct sensor));
    if (queue == NULL) {
        Serial.println("Erro ao criar a fila");
        return;
    }

    // Criação das tarefas
    xTaskCreatePinnedToCore(
        leitura_task,             // Função da tarefa
        "Leitura dos sensores",   // Nome da tarefa
        1024,                     // Tamanho da pilha
        NULL,                     // Parâmetro da tarefa
        1,                        // Prioridade da tarefa
        NULL,                     // Identificador da tarefa
        0                         // Núcleo (0 ou 1 para ESP32)
    );

    xTaskCreatePinnedToCore(
        controlhe_task,           // Função da tarefa
        "Controle da fila",       // Nome da tarefa
        1024,                     // Tamanho da pilha
        NULL,                     // Parâmetro da tarefa
        1,                        // Prioridade da tarefa
        NULL,                     // Identificador da tarefa
        1                         // Núcleo (0 ou 1 para ESP32)
    );
}

void loop() {
    // loop vazio, pois o FreeRTOS gerencia as tarefas
}
