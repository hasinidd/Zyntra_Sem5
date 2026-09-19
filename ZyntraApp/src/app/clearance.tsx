import { useRouter } from 'expo-router';
import React from 'react';
import { Pressable, ScrollView, StyleSheet, Text, View } from 'react-native';
import { useZyntra } from '../services/ZyntraContext';
import { colors, radius } from '../theme';

export default function Clearance() {
  const router = useRouter();
  const { result, selectedUser, acknowledgeResult } = useZyntra();

  const done = () => {
    acknowledgeResult();
    router.dismissTo('/(tabs)');
  };

  if (!result) {
    return (
      <View style={styles.screen}>
        <Text style={styles.noResult}>No clearance result to display.</Text>
        <Pressable style={[styles.btn, styles.btnGold]} onPress={done}>
          <Text style={styles.btnGoldText}>Back to dashboard</Text>
        </Pressable>
      </View>
    );
  }

  const sensorError  = !result.hrvDataValid;
  const verdictColor = sensorError ? colors.warn : result.cleared ? colors.ready : colors.notReady;
  const verdictText  = sensorError
    ? 'Sensor Data Error'
    : result.cleared
      ? `${selectedUser?.name ?? 'User'} is ready`
      : `${selectedUser?.name ?? 'User'} is not ready`;

  // Failed signals summary string
  const failedList: string[] = [];
  if (!result.hrvPass)  failedList.push('Heart-Rate Variability (HRV)');
  if (!result.tempPass) failedList.push('Skin Temperature');
  if (!result.rtPass)   failedList.push('Reaction Time');

  const verdictSub = sensorError
    ? 'Sensor contact lost. Re-seat wristband and retry.'
    : result.cleared
      ? 'All 3 physiological signals passed baseline comparison.'
      : `These signals failed: ${failedList.join(', ')}`;

  return (
    <ScrollView style={styles.screen} contentContainerStyle={styles.content}>

      {/* Hero verdict banner */}
      <View style={[styles.verdictCard, { borderColor: verdictColor }]}>
        <Text style={styles.verdictLabel}>POST-WORKOUT READINESS</Text>
        <Text style={[styles.verdict, { color: verdictColor }]}>{verdictText}</Text>
        <Text style={styles.verdictSub}>{verdictSub}</Text>

        {!result.cleared && !sensorError && (
          <View style={styles.recoveryTimeCard}>
            <Text style={styles.recoveryTimeLabel}>This much time required to recover:</Text>
            <Text style={styles.recoveryTimeValue}>{result.minutesToClearance} minutes</Text>
          </View>
        )}
      </View>

      {/* Signal Values vs Baseline */}
      <View style={styles.valuesRow}>
        <ValueBox
          label="RMSSD"
          value={result.rmssd != null ? `${result.rmssd.toFixed(1)} ms` : '—'}
          pass={result.hrvPass}
          invalid={sensorError}
        />
        <ValueBox
          label="TEMP Δ"
          value={result.tempDeltaC != null ? `${result.tempDeltaC.toFixed(1)} °C` : '—'}
          pass={result.tempPass}
        />
        <ValueBox
          label="MEDIAN RT"
          value={result.medianRtMs != null ? `${result.medianRtMs} ms` : '—'}
          pass={result.rtPass}
        />
      </View>

      {/* Per-signal Breakdown */}
      <View style={styles.card}>
        <Text style={styles.sectionLabel}>SIGNAL ANALYSIS</Text>

        <SignalRow
          name="Heart-Rate Variability (RMSSD)"
          detail={result.rmssd != null ? `${result.rmssd.toFixed(1)} ms (Threshold ≥ 90% baseline)` : 'No valid beat data'}
          pass={result.hrvPass}
          invalid={sensorError}
        />
        <View style={styles.divider} />

        <SignalRow
          name="Skin Temperature"
          detail={result.tempDeltaC != null ? `Deviation: Δ ${result.tempDeltaC.toFixed(1)} °C (Threshold ≤ 0.8 °C)` : '—'}
          pass={result.tempPass}
        />
        <View style={styles.divider} />

        <SignalRow
          name="Reaction Time"
          detail={result.medianRtMs != null ? `Median response: ${result.medianRtMs} ms (Threshold < 500 ms)` : '—'}
          pass={result.rtPass}
        />
      </View>

      <Text style={styles.gateNote}>
        All 3 signals must pass baseline criteria simultaneously (tri-modal AND-gate).
      </Text>

      <Pressable style={[styles.btn, styles.btnGold]} onPress={done}>
        <Text style={styles.btnGoldText}>Acknowledge & return to main</Text>
      </Pressable>

    </ScrollView>
  );
}

function ValueBox({ label, value, pass, invalid }: {
  label: string; value: string; pass: boolean; invalid?: boolean;
}) {
  const col = invalid ? colors.warn : pass ? colors.ready : colors.notReady;
  return (
    <View style={[styles.valueBox, { borderColor: col }]}>
      <Text style={[styles.valueNum, { color: col }]}>{value}</Text>
      <Text style={styles.valueLabel}>{label}</Text>
      <View style={[styles.valueBadge, { backgroundColor: col + '22' }]}>
        <Text style={[styles.valueBadgeText, { color: col }]}>
          {invalid ? 'NO DATA' : pass ? 'PASS' : 'FAIL'}
        </Text>
      </View>
    </View>
  );
}

function SignalRow({ name, detail, pass, invalid }: {
  name: string; detail: string; pass: boolean; invalid?: boolean;
}) {
  const badgeColor = invalid ? colors.warn : pass ? colors.ready : colors.notReady;
  const badgeText  = invalid ? 'NO DATA' : pass ? 'PASS' : 'FAIL';
  return (
    <View style={styles.signalRow}>
      <View style={{ flex: 1 }}>
        <Text style={styles.signalName}>{name}</Text>
        <Text style={styles.signalDetail}>{detail}</Text>
      </View>
      <View style={[styles.badge, { backgroundColor: badgeColor + '22', borderColor: badgeColor, borderWidth: 1 }]}>
        <Text style={[styles.badgeText, { color: badgeColor }]}>{badgeText}</Text>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  screen:             { flex: 1, backgroundColor: colors.bg },
  content:            { padding: 20, gap: 14 },
  noResult:           { color: colors.textDim, fontSize: 15, textAlign: 'center', marginTop: 60, marginBottom: 24 },

  verdictCard:        { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 2, padding: 24, gap: 10, alignItems: 'center' },
  verdictLabel:       { fontSize: 11, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  verdict:            { fontSize: 32, fontWeight: '900', letterSpacing: 0.5, textAlign: 'center' },
  verdictSub:         { color: colors.textDim, fontSize: 13, textAlign: 'center', lineHeight: 18 },

  recoveryTimeCard:   { marginTop: 8, backgroundColor: colors.bg, borderRadius: 10, paddingVertical: 10, paddingHorizontal: 16, alignItems: 'center', borderWidth: 1, borderColor: colors.notReady + '66' },
  recoveryTimeLabel:  { fontSize: 12, color: colors.textDim, fontWeight: '600' },
  recoveryTimeValue:  { fontSize: 20, fontWeight: '900', color: colors.notReady, marginTop: 2 },

  valuesRow:          { flexDirection: 'row', gap: 10 },
  valueBox:           { flex: 1, backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1.5, padding: 12, alignItems: 'center', gap: 4 },
  valueNum:           { fontSize: 17, fontWeight: '900' },
  valueLabel:         { fontSize: 9, fontWeight: '700', color: colors.textDim, letterSpacing: 0.6 },
  valueBadge:         { borderRadius: 6, paddingHorizontal: 8, paddingVertical: 3, marginTop: 2 },
  valueBadgeText:     { fontSize: 10, fontWeight: '900' },

  card:               { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.border, padding: 18, gap: 14 },
  sectionLabel:       { fontSize: 11, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  divider:            { height: 1, backgroundColor: colors.border },
  signalRow:          { flexDirection: 'row', alignItems: 'center', gap: 12 },
  signalName:         { fontSize: 14, fontWeight: '700', color: colors.text },
  signalDetail:       { fontSize: 12, color: colors.textDim, marginTop: 2 },
  badge:              { borderRadius: 8, paddingHorizontal: 10, paddingVertical: 5 },
  badgeText:          { fontSize: 12, fontWeight: '900' },

  gateNote:           { color: colors.textMuted, fontSize: 12, textAlign: 'center' },
  btn:                { borderRadius: radius.button, paddingVertical: 14, alignItems: 'center' },
  btnGold:            { backgroundColor: colors.gold },
  btnGoldText:        { color: '#0A0A0A', fontSize: 15, fontWeight: '800' },
});
