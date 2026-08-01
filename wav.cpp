// WAV file header structures
#pragma pack(push, 1)
struct WavHeader
{
 // RIFF Header
 char riff[4];        // "RIFF"
 uint32_t fileSize;   // File size - 8
 char wave[4];        // "WAVE"
 
 // fmt chunk
 char fmt[4];         // "fmt "
 uint32_t fmtSize;    // 16 for PCM
 uint16_t audioFormat;// 1 for PCM
 uint16_t numChannels;// 1 for mono, 2 for stereo
 uint32_t sampleRate; // 44100
 uint32_t byteRate;   // sampleRate * numChannels * bitsPerSample/8
 uint16_t blockAlign; // numChannels * bitsPerSample/8
 uint16_t bitsPerSample; // 16
 
 // data chunk
 char data[4];        // "data"
 uint32_t dataSize;   // numSamples * numChannels * bitsPerSample/8
};
#pragma pack(pop)

// Create WAV file in memory from expression
bool calculator::CreateWav(const char *sexpr, const char *svar, 
                           float__t vfrom, float__t vto, 
                           calculator *child, value &res)
{
 const uint32_t SAMPLE_RATE = 44100;
 const uint16_t BITS_PER_SAMPLE = 16;
 const uint16_t NUM_CHANNELS = 1; // mono
 
 // Calculate number of samples
 float__t duration = vto - vfrom; // duration in seconds
 if (duration <= 0)
  {
   errorf(pos, "Invalid time range for WAV generation");
   return false;
  }
 
 uint32_t numSamples = (uint32_t)(duration * SAMPLE_RATE);
 if (numSamples == 0)
  {
   errorf(pos, "Duration too short for WAV generation");
   return false;
  }
 
 // Allocate memory for WAV file
 uint32_t dataSize = numSamples * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
 uint32_t fileSize = sizeof(WavHeader) + dataSize;
 
 char *wavData = (char *)malloc(fileSize);
 if (!wavData)
  {
   errorf(pos, "Out of memory for WAV generation");
   return false;
  }
 
 // Initialize WAV header
 WavHeader *header = (WavHeader *)wavData;
 memcpy(header->riff, "RIFF", 4);
 header->fileSize = fileSize - 8;
 memcpy(header->wave, "WAVE", 4);
 memcpy(header->fmt, "fmt ", 4);
 header->fmtSize = 16;
 header->audioFormat = 1; // PCM
 header->numChannels = NUM_CHANNELS;
 header->sampleRate = SAMPLE_RATE;
 header->bitsPerSample = BITS_PER_SAMPLE;
 header->byteRate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 header->blockAlign = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
 memcpy(header->data, "data", 4);
 header->dataSize = dataSize;
 
 int16_t *samples = (int16_t *)(wavData + sizeof(WavHeader));
 
 uint64_t init_ms = GetTickCount64();
 uint64_t last_gui_check = 0;
 
 float__t maxAmplitude = 0.0;
 float__t save_vfrom = vfrom;
 
 // First pass: find maximum amplitude (sample every 2nd point for speed - 22kHz)
 float__t step_pass1 = duration / (numSamples / 2);
 float__t t = vfrom;
 
 for (uint32_t i = 0; i < numSamples / 2; i++)
  {
   if (check_break(init_ms, last_gui_check) != brNONE)
    {
     free(wavData);
     return false;
    }
   
   child->addfvar(svar, t);
   float__t fvx = child->evaluate_f(sexpr);
   
   if (isnan(fvx) && child->errt() == teSyntax)
    {
     errorf(pos, "%s", child->err);
     free(wavData);
     return false;
    }
   
   // Check if result is real (not complex)
   if (!isnan(fvx) && isChildResReal(child))
    {
     float__t absVal = fabsl(fvx);
     if (absVal > maxAmplitude)
      maxAmplitude = absVal;
    }
   // TODO: Check if result goes into complex plane for future stereo WAV support
   
   t += step_pass1;
  }
 
 // Avoid division by zero
 if (maxAmplitude == 0.0)
  maxAmplitude = 1.0;
 
 // Second pass: generate samples at full 44100 Hz
 float__t step_pass2 = duration / numSamples;
 t = save_vfrom;
 
 for (uint32_t i = 0; i < numSamples; i++)
  {
   if (check_break(init_ms, last_gui_check) != brNONE)
    {
     free(wavData);
     return false;
    }
   
   child->addfvar(svar, t);
   float__t fvx = child->evaluate_f(sexpr);
   
   if (isnan(fvx) && child->errt() == teSyntax)
    {
     errorf(pos, "%s", child->err);
     free(wavData);
     return false;
    }
   
   // Normalize and convert to 16-bit PCM
   int16_t sample = 0;
   if (!isnan(fvx) && isChildResReal(child))
    {
     // Normalize to [-1, 1] range, then scale to 16-bit range
     float__t normalized = fvx / maxAmplitude;
     // Clamp to [-1, 1]
     if (normalized > 1.0) normalized = 1.0;
     if (normalized < -1.0) normalized = -1.0;
     
     sample = (int16_t)(normalized * 32767.0);
    }
   
   samples[i] = sample;
   t += step_pass2;
  }
 
 // Set result
 res.tag = tvWAV;
 res.sval = wavData;
 register_mem(res.sval, ptMALLOC);
 
 return true;
}