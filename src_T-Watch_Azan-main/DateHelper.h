uint8_t getWeekDay( uint8_t day, uint8_t month, uint16_t year )
{
  if ( month <= 2 )
  {
    month += 12;
    --year;
  }

  uint8_t j = year % 100;
  uint8_t e = year / 100;
  return ( ( ( day + ( ( ( month + 1 ) * 26 ) / 10 ) + j + ( j / 4 ) + ( e / 4 ) - ( 2 * e ) ) - 2 ) % 7 );
}

bool isLeapYear( const uint16_t year )
{
  return ( (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0) );
}

uint8_t getDaysInMonth( const uint8_t month, const uint16_t year )
{
  if ( month == 2 )  
    return isLeapYear( year ) ? 29 : 28;
    
  else if ( month == 4 || month == 6 || month == 9 || month == 11 )  
      return 30;
      
  return 31;
}

uint16_t getDaysInYear( const uint16_t year )
{
  return isLeapYear( year ) ? 366 : 365;
}

uint8_t getLastSundayInMonth( const uint8_t month, const uint16_t year )
{
  uint8_t d = getDaysInMonth( month, year );
  while ( getWeekDay( --d, month, year ) != 6 );
  return d;
}

uint16_t getDayOfYear( const uint8_t day, const uint8_t month, const uint16_t year )
{
  uint16_t d = day;
  uint8_t m = month;
  while ( --m ) d += getDaysInMonth( m, year );
  return d;
}
