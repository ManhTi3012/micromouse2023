TaskHandle_t Task1;
TaskHandle_t Task2;

#define ENCODER_1A 32
#define ENCODER_2A 33

int _1msclock = 0;

int L_set_rpm = 1000;
float L_dutyCycle = 100;
unsigned long L_prev_time = 0;
float L_rpm = 0;
float L_rpm_error_pre = 0;
int L_deltaT;
unsigned long L_distance_covered = 0;//mm

int R_set_rpm = 2000;
float R_dutyCycle = 100;
unsigned long R_prev_time = 0;
float R_rpm = 0;
int R_deltaT;
unsigned long R_distance_covered = 0;//mm
//rpm meter
void IRAM_ATTR L_encoder_isr() {
  L_deltaT = micros()-L_prev_time;
  L_prev_time = micros();
}
void IRAM_ATTR R_encoder_isr() {
  R_deltaT = micros()-R_prev_time;
  R_prev_time = micros();
}
// motor setup
const int Motor1A = 26;
const int Motor1B = 27;
const int Motor2A = 27;
const int Motor2B= 35;
// setting PWM properties
const int freq = 15625;
const int L_motor_channel = 0;
const int R_motor_channel = 1;
const int resolution = 12;

void setup() {
  Serial.begin(115200); 

  pinMode(ENCODER_1A, INPUT_PULLUP);
  pinMode(ENCODER_2A, INPUT_PULLUP);

  // Attaching the ISR to encoder A
  attachInterrupt(digitalPinToInterrupt(ENCODER_1A), L_encoder_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_2A), R_encoder_isr, RISING);


  ledcSetup(L_motor_channel, freq, resolution);
  ledcAttachPin(Motor1A, L_motor_channel);
  ledcSetup(R_motor_channel, freq, resolution);
  ledcAttachPin(Motor2A, R_motor_channel);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(50); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(50); 
}

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    if(millis() - _1msclock >=1){
    L_rpm = (float)(60000000)/(L_deltaT*334);
    float L_rpm_error = L_set_rpm - L_rpm;
    L_dutyCycle += L_rpm_error * 0.005 + ((L_rpm_error-L_rpm_error_pre)/0.001) * 5;
    L_dutyCycle = constrain(L_dutyCycle,0,4095);
    ledcWrite(L_motor_channel, L_dutyCycle);
    L_rpm_error_pre = L_rpm_error;


    R_rpm = (float)(60000000)/(R_deltaT*334);
    float R_rpm_error = R_set_rpm - R_rpm;
    R_dutyCycle += R_rpm_error * 0.01;
    R_dutyCycle = constrain(R_dutyCycle,0,4095);
    ledcWrite(R_motor_channel, R_dutyCycle);
    }

  }
}

void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
      Serial.print(R_rpm);
      Serial.print(",");
      Serial.println(L_rpm);
    delay(55);
  }
}

void loop() {
  //do not write anything
  vTaskDelete(NULL);
}