
/* Заголовочный файл для GenericHID.c */

#ifndef _GENERICHID_H_
#define _GENERICHID_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "Descriptors.h"
#include "Config/AppConfig.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

void hardwareInit(void);
void HIDTask(void);

void deviceConnect(void);
void deviceDisconnect(void);
void deviceConfigurationChanged(void);
void deviceControlRequest(void);

void processGenericHIDReport(uint8_t* DataArray);
void createGenericHIDReport(uint8_t* DataArray);

#endif

