// Controllo motore DC con L298N

int in1 = 8;
int in2 = 7;
int ena = 9;
int in3 = 12;
int in4 = 11;
int ena1= 10;
int trigger= 6;
int echo = 5;
int durata;
int distanza;

// ------------------------------
// LED segnalazione
// ------------------------------
int led1 = A0;
int led2 = A1;

int velAvantidx= 90;
int velAvantisx= 132;
int velIndietro= 65;
int velRallentata = 50;   // velocità quando distanza < 100 cm

void setup() {
  Serial.begin(9600);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(ena1, OUTPUT);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT); 
  pinMode(trigger, OUTPUT);
  pinMode(echo,INPUT);
}

void loop() {

  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  durata = pulseIn(echo, HIGH);
  distanza = durata * 0.034 / 2;

  Serial.print("Distanza: ");
  Serial.println(distanza);

  //--- LED VERDE ACCESO DEFAULT ---
  if(distanza>100){
    delay(500);
    digitalWrite(led1, LOW); //LED ROSSO SPENTO
    delay(500);
    digitalWrite(led2, HIGH); // LED VERDE ACCESO
  }

  // ---- CURVA SE < 40 cm ----
  if(distanza < 40){
    motoreStop();
    motoreStop2();
    curvaDx();
    delay(400);
    digitalWrite(led1, HIGH); //LED ROSSO ACCESO
    delay(500);
    digitalWrite(led2, LOW); // LED VERDE SPENTO
  }

  // ---- RALLENTA SE < 100 cm ----
  if(distanza < 100){
    motoreAvantiRallentato();
    delay(500);
    digitalWrite(led1, HIGH); //LED ROSSO ACCESO
    delay(500);
    digitalWrite(led2, LOW); // LED VERDE SPENTO
    return;
  }

  // ---- ALTRIMENTI AVANTI NORMALE ----
  motoreAvanti();
  motoreAvanti2();
}



// **** FUNZIONI MOTORI ****

void motoreStop(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(ena, 0);
}

void motoreStop2(){
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(ena1, 0);
}

void motoreAvanti(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena, velAvantisx);
}

void motoreAvanti2(){
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena1, velAvantidx);
}

void motoreAvantiRallentato(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena, velRallentata);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena1, velRallentata);
}



void curvaDx(){
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(ena1, 120);

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena, 70);
}
