import { BleManager, BleError, Characteristic, Device } from 'react-native-ble-plx';
import { PermissionsAndroid, Platform } from 'react-native';
import { ClearanceResult, DeviceState, LiveVitals, ZyntraLink } from './types';

const SERVICE_UUID = '12345678-1234-1234-1234-123456789012';
const CHAR_STATE   = 'aaaaaaaa-1234-1234-1234-123456789001';
const CHAR_VITALS  = 'aaaaaaaa-1234-1234-1234-123456789002';
const CHAR_RESULT  = 'aaaaaaaa-1234-1234-1234-123456789003';
const CHAR_CMD     = 'aaaaaaaa-1234-1234-1234-123456789004';
const DEVICE_NAME  = 'Zyntra_01';

const STATE_MAP: Record<number, DeviceState> = {
  0: 'DISCONNECTED',
  1: 'BASELINE',
  2: 'SHIFT',
  3: 'RECOVERY',
  4: 'CLEARANCE',
  5: 'CLEARED',
  6: 'NOT_CLEARED',
};

export class BleZyntraLink implements ZyntraLink {
  private manager = new BleManager();
  private device: Device | null = null;
  private stateCbs  = new Set<(s: DeviceState) => void>();
  private vitalsCbs = new Set<(v: LiveVitals) => void>();
  private resultCbs = new Set<(r: ClearanceResult) => void>();
  private currentState: DeviceState = 'DISCONNECTED';

  // ── Connect ──────────────────────────────────────────────────────────────
  async connect(): Promise<void> {
    // Request Android BLE permissions first
    if (Platform.OS === 'android') {
      try {
        const granted = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        ]);
        console.log('[BLE] Permissions:', JSON.stringify(granted));
      } catch (e) {
        console.warn('[BLE] Permission request failed:', e);
      }
    }

    return new Promise((resolve, reject) => {
      let timeoutHandle: ReturnType<typeof setTimeout>;
      let resolved = false;

      // Scan all devices (null = no UUID filter — Android sometimes misses UUID filter)
      this.manager.startDeviceScan(
        null,
        { allowDuplicates: false },
        async (error: BleError | null, scannedDevice: Device | null) => {
          if (error) {
            clearTimeout(timeoutHandle);
            if (!resolved) { resolved = true; reject(error); }
            return;
          }

          // Filter by device name
          if (!scannedDevice?.name?.includes(DEVICE_NAME)) return;

          // Found it — stop scan and clear timeout immediately
          this.manager.stopDeviceScan();
          clearTimeout(timeoutHandle);

          if (resolved) return;
          resolved = true;

          console.log('[BLE] Found', DEVICE_NAME, '— connecting...');

          try {
            const connected = await scannedDevice.connect({ timeout: 10000 });
            const discovered = await connected.discoverAllServicesAndCharacteristics();
            this.device = discovered;

            // Subscribe to STATE notifications
            discovered.monitorCharacteristicForService(
              SERVICE_UUID, CHAR_STATE,
              (err, char) => { if (!err && char) this._handleState(char); }
            );

            // Subscribe to VITALS notifications
            discovered.monitorCharacteristicForService(
              SERVICE_UUID, CHAR_VITALS,
              (err, char) => { if (!err && char) this._handleVitals(char); }
            );

            // Subscribe to RESULT notifications
            discovered.monitorCharacteristicForService(
              SERVICE_UUID, CHAR_RESULT,
              (err, char) => { if (!err && char) this._handleResult(char); }
            );

            // Handle unexpected disconnection
            connected.onDisconnected((err, dev) => {
              console.log('[BLE] Device disconnected');
              this.device = null;
              this._emitState('DISCONNECTED');
            });

            console.log('[BLE] Connected and subscribed to', DEVICE_NAME);
            this._emitState('BASELINE'); // ESP32 starts in BASELINE
            resolve();
          } catch (e) {
            reject(e);
          }
        }
      );

      // Timeout after 30 seconds — cleared immediately when device found
      timeoutHandle = setTimeout(() => {
        if (!resolved) {
          resolved = true;
          this.manager.stopDeviceScan();
          reject(new Error('BLE scan timeout — make sure Zyntra-01 is powered on'));
        }
      }, 30000);
    });
  }

  // ── Disconnect ────────────────────────────────────────────────────────────
  async disconnect(): Promise<void> {
    if (this.device) {
      await this.device.cancelConnection();
      this.device = null;
    }
    this._emitState('DISCONNECTED');
  }

  // ── Supervisor commands ───────────────────────────────────────────────────
  triggerBreak(): void { this._writeCommand(0x01); }
  acknowledgeResult(): void { this._writeCommand(0x02); }

  // ── Subscriptions ─────────────────────────────────────────────────────────
  onStateChange(cb: (s: DeviceState) => void): () => void {
    this.stateCbs.add(cb);
    cb(this.currentState);
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

  // ── Packet parsers ────────────────────────────────────────────────────────

  private _handleState(char: Characteristic) {
    const bytes = this._decode(char.value);
    if (!bytes || bytes.length < 1) return;
    const state = STATE_MAP[bytes[0]] ?? 'DISCONNECTED';
    console.log('[BLE] State received:', state);
    this._emitState(state);
  }

  private _handleVitals(char: Characteristic) {
    const bytes = this._decode(char.value);
    if (!bytes || bytes.length < 8) return;

    const rmssdRaw    = this._readInt16(bytes, 0);
    const tempBaseRaw = this._readInt16(bytes, 2);
    const tempCurRaw  = this._readInt16(bytes, 4);
    const secsLeft    = this._readUint16(bytes, 6);

    const vitals: LiveVitals = {
      rmssd:            rmssdRaw === -1 ? null : rmssdRaw / 10,
      hrvBaseline:      80.0,
      skinTempC:        tempCurRaw / 10,
      tempBaselineC:    tempBaseRaw / 10,
      secondsRemaining: secsLeft,
    };

    this.vitalsCbs.forEach(cb => cb(vitals));
  }

  private _handleResult(char: Characteristic) {
    const bytes = this._decode(char.value);
    if (!bytes || bytes.length < 8) return;

    const flags        = bytes[0];
    const hrvDataValid = !!(flags & (1 << 0));
    const cleared      = !!(flags & (1 << 1));
    const hrvPass      = !!(flags & (1 << 2));
    const tempPass     = !!(flags & (1 << 3));
    const rtPass       = !!(flags & (1 << 4));

    const hrvRaw  = this._readInt16(bytes, 1);
    const tempRaw = this._readInt16(bytes, 3);
    const rtRaw   = this._readUint16(bytes, 5);
    const mins    = bytes[7];

    const result: ClearanceResult = {
      hrvDataValid,
      cleared,
      hrvPass,
      tempPass,
      rtPass,
      rmssd:              hrvRaw === -1 ? null : hrvRaw / 10,
      tempDeltaC:         tempRaw / 10,
      medianRtMs:         rtRaw,
      minutesToClearance: mins,
    };

    console.log('[BLE] Result received:', cleared ? 'CLEARED' : 'NOT CLEARED');
    this.resultCbs.forEach(cb => cb(result));
  }

  // ── Helpers ───────────────────────────────────────────────────────────────

  private async _writeCommand(cmd: number) {
    if (!this.device) {
      console.warn('[BLE] Cannot send command — not connected');
      return;
    }
    try {
      const b64 = Buffer.from([cmd]).toString('base64');
      await this.device.writeCharacteristicWithResponseForService(
        SERVICE_UUID, CHAR_CMD, b64
      );
      console.log('[BLE] Command sent: 0x' + cmd.toString(16));
    } catch (e) {
      console.error('[BLE] Command write failed:', e);
    }
  }

  private _emitState(state: DeviceState) {
    this.currentState = state;
    this.stateCbs.forEach(cb => cb(state));
  }

  private _decode(b64: string | null): Uint8Array | null {
    if (!b64) return null;
    return new Uint8Array(Buffer.from(b64, 'base64'));
  }

  private _readInt16(bytes: Uint8Array, offset: number): number {
    const val = bytes[offset] | (bytes[offset + 1] << 8);
    return val > 32767 ? val - 65536 : val;
  }

  private _readUint16(bytes: Uint8Array, offset: number): number {
    return bytes[offset] | (bytes[offset + 1] << 8);
  }
}