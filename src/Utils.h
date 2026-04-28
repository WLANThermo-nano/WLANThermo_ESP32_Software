#pragma once

// Bounds-safe strncpy: copies at most sizeof(dst)-1 chars, always null-terminates.
// No-op if src is nullptr (ArduinoJson v7 .as<const char*>() can return nullptr).
// dst must be a fixed-size char array (sizeof must be resolvable at compile time).
#define SAFE_STRNCPY(dst, src) \
  do { if (src) { strncpy((dst), (src), sizeof(dst) - 1u); (dst)[sizeof(dst) - 1u] = '\0'; } } while (0)
