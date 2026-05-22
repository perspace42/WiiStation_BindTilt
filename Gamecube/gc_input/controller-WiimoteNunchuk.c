/**
 * WiiSX - controller-WiimoteNunchuk.c
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009, 2010 sepp256
 * 
 * Wiimote + Nunchuk input module
 *
 * WiiSX homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#ifdef HW_RVL

#include <string.h>
#include <math.h>
#include <wiiuse/wpad.h>
#include "controller.h"
#include "../wiiSXconfig.h"

#ifndef PI
#define PI 3.14159f
#endif

/*
 * Local shorthands that alias the canonical defaults from controller.h.
 * Using shorthands keeps the existing code readable while ensuring
 * there is only one source of truth for the numeric values.
 */
#define TILT_DEADZONE_DEG  TILT_DEADZONE_DEG_DEFAULT
#define TILT_MAX_DEG       TILT_MAX_DEG_DEFAULT

enum { STICK_X, STICK_Y };
static int getStickValue(joystick_t* j, int axis, int maxAbsValue){
	float angle = PI * j->ang/180.0f;
	float magnitude = (j->mag > 1.0f) ? 1.0f :
	                    (j->mag < -1.0f) ? -1.0f : j->mag;
	float value;
	if(axis == STICK_X)
		value = magnitude * sin( angle );
	else
		value = magnitude * cos( angle );
	return (int)(value * maxAbsValue);
}

enum {
	NUNCHUK_AS_ANALOG, IR_AS_ANALOG,
	TILT_AS_ANALOG, WHEEL_AS_ANALOG,
	/*
	 * TILT_STEERING_AS_ANALOG
	 * Optimised for kart / racing games.
	 * Maps Wiimote roll (side-tilt) to the left-stick X axis with:
	 *   - A TILT_DEADZONE_DEG dead-zone around centre
	 *   - A square-root sensitivity curve (progressive near centre)
	 *   - Pitch axis forced to 0 (pure left/right steering only)
	 * Kept separate from TILT_AS_ANALOG so existing profiles are
	 * unaffected and users can still choose the legacy tilt mode.
	 */
	TILT_STEERING_AS_ANALOG,
	NO_ANALOG,
};

enum {
	NUNCHUK_L = 0x10 << 16,
	NUNCHUK_R = 0x20 << 16,
	NUNCHUK_U = 0x40 << 16,
	NUNCHUK_D = 0x80 << 16,
};

#define NUM_WIIMOTE_BUTTONS 12
static button_t buttons[] = {
	{  0, ~0,                    "None" },
	{  1, WPAD_BUTTON_UP,        "D-Up" },
	{  2, WPAD_BUTTON_LEFT,      "D-Left" },
	{  3, WPAD_BUTTON_RIGHT,     "D-Right" },
	{  4, WPAD_BUTTON_DOWN,      "D-Down" },
	{  5, WPAD_BUTTON_A,         "A" },
	{  6, WPAD_BUTTON_B,         "B" },
	{  7, WPAD_BUTTON_PLUS,      "+" },
	{  8, WPAD_BUTTON_MINUS,     "-" },
	{  9, WPAD_BUTTON_HOME,      "Home" },
	{ 10, WPAD_BUTTON_1,         "1" },
	{ 11, WPAD_BUTTON_2,         "2" },
	{ 12, WPAD_NUNCHUK_BUTTON_C, "C" },
	{ 13, WPAD_NUNCHUK_BUTTON_Z, "Z" },
	{ 14, NUNCHUK_U,             "NC-Up" },
	{ 15, NUNCHUK_L,             "NC-Left" },
	{ 16, NUNCHUK_R,             "NC-Right" },
	{ 17, NUNCHUK_D,             "NC-Down" },
};

/*
 * Analog source tables control what options appear in the
 * Configure Buttons menu under "Analog Stick L/R".
 * Entries are saved/loaded by their array index, so appending
 * new entries here is safe — existing saved configs keep working.
 */
static button_t analog_sources_wmn[] = {
	{ 0, NUNCHUK_AS_ANALOG,         "Nunchuk" },
	{ 1, IR_AS_ANALOG,              "IR" },
	/* New: steering-optimised tilt available even with Nunchuk attached */
	{ 2, TILT_STEERING_AS_ANALOG,   "Tilt Steering" },
};

static button_t analog_sources_wm[] = {
	{ 0, TILT_AS_ANALOG,            "Tilt" },
	{ 1, WHEEL_AS_ANALOG,           "Wheel" },
	{ 2, IR_AS_ANALOG,              "IR" },
	/* New: steering-optimised tilt with deadzone + sqrt curve */
	{ 3, TILT_STEERING_AS_ANALOG,   "Tilt Steering" },
	{ 4, NO_ANALOG,                 "None" },
};

static button_t menu_combos[] = {
	{ 0, WPAD_BUTTON_1|WPAD_BUTTON_2, "1+2" },
	{ 1, WPAD_BUTTON_PLUS|WPAD_BUTTON_MINUS, "+&-" },
	{ 2, WPAD_BUTTON_HOME, "Home" },
};

static int _GetKeys(int Control, BUTTONS * Keys, controller_config_t* config,
                    int (*available)(int),
                    unsigned int (*getButtons)(WPADData*))
{
	if(wpadNeedScan){ WPAD_ScanPads(); wpadNeedScan = 0; }
	WPADData* wpad = WPAD_Data(Control);
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));
	//Reset buttons & sticks
	c->btns.All = 0xFFFF;
	c->leftStickX = c->leftStickY = c->rightStickX = c->rightStickY = 128;

	// Only use a connected nunchuck controller
	if(!available(Control))
		return 0;

	unsigned int b = getButtons(wpad);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask ? 0 : 1;
	}
	
	c->btns.SQUARE_BUTTON    = isHeld(config->SQU);
	c->btns.CROSS_BUTTON     = isHeld(config->CRO);
	c->btns.CIRCLE_BUTTON    = isHeld(config->CIR);
	c->btns.TRIANGLE_BUTTON  = isHeld(config->TRI);

	c->btns.R1_BUTTON    = isHeld(config->R1);
	c->btns.L1_BUTTON    = isHeld(config->L1);
	c->btns.R2_BUTTON    = isHeld(config->R2);
	c->btns.L2_BUTTON    = isHeld(config->L2);

	c->btns.L_DPAD       = isHeld(config->DL);
	c->btns.R_DPAD       = isHeld(config->DR);
	c->btns.U_DPAD       = isHeld(config->DU);
	c->btns.D_DPAD       = isHeld(config->DD);

	c->btns.START_BUTTON  = isHeld(config->START);
	c->btns.R3_BUTTON    = isHeld(config->R3);
	c->btns.L3_BUTTON    = isHeld(config->L3);
	c->btns.SELECT_BUTTON = isHeld(config->SELECT);

	//adjust values by 128 cause PSX sticks range 0-255 with a 128 center pos
	s8 stickX = 0;
	s8 stickY = 0;
	if(config->analogL->mask == NUNCHUK_AS_ANALOG){
		stickX = getStickValue(&wpad->exp.nunchuk.js, STICK_X, 127);
		stickY = getStickValue(&wpad->exp.nunchuk.js, STICK_Y, 127);
	} else if(config->analogL->mask == IR_AS_ANALOG){
		if(wpad->ir.smooth_valid){
			stickX = ((short)(wpad->ir.sx - 512)) >> 2;
			stickY = -(signed char)((wpad->ir.sy - 384) / 3);
		} else {
			stickX = 0;
			stickY = 0;
		}
	} else if(config->analogL->mask == TILT_AS_ANALOG){
		/* Legacy tilt mode: unchanged from original WiiSX mapping */
		stickX = -wpad->orient.pitch;
		stickY = wpad->orient.roll;
	} else if(config->analogL->mask == WHEEL_AS_ANALOG){
		stickX = 512 - wpad->accel.y;
		stickY = wpad->accel.z - 512;
	} else if(config->analogL->mask == TILT_STEERING_AS_ANALOG){
		/*
		 * Kart-steering tilt mode
		 * ─────────────────────────────────────────────────────
		 * Physical → logical axis mapping:
		 *   Roll  (tilt left/right)  ──► stickX  (PS1 L-stick X)
		 *   Pitch (tilt fwd/back)    ──► ignored  (stickY = 0)
		 *
		 * Why roll → stickX?
		 *   In kart games the left-stick X axis is steering.
		 *   Tilting the Wiimote sideways (roll) is the natural
		 *   gesture for turning, mirroring a steering wheel.
		 *
		 * Processing steps:
		 *   1. Read wpad->orient.roll  (degrees, +right / −left)
		 *   2. Subtract TILT_DEADZONE_DEG from the magnitude so
		 *      small wobbles near level produce exactly 0.
		 *   3. Clamp to ±TILT_MAX_DEG (the comfortable range).
		 *   4. Normalise to [−1.0, +1.0].
		 *   5. Apply square-root curve: output = sign * sqrt(|input|)
		 *      This makes small tilts produce a larger proportional
		 *      output than a pure linear map would, compensating for
		 *      the narrow physical tilt range the user actually uses.
		 *   6. Scale by 127 and the global sensitivity multiplier.
		 */
		{
			float roll = (float)wpad->orient.roll; /* +right, −left, degrees */

			/* Use per-profile values so each saved config slot can be
			 * tuned for a specific game (e.g. CTR vs Gran Turismo). */
			float dz  = config->tiltDeadzone; /* degrees stripped near centre */
			float max = config->tiltMaxAngle; /* degrees that reach full lock  */

			/* Step 2: remove dead-zone from magnitude */
			if      (roll >  dz) roll -= dz;
			else if (roll < -dz) roll += dz;
			else                 roll  = 0.0f;

			/* Step 3: clamp to usable physical range */
			if (roll >  max) roll =  max;
			if (roll < -max) roll = -max;

			/* Step 4: normalise so max -> 1.0 */
			float normRoll = roll / max;

			/* Step 5: square-root response curve
			 *   sqrt() only works on positive values, so we extract
			 *   the sign, apply sqrt to the absolute value, re-apply
			 *   the sign.  This gives a curve that rises quickly near
			 *   centre and flattens toward the extremes — ideal for
			 *   steering where most inputs are small corrections. */
			float signR = (normRoll >= 0.0f) ? 1.0f : -1.0f;
			float curved = signR * sqrtf(normRoll * signR);

			/* Step 6: scale to s8 range, apply sensitivity */
			stickX = (s8)(curved * 127.0f * config->sensitivity);
			stickY = 0; /* pitch not used — set to centre */
		}
	} /* end else if(TILT_STEERING_AS_ANALOG) */
	c->leftStickX  = (u8)(stickX+127) & 0xFF;
	if(config->invertedYL)	c->leftStickY = (u8)(stickY+127) & 0xFF;
	else					c->leftStickY = (u8)(-stickY+127) & 0xFF;

	stickX = 0;
	stickY = 0;
	if(config->analogR->mask == NUNCHUK_AS_ANALOG){
		stickX = getStickValue(&wpad->exp.nunchuk.js, STICK_X, 127);
		stickY = getStickValue(&wpad->exp.nunchuk.js, STICK_Y, 127);
	} else if(config->analogR->mask == IR_AS_ANALOG){
		if(wpad->ir.smooth_valid){
			stickX = ((short)(wpad->ir.sx - 512)) >> 2;
			stickY = -(signed char)((wpad->ir.sy - 384) / 3);
		} else {
			stickX = 0;
			stickY = 0;
		}
	} else if(config->analogR->mask == TILT_AS_ANALOG){
		stickX = -wpad->orient.pitch;
		stickY = wpad->orient.roll;
	} else if(config->analogR->mask == WHEEL_AS_ANALOG){
		stickX = 512 - wpad->accel.y;
		stickY = wpad->accel.z - 512;
	} else if(config->analogR->mask == TILT_STEERING_AS_ANALOG){
		/* Same steering-tilt logic as analogL — see comment block above */
		{
			float roll = (float)wpad->orient.roll;
			float dz  = config->tiltDeadzone;
			float max = config->tiltMaxAngle;
			if      (roll >  dz) roll -= dz;
			else if (roll < -dz) roll += dz;
			else                 roll  = 0.0f;
			if (roll >  max) roll =  max;
			if (roll < -max) roll = -max;
			float normRoll = roll / max;
			float signR = (normRoll >= 0.0f) ? 1.0f : -1.0f;
			float curved = signR * sqrtf(normRoll * signR);
			stickX = (s8)(curved * 127.0f * config->sensitivity);
			stickY = 0;
		}
	} /* end else if(TILT_STEERING_AS_ANALOG) */
	c->rightStickX  = (u8)(stickX+127) & 0xFF;
	if(config->invertedYR)	c->rightStickY = (u8)(stickY+127) & 0xFF;
	else					c->rightStickY = (u8)(-stickY+127) & 0xFF;

	// Return 1 if exit, 2 if fastforward
	if (!isHeld(config->exit)) return 1;
	if (!isHeld(config->fastf)) return 2;
	else 
		return 0;
}

static int checkType(int Control, int type){
	int err;
	u32 expType;
	err = WPAD_Probe(Control, &expType);

	if(err != WPAD_ERR_NONE)
		return -1;
	
	switch(expType){
	case WPAD_EXP_NONE:
		controller_Wiimote.available[Control] = 1;
		break;
	case WPAD_EXP_NUNCHUK:
		controller_WiimoteNunchuk.available[Control] = 1;
		break;
	case WPAD_EXP_CLASSIC:
		controller_Classic.available[Control] = 1;
		break;
	}
	
	return expType;
}

static int availableWM(int Control){
	if(checkType(Control, WPAD_EXP_NONE) != WPAD_EXP_NONE){
		controller_Wiimote.available[Control] = 0;
		return 0;
	} else {
		return 1;
	}
}

static int availableWMN(int Control){
	if(checkType(Control, WPAD_EXP_NUNCHUK) != WPAD_EXP_NUNCHUK){
		controller_WiimoteNunchuk.available[Control] = 0;
		return 0;
	} else {
		return 1;
	}
}

static unsigned int getButtonsWM(WPADData* controller){
	return controller->btns_h;
}

static unsigned int getButtonsWMN(WPADData* controller){
	unsigned int b = controller->btns_h;
	s8 stickX      = getStickValue(&controller->exp.nunchuk.js, STICK_X, 7);
	s8 stickY      = getStickValue(&controller->exp.nunchuk.js, STICK_Y, 7);
	
	if(stickX    < -3) b |= NUNCHUK_L;
	if(stickX    >  3) b |= NUNCHUK_R;
	if(stickY    >  3) b |= NUNCHUK_U;
	if(stickY    < -3) b |= NUNCHUK_D;
	
	return b;
}

static int GetKeysWM(int con, BUTTONS * keys, controller_config_t* cfg){
	return _GetKeys(con, keys, cfg, availableWM, getButtonsWM);
}

static int GetKeysWMN(int con, BUTTONS * keys, controller_config_t* cfg){
	return _GetKeys(con, keys, cfg, availableWMN, getButtonsWMN);
}

static void pause(int Control){
	WPAD_Rumble(Control, 0);
}

static void resume(int Control){ }

static void rumble(int Control, int rumble){
	WPAD_Rumble(Control, (rumble && rumbleEnabled) ? 1 : 0);
}

static void configure(int Control, controller_config_t* config){
	/*
	 * analog_fmts maps each analog source enum value to the WPAD data
	 * format flag that must be OR'd into WPAD_SetDataFormat().
	 * TILT_STEERING_AS_ANALOG needs WPAD_DATA_ACCEL just like TILT_AS_ANALOG
	 * and WHEEL_AS_ANALOG — the orient.roll field is derived from accel data.
	 */
	static s32 analog_fmts[] = {
		WPAD_DATA_EXPANSION, /* NUNCHUK_AS_ANALOG        */
		WPAD_DATA_IR,        /* IR_AS_ANALOG             */
		WPAD_DATA_ACCEL,     /* TILT_AS_ANALOG           */
		WPAD_DATA_ACCEL,     /* WHEEL_AS_ANALOG          */
		WPAD_DATA_ACCEL,     /* TILT_STEERING_AS_ANALOG  */
		WPAD_DATA_BUTTONS,   /* NO_ANALOG                */
	};
	WPAD_SetDataFormat(Control, analog_fmts[config->analogL->mask] | analog_fmts[config->analogR->mask]);
}

static void assign(int p, int v){
	// TODO: Light up the LEDs appropriately
}

static void refreshAvailableWM(void);
static void refreshAvailableWMN(void);

controller_t controller_Wiimote =
	{ 'W',
	  GetKeysWM,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailableWM,
	  {0, 0, 0, 0},
	  NUM_WIIMOTE_BUTTONS,
	  buttons,
	  sizeof(analog_sources_wm)/sizeof(analog_sources_wm[0]),
	  analog_sources_wm,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .SQU        = &buttons[0],  // None
	    .CRO        = &buttons[10], // 1
	    .CIR        = &buttons[11], // 2
	    .TRI        = &buttons[0],  // None
	    .R1         = &buttons[7],  // +
	    .L1         = &buttons[8],  // -
	    .R2         = &buttons[0],  // None
	    .L2         = &buttons[0],  // None
	    .R3         = &buttons[0],  // None
	    .L3         = &buttons[0],  // None
	    .DL         = &buttons[1],  // D Up
	    .DR         = &buttons[4],  // D Down
	    .DU         = &buttons[3],  // D Right
	    .DD         = &buttons[2],  // D Left
	    .START      = &buttons[9],  // Home
	    .SELECT     = &buttons[0],  // None
	    .analogL    = &analog_sources_wm[0], // Tilt
	    .analogR    = &analog_sources_wm[3], // None
	    .exit       = &menu_combos[1], // +&-
	    .invertedYL = 0,
	    .invertedYR = 0,
		.sensitivity  = 1.0,
		.fastf        = &menu_combos[0], // 1+2
		/* Tilt steering defaults — reasonable for most kart/racing games */
		.tiltDeadzone = TILT_DEADZONE_DEG,  // 5.0° — absorbs hand-shake
		.tiltMaxAngle = TILT_MAX_DEG,       // 35.0° — full lock angle
	  }
	};

controller_t controller_WiimoteNunchuk =
	{ 'N',
	  GetKeysWMN,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailableWMN,
	  {0, 0, 0, 0},
	  sizeof(buttons)/sizeof(buttons[0]),
	  buttons,
	  sizeof(analog_sources_wmn)/sizeof(analog_sources_wmn[0]),
	  analog_sources_wmn,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .SQU        = &buttons[2], // D Left
	    .CRO        = &buttons[4], // D Down
	    .CIR        = &buttons[3], // D Right
	    .TRI        = &buttons[1], // D Up
	    .R1         = &buttons[5],  // A
	    .L1         = &buttons[13], // Z
	    .R2         = &buttons[6],  // B
	    .L2         = &buttons[12], // C
	    .R3         = &buttons[0],  // None
	    .L3         = &buttons[0],  // None
	    .DL         = &buttons[0],  // None
	    .DR         = &buttons[0],  // None
	    .DU         = &buttons[0],  // None
	    .DD         = &buttons[0],  // None
	    .START      = &buttons[8],  // -
	    .SELECT     = &buttons[7],  // +
	    .analogL    = &analog_sources_wmn[0], // Nunchuck
	    .analogR    = &analog_sources_wmn[1], // IR
	    .exit       = &menu_combos[0], // 1+2
	    .invertedYL = 0,
	    .invertedYR = 0,
		.sensitivity  = 1.0,
		.fastf        = &menu_combos[1], // +&-
		/* Tilt steering defaults */
		.tiltDeadzone = TILT_DEADZONE_DEG,  // 5.0°
		.tiltMaxAngle = TILT_MAX_DEG,       // 35.0°
	  }
	 };

static void refreshAvailableWM(void){
	int i;
	WPAD_ScanPads();
	for(i=0; i<4; ++i){
		availableWM(i);
	}
}

static void refreshAvailableWMN(void){
	int i;
	WPAD_ScanPads();
	for(i=0; i<4; ++i){
		availableWMN(i);
	}
}
#endif
