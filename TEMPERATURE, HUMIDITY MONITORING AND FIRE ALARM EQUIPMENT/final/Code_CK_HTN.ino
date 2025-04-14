#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define DHT11_ID 0
#define Light_ID 1
static void DHT11( void *pvParameters );
static void Light( void *pvParameters );
static void LCDTask( void *pvParameters );
static void IRAM_ATTR InterruptHandler1(void);
static void IRAM_ATTR InterruptHandler2(void);
static void Warning(void *pvParameters);
static void Huy(void *pvParameters);
typedef struct {
  byte deviceID;
  float value1;
  float value2;
} SENSOR;
const byte dhtPin = 25;
const byte buzzerPIN = 33;
const byte lightPIN = 26;
const byte ledPIN = 14;
const byte firePIN = 13;
const byte ControlPIN = 32;
int dem = 0;

long debouncing_time = 200;
volatile unsigned long last_micros;
volatile unsigned long last_micros1;

QueueHandle_t queueSensor;
SemaphoreHandle_t xMutex1;
SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xBinarySemaphore1;
TaskHandle_t handle1,handle2, handle3 ;
void DHT11(void *pvParameters) {
  DHTesp dhtSensor;
  dhtSensor.setup(dhtPin, DHTesp::DHT11);
  SENSOR dht11Sensor;
  while (1 ) {
    xSemaphoreTake(xMutex1, portMAX_DELAY);
    const TickType_t timeOut = 2000 / portTICK_PERIOD_MS;
    dht11Sensor.deviceID = DHT11_ID;
    TempAndHumidity  data = dhtSensor.getTempAndHumidity();
    dht11Sensor.value1 = data.temperature;
    dht11Sensor.value2 = data.humidity;
    Serial.println("DHT11: Sent.");
    if (xQueueSend(queueSensor, &dht11Sensor, timeOut) != pdPASS) {
      Serial.println("DHT11: Queue is full.");
     
    }
    xSemaphoreGive(xMutex1);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void Light(void *pvParameters) {
  SENSOR lightSensor;
  while (1) {
    xSemaphoreTake(xMutex1, portMAX_DELAY);
    lightSensor.deviceID = Light_ID;
    const TickType_t timeOut = 1000 / portTICK_PERIOD_MS;
    float light =  digitalRead(lightPIN);
    lightSensor.value1 = light;
    Serial.println("Light: Sent.");
    if (xQueueSend(queueSensor, &lightSensor, timeOut) != pdPASS) {
      Serial.println("Light: Queue is full.");
    }
    xSemaphoreGive(xMutex1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void LCDTask(void *pvParameters) {  
  SENSOR data;
  while (1) {
    xSemaphoreTake(xMutex1, portMAX_DELAY);
    TickType_t timeOut = 1000/ portTICK_PERIOD_MS;
    if (xQueueReceive(queueSensor, &data, timeOut) == pdPASS) {
      switch (data.deviceID) {
        case DHT11_ID:
            lcd.setCursor(0, 0);
            lcd.print("T:" + String(data.value1, 1) + "oC");
            lcd.setCursor(0, 1);
            lcd.print("H:" + String(data.value2, 1) + "% ");
            Serial.println("T:" + String(data.value1, 1) + "oC");
            Serial.println("H:" + String(data.value2, 1) + "%");
            break;
        case Light_ID:
            lcd.setCursor(8, 0);
            lcd.print("| LIGHT*");
            lcd.setCursor(8, 1);
            if (data.value1 ==1) {
              lcd.print("|  On   ");
              digitalWrite(ledPIN, HIGH);
              Serial.println("On ");
            } 
            else if(data.value1 ==0) {
              digitalWrite(ledPIN, LOW);
              lcd.print("|  Off  ");
              Serial.println("Off");
            }
            break;
        default:
            Serial.println("LCD: Unkown Device");
            break;
        }
      }    
    xSemaphoreGive(xMutex1);
    //vTaskDelay(500);
  }
}
void Warning(void *pvParameters) {
  while(1){
    const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;
    if( xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ) == pdPASS){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" ----Warning----");
      lcd.setCursor(0, 1);
      lcd.print(" ----Warning----");
      vTaskSuspend(handle1);
      vTaskSuspend(handle2);
      vTaskSuspend(handle3);
      for (int i = 0; i < 5; i++) {
        digitalWrite(buzzerPIN, HIGH);
        digitalWrite(ledPIN, HIGH);
        delay(500);
        digitalWrite(buzzerPIN, LOW);
        digitalWrite(ledPIN, LOW);
        delay(500);
        Serial.println("Interrupted");
      }  
      delay(1000) ;
      vTaskResume(handle1);
      vTaskResume(handle2);
      vTaskResume(handle3);  
    }
      
    //vTaskDelete( NULL) ;
  }
}
void ControlLight(void *pvParameters){
  while(1){
  const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;
  if( xSemaphoreTake( xBinarySemaphore1, portMAX_DELAY ) == pdPASS){
      if (dem == 1){
          vTaskSuspend(handle1);// đình chỉ task
          digitalWrite(ledPIN, LOW);
          lcd.setCursor(15, 0);
          lcd.print(" " );
          lcd.setCursor(8, 1);
          lcd.print("|  Off  " );
      }else if(dem == 2){
          digitalWrite(ledPIN, HIGH);
          lcd.setCursor(8, 1);
          lcd.print("|  On   ");
      }else if(dem >= 3){
        vTaskResume(handle1);// gỡ đình chỉ
        dem = 0;
        lcd.setCursor(15, 0);
        lcd.print(" * ");
      }
  }
}
}
void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(lightPIN,INPUT_PULLUP);
  pinMode(firePIN, INPUT_PULLUP);
  pinMode(ControlPIN, INPUT_PULLUP);
  pinMode(ledPIN, OUTPUT);
  pinMode(buzzerPIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(firePIN), InterruptHandler1, HIGH);
  attachInterrupt(digitalPinToInterrupt(ControlPIN), InterruptHandler2, FALLING);
  queueSensor = xQueueCreate(5, sizeof(SENSOR));
  xMutex1 = xSemaphoreCreateMutex();
  xBinarySemaphore = xSemaphoreCreateBinary();
  xBinarySemaphore1 = xSemaphoreCreateBinary();
  xSemaphoreGive(xMutex1);
  if(queueSensor != NULL ){
  xTaskCreatePinnedToCore(Light, "Light sensor", 1024 * 4, NULL, 2, &handle1, 0);
  xTaskCreatePinnedToCore(DHT11, "DHT11", 1024 * 4, NULL, 3, &handle2, 0);
  xTaskCreatePinnedToCore(LCDTask, "lcd", 1024 * 4, NULL, 1, &handle3, 0);
  xTaskCreatePinnedToCore(Warning, "warning", 1024 * 4, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(ControlLight, "Control_light", 1024 * 4, NULL, 4, NULL, 0);

  }
}
void IRAM_ATTR InterruptHandler1(void) {
  if((long)(micros() - last_micros) >= debouncing_time *1000){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    last_micros = micros();
    xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
  }
}
void IRAM_ATTR InterruptHandler2(void) {
 if((long)(micros() - last_micros1) >= debouncing_time *1000){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    last_micros1 = micros();
    dem += 1;
    xSemaphoreGiveFromISR(xBinarySemaphore1, &xHigherPriorityTaskWoken);
  }
}
void loop() {}