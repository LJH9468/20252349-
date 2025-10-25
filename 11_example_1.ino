#include <Servo.h>

// Arduino pin assignment
#define PIN_LED 9    
#define PIN_TRIG 12  
#define PIN_ECHO 13  
#define PIN_SERVO 10 

// configurable parameters for sonar
#define SND_VEL 346.0     
#define INTERVAL 25       
#define PULSE_DURATION 10 
#define _DIST_MIN 180.0   
#define _DIST_MAX 360.0   

#define TIMEOUT ((INTERVAL * 1000L) - (1000L)) 
#define SCALE (0.001 * 0.5 * SND_VEL) 

#define _EMA_ALPHA 0.3    

// Target Distance 
#define _TARGET_LOW _DIST_MIN
#define _TARGET_HIGH _DIST_MAX

// duty duration for myservo.writeMicroseconds()
#define _DUTY_MIN 550 
#define _DUTY_NEU 1475 
#define _DUTY_MAX 2400 

// global variables
float  dist_ema, dist_prev = _DIST_MAX; 
unsigned long last_sampling_time;       
int duty_target; 
float duty_change_per_interval_f;
int duty_change_per_interval;

Servo myservo;

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; 
}

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);    
  pinMode(PIN_ECHO, INPUT);     
  digitalWrite(PIN_TRIG, LOW);  

  myservo.attach(PIN_SERVO);
  duty_target = _DUTY_NEU;
  myservo.writeMicroseconds(duty_target);

  // initialize USS related variables
  dist_prev = (_DIST_MIN + _DIST_MAX) / 2.0; 
  dist_ema = dist_prev;

  // initialize serial port
  Serial.begin(57600);
  
  // convert angular speed into duty change per interval.
  duty_change_per_interval_f = 
    (_DUTY_MAX - _DUTY_MIN) * (30.0 / 180.0) * (INTERVAL / 1000.0);
  duty_change_per_interval = (int)duty_change_per_interval_f;
  
  Serial.print("duty_change_per_interval:");
  Serial.println(duty_change_per_interval);
  
  last_sampling_time = 0;
}

void loop() {
  float  dist_raw, dist_filtered;
  
  // wait until next sampling time.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // the range filter
  // 0mm 또는 10mm 미만의 값도 측정 실패로 간주하고 LED 끄기
  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX) || (dist_raw < 10.0)) {
       dist_filtered = dist_prev;
       digitalWrite(PIN_LED, LOW); 
  } else {    
       dist_filtered = dist_raw;
       dist_prev = dist_raw; 
       digitalWrite(PIN_LED, HIGH); 
  }

  // Modify the below line to implement the EMA equation
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // adjust servo position according to the USS read value
  if (dist_ema <= _DIST_MIN) { 
      duty_target = _DUTY_MIN;
  } else if (dist_ema >= _DIST_MAX) {
      duty_target = _DUTY_MAX;
  } else {
    duty_target = (int) (_DUTY_MIN + 
      (dist_ema - _DIST_MIN) * ((float)(_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN)));
  }

  // constrain duty_target 
  duty_target = constrain(duty_target, _DUTY_MIN, _DUTY_MAX);


  // adjust duty_curr toward duty_target by duty_change_per_interval
  if (duty_target > myservo.readMicroseconds()) {
    myservo.writeMicroseconds(min(duty_target, myservo.readMicroseconds() + duty_change_per_interval));
  } else {
    myservo.writeMicroseconds(max(duty_target, myservo.readMicroseconds() - duty_change_per_interval));
  }


  // output the distance to the serial port
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",dist:"); Serial.print(dist_raw, 1);
  Serial.print(",ema:");  Serial.print(dist_ema, 1);
  Serial.print(",Servo:"); Serial.print(myservo.read());
  Serial.print(",Max:");   Serial.print(_DIST_MAX);
  Serial.println("");
  
  // update last sampling time
  last_sampling_time += INTERVAL;
}
