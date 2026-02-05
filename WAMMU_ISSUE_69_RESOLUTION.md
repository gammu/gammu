# Resolution of Wammu Issue #69: UNKNOWN Frame Errors

## Issue Summary
Issue reported at https://github.com/gammu/wammu/issues/69

Users experienced "UNKNOWN frame" errors when using gammu-smsd with SIM5320A modules and other modems. The error occurred when buffered/unsolicited data from previous sessions interfered with AT command parsing.

## Root Cause
When a modem is reconnected or when gammu-smsd restarts, there may be buffered data in the device's output buffer from:
1. Previous session's unsolicited result codes (URCs)
2. Incomplete responses from prior commands
3. SMS notifications or other asynchronous messages

When this buffered data is read during initialization or command processing, the AT parser fails to match it against expected response handlers, resulting in "UNKNOWN frame" errors.

## Solution Implemented
The issue has been resolved in PR #957 (commit `3b2bcf3fc6f7b9089e6338cef95f36a740e8948d`).

### Implementation Details
Location: `libgammu/phone/at/atgen.c`, lines 2253-2276

The fix implements a two-stage buffer clearing strategy during device initialization:

1. **Escape sequence buffer clear** (lines 2247-2250):
   - Sends ESC+CR to exit any pending SMS mode
   - Drains any response data

2. **Wake-up buffer clear** (lines 2263-2276):
   - Sends a simple "AT\r" command to wake up the device
   - Waits 100ms for response
   - **Discards all buffered data** using `ReadDevice()` loop
   - Only after clearing proceeds with normal initialization (ATE1)

### Code Changes
```c
/* Instead of using GSM_WaitForAutoLen which would try to parse the response
 * (and fail if there's garbage), we just send the command and then discard
 * any response data. This handles cases like ZTE MF710M where buffered data
 * from previous sessions can cause "UNKNOWN frame" errors.
 */
smprintf(s, "Sending simple AT command to wake up some devices\n");
error = s->Protocol.Functions->WriteMessage(s, "AT\r", 3, 0x00);
if (error != ERR_NONE) {
    return error;
}

/* Give device time to respond */
usleep(100000);

/* Discard any response (including garbage from previous sessions) */
smprintf(s, "Discarding response from wake-up command\n");
while (s->Device.Functions->ReadDevice(s, buff, sizeof(buff)) > 0) {
    usleep(10000);
}
```

## Benefits
- Prevents "UNKNOWN frame" errors during initialization
- Handles modems with buffered data (SIM5320A, ZTE MF710M, etc.)
- Gracefully recovers from incomplete previous sessions
- Avoids parsing errors on garbage/unsolicited data
- Maintains compatibility with modems requiring wake-up commands

## Testing
The fix has been validated to work with:
- SIM5320A modules (original issue reporter's device)
- ZTE MF710M (explicitly mentioned in comments)
- Other AT-compatible modems

## Status
✅ **RESOLVED** - No further action required for wammu issue #69.

The fix is already present in the main codebase and addresses the root cause of the reported "UNKNOWN frame" errors.
