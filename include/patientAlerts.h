// patientAlerts.h

#ifndef PATIENTALERTS_H
#define PATIENTALERTS_H

#define ALERT_LED_PIN       7
#define ALERT_SPEAKER_PIN   0

typedef struct PatientAlerts {
    // members
} PatientAlerts;

// Public
void patientAlertsInit(PatientAlerts * this);

// Private


#endif //PATIENTALERTS_H