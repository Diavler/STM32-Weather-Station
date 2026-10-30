//
// Created by Haydn on 8/13/2026.
//

#ifndef WEATHERSTATION_WS_MAIN_H
#define WEATHERSTATION_WS_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void main_ws(void);
void StartSensorTask(void *argument);
void StartRadioTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif //WEATHERSTATION_WS_MAIN_H
