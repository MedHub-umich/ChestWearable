// patientAlerts.h

#ifndef PATIENTALERTS_H
#define PATIENTALERTS_H

#define LED_ALERT_PIN       7
#define SPEAKER_ALERT_PIN   27

#define BLINK_DURATION        300
#define BEEP_DURATION         600

/*typedef struct PatientAlerts {
    // members
} PatientAlerts;*/

// Public
int patientAlertsInit(/*PatientAlerts * this*/);

// Private


#endif //PATIENTALERTS_H