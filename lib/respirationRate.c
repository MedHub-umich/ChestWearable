// respirationRate.c

#include "respirationRate.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


void respirationRateInit(respirationRate_t * this)
{
    this->numPeaks = 0;
}

void respirationRateAddPair(float32_t inMagnitude, float32_t inTime, respirationRate_t * this)
{
    peakPair_t newPeak;
    newPeak.magnitude = inMagnitude;
    newPeak.time = inTime;
    if(this->numPeaks >= PEAKS_SIZE)
    {
        NRF_LOG_ERROR("BAADDD: too many peaks, BUFFER OVERFFLOOW ATTACK");
    }
    this->peaks[this->numPeaks] = newPeak;
    this->numPeaks++;

    if(this->numPeaks >= PEAKS_SIZE)
    {
        respirationRateProcess(this);
    }
}

void respirationRateProcess(respirationRate_t * this)
{
    int sum = 0, average = 0;
    int i;
    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        sum += this->peaks[i].magnitude;
    }
    average = sum / PEAKS_SIZE;
    NRF_LOG_INFO("average: %d", average);

    int isAbove, crossings = 0;

    if(this->peaks[0].magnitude > average)
    {
        isAbove = 1;
    }
    else
    {
        isAbove = 0;
    }

    for(i = 0; i < PEAKS_SIZE; ++i)
    {
        if (this->peaks[i].magnitude > average && isAbove == 0)
        {
            isAbove = 1;
            crossings++;
        }
        else if (this->peaks[i].magnitude > average && isAbove == 1)
        {
            // Nothing
        }
        else if (this->peaks[i].magnitude < average && isAbove == 1)
        {
            isAbove = 0;
            crossings++;
        }
        else // (this->peaks[i] < average && isAbove = 0)
        {
            // Nothing
        }
    }

    NRF_LOG_INFO("crossings: %d", crossings);
    NRF_LOG_INFO("breaths: %d", crossings/2);

    this->numPeaks = 0;
}
