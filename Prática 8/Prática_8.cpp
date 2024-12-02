/*Código de base:
https://wokwi.com/projects/414350134543990785
*/
/*
Colocar prioridade pelo semaforo de contagem
  */
#define LED 2
#define BT 5

SemaphoreHandle_t semaforo;


TaskHandle_t taskTrataBTHandle;
TaskHandle_t xTaskADCHandle;

QueueHandle_t fluxoCarros1;
volatile bool fluxo1Ativo = true; 
volatile bool fluxo2Ativo = false; 
QueueHandle_t fluxoCarros2;



// Task A - Escuta a entrada do Serial Monitor
void vTaskLeitura(void *arg) {
  char c;
  char buf[50];
  int idx = 0;
  int tam;
  char *msg_ptr;  // ponteiro mensagem
  const uint8_t led = 2;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // Variável corretamente declarada

  while (1) {
    while (Serial.available() > 0) {  // verifica se há algo na serial

      c = Serial.read();  // ler o caracter
      buf[idx] = c;       // inseri o caracter no vetor
      idx++;              // incrimenta o indice

      if (c == '\n') {        // quando for dado o enter na porta serial
        buf[idx - 1] = '\0';  // inseri o /0 ao final da string

        msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));
        memcpy(msg_ptr, buf, idx * sizeof(char));
        tam = idx;  // ver o tamanho do dado recebido
        idx = 0;    // zero o indice


        if (strcmp(msg_ptr, "P1") == 0 && fluxo2Ativo == false) {
          xSemaphoreGive(semaforo);  // Corrigido aqui
          //Parar semáfaro 1
        } else if (strcmp(msg_ptr, "P2") == 0  && fluxo2Ativo == false ) {
          xSemaphoreGive(semaforo);  // Corrigido aqui
          //Parar semáfaro 2
        }
        vPortFree(msg_ptr);
        if (xHigherPriorityTaskWoken == pdTRUE) {  // Corrigido aqui
          portYIELD_FROM_ISR();                    // Realiza a troca de contexto se necessário
        }
      }
    }
        vTaskDelay(10 / portTICK_PERIOD_MS);

  }
}


void vTaskFluxo1(void *pvParameters) {
   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  int contador = 0;

  char C1 = 'C1';

  while (1) {


  if (fluxo1Ativo){
      vTaskDelay(pdMS_TO_TICKS(2000));
      Serial.println("enviando o carro FLUXO 1");
      Serial.println("FLUXO 2 PARADO");
      Serial.println();

      xQueueSend(fluxoCarros1, &C1, (TickType_t)0);

      contador++;
      if(contador == 5){
        contador = 0;
        xSemaphoreGive(semaforo); 
      }

    }
  }
}

void vTaskFluxo2(void *pvParameters) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  char C2 = 'C2';
  int contador = 0;

  while (1) {
  if (fluxo2Ativo){
      vTaskDelay(pdMS_TO_TICKS(2000));
      Serial.println("enviando o carro FLUXO 2");
      Serial.println("FLUXO 1 PARADO");
      xQueueSend(fluxoCarros2, &C2, (TickType_t)0);
      Serial.println();
      contador++;
      if(contador == 5){
        contador = 0;
        xSemaphoreGive(semaforo); 
      }


    }
  }
}


void vTaskControle(void *pvParameters){
  char ultimoCarro = 'C1';
  while (1) {

  if (xSemaphoreTake(semaforo, portMAX_DELAY) == pdTRUE){
      Serial.println("TROCANDO");
      Serial.println();

      fluxo1Ativo = !fluxo1Ativo;
      fluxo2Ativo =  !fluxo2Ativo;




    }

  }

}



void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  //  pinMode(BT,INPUT_PULLUP);

  semaforo = xSemaphoreCreateBinary();
  if (semaforo == NULL) {
   Serial.println("Erro ao criar o semáforo 1");
   return;
  }



  fluxoCarros1 = xQueueCreate(10, sizeof(char));
  if (fluxoCarros1 == NULL) {
    Serial.println("Erro ao criar a fila");
    return;
  }
  fluxoCarros2 = xQueueCreate(10, sizeof(char));
  if (fluxoCarros2 == NULL) {
    Serial.println("Erro ao criar a fila");
    return;
  }


  xTaskCreate(vTaskLeitura, "Task Leitura", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &taskTrataBTHandle);
  xTaskCreate(vTaskControle, "Task controle ", configMINIMAL_STACK_SIZE + 1024, NULL, 3, &taskTrataBTHandle);
  xTaskCreate(vTaskFluxo1, "Task Fluxo 1", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskADCHandle);
  xTaskCreate(vTaskFluxo2, "Task Fluxo 2", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &taskTrataBTHandle);
}

void loop() {

}
