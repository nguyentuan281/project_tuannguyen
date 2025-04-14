// #include <Arduino_FreeRTOS.h>
/* The task function. */
void vTaskFunction( void *pvParameters );

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Task 1 is running\r\n";
const char *pcTextForTask2 = "Task 2 is running\t\n";

/*-----------------------------------------------------------*/

void setup( void )
{
  Serial.begin(9600);
  /* Create the first task at priority 1... */
  xTaskCreate( vTaskFunction, "Task 1", 2000, (void*)pcTextForTask1, 1, NULL );

  /* ... and the second task at priority 2.  The priority is the second to
  last parameter. */
  xTaskCreate( vTaskFunction, "Task 2", 2000, (void*)pcTextForTask2, 2, NULL );

  /* Start the scheduler so our tasks start executing. */
  // vTaskStartScheduler();

  for( ;; );
//  return 0;
}
/*-----------------------------------------------------------*/

void vTaskFunction( void *pvParameters )
{
char *pcTaskName;
TickType_t xLastWakeTime;

  /* The string to print out is passed in via the parameter.  Cast this to a
  character pointer. */
  pcTaskName = ( char * ) pvParameters;

  /* The xLastWakeTime variable needs to be initialized with the current tick
  count.  Note that this is the only time we access this variable.  From this
  point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
  API function. */
  xLastWakeTime = xTaskGetTickCount();

  /* As per most tasks, this task is implemented in an infinite loop. */
  for( ;; )
  {
    /* Print out the name of this task. */
    Serial.print( pcTaskName );

    /* We want this task to execute exactly every 250 milliseconds.  As per
    the vTaskDelay() function, time is measured in ticks, and the
    portTICK_PERIOD_MS constant is used to convert this to milliseconds.
    xLastWakeTime is automatically updated within vTaskDelayUntil() so does not
    have to be updated by this task code. */
    vTaskDelayUntil( &xLastWakeTime, ( 250 / portTICK_PERIOD_MS ) );
  }
}
//------------------------------------------------------------------------------
void loop() {}
