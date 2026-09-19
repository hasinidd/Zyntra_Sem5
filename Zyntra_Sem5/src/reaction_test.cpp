#include "reaction_test.h"
#include "config.h"

// ── Pin definitions ───────────────────────────────────────────────────────
#define PIN_MOTOR   4    // Vibration motor via BC547 transistor
#define PIN_BUTTON  15   // Tactile button (INPUT_PULLUP — LOW when pressed)

// ── Test parameters ───────────────────────────────────────────────────────
#define RT_NUM_STIMULI      5      // Number of vibration stimuli per test
#define RT_MIN_INTERVAL_MS  3000   // Minimum random wait between stimuli (3s)
#define RT_MAX_INTERVAL_MS  8000   // Maximum random wait between stimuli (8s)
#define RT_STIMULUS_MS      200    // How long motor vibrates per stimulus
#define RT_TIMEOUT_MS       2000   // Max time to wait for button press
#define RT_FALSE_START_MS   100    // Faster than this = anticipatory press
#define RT_THRESHOLD_MS     500    // Median must be under this to pass

// Store last test results
static uint16_t last_results[RT_NUM_STIMULI];
static uint16_t last_median = 9999;

// ── Initialise pins ───────────────────────────────────────────────────────
void rt_init() {
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  digitalWrite(PIN_MOTOR, LOW);
  Serial.println("[RT] Vibration motor and button initialised");
}

// ── Sort helper for median computation ───────────────────────────────────
static void bubble_sort(uint16_t* arr, int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        uint16_t tmp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tmp;
      }
    }
  }
}

// ── Run the full RT test ──────────────────────────────────────────────────
uint16_t rt_run_test() {
  Serial.println("[RT] Starting reaction time test...");
  Serial.println("[RT] Tap the button each time you feel a vibration.");

  uint16_t results[RT_NUM_STIMULI];
  int valid_count = 0;
  int stimulus_num = 0;

  while (stimulus_num < RT_NUM_STIMULI) {

    // Wait a random interval before next stimulus
    // Random interval prevents anticipatory pressing
    uint32_t wait_ms = random(RT_MIN_INTERVAL_MS, RT_MAX_INTERVAL_MS);
    Serial.print("[RT] Waiting ");
    Serial.print(wait_ms);
    Serial.println("ms before next stimulus...");
    delay(wait_ms);

    // Deliver stimulus — vibrate motor for 200ms
    digitalWrite(PIN_MOTOR, HIGH);
    uint32_t stimulus_time = millis();
    Serial.print("[RT] Stimulus ");
    Serial.print(stimulus_num + 1);
    Serial.print("/5 delivered. Waiting for response...");
    delay(RT_STIMULUS_MS);
    digitalWrite(PIN_MOTOR, LOW);

    // Wait for button press — up to 2000ms timeout
    uint32_t response_time = 0;
    uint32_t wait_start = millis();
    bool timed_out = true;

    while (millis() - wait_start < RT_TIMEOUT_MS) {
      if (digitalRead(PIN_BUTTON) == LOW) {
        response_time = millis() - stimulus_time;
        timed_out = false;
        // Wait for button release before continuing
        while (digitalRead(PIN_BUTTON) == LOW);
        delay(50); // Debounce
        break;
      }
    }

    // Handle result
    if (timed_out) {
      // No response — record as timeout
      Serial.println(" TIMEOUT");
      results[valid_count++] = 9999;
      stimulus_num++;

    } else if (response_time < RT_FALSE_START_MS) {
      // Too fast — false start, repeat this stimulus
      Serial.print(" FALSE START (");
      Serial.print(response_time);
      Serial.println("ms) — repeating stimulus");
      // Do NOT increment stimulus_num — repeat this stimulus

    } else {
      // Valid response
      Serial.print(" Response: ");
      Serial.print(response_time);
      Serial.println("ms");
      results[valid_count++] = (uint16_t)response_time;
      stimulus_num++;
    }
  }

  // Compute median of results
  uint16_t sorted[RT_NUM_STIMULI];
  memcpy(sorted, results, sizeof(results));
  bubble_sort(sorted, RT_NUM_STIMULI);
  last_median = sorted[RT_NUM_STIMULI / 2];

  // Store results
  memcpy(last_results, results, sizeof(results));

  Serial.print("[RT] Test complete. Median RT: ");
  Serial.print(last_median);
  Serial.print("ms. Result: ");
  Serial.println(last_median < RT_THRESHOLD_MS ? "PASS" : "FAIL");

  return last_median;
}

// ── Get last median RT ────────────────────────────────────────────────────
uint16_t rt_get_last_median() {
  return last_median;
}

// ── Check clearance ───────────────────────────────────────────────────────
bool rt_is_cleared() {
  return (last_median < RT_THRESHOLD_MS);
}