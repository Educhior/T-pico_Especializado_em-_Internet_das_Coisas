/*Código de base:
https://wokwi.com/projects/414350134543990785
*/
/*
Criar leitura pelo serial
  */
#define LED 2
#define BT 5

SemaphoreHandle_t semaforo;
SemaphoreHandle_t semaforo2;

TaskHandle_t taskTrataBTHandle;
TaskHandle_t xTaskADCHandle;

QueueHandle_t fluxoCarros1;
QueueHandle_t fluxoCarros2;



// Task A - Escuta a entrada do Serial Monitor
void vTaskLeitura(void *arg) {
  char c;
  char buf[50];
  int idx = 0;
  int tam;
  char *msg_ptr; // ponteiro mensagem
  const uint8_t led = 2;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // Variável corretamente declarada

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


        if (strcmp(msg_ptr, "P1") == 0) {
          xSemaphoreGiveFromISR(semaforo, &xHigherPriorityTaskWoken); // Corrigido aqui
          //Parar semáfaro 1
        }
        else if (strcmp(msg_ptr, "P2") == 0) {
          xSemaphoreGiveFromISR(semaforo2, &xHigherPriorityTaskWoken); // Corrigido aqui
          //Parar semáfaro 2
        }

        if (xHigherPriorityTaskWoken == pdTRUE) {  // Corrigido aqui
          portYIELD_FROM_ISR();  // Realiza a troca de contexto se necessário
        }
      }
    }
  }
}


void vTaskFluxo1(void * pvParameters){

  char C1 = 'C1';

  while(1){

    xSemaphoreTake(semaforo,portMAX_DELAY);
    Serial.println("enviando o carro FLUXO 1");
    xQueueSend(fluxoCarros1, &C1, (TickType_t)0);
  }

}

void vTaskFluxo2(void * pvParameters){

  char C2 = 'C2';

  while(1){
   xSemaphoreTake(semaforo,portMAX_DELAY);
   Serial.println("enviando o carro FLUXO 2");
   xQueueSend(fluxoCarros2, &C2, (TickType_t)0);
  }
}




void setup() {
  Serial.begin(9600);
  pinMode(LED,OUTPUT);
//  pinMode(BT,INPUT_PULLUP);


  semaforo = xSemaphoreCreateBinary();
  xTaskCreate(vTaskLeitura, "Task Leitura",configMINIMAL_STACK_SIZE + 1024,NULL,3,&taskTrataBTHandle); 
  xTaskCreate(vTaskFluxo2, "Task BT",configMINIMAL_STACK_SIZE + 1024,NULL,3,&taskTrataBTHandle);  
  xTaskCreate(vTaskFluxo1,"TaskADC",configMINIMAL_STACK_SIZE + 1024, NULL,1,&xTaskADCHandle);


}

void loop() {

}

