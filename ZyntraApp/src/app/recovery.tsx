import { useRouter } from 'expo-router';
import React, { useEffect, useRef } from 'react';
import { StyleSheet, Text, View } from 'react-native';
import { useZyntra } from '../services/ZyntraContext';
import { DeviceState } from '../services/types';
import { colors, radius } from '../theme';

export default function Recovery() {
  const router = useRouter();
  const { deviceState, vitals } = useZyntra();

  const prev = useRef<DeviceState>(deviceState);
  useEffect(() => {
    if ((deviceState === 'CLEARED' || deviceState === 'NOT_CLEARED') && prev.current !== deviceState) {
      router.replace('/clearance');
    }
    prev.current = deviceState;
  }, [deviceState, router]);

  const inClearance = deviceState === 'CLEARANCE';
  const secs = vitals?.secondsRemaining ?? 0;
  const mm = String(Math.floor(secs / 60)).padStart(2, '0');
  const ss = String(secs % 60).padStart(2, '0');

  const hrvNow = vitals?.rmssd;
  const hrvBase = vitals?.hrvBaseline ?? 0;
  const hrvTarget = hrvBase * 0.9;
  const hrvPct = hrvNow != null ? Math.min((hrvNow / hrvTarget) * 100, 100) : 0;

  const tempNow = vitals?.skinTempC;
  const tempBase = vitals?.tempBaselineC ?? 0;
  const tempDelta = tempNow != null ? Math.abs(tempNow - tempBase) : null;
  const tempOk = tempDelta != null && tempDelta <= 0.8;

  return (
    <View style={styles.screen}>
      {inClearance ? (
        <View style={[styles.card, { borderColor: colors.gold }]}>
          <Text style={[styles.bigLabel, { color: colors.gold }]}>Clearance test running</Text>
          <Text style={styles.sub}>Reaction-time stimuli in progress on wristband…</Text>
        </View>
      ) : (
        <View style={styles.countdownBlock}>
          <Text style={styles.countdownLabel}>TIME REMAINING</Text>
          <Text style={styles.countdown}>{mm}:{ss}</Text>
        </View>
      )}

      {/* HRV card */}
      <View style={styles.card}>
        <Text style={styles.cardLabel}>HEART-RATE VARIABILITY (RMSSD)</Text>
        {hrvNow == null ? (
          <Text style={styles.signalLost}>⚠ Signal lost — check wrist contact</Text>
        ) : (
          <>
            <View style={styles.valueRow}>
              <Text style={styles.bigValue}>{hrvNow.toFixed(1)}</Text>
              <Text style={styles.unit}>ms</Text>
            </View>
            <View style={styles.bar}>
              <View style={[styles.barFill, { width: `${hrvPct}%`, backgroundColor: hrvPct >= 100 ? colors.ready : colors.gold }]} />
            </View>
          </>
        )}
        <Text style={styles.sub}>
          Target ≥ {hrvTarget.toFixed(1)} ms · baseline {hrvBase.toFixed(1)} ms
        </Text>
      </View>

      {/* Temperature card */}
      <View style={styles.card}>
        <Text style={styles.cardLabel}>SKIN TEMPERATURE</Text>
        <View style={styles.valueRow}>
          <Text style={styles.bigValue}>{tempNow != null ? tempNow.toFixed(1) : '—'}</Text>
          <Text style={styles.unit}>°C</Text>
          {tempDelta != null && (
            <View style={[styles.deltaBadge, { backgroundColor: tempOk ? colors.ready + '33' : colors.notReady + '33' }]}>
              <Text style={[styles.deltaText, { color: tempOk ? colors.ready : colors.notReady }]}>
                Δ {tempDelta.toFixed(1)} °C
              </Text>
            </View>
          )}
        </View>
        <Text style={styles.sub}>Must stay within 0.8 °C of baseline {tempBase.toFixed(1)} °C</Text>
      </View>

      <Text style={styles.note}>
        Reaction-time test fires automatically at end of window.
      </Text>
    </View>
  );
}

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: colors.bg, padding: 20, gap: 14 },
  countdownBlock: { alignItems: 'center', paddingVertical: 8 },
  countdownLabel: { fontSize: 11, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  countdown: { fontSize: 80, fontWeight: '900', color: colors.text, fontVariant: ['tabular-nums'] },
  card: { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.border, padding: 18, gap: 8 },
  bigLabel: { fontSize: 20, fontWeight: '800' },
  cardLabel: { fontSize: 11, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  valueRow: { flexDirection: 'row', alignItems: 'flex-end', gap: 6 },
  bigValue: { fontSize: 44, fontWeight: '900', color: colors.text },
  unit: { fontSize: 18, color: colors.textDim, fontWeight: '600', marginBottom: 8 },
  bar: { height: 6, backgroundColor: colors.surfaceHigh, borderRadius: 3, overflow: 'hidden' },
  barFill: { height: 6, borderRadius: 3 },
  signalLost: { fontSize: 18, fontWeight: '700', color: colors.warn },
  sub: { fontSize: 12, color: colors.textDim },
  deltaBadge: { borderRadius: 8, paddingHorizontal: 8, paddingVertical: 3, marginBottom: 8 },
  deltaText: { fontSize: 12, fontWeight: '700' },
  note: { color: colors.textMuted, fontSize: 12, textAlign: 'center', marginTop: 'auto' },
});
