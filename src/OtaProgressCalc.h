#pragma once
#include <stdint.h>
#include <stddef.h>

// Calculates OTA firmware update progress as a percentage [0..100].
// Returns 0 when size == 0 to avoid division by zero during update
// initialisation or after a failed start.
inline uint8_t calcOtaProgress(size_t size, size_t done)
{
  if (size == 0u) return 0u;
  return (uint8_t)((100.0f / (float)size) * (float)done);
}
