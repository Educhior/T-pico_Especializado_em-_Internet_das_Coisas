#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

int leds[] = {22, 18, 16, 2};
int bts[] = {23, 19, 17, 4};

#define MAX_SEQUENCE 20
#define DEBOUNCE_DELAY 50  // Tempo de debounce em milissegundos

#define EVENT_SEQUENCE_READY (1 << 0)
#define EVENT_PLAYER_DONE (1 << 1)
#define EVENT_GAME_OVER (1 << 2)

// Variáveis globais
QueueHandle_t sequenceQueue;
SemaphoreHandle_t sequenceMutex;
EventGroupHandle_t gameEvents;
int generatedSequence[MAX_SEQUENCE];
int playerInput[MAX_SEQUENCE];
int currentLength = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Jogo Simon Iniciado");

  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(bts[i], INPUT_PULLUP);
  }

  // inicialização
  sequenceQueue = xQueueCreate(MAX_SEQUENCE, sizeof(int));
  sequenceMutex = xSemaphoreCreateMutex();
  gameEvents = xEventGroupCreate();

  xTaskCreatePinnedToCore(generateSequenceTask, "Gerar Sequência", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(displaySequenceTask, "Exibir Sequência", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(playerInputTask, "Entrada Jogador", 4096, NULL, 1, NULL, 1);

  // Reinicia o jogo
  resetGame();
  xEventGroupSetBits(gameEvents, EVENT_SEQUENCE_READY); // Inicia a geração da sequência
}

void loop() {
  
}

// Tarefa de Geração da sequência
void generateSequenceTask(void *param) {
  while (1) {
    // Aguarda o fim de uma rodada ou evento de reinício
    xEventGroupWaitBits(gameEvents, EVENT_SEQUENCE_READY | EVENT_GAME_OVER, pdTRUE, pdFALSE, portMAX_DELAY);

    if (xEventGroupGetBits(gameEvents) & EVENT_GAME_OVER) {
      break; // Finaliza a tarefa caso o jogo termine
    }

    // Gera o próximo número da sequência
    int newNumber = random(0, 4);
    Serial.printf("Gerando número: %d\n", newNumber);
    xSemaphoreTake(sequenceMutex, portMAX_DELAY);
    generatedSequence[currentLength] = newNumber;
    currentLength++;
    xSemaphoreGive(sequenceMutex);

    // Sinaliza que a sequência foi gerada e está pronta para exibição
    xEventGroupSetBits(gameEvents, EVENT_SEQUENCE_READY);
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Atraso entre as gerações
  }
}

// Tarefa de Exibição da sequência
void displaySequenceTask(void *param) {
  while (1) {
    // Aguarda o sinal de que a sequência está pronta para ser exibida
    xEventGroupWaitBits(gameEvents, EVENT_SEQUENCE_READY, pdTRUE, pdFALSE, portMAX_DELAY);

    // Exibe a sequência armazenada
    for (int i = 0; i < currentLength; i++) {
      int number = generatedSequence[i];
      Serial.printf("Exibindo: %d\n", number);
      digitalWrite(leds[number], HIGH);
      delay(500);
      digitalWrite(leds[number], LOW);
      delay(300);
    }

    // Sinaliza que o jogo está aguardando a entrada do jogador
    xEventGroupClearBits(gameEvents, EVENT_SEQUENCE_READY);
    xEventGroupSetBits(gameEvents, EVENT_PLAYER_DONE);
  }
}

// Tarefa da entrada do jogador
void playerInputTask(void *param) {
  while (1) {
    // Aguarda o sinal de que a sequência foi exibida e o jogo está esperando a entrada do jogador
    xEventGroupWaitBits(gameEvents, EVENT_PLAYER_DONE, pdTRUE, pdFALSE, portMAX_DELAY);

    Serial.println("Aguardando entrada do jogador...");

    bool sequenceCorrect = true;

    // Aguarda o jogador pressionar os botões
    for (int i = 0; i < currentLength; i++) {
      bool inputReceived = false;

      // Aguarda o jogador pressionar um botão
      while (!inputReceived) {
        for (int j = 0; j < 4; j++) {
          if (isButtonPressed(bts[j])) {
            digitalWrite(leds[j], HIGH);
            delay(300);
            digitalWrite(leds[j], LOW);
            playerInput[i] = j;
            inputReceived = true;
            break;
          }
        }
      }

      // Verifica se a entrada do jogador
      if (playerInput[i] != generatedSequence[i]) {
        sequenceCorrect = false;
        break;
      }
    }

    // Verifica a sequência e define o próximo passo
    if (sequenceCorrect) {
      Serial.println("Sequência correta! Próxima rodada.");
      xEventGroupSetBits(gameEvents, EVENT_SEQUENCE_READY); // Prepara para a próxima rodada
    } else {
      Serial.println("Sequência errada. Fim de jogo.");
      xEventGroupSetBits(gameEvents, EVENT_GAME_OVER);
      resetGame();
    }
  }
}

// Função para resetar o jogo
void resetGame() {
  currentLength = 0;
  Serial.println("Jogo reiniciado. Nova sequência gerada.");

  // Indicação visual do reset
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(200);
    digitalWrite(leds[i], LOW);
  }

  xEventGroupClearBits(gameEvents, EVENT_PLAYER_DONE | EVENT_GAME_OVER);
  delay(1000);
}

// Função de verificação do botão (com debounce)
bool isButtonPressed(int buttonPin) {
  if (digitalRead(buttonPin) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(buttonPin) == LOW) {
      while (digitalRead(buttonPin) == LOW) {
        delay(10);
      }
      return true;
    }
  }
  return false;
}
