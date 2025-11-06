#include <Arduino.h>
#include <Wire.h>

#include "buffer_log.h"
#include "magic.h"

#pragma pack(push, 1)
struct InputReadings
{
    uint16_t tuner;
    uint16_t aKnob;
    uint16_t bKnob;
    uint16_t cKnob;
    uint8_t swtch;
};
#pragma pack(pop)

static const int recvBufSz = 8;
static const int sendBufSz = 10;

static volatile uint8_t recvBuf[recvBufSz];
static volatile int recvBufPtr = 0;
static volatile uint8_t sendBuf[sendBufSz];
static volatile int sendBytes = 0;
static volatile BufferLog aKnobLog;
static volatile BufferLog bKnobLog;
static volatile BufferLog cKnobLog;
static volatile BufferLog tunerLog;
static volatile uint8_t swtch;

static void onWireReceive(int bytecount);
static void onWireRequest();
static void handleCommand(uint8_t b);

void setup()
{
    // Onboard LED
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN, HIGH);

    // Device inputs
    analogReference(VDD);
    pinMode(KNOB_A_PIN, INPUT);
    pinMode(KNOB_B_PIN, INPUT);
    pinMode(KNOB_C_PIN, INPUT);
    pinMode(TUNER_PIN, INPUT);
    pinMode(KNOB_SWITCH_PIN, INPUT);

    // Device outputs
    pinMode(LIGHT_CTRL_PIN, OUTPUT);
    // digitalWrite(LIGHT_CTRL_PIN, HIGH);

    // CPU clock 10MHz: prescaler enabled, DIV2
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;

    // Timer B, 250 times per second
    TCB0.CTRLA = 0;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.CCMP = (10000000UL / MEASURE_FREQ) - 1;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;

    // I2C slave
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(onWireReceive);
    Wire.onRequest(onWireRequest);
}

ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm;

    aKnobLog.logValue(analogRead(KNOB_A_PIN));
    bKnobLog.logValue(analogRead(KNOB_B_PIN));
    cKnobLog.logValue(analogRead(KNOB_C_PIN));
    tunerLog.logValue(analogRead(TUNER_PIN));

    pinMode(KNOB_SWITCH_PIN, OUTPUT);
    digitalWrite(KNOB_SWITCH_PIN, HIGH);
    pinMode(KNOB_SWITCH_PIN, INPUT);
    for (swtch = 0; swtch < 16; ++swtch)
    {
        if (digitalRead(KNOB_SWITCH_PIN) == LOW) break;
    }
}

static void onWireReceive(int byteCount)
{
    while (byteCount > 0)
    {
        int dd = Wire.read();
        if (dd == -1) break;
        if (recvBufPtr == recvBufSz) continue;
        recvBuf[recvBufPtr++] = (uint8_t)dd;
    }
}

static void onWireRequest()
{
    Wire.write((const uint8_t *)sendBuf, sendBytes);
}

void loop()
{
    uint8_t buf[recvBufSz];
    int nBytes;
    cli();
    nBytes = recvBufPtr;
    memcpy(buf, (const void *)recvBuf, nBytes);
    recvBufPtr = 0;
    sei();

    for (int i = 0; i < nBytes; ++i)
    {
        handleCommand(buf[i]);
    }
}

void handleCommand(uint8_t cmd)
{
    if (cmd == 0x00)
    {
        InputReadings data;
        cli();
        data.tuner = tunerLog.getAvg();
        data.aKnob = aKnobLog.getAvg();
        data.bKnob = bKnobLog.getAvg();
        data.cKnob = cKnobLog.getAvg();
        data.swtch = swtch;
        sendBytes = sizeof(InputReadings);
        if (sendBufSz < sendBytes) sendBytes = sendBufSz;
        memcpy((void *)sendBuf, &data, sendBytes);
        sei();
    }
    else if (cmd == 0x10)
    {
        digitalWrite(LIGHT_CTRL_PIN, LOW);
    }
    else if (cmd == 0x11)
    {
        digitalWrite(LIGHT_CTRL_PIN, HIGH);
    }
}
