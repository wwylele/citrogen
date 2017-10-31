#pragma once

#include <QString>

template <typename T> QString ToHex(T value) {
  return QString("%1")
      .arg(value, sizeof(T) * 2, 16, QLatin1Char('0'))
      .toUpper();
}

template <> inline QString ToHex<byte>(byte value) { return ToHex((u8)value); }

inline int DigitFromHex(QChar c) {
  if (c.isDigit())
    return c.digitValue();

  char a = c.toUpper().toLatin1();
  a -= 'A';
  if (a < 0 || a >= 6)
    return -1;
  return a + 10;
}