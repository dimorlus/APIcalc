uint8_t thermo(uint8_t adc)
{
 const uint8_t Toffs = {
  139,  133,  128,  122,  117,  112,  108,  103,  99,  94,  90,  85,  81,  76,  71,  66,  61,  54,  48,  39,  29  };
 const uint8_t Tslop = {
  48,  45,  42,  40,  38,  37,  36,  36,  35,  35,  36,  36,  38,  39,  41,  44,  49,  55,  66,  84,  127  };
 
  uint8_t i, offset, slope;
  adc=(adc<80)?0:adc-80;
  i = adc / 8;
  offset = Toffs[i];
  slope = Tslop[i];
  return (uint8_t)(offset - ((uint16_t)(adc % 8)) * slope / 64);
}
 
