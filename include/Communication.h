//
// Created by Dominic on 1/16/2022.
//

#ifndef ECVTVIEWER_COMMUNICATION_H
#define ECVTVIEWER_COMMUNICATION_H
struct Data
{
    uint32_t time;
    int16_t rwSpeed;
    // Engine
    int8_t eState;
    int16_t eSpeed;
    int16_t ePID;
    int16_t eP;
    int16_t eI;
    int16_t eD;
    // Primary
    int8_t pState;
    int32_t pEncoder;
    int16_t pLoadCellForce;
    int16_t pMotorCurrent;
    int16_t pControllerOutput;
    // Secondary
    int8_t sState;
    int32_t sEncoder;
    int16_t sLoadCellForce;
    int16_t sMotorCurrent;
    int16_t sControllerOutput;
    int16_t sEncoderPID;
    int16_t sLoadCellPID;
    int16_t sLoadCellP;
    int16_t sLoadCellI;
    int16_t sLoadCellD;
} __attribute__((packed)) data;
#endif //ECVTVIEWER_COMMUNICATION_H
