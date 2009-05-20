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
#define kPRSuccess (1) /**< Success value for #PRResult. */
#define kPRFailure (0) /**< Failure value for #PRResult. */

typedef void * PRHandle; /**< Opaque type used to reference an individual P-ROC device.  Created with PRCreate() and destroyed with PRDelete().  This value is used as the first parameter to all P-ROC API function calls. */
#define kPRHandleInvalid (0)	/**< Value returned by PRCreate() on failure.  Indicates an invalid #PRHandle. */
	
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
	PREventType type;	/**< The type of event that has occurred.  Usually a switch event at this point. */
	uint32_t value;		/**< For switch events, the switch number that has changed. */
	uint32_t time;		/**< Time (in milliseconds) that this event occurred. */
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
	bool_t tickleWatchdog;
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

PR_EXPORT PRResult PRDriverPatter(PRHandle handle, uint16_t driverNum, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t originalOnTime);



// Switches

/** @defgroup switchconsts Switch Constants 
 * @{
 */
#define kPRSwitchPhysicalFirst (0)	/**< Switch number of the first physical switch. */
#define kPRSwitchPhysicalLast (223)  /**< Switch number of the last physical switch.  */
#define kPRSwitchVirtualFirst (224)  /**< Switch number of the first virtual switch.  */
#define kPRSwitchVirtualLast (255)	/**< Switch number of the last virtual switch.   */
/** @} */

typedef struct PRSwitchRule {
	uint16_t switchNum;	/**< Number of the physical switch, or for linked driver changes the virtual switch number (224 and up). */
  	PREventType eventType;	/**< The event type that this rule generates.  Determines closed/open, debounced/non-debounced. */
	bool_t notifyHost;
	bool_t changeOutput;   /**< True if this switch rule should affect a driver output change. */
	bool_t linkActive;		/**< True if this switch rule has additional linked driver updates. */
	uint32_t linkAddress;  /**< Switch number of the linked driver update. */
	PRDriverState driver;   /**< Driver state change to affect once this rule is triggered. */
} PRSwitchRule;

/** Updates the rules for the given switch and PREventType combinations. */
PR_EXPORT PRResult PRSwitchesUpdateRules(PRHandle handle, PRSwitchRule *rules, int numRules);



// DMD

typedef struct PRDMDGlobalConfig {
	uint8_t numRows;
	uint16_t numColumns;
	uint8_t numSubFrames;
	uint16_t cyclesPerRow;
	bool_t enable; 
	uint8_t rclkLowCycles[8];
	uint8_t latchHighCycles[8];
	uint16_t deHighCycles[8];
	uint8_t dotclkHalfPeriod[8];
} PRDMDConfig;

/** Sets the configuration registers for the DMD driver. */
PR_EXPORT int32_t PRDMDUpdateGlobalConfig(PRHandle handle, PRDMDGlobalConfig *dmdGlobalConfig);
/** Updates the DMD frame buffer with the given data. */
PR_EXPORT PRResult PRDMDDraw(PRHandle handle, uint8_t * dots, uint16_t columns, uint8_t rows, uint8_t numSubFrames);

/** @cond */
PR_EXTERN_C_END
/** @endcond */

#endif // _PINPROC_H_
