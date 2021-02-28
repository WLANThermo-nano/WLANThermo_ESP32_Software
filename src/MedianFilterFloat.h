#include "MedianFilterLib.h"

#pragma once

#define MEDIAN_FLOAT_OFFSET 1000

class MedianFilterFloat
{
public:
    MedianFilterFloat(const size_t windowSize)
    {
        medianFilter = new MedianFilter<float>(windowSize);
    };
    
    void addValue(float value)
    {
        medianFilter->AddValue(value + ((float)(MEDIAN_FLOAT_OFFSET)));
    }

    float getFiltered()
    {
        return medianFilter->GetFiltered() - ((float)(MEDIAN_FLOAT_OFFSET));
    }
    
private:
    MedianFilter<float> *medianFilter;
};
