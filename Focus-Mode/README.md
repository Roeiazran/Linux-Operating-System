# Focus Mode Simulation

This project simulates a "Focus Mode" environment where the user is periodically exposed to distractions, such as emails, reminders, or doorbell notifications, and the program demonstrates how signals in Unix-like systems can be blocked, handled, and cleared.

---

## Features

- **Focus Rounds:** Run multiple rounds of uninterrupted focus sessions.
- **Distractions:** Simulate interruptions via signals:
  - `SIGUSR1` → Email notification
  - `SIGUSR2` → Reminder to pick up delivery
  - `SIGINT`  → Doorbell ringing
- **Blocking and Handling Signals:** Distractions are blocked during focus mode and processed only after a round ends.
- **Interactive User Input:** Users can trigger distractions manually.
- **Clean Console Output:** Formatted display of focus rounds, pending distractions, and outcomes.

---

## How It Works

1. **Initialize Focus Mode:**  
   All distraction signals are blocked.
2. **Run Rounds:**  
   For each round:
   - Display round information.
   - Get user-triggered interruptions.
   - Handle all pending distractions.
   - Clear pending signals for the next round.
3. **Finish Focus Mode:**  
   All distractions are unblocked, and the session ends.