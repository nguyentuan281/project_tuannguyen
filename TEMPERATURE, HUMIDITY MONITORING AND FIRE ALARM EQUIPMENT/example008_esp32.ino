//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>

/* The two task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/* Used to hold the handle of Task2. */
TaskHandle_t xTask2Handle;

/*-----------------------------------------------------------*/

void setup( void )
{
  Serial.begin(9600);
  /* Create the first task at priority 2.  This time the task parameter is
  not used and is set to NULL.  The task handle is also not used so likewise
  is also set to NULL. */
  xTaskCreate( vTask1, "Task 1", 1000, NULL, 2, NULL );
        /* The task is created at priority 2 ^. */

  /* Create the second task at priority 1 - which is lower than the priority
  given to Task1.  Again the task parameter is not used so is set to NULL -
  BUT this time we want to obtain a handle to the task so pass in the address
  of the xTask2Handle variable. */
  xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, &xTask2Handle);
         /* The task handle is the last parameter ^^^^^^^^^^^^^ */

  /* Start the scheduler so our tasks start executing. */
  // vTaskStartScheduler();

  for( ;; );
//  return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
unsigned portBASE_TYPE uxPriority;
unsigned portBASE_TYPE uxPriority2 ;

  /* This task will always run before Task2 as it has the higher priority.
  Neither Task1 nor Task2 ever block so both will always be in either the
  Running or the Ready state.
  Query the priority at which this task is running - passing in NULL means
  "return our own priority". */
  uxPriority = uxTaskPriorityGet( NULL );
  //uxPriority2 = uxTaskPriorityGet(xTask2Handle);

  for( ;; )
  {
    /* Print out the name of this task. */
    Serial.print( "Task1 is running with priority " );
    Serial.println(uxPriority);
    //Serial.println(uxPriority2);

    /* Setting the Task2 priority above the Task1 priority will cause
    Task2 to immediately start running (as then Task2 will have the higher
    priority of the    two created tasks). */
    Serial.print( "About to raise the Task2 priority\r\n" );
    vTaskPrioritySet(xTask2Handle, (uxPriority + 1) );
    vTaskDelay(500/portTICK_PERIOD_MS); 

    /* Task1 will only run when it has a priority higher than Task2.
    Therefore, for this task to reach this point Task2 must already have
    executed and set its priority back down to 0. */
  }
}

/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
unsigned portBASE_TYPE uxPriority;

  /* Task1 will always run before this task as Task1 has the higher priority.
  Neither Task1 nor Task2 ever block so will always be in either the
  Running or the Ready state.
  Query the priority at which this task is running - passing in NULL means
  "return our own priority". */
  uxPriority = uxTaskPriorityGet( NULL );

  for( ;; )
  {
    /* For this task to reach this point Task1 must have already run and
    set the priority of this task higher than its own.
    Print out the name of this task. */
    Serial.print( "Task2 is running with priority " );
    Serial.println(uxPriority); 
    /* Set our priority back down to its original value.  Passing in NULL
    as the task handle means "change our own priority".  Setting the
    priority below that of Task1 will cause Task1 to immediately start
    running again. */
    Serial.print( "About to lower the Task2 priority\r\n" );
    vTaskPrioritySet( NULL, (uxPriority-2));
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}
/*-----------------------------------------------------------*/
void loop(){
  delay(10);
}
