import { ClearanceResult, DeviceState, LiveVitals, ZyntraLink } from './types';

const RECOVERY_SECONDS = 30;
const CLEARANCE_SECONDS = 8;
const HRV_BASELINE = 80.0;
const TEMP_BASELINE = 33.5;

export class MockZyntraLink implements ZyntraLink {
  private state: DeviceState = 'DISCONNECTED';
  private runCount = 0;
  private timer: ReturnType<typeof setInterval> | null = null;

  private stateCbs = new Set<(s: DeviceState) => void>();
  private vitalsCbs = new Set<(v: LiveVitals) => void>();
  private resultCbs = new Set<(r: ClearanceResult) => void>();

  async connect(): Promise<void> {
    await this.delay(1200);
    this.setState('SHIFT');
  }

  async disconnect(): Promise<void> {
    this.stopTimer();
    this.setState('DISCONNECTED');
  }

  triggerBreak(): void {
    if (this.state !== 'SHIFT') return;
    this.runCount += 1;
    this.startRecovery();
  }

  acknowledgeResult(): void {
    if (this.state === 'CLEARED' || this.state === 'NOT_CLEARED') {
      this.setState('SHIFT');
    }
  }

  onStateChange(cb: (s: DeviceState) => void): () => void {
    this.stateCbs.add(cb);
    cb(this.state);
    return () => this.stateCbs.delete(cb);
  }

  onVitals(cb: (v: LiveVitals) => void): () => void {
    this.vitalsCbs.add(cb);
    return () => this.vitalsCbs.delete(cb);
  }

  onResult(cb: (r: ClearanceResult) => void): () => void {
    this.resultCbs.add(cb);
    return () => this.resultCbs.delete(cb);
  }

  private startRecovery() {
    this.setState('RECOVERY');
    const scenario = ((this.runCount - 1) % 3) + 1;
    let remaining = RECOVERY_SECONDS;
    let rmssd = 52;
    const rmssdTarget = scenario === 2 ? 64 : 88;
    let temp = TEMP_BASELINE + 1.1;
    const tempTarget = TEMP_BASELINE + (scenario === 2 ? 0.4 : 0.2);

    this.stopTimer();
    this.timer = setInterval(() => {
      remaining -= 1;
      rmssd += (rmssdTarget - rmssd) * 0.12 + (Math.random() - 0.5) * 3;
      temp += (tempTarget - temp) * 0.10 + (Math.random() - 0.5) * 0.04;

      this.emitVitals({
        rmssd: scenario === 3 && remaining < RECOVERY_SECONDS - 6 ? null : round1(rmssd),
        hrvBaseline: HRV_BASELINE,
        skinTempC: round1(temp),
        tempBaselineC: TEMP_BASELINE,
        secondsRemaining: Math.max(remaining, 0),
      });

      if (remaining <= 0) {
        this.stopTimer();
        this.startClearance(scenario, rmssd, temp);
      }
    }, 1000);
  }

  private startClearance(scenario: number, rmssd: number, temp: number) {
    this.setState('CLEARANCE');
    setTimeout(() => {
      const result = this.buildResult(scenario, rmssd, temp);
      this.setState(result.hrvDataValid && result.cleared ? 'CLEARED' : 'NOT_CLEARED');
      this.resultCbs.forEach((cb) => cb(result));
    }, CLEARANCE_SECONDS * 1000);
  }

  private buildResult(scenario: number, rmssd: number, temp: number): ClearanceResult {
    const tempDelta = round1(Math.abs(temp - TEMP_BASELINE));
    if (scenario === 3) {
      return { hrvDataValid: false, cleared: false, hrvPass: false, tempPass: true, rtPass: true, rmssd: null, tempDeltaC: tempDelta, medianRtMs: 402, minutesToClearance: 0 };
    }
    const hrvPass = rmssd >= HRV_BASELINE * 0.9;
    const tempPass = tempDelta <= 0.8;
    const medianRt = scenario === 2 ? 361 : 379;
    const rtPass = medianRt < 500;
    return { hrvDataValid: true, cleared: hrvPass && tempPass && rtPass, hrvPass, tempPass, rtPass, rmssd: round1(rmssd), tempDeltaC: tempDelta, medianRtMs: medianRt, minutesToClearance: hrvPass && tempPass && rtPass ? 0 : 5 };
  }

  private setState(s: DeviceState) { this.state = s; this.stateCbs.forEach((cb) => cb(s)); }
  private emitVitals(v: LiveVitals) { this.vitalsCbs.forEach((cb) => cb(v)); }
  private stopTimer() { if (this.timer) { clearInterval(this.timer); this.timer = null; } }
  private delay(ms: number) { return new Promise<void>((res) => setTimeout(res, ms)); }
}

function round1(n: number) { return Math.round(n * 10) / 10; }
