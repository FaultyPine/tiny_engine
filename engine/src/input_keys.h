#ifndef TINY_INPUT_BUTTONS_H
#define TINY_INPUT_BUTTONS_H

// From GLFW



#define TINY_RELEASE                0

#define TINY_PRESS                  1

#define TINY_REPEAT                 2

#define TINY_HAT_CENTERED           0
#define TINY_HAT_UP                 1
#define TINY_HAT_RIGHT              2
#define TINY_HAT_DOWN               4
#define TINY_HAT_LEFT               8
#define TINY_HAT_RIGHT_UP           (TINY_HAT_RIGHT | TINY_HAT_UP)
#define TINY_HAT_RIGHT_DOWN         (TINY_HAT_RIGHT | TINY_HAT_DOWN)
#define TINY_HAT_LEFT_UP            (TINY_HAT_LEFT  | TINY_HAT_UP)
#define TINY_HAT_LEFT_DOWN          (TINY_HAT_LEFT  | TINY_HAT_DOWN)

/* The unknown key */
#define TINY_KEY_UNKNOWN            -1

/* Printable keys */
#define TINY_KEY_SPACE              32
#define TINY_KEY_APOSTROPHE         39  /* ' */
#define TINY_KEY_COMMA              44  /* , */
#define TINY_KEY_MINUS              45  /* - */
#define TINY_KEY_PERIOD             46  /* . */
#define TINY_KEY_SLASH              47  /* / */
#define TINY_KEY_0                  48
#define TINY_KEY_1                  49
#define TINY_KEY_2                  50
#define TINY_KEY_3                  51
#define TINY_KEY_4                  52
#define TINY_KEY_5                  53
#define TINY_KEY_6                  54
#define TINY_KEY_7                  55
#define TINY_KEY_8                  56
#define TINY_KEY_9                  57
#define TINY_KEY_SEMICOLON          59  /* ; */
#define TINY_KEY_EQUAL              61  /* = */
#define TINY_KEY_A                  65
#define TINY_KEY_B                  66
#define TINY_KEY_C                  67
#define TINY_KEY_D                  68
#define TINY_KEY_E                  69
#define TINY_KEY_F                  70
#define TINY_KEY_G                  71
#define TINY_KEY_H                  72
#define TINY_KEY_I                  73
#define TINY_KEY_J                  74
#define TINY_KEY_K                  75
#define TINY_KEY_L                  76
#define TINY_KEY_M                  77
#define TINY_KEY_N                  78
#define TINY_KEY_O                  79
#define TINY_KEY_P                  80
#define TINY_KEY_Q                  81
#define TINY_KEY_R                  82
#define TINY_KEY_S                  83
#define TINY_KEY_T                  84
#define TINY_KEY_U                  85
#define TINY_KEY_V                  86
#define TINY_KEY_W                  87
#define TINY_KEY_X                  88
#define TINY_KEY_Y                  89
#define TINY_KEY_Z                  90
#define TINY_KEY_LEFT_BRACKET       91  /* [ */
#define TINY_KEY_BACKSLASH          92  /* \ */
#define TINY_KEY_RIGHT_BRACKET      93  /* ] */
#define TINY_KEY_GRAVE_ACCENT       96  /* ` */
#define TINY_KEY_WORLD_1            161 /* non-US #1 */
#define TINY_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define TINY_KEY_ESCAPE             256
#define TINY_KEY_ENTER              257
#define TINY_KEY_TAB                258
#define TINY_KEY_BACKSPACE          259
#define TINY_KEY_INSERT             260
#define TINY_KEY_DELETE             261
#define TINY_KEY_RIGHT              262
#define TINY_KEY_LEFT               263
#define TINY_KEY_DOWN               264
#define TINY_KEY_UP                 265
#define TINY_KEY_PAGE_UP            266
#define TINY_KEY_PAGE_DOWN          267
#define TINY_KEY_HOME               268
#define TINY_KEY_END                269
#define TINY_KEY_CAPS_LOCK          280
#define TINY_KEY_SCROLL_LOCK        281
#define TINY_KEY_NUM_LOCK           282
#define TINY_KEY_PRINT_SCREEN       283
#define TINY_KEY_PAUSE              284
#define TINY_KEY_F1                 290
#define TINY_KEY_F2                 291
#define TINY_KEY_F3                 292
#define TINY_KEY_F4                 293
#define TINY_KEY_F5                 294
#define TINY_KEY_F6                 295
#define TINY_KEY_F7                 296
#define TINY_KEY_F8                 297
#define TINY_KEY_F9                 298
#define TINY_KEY_F10                299
#define TINY_KEY_F11                300
#define TINY_KEY_F12                301
#define TINY_KEY_F13                302
#define TINY_KEY_F14                303
#define TINY_KEY_F15                304
#define TINY_KEY_F16                305
#define TINY_KEY_F17                306
#define TINY_KEY_F18                307
#define TINY_KEY_F19                308
#define TINY_KEY_F20                309
#define TINY_KEY_F21                310
#define TINY_KEY_F22                311
#define TINY_KEY_F23                312
#define TINY_KEY_F24                313
#define TINY_KEY_F25                314
#define TINY_KEY_KP_0               320
#define TINY_KEY_KP_1               321
#define TINY_KEY_KP_2               322
#define TINY_KEY_KP_3               323
#define TINY_KEY_KP_4               324
#define TINY_KEY_KP_5               325
#define TINY_KEY_KP_6               326
#define TINY_KEY_KP_7               327
#define TINY_KEY_KP_8               328
#define TINY_KEY_KP_9               329
#define TINY_KEY_KP_DECIMAL         330
#define TINY_KEY_KP_DIVIDE          331
#define TINY_KEY_KP_MULTIPLY        332
#define TINY_KEY_KP_SUBTRACT        333
#define TINY_KEY_KP_ADD             334
#define TINY_KEY_KP_ENTER           335
#define TINY_KEY_KP_EQUAL           336
#define TINY_KEY_LEFT_SHIFT         340
#define TINY_KEY_LEFT_CONTROL       341
#define TINY_KEY_LEFT_ALT           342
#define TINY_KEY_LEFT_SUPER         343
#define TINY_KEY_RIGHT_SHIFT        344
#define TINY_KEY_RIGHT_CONTROL      345
#define TINY_KEY_RIGHT_ALT          346
#define TINY_KEY_RIGHT_SUPER        347
#define TINY_KEY_MENU               348

#define TINY_KEY_LAST               TINY_KEY_MENU

#define TINY_MOD_SHIFT           0x0001

#define TINY_MOD_CONTROL         0x0002

#define TINY_MOD_ALT             0x0004

#define TINY_MOD_SUPER           0x0008

#define TINY_MOD_CAPS_LOCK       0x0010

#define TINY_MOD_NUM_LOCK        0x0020


#define TINY_MOUSE_BUTTON_1         0
#define TINY_MOUSE_BUTTON_2         1
#define TINY_MOUSE_BUTTON_3         2
#define TINY_MOUSE_BUTTON_4         3
#define TINY_MOUSE_BUTTON_5         4
#define TINY_MOUSE_BUTTON_6         5
#define TINY_MOUSE_BUTTON_7         6
#define TINY_MOUSE_BUTTON_8         7
#define TINY_MOUSE_BUTTON_LAST      TINY_MOUSE_BUTTON_8
#define TINY_MOUSE_BUTTON_LEFT      TINY_MOUSE_BUTTON_1
#define TINY_MOUSE_BUTTON_RIGHT     TINY_MOUSE_BUTTON_2
#define TINY_MOUSE_BUTTON_MIDDLE    TINY_MOUSE_BUTTON_3



#define TINY_JOYSTICK_1             0
#define TINY_JOYSTICK_2             1
#define TINY_JOYSTICK_3             2
#define TINY_JOYSTICK_4             3
#define TINY_JOYSTICK_5             4
#define TINY_JOYSTICK_6             5
#define TINY_JOYSTICK_7             6
#define TINY_JOYSTICK_8             7
#define TINY_JOYSTICK_9             8
#define TINY_JOYSTICK_10            9
#define TINY_JOYSTICK_11            10
#define TINY_JOYSTICK_12            11
#define TINY_JOYSTICK_13            12
#define TINY_JOYSTICK_14            13
#define TINY_JOYSTICK_15            14
#define TINY_JOYSTICK_16            15
#define TINY_JOYSTICK_LAST          TINY_JOYSTICK_16

#define TINY_GAMEPAD_BUTTON_A               0
#define TINY_GAMEPAD_BUTTON_B               1
#define TINY_GAMEPAD_BUTTON_X               2
#define TINY_GAMEPAD_BUTTON_Y               3
#define TINY_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define TINY_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define TINY_GAMEPAD_BUTTON_BACK            6
#define TINY_GAMEPAD_BUTTON_START           7
#define TINY_GAMEPAD_BUTTON_GUIDE           8
#define TINY_GAMEPAD_BUTTON_LEFT_THUMB      9
#define TINY_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define TINY_GAMEPAD_BUTTON_DPAD_UP         11
#define TINY_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define TINY_GAMEPAD_BUTTON_DPAD_DOWN       13
#define TINY_GAMEPAD_BUTTON_DPAD_LEFT       14
#define TINY_GAMEPAD_BUTTON_LAST            TINY_GAMEPAD_BUTTON_DPAD_LEFT

#define TINY_GAMEPAD_BUTTON_CROSS       TINY_GAMEPAD_BUTTON_A
#define TINY_GAMEPAD_BUTTON_CIRCLE      TINY_GAMEPAD_BUTTON_B
#define TINY_GAMEPAD_BUTTON_SQUARE      TINY_GAMEPAD_BUTTON_X
#define TINY_GAMEPAD_BUTTON_TRIANGLE    TINY_GAMEPAD_BUTTON_Y

#define TINY_GAMEPAD_AXIS_LEFT_X        0
#define TINY_GAMEPAD_AXIS_LEFT_Y        1
#define TINY_GAMEPAD_AXIS_RIGHT_X       2
#define TINY_GAMEPAD_AXIS_RIGHT_Y       3
#define TINY_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define TINY_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define TINY_GAMEPAD_AXIS_LAST          TINY_GAMEPAD_AXIS_RIGHT_TRIGGER






#endif