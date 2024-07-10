#include "buzzer.h"

void Buzzer::beep()
{
    static unsigned long startAlarm = millis();

    if (millis() - startAlarm > 60000)
    {
        printf("Starting buzzer\n");
        startAlarm = millis();

        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 1000);

        tone(BUZZER_PIN, NOTE_F4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 1000);

        tone(BUZZER_PIN, NOTE_C4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_F4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_G4, 1000);

        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 1000);

        tone(BUZZER_PIN, NOTE_F4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 1000);

        tone(BUZZER_PIN, NOTE_C4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_C4, 2000);

        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_F4, 1000);

        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_F4, 500);
        tone(BUZZER_PIN, NOTE_G4, 1000);

        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_E4, 1000);

        tone(BUZZER_PIN, NOTE_F4, 500);
        tone(BUZZER_PIN, NOTE_D4, 500);
        tone(BUZZER_PIN, NOTE_D4, 1000);

        tone(BUZZER_PIN, NOTE_C4, 500);
        tone(BUZZER_PIN, NOTE_E4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_G4, 500);
        tone(BUZZER_PIN, NOTE_C4, 2000);
    }
}

void Buzzer::stop()
{
    printf("Stopping buzzer\n");
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
}