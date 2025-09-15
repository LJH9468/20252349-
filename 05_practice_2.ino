#define PIN_LED 7
unsigned int count;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  while (!Serial) {
    ; 
  }
  count = 1; 
}
void loop() {
  Serial.println(--count);
  digitalWrite(PIN_LED, count);
  delay(1000);
  
  for(int i = 0; i<5; i++){
    Serial.println(--count);
    digitalWrite(PIN_LED, count); 
    delay(100);
    Serial.println(++count);
    digitalWrite(PIN_LED, count); 
    delay(100);
  }
  while(1){
    Serial.println(++count);
    digitalWrite(PIN_LED, count);
  }
}
