#include "Max30102Sensor.h"
#include <math.h>

Max30102Sensor::Max30102Sensor() {
  _wire = nullptr;
  _addr = 0x57;
  _fingerDetected = false;
  _bpm = 0.0f;
  _spo2 = 0.0f;
  _bufIdx = 0;
  _sampleCount = 0;
  _emaLong = 0.0f;
  _emaShort = 0.0f;
  _prev1 = _prev2 = _curr = 0;
  _lastPeakSample = 0;
  _peakIdx = 0;
  _peakCountStored = 0;
  _irThreshold = 5000; // default; tune for your hardware
  _minBeatIntervalSamples = (int)(0.30f * SAMPLE_RATE); // 300 ms
  _maxBeatIntervalSamples = (int)(2.0f * SAMPLE_RATE);  // 2000 ms
  _sampleRate = SAMPLE_RATE;
}

bool Max30102Sensor::begin(TwoWire &wireInst, uint8_t addr) {
  _wire = &wireInst;
  _addr = addr;

  // Note: Wire.begin(SDA,SCL) should be called in main before this begin()
  if (!particle.begin(*_wire, _addr)) {
    Serial.println("❌ MAX30102 not found on I2C bus!");
    return false;
  }

  Serial.println("✅ MAX30102 detected. Initializing...");

  // Use configuration recommended for SPO2 waveform capture (mirror SparkFun Example8_SPO2)
  byte ledBrightness = 0x3F;  // brightness (0..255)
  byte sampleAverage = 4;
  byte ledMode = 2;           // RED + IR
  int sampleRate = SAMPLE_RATE; // 100 Hz
  int pulseWidth = 411;
  int adcRange = 4096;

  particle.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particle.enableDIETEMPRDY();
  particle.enableFIFORollover();

  // Try safer I2C clock for some clones
  #if defined(TWBR)
  // do nothing for AVR
  #else
  // for ESP8266
  _wire->setClock(100000);
  #endif

  delay(100);

  // Initialize EMA roughly to first readings (if available)
  if (particle.getIR() > 0) {
    long initIR = particle.getIR();
    _emaLong = (float)initIR;
    _emaShort = (float)initIR;
  }

  Serial.println("✅ MAX30102 initialized successfully!");
  return true;
}

void Max30102Sensor::powerOn() {
  // Wake sensor from shutdown (SparkFun API)
  particle.wakeUp();
}

void Max30102Sensor::powerOff() {
  // Put sensor in low-power shutdown
  particle.shutDown();
}

void Max30102Sensor::reset() {
  _fingerDetected = false;
  _bpm = 0.0f;
  _spo2 = 0.0f;
  _bufIdx = 0;
  _sampleCount = 0;
  _emaLong = 0.0f;
  _emaShort = 0.0f;
  _prev1 = _prev2 = _curr = 0;
  _lastPeakSample = 0;
  _peakIdx = 0;
  _peakCountStored = 0;
}

void Max30102Sensor::pushSample(uint32_t ir, uint32_t red) {
  _irBuf[_bufIdx] = ir;
  _redBuf[_bufIdx] = red;
  _bufIdx++;
  if (_bufIdx >= BUF_MAX) _bufIdx = 0;
  _sampleCount++;
}

int Max30102Sensor::computeFiltered(int raw) {
  // DC removal with long EMA + short EMA smoothing
  // long EMA alpha ~ 1/ (sampleRate * 0.8s) -> slow baseline
  float alphaLong = 1.0f / (float)(_sampleRate * 0.75f); // ~1/75 for 100Hz
  float alphaShort = 0.2f; // smoothing

  if (_emaLong == 0.0f) _emaLong = raw;
  _emaLong = (_emaLong * (1.0f - alphaLong)) + ((float)raw * alphaLong);

  if (_emaShort == 0.0f) _emaShort = raw;
  _emaShort = (_emaShort * (1.0f - alphaShort)) + ((float)raw * alphaShort);

  float filtered = _emaShort - _emaLong; // remove slow baseline
  // scale to int for peak detection
  int f = (int)filtered;
  return f;
}

bool Max30102Sensor::detectPeak(int filteredValue) {
  // We detect a peak when prev1 is local maximum (prev1 > prev2 && prev1 > curr) and above dynamic threshold.
  // Shift window
  _prev2 = _prev1;
  _prev1 = _curr;
  _curr = filteredValue;

  // Need at least 3 samples history
  if ((_prev1 > _prev2) && (_prev1 > _curr)) {
    // dynamic threshold based on signal amplitude
    int amplitude = abs(_prev1);
    int dynamicThreshold = max(200, amplitude / 3); // protect from noise
    if ((_prev1 > dynamicThreshold) && (_sampleCount - _lastPeakSample) > (uint32_t)_minBeatIntervalSamples) {
      // Detected peak at sample index (_sampleCount -1)
      return true;
    }
  }
  return false;
}

void Max30102Sensor::recordPeak(uint32_t sampleIndex) {
  if (_lastPeakSample != 0) {
    uint32_t interval = sampleIndex - _lastPeakSample; // in samples
    if (interval < (uint32_t)_minBeatIntervalSamples || interval > (uint32_t)_maxBeatIntervalSamples) {
      // reject improbable intervals
    } else {
      _peakIntervals[_peakIdx] = interval;
      _peakIdx = (_peakIdx + 1) % (int)(sizeof(_peakIntervals)/sizeof(_peakIntervals[0]));
      if (_peakCountStored < (int)(sizeof(_peakIntervals)/sizeof(_peakIntervals[0])))
        _peakCountStored++;
    }
  }
  _lastPeakSample = sampleIndex;
}

float Max30102Sensor::computeBPMFromIntervals() const {
  if (_peakCountStored == 0) return 0.0f;
  uint32_t sum = 0;
  for (int i = 0; i < _peakCountStored; ++i) sum += _peakIntervals[i];
  float avgSamples = (float)sum / (float)_peakCountStored;
  if (avgSamples <= 0.0f) return 0.0f;
  float bpm = 60.0f * (float)_sampleRate / avgSamples;
  return bpm;
}

float Max30102Sensor::computeSpO2Simple() {
  // compute simple ratio-of-RMS over last N samples (use up to 100 samples)
  int n = 100;
  if (_sampleCount < (uint32_t)n) n = _sampleCount < BUF_MAX ? (int)_sampleCount : n;
  if (n < 10) return _spo2; // not enough data

  // build arrays from circular buffer: take newest n samples
  int idx = (_bufIdx - 1 + BUF_MAX) % BUF_MAX;
  double meanIr = 0.0, meanRed = 0.0;
  // compute DC mean
  for (int i = 0; i < n; ++i) {
    uint32_t vIr = _irBuf[(idx - i + BUF_MAX) % BUF_MAX];
    uint32_t vRed = _redBuf[(idx - i + BUF_MAX) % BUF_MAX];
    meanIr += (double)vIr;
    meanRed += (double)vRed;
  }
  meanIr /= n; meanRed /= n;
  // compute AC RMS
  double rmsIr = 0.0, rmsRed = 0.0;
  for (int i = 0; i < n; ++i) {
    double vIr = (double)_irBuf[(idx - i + BUF_MAX) % BUF_MAX] - meanIr;
    double vRed = (double)_redBuf[(idx - i + BUF_MAX) % BUF_MAX] - meanRed;
    rmsIr += vIr * vIr;
    rmsRed += vRed * vRed;
  }
  rmsIr = sqrt(rmsIr / n);
  rmsRed = sqrt(rmsRed / n);

  if (rmsIr <= 0.00001) return _spo2;
  double R = (rmsRed / rmsIr);
  double spo2 = 110.0 - 25.0 * R;
  if (spo2 > 100.0) spo2 = 100.0;
  if (spo2 < 50.0) spo2 = 50.0;
  return (float)spo2;
}

void Max30102Sensor::update() {
  // Read available FIFO samples if SparkFun library supports available()
  // Fallback: call getIR()/getRed() once per update if available() not implemented
  // We'll try to read until FIFO empty (safe) and process each sample
  bool anyRead = false;
  // Some SparkFun libs provide particle.available(); if not, just read once per call
  // Use try/catch style (but in C++ we just attempt reading)
  // We'll read up to a safety cap to avoid long blocking
  const int SAFETY_READ_CAP = 50;
  int reads = 0;

  while (reads < SAFETY_READ_CAP) {
    // check if sample available: SparkFun has available(), but some forks don't
    bool hasSample = false;
    // try calling available() if exists - since it's compiled it will exist in the library
    hasSample = particle.available(); // returns number of samples in many libs; treat nonzero as available
    if (!hasSample) {
      // try reading one set using getIR(); if returns 0 maybe no sample
      /* probe one read if needed; skip storing to avoid unused var */
      (void)particle.getIR();
      // if test is zero and FIFO empty, break (but zero could be real)
      // we'll assume if available() == 0 then break
      break;
    }

    // If available() reports samples, read them
    long ir = particle.getIR();
    long red = particle.getRed();

    pushSample((uint32_t)ir, (uint32_t)red);

    // compute filtered value for peak detection
    int filtered = computeFiltered((int)ir);

    // finger detection based on raw IR amplitude
    _fingerDetected = ((uint32_t)ir > _irThreshold);

    // detect peak using three-sample window (prev2, prev1, curr)
    if (detectPeak(filtered)) {
      uint32_t peakSampleIdx = _sampleCount; // current sample count; peak happened at sampleCount-1
      recordPeak(peakSampleIdx - 1);
      // update BPM
      _bpm = computeBPMFromIntervals();
      // keep BPM reasonable
      if (_bpm < 30.0f || _bpm > 220.0f) { /* ignore spikes */ }
    }

    // update spo2 every loop (simple)
    _spo2 = computeSpO2Simple();

    anyRead = true;
    reads++;
    // avoid starving watchdog
    if ((reads & 0xF) == 0) yield();
  }

  // If library's available() returned 0, we may still want to poll once (non-blocking)
  if (!anyRead) {
    // single sample read (non-blocking)
    long ir = particle.getIR();
    long red = particle.getRed();
    pushSample((uint32_t)ir, (uint32_t)red);
    int filtered = computeFiltered((int)ir);
    _fingerDetected = ((uint32_t)ir > _irThreshold);
    if (detectPeak(filtered)) {
      uint32_t peakSampleIdx = _sampleCount;
      recordPeak(peakSampleIdx - 1);
      _bpm = computeBPMFromIntervals();
    }
    _spo2 = computeSpO2Simple();
  }

  // Output for Serial Plotter (CSV) and Teleplot style
  if (_fingerDetected) {
    // Teleplot friendly:
    Serial.printf(">IR:%lu\n>RED:%lu\n>BPM:%.1f\n>SpO2:%.1f\n", (unsigned long)_irBuf[(_bufIdx - 1 + BUF_MAX) % BUF_MAX],
                                                      (unsigned long)_redBuf[(_bufIdx - 1 + BUF_MAX) % BUF_MAX],
                                                      _bpm, _spo2);
    // CSV (IR,RED,BPM,SpO2)
    Serial.printf("%lu,%lu,%.1f,%.1f\n",
                  (unsigned long)_irBuf[(_bufIdx - 1 + BUF_MAX) % BUF_MAX],
                  (unsigned long)_redBuf[(_bufIdx - 1 + BUF_MAX) % BUF_MAX],
                  _bpm, _spo2);
  } else {
    // no finger: print zeros for plotter/CSV
    Serial.println("0,0,0,0");
  }

  // allow background tasks on ESP8266
  delay(0);
}
