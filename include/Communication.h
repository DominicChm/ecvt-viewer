//
// Created by Dominic on 1/16/2022.
//

#ifndef ECVTVIEWER_COMMUNICATION_H
#define ECVTVIEWER_COMMUNICATION_H
struct Data
{
    uint32_t time;
    // Engine
    bool engaged;
    int8_t eState;
    int16_t eSpeed;
    int16_t ePID;
    int16_t eP;
    int16_t eI;
    int16_t eD;
    // Primary
    int8_t pState;
    int32_t pEnc;
    int16_t pLC;
    int16_t pPID;
    // Secondary
    int8_t sState;
    int32_t sEnc;
    int16_t sLC;
    int16_t sPID;
} __attribute__((packed));
#endif //ECVTVIEWER_COMMUNICATION_H
