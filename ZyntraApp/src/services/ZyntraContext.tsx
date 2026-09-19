import React, { createContext, useCallback, useContext, useEffect, useMemo, useRef, useState } from 'react';
import { MqttZyntraLink } from './mqttLink';
import { loadUsers, updateUser } from './storage';
import { BaselineData, ClearanceResult, DeviceState, GymUser, LiveVitals, TestResult, ZyntraLink } from './types';

interface ZyntraStore {
  // MQTT state
  deviceState: DeviceState;
  vitals: LiveVitals | null;
  result: ClearanceResult | null;
  connecting: boolean;
  connect: (brokerUrl?: string) => Promise<void>;
  disconnect: () => Promise<void>;
  triggerBaseline: () => void;
  triggerBreak: () => void;
  acknowledgeResult: () => void;

  // User management
  users: GymUser[];
  selectedUser: GymUser | null;
  selectUser: (user: GymUser | null) => void;
  addUser: (user: Omit<GymUser, 'id' | 'createdAt' | 'baseline' | 'testResults'>) => Promise<void>;
  deleteUser: (id: string) => Promise<void>;

  // Baseline — saves to selected user
  saveBaseline: (data: BaselineData) => Promise<void>;

  // Test result — saves to selected user
  saveTestResult: (result: ClearanceResult) => Promise<void>;
}

const Ctx = createContext<ZyntraStore | null>(null);

export function ZyntraProvider({ children }: { children: React.ReactNode }) {
  const linkRef = useRef<ZyntraLink>(new MqttZyntraLink());

  const [deviceState, setDeviceState] = useState<DeviceState>('DISCONNECTED');
  const [vitals, setVitals] = useState<LiveVitals | null>(null);
  const [result, setResult] = useState<ClearanceResult | null>(null);
  const [connecting, setConnecting] = useState(false);
  const [users, setUsers] = useState<GymUser[]>([]);
  const [selectedUser, setSelectedUser] = useState<GymUser | null>(null);

  // Load users from storage and auto-connect MQTT on mount
  useEffect(() => {
    loadUsers().then(loaded => {
      setUsers(loaded);
      if (loaded.length > 0) setSelectedUser(loaded[0]);
    });
    // Auto-connect to MQTT on startup
    linkRef.current.connect().catch(err => {
      console.log('[ZyntraProvider] Auto-connect info:', err);
    });
  }, []);

  // Save baseline to currently selected user
  const saveBaseline = useCallback(async (data: BaselineData) => {
    if (!selectedUser) return;
    const updated: GymUser = { ...selectedUser, baseline: data };
    const all = await updateUser(updated);
    setUsers(all);
    setSelectedUser(updated);
  }, [selectedUser]);

  // Save test result to currently selected user
  const saveTestResult = useCallback(async (r: ClearanceResult) => {
    if (!selectedUser) return;
    const testResult: TestResult = {
      id: `test_${Date.now()}`,
      timestamp: new Date().toISOString(),
      cleared: r.cleared,
      hrvDataValid: r.hrvDataValid,
      hrvPass: r.hrvPass,
      tempPass: r.tempPass,
      rtPass: r.rtPass,
      rmssd: r.rmssd,
      tempDeltaC: r.tempDeltaC,
      medianRtMs: r.medianRtMs,
      minutesToClearance: r.minutesToClearance,
    };
    const updated: GymUser = {
      ...selectedUser,
      testResults: [testResult, ...selectedUser.testResults],
    };
    const all = await updateUser(updated);
    setUsers(all);
    setSelectedUser(updated);
  }, [selectedUser]);

  // Subscribe to MQTT hardware link events
  useEffect(() => {
    const link = linkRef.current;
    const offState    = link.onStateChange(setDeviceState);
    const offVitals   = link.onVitals(setVitals);
    const offBaseline = link.onBaseline(b => {
      saveBaseline(b);
    });
    const offResult   = link.onResult(r => {
      setResult(r);
      saveTestResult(r);
    });

    return () => {
      offState();
      offVitals();
      offBaseline();
      offResult();
    };
  }, [saveBaseline, saveTestResult]);

  const connect = useCallback(async (brokerUrl?: string) => {
    setConnecting(true);
    try {
      await linkRef.current.connect(brokerUrl);
    } catch (e) {
      console.warn('[ZyntraProvider] Connect error:', e);
    } finally {
      setConnecting(false);
    }
  }, []);

  const disconnect = useCallback(async () => {
    await linkRef.current.disconnect();
    setVitals(null);
    setResult(null);
  }, []);

  const triggerBaseline = useCallback(() => {
    linkRef.current.triggerBaseline();
  }, []);

  const triggerBreak = useCallback(() => {
    setResult(null);
    linkRef.current.triggerBreak();
  }, []);

  const acknowledgeResult = useCallback(() => {
    linkRef.current.acknowledgeResult();
    setResult(null);
    setVitals(null);
  }, []);

  const selectUser = useCallback((user: GymUser | null) => {
    setSelectedUser(user);
  }, []);

  const addUser = useCallback(async (data: Omit<GymUser, 'id' | 'createdAt' | 'baseline' | 'testResults'>) => {
    const newUser: GymUser = {
      ...data,
      id: `user_${Date.now()}`,
      createdAt: new Date().toISOString(),
      baseline: null,
      testResults: [],
    };
    const updated = await import('./storage').then(s => s.addUser(newUser));
    setUsers(updated);
    setSelectedUser(newUser);
  }, []);

  const deleteUser = useCallback(async (id: string) => {
    const updated = await import('./storage').then(s => s.deleteUser(id));
    setUsers(updated);
    if (selectedUser?.id === id) {
      setSelectedUser(updated.length > 0 ? updated[0] : null);
    }
  }, [selectedUser]);

  const store = useMemo<ZyntraStore>(() => ({
    deviceState, vitals, result, connecting,
    connect, disconnect, triggerBaseline, triggerBreak, acknowledgeResult,
    users, selectedUser, selectUser, addUser, deleteUser,
    saveBaseline, saveTestResult,
  }), [deviceState, vitals, result, connecting, connect, disconnect,
       triggerBaseline, triggerBreak, acknowledgeResult, users, selectedUser,
       selectUser, addUser, deleteUser, saveBaseline, saveTestResult]);

  return <Ctx.Provider value={store}>{children}</Ctx.Provider>;
}

export function useZyntra(): ZyntraStore {
  const ctx = useContext(Ctx);
  if (!ctx) throw new Error('useZyntra must be inside <ZyntraProvider>');
  return ctx;
}
