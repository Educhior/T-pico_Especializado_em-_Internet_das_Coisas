TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;
QueueHandle_t queue;
int cont  = 30;

void Demo_Task(void *arg)
{
    char txBuffer[50];
    queue = xQueueCreate(cont, sizeof(txBuffer)); 
    if (queue == 0)
    {
     printf("Failed to create queue= %p\n", queue);
    }
    while(1){
        for(int i  = 0; i <= cont; i++){
            sprintf(txBuffer, "Hello from Demo_Task %d", i);
            xQueueSend(queue, (void*)txBuffer, (TickType_t)0);
        }
  
        vTaskDelay(1000/ portTICK_RATE_MS);
    }
}

void Demo_Task2(void *arg)
{
    char rxBuffer[50];
    while(1){
     if( xQueueReceive(queue, &(rxBuffer), (TickType_t)5))
     {
      printf("Received data from queue == %s\n", rxBuffer);
      vTaskDelay(1000/ portTICK_RATE_MS);

     }
    }
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Fila");  
   xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 1, &myTaskHandle);
   xTaskCreatePinnedToCore(Demo_Task2, "Demo_Task2", 4096, NULL, 1, &myTaskHandle2, 1);
 }

 void loop(){

 }