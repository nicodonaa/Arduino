// ------------------------------
// PIN CONFIGURAZIONE MOTORI (L298N)
// ------------------------------
int in1 = 8;
int in2 = 7;
int ena = 9;   // PWM Motore Sinistro (ENA)
int in3 = 12;
int in4 = 11;
int ena1 = 10; // PWM Motore Destro (ENB)

// ------------------------------
// PIN SENSORI ULTRASUONI
// ------------------------------
int trigger = 6;
int echo = 5;
long durata;
int distanza; // Distanza in cm

// ------------------------------
// PIN SENSORI DI LINEA (5 CANALI DIGITALI)
// ------------------------------
const int sensore1 = A2; // Estremo Sinistra
const int sensore2 = A3; // Sinistra
const int sensore3 = A4; // Centro
const int sensore4 = A5; // Destra
const int sensore5 = 4;  // Estremo Destra (Pin Digitale 4)

// Variabili per la lettura dei sensori (1 = Linea Nera, 0 = Sfondo Bianco)
int s1, s2, s3, s4, s5;

// ------------------------------
// VARIABILI DI CONTROLLO GLOBALI
// ------------------------------
int lastError = 0;       // Memorizza l'ultima direzione della linea (- = SX, + = DX)
const int searchSpeed = 87; // Velocità per la rotazione di ricerca
const int baseSpeedSX = 70;  // Velocità base Bassa Motore Sinistro
const int baseSpeedDX = 50;   // Velocità base Bassa Motore Destro
int Kp = 35;            // Costante Proporzionale 

// ------------------------------
// LED SEGNALAZIONE
// ------------------------------
int led1 = A0; // LED ROSSO (Per ostacoli)
int led2 = A1; // LED VERDE (Per OK / Line Follower attivo)

void setup() {
  Serial.begin(9600); 
  Serial.println("Avvio Line Follower con Ricerca Linea...");

  // Configurazione Pin... (omessa per brevità, ma presente nel codice caricato)
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT); pinMode(ena, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT); pinMode(ena1, OUTPUT);
  pinMode(led1, OUTPUT); pinMode(led2, OUTPUT);
  pinMode(trigger, OUTPUT); pinMode(echo, INPUT);
  pinMode(sensore1, INPUT); pinMode(sensore2, INPUT); pinMode(sensore3, INPUT);
  pinMode(sensore4, INPUT); pinMode(sensore5, INPUT);
  
  motoreStop(); 
  digitalWrite(led2, HIGH); 
  delay(1000);
}

void loop() {
  // 1. CONTROLLO OSTACOLI
  checkDistanza(); 

  if (distanza < 15 && distanza > 0) {
    motoreStop();
    digitalWrite(led1, HIGH); 
    digitalWrite(led2, LOW);  
    return; 
  } else {
    digitalWrite(led1, LOW);  
    digitalWrite(led2, HIGH); 
  }

  // 2. LETTURA SENSORI DI LINEA
  leggiSensori();
  
  // 3. CALCOLO ERRORE DI POSIZIONE (CON PESI AMMORBIDITI)
  int error = 0;

  // Pesi: -2  -1   0   1   2
  if (s3 == 1) { error = 0; }
  
  // Pesi interni
  if (s2 == 1) { error = -1; }
  if (s4 == 1) { error = 1; }
  
  // Pesi esterni
  if (s1 == 1) { error = -2; }
  if (s5 == 1) { error = 2; }

  // Curva secca
  if (s1 == 1 && s2 == 1) { error = -3; } 
  if (s4 == 1 && s5 == 1) { error = 3; }  

  // ===========================================
  // 4. LOGICA DI RICERCA LINEA
  // ===========================================
  if (s1==0 && s2==0 && s3==0 && s4==0 && s5==0) {
     // LINEA PERSA: RUOTA SUL POSTO
     
     if (lastError > 0) {
         // L'ultima volta la linea era a DESTRA. Ruota a DESTRA per ritrovarla.
         // Pivot Turn Destra: Sinistro avanti (120), Destro indietro (-120)
         muoviMotori(searchSpeed, -searchSpeed); 
     } else if (lastError < 0) {
         // L'ultima volta la linea era a SINISTRA. Ruota a SINISTRA per ritrovarla.
         // Pivot Turn Sinistra: Sinistro indietro (-120), Destro avanti (120)
         muoviMotori(-searchSpeed, searchSpeed); 
     } else {
         // Caso iniziale o eravamo perfettamente dritti: scegli una direzione (es. sinistra)
         muoviMotori(-searchSpeed, searchSpeed); 
     }
     return; // Torna all'inizio del loop per ricontrollare i sensori
  }

  // ===========================================
  // 5. FOLLOW LINE NORMALE
  // ===========================================
  
  // Aggiorna l'ultima direzione solo se stiamo vedendo la linea e non è dritto (error != 0)
  if (error != 0) {
      lastError = error;
  }
  
  // Calcolo Correzione Motori
  int correzione = Kp * error;

  int speedSX = baseSpeedSX + correzione;
  int speedDX = baseSpeedDX - correzione;
  
  // Applicazione ai motori
  muoviMotori(speedSX, speedDX);
}

// ===================================
// **** FUNZIONI MOTORI & SENSORI ****
// ===================================

void leggiSensori() {
  s1 = digitalRead(sensore1);
  s2 = digitalRead(sensore2);
  s3 = digitalRead(sensore3);
  s4 = digitalRead(sensore4);
  s5 = digitalRead(sensore5);
}

void muoviMotori(int speedL, int speedR) {
  speedL = constrain(speedL, -255, 255);
  speedR = constrain(speedR, -255, 255);

  // Motore Sinistro
  if (speedL >= 0) { 
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(ena, speedL);
  } else { 
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(ena, abs(speedL));
  }

  // Motore Destro
  if (speedR >= 0) { 
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(ena1, speedR);
  } else { 
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(ena1, abs(speedR));
  }
}

void checkDistanza() {
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  durata = pulseIn(echo, HIGH); 
  distanza = durata * 0.034 / 2;
}

void motoreStop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(ena, 0);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(ena1, 0);
}