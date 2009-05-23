/*
 * The MIT License
 * Copyright (c) 2009 Gerry Stellenberg, Adam Preble
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/** @file pinproc.h
 * @brief libpinproc, P-ROC Layer 1 API (Preliminary)
 *
 */

#ifndef _PINPROC_H_
#define _PINPROC_H_

#include <stdint.h>

/** @cond */
#if defined(__WIN32__)
    #undef PR_EXPORT
    #if defined(PR_BUILDING_PR)
        #define PR_EXPORT __declspec(dllexport) extern
    #else
        #define PR_EXPORT __declspec(dllimport) extern
    #endif
#endif

#if !defined(PR_EXPORT)
    #define PR_EXPORT extern
#endif

#if !defined(PR_EXTERN_C_BEGIN)
    #if defined(__cplusplus)
        #define PR_EXTERN_C_BEGIN extern "C" {
        #define PR_EXTERN_C_END   }
    #else
        #define PR_EXTERN_C_BEGIN
        #define PR_EXTERN_C_END
    #endif
#endif
/** @endcond */

/** @cond */
PR_EXTERN_C_BEGIN
/** @endcond */

// Types

typedef int32_t bool_t; // FIXME: This needs better platform independence.

typedef int32_t PRResult; /**< See: #kPRSuccess and #kPRFailure. */
#define kPRSuccess (1)    /**< Success value for #PRResult. */
#define kPRFailure (0)    /**< Failure value for #PRResult. */

typedef void * PRHandle;     /**< Opaque type used to reference an individual P-ROC device.  Created with PRCreate() and destroyed with PRDelete().  This value is used as the first parameter to all P-ROC API function calls. */
#define kPRHandleInvalid (0) /**< Value returned by PRCreate() on failure.  Indicates an invalid #PRHandle. */

typedef void (*PRLogCallback)(const char *text); /**< Function pointer type for a custom logging callback.  See: PRLogSetCallback(). */
PR_EXPORT void PRLogSetCallback(PRLogCallback callback); /**< Replaces the default logging handler with the given callback function. */

typedef enum PRMachineType {
    kPRMachineInvalid = 0,
    kPRMachineCustom = 1,
    kPRMachineWPC = 2,
    kPRMachineStern = 3, // May be split into kPRMachineWhitestar and kPRMachineSAM.
} PRMachineType;

// PRHandle Creation and Deletion

PR_EXPORT PRHandle PRCreate(PRMachineType machineType); /**< Create a new P-ROC device handle.  Only one handle per device may be created. This handle must be destroyed with PRDelete() when it is no longer needed.  Returns #kPRHandleInvalid if an error occurred. */
PR_EXPORT void PRDelete(PRHandle handle);               /**< Destroys an existing P-ROC device handle. */


// Events
// Closed == 0, Open == 1
typedef enum PREventType {
    kPREventTypeInvalid = 0,
    kPREventTypeSwitchClosedDebounced    = 1, /**< The switch has gone from open to closed and the signal has been debounced. */
    kPREventTypeSwitchOpenDebounced      = 2, /**< The switch has gone from closed to open and the signal has been debounced. */
    kPREventTypeSwitchClosedNondebounced = 3, /**< The switch has gone from open to closed and the signal has not been debounced. */
    kPREventTypeSwitchOpenNondebounced   = 4, /**< The switch has gone from closed to open and the signal has not been debounced. */
    kPREventTypetLast = kPREventTypeSwitchOpenNondebounced
} PREventType;

typedef struct PREvent {
    PREventType type;  /**< The type of event that has occurred.  Usually a switch event at this point. */
    uint32_t value;    /**< For switch events, the switch number that has changed. */
    uint32_t time;     /**< Time (in milliseconds) that this event occurred. */
} PREvent;

/** Get all of the available events that have been received. */
PR_EXPORT int PRGetEvents(PRHandle handle, PREvent *eventsOut, int maxEvents);


// Drivers

typedef struct PRDriverGlobalConfig {
    bool_t enableOutputs; // Formerly enable_direct_outputs
    bool_t globalPolarity;
    bool_t useClear;
    bool_t strobeStartSelect;
    uint8_t startStrobeTime;
    uint8_t matrixRowEnableIndex1;
    uint8_t matrixRowEnableIndex0;
    bool_t activeLowMatrixRows;
    bool_t encodeEnables;
    bool_t tickleSternWatchdog;
    bool_t watchdogExpired;
    bool_t watchdogEnable;
    uint16_t watchdogResetTime;
} PRDriverGlobalConfig;

typedef struct PRDriverGroupConfig {
    uint8_t groupNum;
    uint16_t slowTime;
    uint8_t enableIndex;
    uint8_t rowActivateIndex;
    uint8_t rowEnableSelect;
    bool_t matrixed;
    bool_t polarity;
    bool_t active;
    bool_t disableStrobeAfter;
} PRDriverGroupConfig;

typedef struct PRDriverState {
    uint16_t driverNum;
    uint32_t outputDriveTime;
    bool_t polarity;
    bool_t state;
    bool_t waitForFirstTimeSlot;
    uint32_t timeslots;
    uint8_t patterOnTime;
    uint8_t patterOffTime;
    bool_t patterEnable;
} PRDriverState;

/** Update registers for the global driver configuration. */
PR_EXPORT PRResult PRDriverUpdateGlobalConfig(PRHandle handle, PRDriverGlobalConfig *driverGlobalConfig);

PR_EXPORT PRResult PRDriverGetGroupConfig(PRHandle handle, uint8_t groupNum, PRDriverGroupConfig *driverGroupConfig);
/** Update registers for the given driver group configuration. */
PR_EXPORT PRResult PRDriverUpdateGroupConfig(PRHandle handle, PRDriverGroupConfig *driverGroupConfig);

PR_EXPORT PRResult PRDriverGetState(PRHandle handle, uint8_t driverNum, PRDriverState *driverState);
/** Sets the state of the given driver (lamp, coil, etc.). */
PR_EXPORT PRResult PRDriverUpdateState(PRHandle handle, PRDriverState *driverState);

// Driver Helper functions:

/** Disables (turns off) the given driver. */
PR_EXPORT PRResult PRDriverDisable(PRHandle handle, uint16_t driverNum);
/** Pulses the given driver for a number of milliseconds. */
PR_EXPORT PRResult PRDriverPulse(PRHandle handle, uint16_t driverNum, int milliseconds);
/** Assigns a repeating schedule to the given driver. */
PR_EXPORT PRResult PRDriverSchedule(PRHandle handle, uint16_t driverNum, uint32_t schedule, uint8_t cycleSeconds, bool_t now);
/** Assigns a pitter-patter schedule (repeating on/off) to the given driver. */
PR_EXPORT PRResult PRDriverPatter(PRHandle handle, uint16_t driverNum, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t originalOnTime);
/** Tickle the watchdog timer. */
PR_EXPORT PRResult PRDriverWatchdogTickle(PRHandle handle);

/** Disables (turns off) the given driver. */
PR_EXPORT void PRDriverStateDisable(PRDriverState *driverState);
/** Pulses the given driver for a number of milliseconds. */
PR_EXPORT void PRDriverStatePulse(PRDriverState *driverState, int milliseconds);
/** Assigns a repeating schedule to the given driver. */
PR_EXPORT void PRDriverStateSchedule(PRDriverState *driverState, uint32_t schedule, uint8_t cycleSeconds, bool_t now);
/** Assigns a pitter-patter schedule (repeating on/off) to the given driver. */
PR_EXPORT void PRDriverStatePatter(PRDriverState *driverState, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t originalOnTime);



// Switches

/** @defgroup switchconsts Switch Constants
 * @{
 */
#define kPRSwitchPhysicalFirst (0)   /**< Switch number of the first physical switch. */
#define kPRSwitchPhysicalLast (223)  /**< Switch number of the last physical switch.  */
#define kPRSwitchVirtualFirst (224)  /**< Switch number of the first virtual switch.  */
#define kPRSwitchVirtualLast (255)   /**< Switch number of the last virtual switch.   */
/** @} */

typedef struct PRSwitchRule {
    bool_t notifyHost; /**< If true this switch change event will provided to the user via PRGetEvents(). */
} PRSwitchRule;

/**
 * @brief Configures the handling of switch rules within P-ROC.
 * 
 * P-ROC's switch event system allows the user to receive and act upon events specific to the individual switch's application.
 * For example, P-ROC can provide debounced switch events to software by means of the PRGetEvents() call (to create 
 * a lane change behavior).   The same switch can also be configured with a non-debounced rule to fire a flipper coil.
 * Multiple driver changes can be tied to a single switch state transition to create more complicated effects: a slingshot
 * switch that fires the slingshot coil, a flash lamp, and a score event.
 * 
 * P-ROC holds four different switch rules for each switch: closed to open and open to closed, each with a debounced and non-debounced versions:
 *  - #kPREventTypeSwitchOpenDebounced
 *  - #kPREventTypeSwitchClosedDebounced 
 *  - #kPREventTypeSwitchOpenNondebounced
 *  - #kPREventTypeSwitchClosedNondebounced
 * 
 * @section Examples
 * 
 * Configuring a basic switch rule with no driver state changes that will appear in PRGetEvents():
 * @code
 * PRSwitchRule rule;
 * rule.notifyHost = true;
 * PRSwitchesUpdateRule(handle, switchNum, kPREventTypeSwitchOpenDebounced, &rule, NULL, 0);
 * @endcode
 * 
 * Configuring a pop bumper switch to pulse the coil and a flash lamp:
 * @code
 * // Configure a switch rule to fire the coil and flash lamp:
 * PRSwitchRule rule;
 * rule.notifyHost = false;
 * PRDriverState drivers[2];
 * PRDriverGetState(handle, drvCoilPopBumper1, &drivers[0]);
 * PRDriverGetState(handle, drvFlashLamp1, &drivers[1]);
 * PRDriverStatePulse(&drivers[0], 50);
 * PRDriverStatePulse(&drivers[1], 50);
 * PRSwitchesUpdateRule(handle, drvSwPopBumper1, kPREventTypeSwitchClosedNondebounced, 
 *                      &rule, drivers, 2);
 * // Now configure a switch rule to process scoring in software:
 * rule.notifyHost = false;
 * PRSwitchesUpdateRule(handle, drvSwPopBumper1, kPREventTypeSwitchClosedDebounced, 
 *                      &rule, NULL, 0);
 * @endcode
 * 
 * @param handle The P-ROC device handle.
 * @param switchNum The index of the switch this configuration affects.
 * @param eventType The switch rule for the specified switchNum to be configured.
 * @param rule A pointer to the #PRSwitchRule structure describing how this state change should be handled.  May not be NULL.
 * @param linkedDrivers An array of #PRDriverState structures describing the driver state changes to be made when this switch rule is triggered.  May be NULL if numDrivers is 0.
 * @param numDrivers Number of elements in the linkedDrivers array.  May be zero or more.
 */
PR_EXPORT PRResult PRSwitchesUpdateRule(PRHandle handle, uint8_t switchNum, PREventType eventType, PRSwitchRule *rule, PRDriverState *linkedDrivers, int numDrivers);



// DMD

typedef struct PRDMDConfig {
    uint8_t numRows;
    uint16_t numColumns;
    uint8_t numSubFrames;
    bool_t enable;
    uint8_t rclkLowCycles[8];
    uint8_t latchHighCycles[8];
    uint16_t deHighCycles[8];
    uint8_t dotclkHalfPeriod[8];
} PRDMDConfig;

/** Sets the configuration registers for the DMD driver. */
PR_EXPORT int32_t PRDMDUpdateConfig(PRHandle handle, PRDMDConfig *dmdConfig);
/** Updates the DMD frame buffer with the given data. */
PR_EXPORT PRResult PRDMDDraw(PRHandle handle, uint8_t * dots);

/** @cond */
PR_EXTERN_C_END
/** @endcond */

#endif // _PINPROC_H_
