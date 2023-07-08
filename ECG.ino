int data[100];
int index = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int val = analogRead(A0);
  data[index] = val;
  index++;
  if (index == 100) {
    for (int i = 0; i < 100; i++) {
      Serial.println(data[i]);
    }
    index = 0;
  }
}
