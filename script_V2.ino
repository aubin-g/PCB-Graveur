// Ports du 74HC595
const int dataPin = 11;   // DS
const int latchPin = 12;  // ST_CP
const int clockPin = 10;  // SH_CP

// Stepper Moteur
#include <AccelStepper.h>

const int stepPin = 3;// Déclaration des broches de contrôle du moteur
const int dirPin = 4;
const int enablePin = 2;

// Création de l'objet Stepper
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

int stepPosition = 0;


const int digitPins[4] = {8, 5, 6, 7};  // Pins des cathodes des digits

// Définition des boutons
const int button0 = A1;   // Bouton pour 0 step\0 mm 
const int button20 = A2;  // Bouton pour 20 steps\0.6 mm
const int button100 = A3; // Bouton pour 100 steps\3 mm
const int button_1 = A4; // Bouton pour -1 Step
const int button1 = A5; // Bouton pour +1 Step

const byte segmentDigits[12] =  {
  0b01011111, // 0
  0b01000010, // 1
  0b10011011, // 2
  0b11010011, // 3
  0b11000110, // 4
  0b11010101, // 5
  0b11011101, // 6
  0b01000111, // 7
  0b11011111, // 8
  0b11010111, // 9
  0b10000000, // - (moins)
  0b00000000  // rien
};

const byte decimalPointMask = 0b00100000;

void shiftOutData(byte data) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void displayDigit(int digit, int value, bool decimalPoint = false) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }

  byte data = segmentDigits[value];
  if (decimalPoint) {
    data |= decimalPointMask; 
  }

  shiftOutData(data);
  digitalWrite(digitPins[digit], LOW);
  delay(5);
}

// Afficher une float à un entier et deux décimales
void displayNumber(float number) {
  int integer = (int)number; 
  int firstDecimal = (int)((number - integer) * 10);
  int secondDecimal = round((number * 100) - (integer * 100 + firstDecimal * 10));
  if (secondDecimal >= 10){
    secondDecimal = 0;
    firstDecimal++;
  }
  if (firstDecimal >= 10){
    firstDecimal = 0;
    integer++;
  }


  if (round(number * 100) == 0) {
    displayDigit(0, 11, false);  
    displayDigit(1, 0, true);   
    displayDigit(2, 0, false);   
    displayDigit(3, 0, false);   
  } else if (number < 10) {
    displayDigit(0, 10, false); // Affiche un moins
    displayDigit(1, integer, true);  // Affiche la partie entière avec le point décimal
    displayDigit(2, firstDecimal, false); // Affiche la première décimale
    displayDigit(3, secondDecimal, false); // Affiche la seconde décimale
  }
}

void moveToStep(int target) {
  digitalWrite(enablePin, LOW); // moteur ON
  stepper.moveTo(target);

  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  digitalWrite(enablePin, HIGH); // moteur OFF
  stepPosition = target;
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }

  // Déclaration de la vitesse du moteur
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(200);

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH); // moteur OFF

  // Déclaration de la direction du moteur
  pinMode(dirPin, OUTPUT);

  // Configuration des boutons avec résistance de tirage interne activée
  pinMode(button0, INPUT_PULLUP);
  pinMode(button20, INPUT_PULLUP);
  pinMode(button100, INPUT_PULLUP);
  pinMode(button_1, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);

  Serial.begin(9600);
}


void loop() {

  if (digitalRead(button0) == LOW) {
    Serial.print("0");
    moveToStep(0);
    delay(200);
    stepPosition = 0;
  }
  if (digitalRead(button20) == LOW) {
    Serial.print("20");
    moveToStep(20);
    delay(200); 
    stepPosition = 20;
  }
  if (digitalRead(button100) == LOW) {
    Serial.print("100");
    moveToStep(100);
    delay(200);
    stepPosition = 100;
  }
  if (digitalRead(button_1) == LOW) {// On fait descendre de 1 Step le plateau
    Serial.print("-");
    if (stepPosition - 1>=0){
      stepPosition = stepPosition - 1;
      digitalWrite(dirPin, -1);
      moveToStep(stepPosition);
      delay(200);
    }
  }
  if (digitalRead(button1) == LOW) {// On fait monter de 1 Step le plateau
    Serial.print("+");
    if (stepPosition + 1<=3){
      stepPosition = stepPosition + 1;
      digitalWrite(dirPin, 1);
      moveToStep(stepPosition);
      delay(200);
    }
  }

  // Calcul et affichage de l'élévation
  float elevation = (stepPosition / 200.0) * 6.0;
  displayNumber(elevation);
}
