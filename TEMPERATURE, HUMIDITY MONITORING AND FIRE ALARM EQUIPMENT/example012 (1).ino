#include "Arduino_FreeRTOS.h"
//#include "task.h"
#include "semphr.h"
//#include "portasm.h"

/* Demo includes. */
//#include "basic_io_arm.h"

/* Compiler includes. */
//#include <dos.h>

/* The tasks to be created. */
static void vHandlerTask( void *pvParameters );
static void vPeriodicTask( void *pvParameters );

/* The service routine for the interrupt.  This is the interrupt that the task
will be synchronized with. */
static void vExampleInterruptHandler( void );

/*-----------------------------------------------------------*/

/* Declare a variable of type SemaphoreHandle_t.  This is used to reference the
semaphore that is used to synchronize a task with an interrupt. */
SemaphoreHandle_t xBinarySemaphore;

// pins to generate interrupts - they must be connected
const uint8_t inputPin = 2;
const uint8_t outputPin = 3;

void setup( void )
{
  Serial.begin(9600);
  /* Before a semaphore is used it must be explicitly created.  In this example
  a counting semaphore is created.  The semaphore is created to have a maximum
  count value of 10, and an initial count value of 0. */
  xBinarySemaphore = xSemaphoreCreateBinary();

  /* Check the semaphore was created successfully. */
  if( xBinarySemaphore != NULL )
  {
    /* Create the 'handler' task.  This is the task that will be synchronized
    with the interrupt.  The handler task is created with a high priority to
    ensure it runs immediately after the interrupt exits.  In this case a
    priority of 3 is chosen. */
    xTaskCreate( vHandlerTask, "Handler", 200, NULL, 3, NULL );

    /* Create the task that will periodically generate a software interrupt.
    This is created with a priority below the handler task to ensure it will
    get preempted each time the handler task exist the Blocked state. */
    xTaskCreate( vPeriodicTask, "Periodic", 200, NULL, 1, NULL );

    /* Install the interrupt handler. */
    pinMode(inputPin, INPUT);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, HIGH);
    bool tmp = digitalRead(inputPin);
    digitalWrite(outputPin, LOW);
    if (digitalRead(inputPin) || !tmp) {
      Serial.println("pin 2 must be connected to pin 3");
      while(1);
    }
    attachInterrupt(0, vExampleInterruptHandler, RISING);    
    
    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();
  }

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
  for( ;; );
//  return 0;
}
/*-----------------------------------------------------------*/

static void vHandlerTask( void *pvParameters )
{
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* Use the semaphore to wait for the event.  The semaphore was created
    before the scheduler was started so before this task ran for the first
    time.  The task blocks indefinitely meaning this function call will only
    return once the semaphore has been successfully obtained - so there is no
    need to check the returned value. */
    xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );

    /* To get here the event must have occurred.  Process the event (in this
    case we just print out a message). */
    Serial.print( "Handler task - Processing event.\r\n" );
  }
}
/*-----------------------------------------------------------*/

static void vPeriodicTask( void *pvParameters )
{
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* This task is just used to 'simulate' an interrupt.  This is done by
    periodically generating a software interrupt. */
    vTaskDelay( 500 / portTICK_PERIOD_MS );

    /* Generate the interrupt, printing a message both before hand and
    afterwards so the sequence of execution is evident from the output. */
    Serial.print( "Periodic task - About to generate an interrupt.\r\n" );
//    __asm{ int 0x82 }
    digitalWrite(outputPin, LOW);
    digitalWrite(outputPin, HIGH);

    Serial.print( "Periodic task - Interrupt generated.\r\n\r\n\r\n" );
  }
}
/*-----------------------------------------------------------*/

static void vExampleInterruptHandler( void )
{
static BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

  
  xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
  
  /* xHigherPriorityTaskWoken was initialised to pdFALSE.  It will have then
  been set to pdTRUE only if reading from or writing to a queue caused a task
  of equal or greater priority than the currently executing task to leave the
  Blocked state.  When this is the case a context switch should be performed.
  In all other cases a context switch is not necessary.
  NOTE: The syntax for forcing a context switch within an ISR varies between
  FreeRTOS ports.  The portEND_SWITCHING_ISR() macro is provided as part of
  the Cortex-M3 port layer for this purpose.  taskYIELD() must never be called
  from an ISR! */
  //portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  vPortYield();
}
//---------------------------------------------------------------
void loop() {}
