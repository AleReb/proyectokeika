// sonidos podria funcionar mejor con buzzer pasivo
void partida(){
 tone(BUZZER_PIN, NOTE_D5, 150, BUZZER_CHANNEL); // beep( -PIN OF SPEAKER-, -THE NOTE WANTING TO BE PLAYED-, -DURATION OF THE NOTE IN MILISECONDS- )
delay(80);
tone(BUZZER_PIN, NOTE_F5, 150, BUZZER_CHANNEL);
delay(80);
tone(BUZZER_PIN, NOTE_D6, 250, BUZZER_CHANNEL);
delay(250);
    }

void entrada(){
   tone(BUZZER_PIN, NOTE_C4, 50, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A3, 50, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_F4, 50, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_B, 250, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
}

void salida(){
  tone(BUZZER_PIN, NOTE_A4, 150, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_F3, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E3, 100, BUZZER_CHANNEL); 
  }

 void okenter(){
  tone(BUZZER_PIN, NOTE_A4, 150, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  } 
