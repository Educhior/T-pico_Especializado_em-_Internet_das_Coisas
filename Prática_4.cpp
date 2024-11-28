char c;
char buf[50];
int idx = 0;
static volatile uint8_t flag1 = 0;
static volatile uint8_t flag2 = 0;
int tam;
char *msg_ptr; // ponteiro mensagem
const uint8_t led = 2;

/*Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task A - Escuta a entrada do Serial Monitor
void receber(void *arg) {

  while (1) {
    while (Serial.available() > 0) { // verifica se há algo na serial

      c = Serial.read(); // ler o caracter
      buf[idx] = c;      // inseri o caracter no vetor
      idx++;             // incrimenta o indice

      if (c == '\n') {       // quando for dado o enter na porta serial
        buf[idx - 1] = '\0'; // inseri o /0 ao final da string

        msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));
        memcpy(msg_ptr, buf, idx * sizeof(char));
        tam = idx; // ver o tamanho do dado recebido
        idx = 0;   // zero o indice
       

        if (strcmp(msg_ptr, "LED") == 0) {
          flag2 = 1;
        }
        else{
           flag1 = 1; // faz a flag igual a 1
          
        }

        
        // imprimi o valor na task
        
   
        
      }
    }
  }
}

// Task B - Envia o valor do contador para o Serial Monitor
void enviar(void *arg) {

  while (1) {
    if (flag1 == 1) { // verifica se a flag foi acionada
      /*
     Serial.print("Recebendo: "); //imprimia mesma coisa recebida na outra task
       for(int i=0;i<tam;i++){
         Serial.print(buf[i]);
       }
      */
      Serial.print(msg_ptr);
      memset(buf, 0, sizeof(buf)); // limpa o vetor;
      idx = 0;
      flag1 = 0; // zera a flag dessa task
      Serial.println();
    }
  }
}

// Task C - Caso a palavra recebida seja LED acender o led.
void LED(void *pvParameters) {
  while (1) {
    if (flag2 == 1) {
      digitalWrite(led, !digitalRead(led));
      flag2 = 0;
    }
  }
}

void setup() {
  // Configure Serial
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  // Start task 1
  xTaskCreatePinnedToCore(enviar, "enviar", 1024, NULL, 1, NULL, 1);

  xTaskCreatePinnedToCore(receber, "Receber", 1024, NULL, 1, NULL, 1);

  xTaskCreatePinnedToCore(LED, "LED", 1024, NULL, 1, NULL, 1);
}

void loop() { delay(100); }