// patientAlerts.h

#ifndef PATIENTALERTS_H
#define PATIENTALERTS_H

#define LED_ALERT_PIN       7
#define SPEAKER_ALERT_PIN   27
#define BUTTON_INPUT_PIN    11

#define BLINK_DURATION        1000
#define BEEP_DURATION         300

#define ALERT_SERIOUS       2
#define ALERT_MILD          1

#define PWM_TIMER_FREQUENCY     125000
#define BEEP_FREQUENCY          2000
#define BEEP_PERIOD_TICKS       PWM_TIMER_FREQUENCY / BEEP_FREQUENCY
#define BEEP_DUTY_CYCLE_TICKS   BEEP_PERIOD_TICKS / 2

#define ALERT_PACKET_SIZE 1

#include "packager.h"

typedef struct PatientAlerts {
    // members
    Packager alertPackager;
} PatientAlerts;


// Public
int patientAlertsInit(PatientAlerts * this);


#endif //PATIENTALERTS_H