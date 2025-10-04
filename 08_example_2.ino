// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance (unit: mm)
#define _DIST_MAX 300.0   // maximum distance (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // initialize serial port
  Serial.begin(57600);
  last_sampling_time = 0;
}

void loop() {
  unsigned long time = millis();

  if (time > last_sampling_time + INTERVAL) {
    last_sampling_time = time;

    float dist_raw = getDistance();      
    float dist_filtered = dist_raw;

    int duty;

    if (dist_filtered <= 100 || dist_filtered >= 300) {
      duty = 255;   // 최소 밝기 (꺼짐)
    } 
    else if (dist_filtered == 200) {
      duty = 0;     
    } 
    else if (dist_filtered == 150 || dist_filtered == 250) {
      duty = 128;   
    } 
    else if (dist_filtered > 100 && dist_filtered < 200) {
      duty = map(dist_filtered, 100, 200, 255, 0);  
    } 
    else if (dist_filtered > 200 && dist_filtered < 300) {
      duty = map(dist_filtered, 200, 300, 0, 255);  
    }

    analogWrite(PIN_LED, duty);

    Serial.print("dist_filtered = ");
    Serial.print(dist_filtered);
    Serial.print(" mm, duty = ");
    Serial.println(duty);
  }
}

// 초음파 센서로 거리 계산
float getDistance() {
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, TIMEOUT);

  // convert duration to distance (mm)
  float distance = duration * SCALE;

  return distance;
}
