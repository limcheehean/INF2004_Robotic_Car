# Decider Module

This module, consisting of `decider.h` and `decider.c`, serves as a decision-making component for a robotic system. It manages various events and coordinates actions based on predefined logic.

## decider.h

### Header File Overview

- **Dependencies:**
  - `pico/stdlib.h`
  - `driver/motor/motor_controller.h`
  - `driver/encoder/wheel_encoder.h`
  - `driver/magnetnometer/magnetnometer.c`
  - `FreeRTOS.h`
  - `task.h`
  - `queue.h`

- **Constants:**
  - Defines constants for different types of events.

- **Structures:**
  - `DeciderMessage_t`: Represents a message for the decider.

- **Queue Handle:**
  - `g_decider_message_queue`: Queue handle for the decider task.

- **Function:**
  - `init_decider()`: Initializes the decider module.

## decider.c

### Implementation File Overview

- **Global Variables:**
  - `g_decider_task_handle`: Task handle for the decider task.
  - `g_decider_message_queue`: Queue handle for the decider task.
  - `xTimers[NUM_TIMERS]`: Array to hold handles to created timers.

- **Function:**
  - `message_decider(int type, int data)`: Sends a message to the decider task.

- **Timer Callback Functions:**
  - `check_wall_callback()`
  - `check_barcode_callback()`
  - `stop_reversing_callback()`
  - `turning_callback()`
  - `reset_speed_callback()`

- **Decider Task Implementation:**
  - Manages various events and executes corresponding actions.

- **Initialization Function:**
  - `init_decider()`: Initializes the decider task and related components.

## Usage

1. **Include `decider.h`:**
   - Include the `decider.h` header file in your project.

2. **Initialize Decider:**
   - Call the `init_decider()` function to initialize the decider module.

3. **Use Decider:**
   - Utilize the decider module for managing events and decision-making logic in your robotic system.

4. **Test (Optional):**
   - If `DECIDER_TEST` is defined, an entry point `main()` is provided for testing purposes.

*Ensure to replace placeholders like `[NUM_TIMERS]` with appropriate values.*

