const int S[3] = {5,4,0};
const int Size = 3;
void setup() {
  Serial.begin(115200);
  pinMode(S[0],INPUT);
  pinMode(S[1],INPUT);
  pinMode(S[2],INPUT);
}

void loop() {
  int state[3];
  int i = 0;
  for(i=0;i<Size;i++){
    state[i] = digitalRead(S[i]);
  }
  int j = 0;
  Serial.print("All state are: ");
  for(j=0;j<Size;j++){
    Serial.print(state[j]);
  }
  Serial.println();
  //using wifi to send data
  delay(5000);
}
