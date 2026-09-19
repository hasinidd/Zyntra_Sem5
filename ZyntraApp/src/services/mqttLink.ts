import mqtt, { MqttClient } from 'mqtt';
import { BaselineData, ClearanceResult, DeviceState, LiveVitals, ZyntraLink } from './types';

const DEFAULT_BROKER_URL = 'wss://broker.hivemq.com:8884/mqtt';

const STATE_MAP: Record<number | string, DeviceState> = {
  0: 'DISCONNECTED',
  1: 'BASELINE',
  2: 'SHIFT',
  3: 'RECOVERY',
  4: 'CLEARANCE',
  5: 'CLEARED',
  6: 'NOT_CLEARED',
  DISCONNECTED: 'DISCONNECTED',
  BASELINE:     'BASELINE',
  SHIFT:        'SHIFT',
  RECOVERY:     'RECOVERY',
  CLEARANCE:    'CLEARANCE',
  CLEARED:      'CLEARED',
  NOT_CLEARED:  'NOT_CLEARED',
};

export class MqttZyntraLink implements ZyntraLink {
  private client: MqttClient | null = null;
  private stateCbs    = new Set<(s: DeviceState) => void>();
  private vitalsCbs   = new Set<(v: LiveVitals) => void>();
  private baselineCbs = new Set<(b: BaselineData) => void>();
  private resultCbs   = new Set<(r: ClearanceResult) => void>();
  private currentState: DeviceState = 'DISCONNECTED';

  async connect(brokerUrl?: string): Promise<void> {
    const url = brokerUrl && brokerUrl.trim().length > 0 ? brokerUrl.trim() : DEFAULT_BROKER_URL;
    console.log('[MQTT] Connecting to broker:', url);

    return new Promise((resolve, reject) => {
      try {
        this.client = mqtt.connect(url, {
          clientId: `Zyntra_App_${Math.random().toString(16).substring(2, 10)}`,
          keepalive: 60,
          reconnectPeriod: 3000,
          connectTimeout: 15000,
        });

        this.client.on('connect', () => {
          console.log('[MQTT] Connected to broker');
          this._emitState('SHIFT');

          this.client?.subscribe(['zyntra/state', 'zyntra/vitals', 'zyntra/baseline', 'zyntra/result'], (err) => {
            if (err) console.error('[MQTT] Subscription error:', err);
            else console.log('[MQTT] Subscribed to zyntra/# topics');
          });

          resolve();
        });

        this.client.on('message', (topic, payload) => {
          this._handleMessage(topic, payload.toString());
        });

        this.client.on('error', (err) => {
          console.warn('[MQTT] Error:', err.message);
          reject(err);
        });

        this.client.on('close', () => {
          console.log('[MQTT] Connection closed');
          this._emitState('DISCONNECTED');
        });

      } catch (e) {
        reject(e);
      }
    });
  }

  async disconnect(): Promise<void> {
    if (this.client) {
      this.client.end(true);
      this.client = null;
    }
    this._emitState('DISCONNECTED');
  }

  triggerBaseline(): void {
    console.log('[MQTT] Sending START_BASELINE to ESP32...');
    this._emitState('BASELINE');
    this._publishCommand('START_BASELINE');
  }

  triggerBreak(): void {
    console.log('[MQTT] Sending START_RECOVERY to ESP32...');
    this._emitState('RECOVERY');
    this._publishCommand('START_RECOVERY');
  }

  acknowledgeResult(): void {
    this._publishCommand('ACK');
    this._emitState('SHIFT');
  }

  onStateChange(cb: (s: DeviceState) => void): () => void {
    this.stateCbs.add(cb);
    cb(this.currentState);
    return () => this.stateCbs.delete(cb);
  }

  onVitals(cb: (v: LiveVitals) => void): () => void {
    this.vitalsCbs.add(cb);
    return () => this.vitalsCbs.delete(cb);
  }

  onBaseline(cb: (b: BaselineData) => void): () => void {
    this.baselineCbs.add(cb);
    return () => this.baselineCbs.delete(cb);
  }

  onResult(cb: (r: ClearanceResult) => void): () => void {
    this.resultCbs.add(cb);
    return () => this.resultCbs.delete(cb);
  }

  private _publishCommand(cmd: string) {
    if (this.client && this.client.connected) {
      this.client.publish('zyntra/command', cmd, { qos: 1 });
      console.log('[MQTT] Command published to zyntra/command:', cmd);
    } else {
      console.warn('[MQTT] Cannot send command — client not connected to broker');
    }
  }

  private _handleMessage(topic: string, raw: string) {
    try {
      const data = JSON.parse(raw);

      if (topic === 'zyntra/state') {
        const stateKey = data.state !== undefined ? data.state : data.name;
        const state = STATE_MAP[stateKey] ?? 'DISCONNECTED';
        console.log('[MQTT] Real hardware state update:', state);
        this._emitState(state);
      } else if (topic === 'zyntra/vitals') {
        const vitals: LiveVitals = {
          rmssd:            data.rmssd < 0 ? null : data.rmssd,
          hrvBaseline:      data.hrvBaseline ?? 80.0,
          skinTempC:        data.tempCurrent,
          tempBaselineC:    data.tempBaseline ?? 33.2,
          secondsRemaining: data.secondsRemaining ?? 0,
        };
        this.vitalsCbs.forEach(cb => cb(vitals));
      } else if (topic === 'zyntra/baseline') {
        const baseline: BaselineData = {
          hrvRmssd:   data.hrvRmssd ?? 80.0,
          tempC:      data.tempC ?? 33.2,
          rtMedianMs: data.rtMedianMs ?? 0,
          capturedAt: new Date().toISOString(),
        };
        console.log('[MQTT] Real baseline received from ESP32:', baseline);
        this.baselineCbs.forEach(cb => cb(baseline));
      } else if (topic === 'zyntra/result') {
        const result: ClearanceResult = {
          cleared:            !!data.cleared,
          hrvDataValid:       data.hrvDataValid !== undefined ? !!data.hrvDataValid : true,
          hrvPass:            !!data.hrvPass,
          tempPass:           !!data.tempPass,
          rtPass:             !!data.rtPass,
          rmssd:              data.rmssd < 0 ? null : data.rmssd,
          tempDeltaC:         data.tempDeltaC,
          medianRtMs:         data.medianRtMs,
          minutesToClearance: data.minutesToClearance ?? 0,
        };
        console.log('[MQTT] Real clearance result received from ESP32:', result.cleared ? 'CLEARED' : 'NOT CLEARED');
        this.resultCbs.forEach(cb => cb(result));
      }
    } catch {
      console.warn('[MQTT] Error parsing payload on topic', topic, raw);
    }
  }

  private _emitState(state: DeviceState) {
    this.currentState = state;
    this.stateCbs.forEach(cb => cb(state));
  }
}
