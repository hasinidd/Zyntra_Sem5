import React, { useState } from 'react';
import {
  Alert, FlatList, Platform, Pressable,
  StyleSheet, Text, View,
} from 'react-native';
import { getRtThresholdForAge, getTempMarginForAge, useZyntra } from '../../services/ZyntraContext';
import { GymUser, TestResult } from '../../services/types';
import { colors, radius } from '../../theme';

export default function ResultsTab() {
  const { users, selectedUser, deleteUser } = useZyntra();
  const [expandedUser, setExpandedUser] = useState<string | null>(
    selectedUser?.id ?? null
  );

  if (users.length === 0) {
    return (
      <View style={styles.empty}>
        <Text style={styles.emptyEmoji}>📊</Text>
        <Text style={styles.emptyTitle}>No captured results yet</Text>
        <Text style={styles.emptySub}>Add users in the Users tab and capture baselines/tests to see results here.</Text>
      </View>
    );
  }

  return (
    <FlatList
      data={users}
      keyExtractor={u => u.id}
      contentContainerStyle={{ padding: 16, gap: 14 }}
      renderItem={({ item }) => (
        <UserResultCard
          user={item}
          expanded={expandedUser === item.id}
          onToggle={() => setExpandedUser(prev => prev === item.id ? null : item.id)}
          onDelete={() => deleteUser(item.id)}
        />
      )}
    />
  );
}

function UserResultCard({ user, expanded, onToggle, onDelete }: {
  user: GymUser;
  expanded: boolean;
  onToggle: () => void;
  onDelete: () => void;
}) {
  const latest = user.testResults[0];

  const handleDelete = () => {
    if (Platform.OS === 'web') {
      if (typeof window !== 'undefined' && window.confirm(`Remove ${user.name} and all recorded test data?`)) {
        onDelete();
      }
    } else {
      Alert.alert(
        'Delete participant',
        `Remove ${user.name} and all recorded test data?`,
        [
          { text: 'Cancel', style: 'cancel' },
          { text: 'Delete', style: 'destructive', onPress: onDelete },
        ]
      );
    }
  };

  return (
    <View style={styles.card}>
      {/* User header */}
      <Pressable style={styles.cardHeader} onPress={onToggle}>
        <View style={styles.avatar}>
          <Text style={styles.avatarText}>{user.name.charAt(0).toUpperCase()}</Text>
        </View>
        <View style={{ flex: 1 }}>
          <Text style={styles.userName}>{user.name}</Text>
          <Text style={styles.userMeta}>{user.age}y · {user.heightCm}cm · {user.weightKg}kg · {user.role}</Text>
        </View>
        {latest ? (
          <View style={[styles.verdictBadge, {
            backgroundColor: !latest.hrvDataValid ? colors.warn + '33' :
              latest.cleared ? colors.ready + '33' : colors.notReady + '33'
          }]}>
            <Text style={[styles.verdictBadgeText, {
              color: !latest.hrvDataValid ? colors.warn :
                latest.cleared ? colors.ready : colors.notReady
            }]}>
              {!latest.hrvDataValid ? 'DATA ERR' : latest.cleared ? 'READY' : 'NOT READY'}
            </Text>
          </View>
        ) : (
          <Text style={styles.noTests}>No tests run</Text>
        )}
        <Text style={styles.chevron}>{expanded ? '▲' : '▼'}</Text>
      </Pressable>

      {expanded && (
        <View style={styles.expanded}>
          {/* Baseline Data */}
          {user.baseline ? (
            <View style={styles.section}>
              <Text style={styles.sectionTitle}>RESTING BASELINE DATA</Text>
              <View style={styles.baselineRow}>
                <MiniStat label="HRV RMSSD" value={`${user.baseline.hrvRmssd.toFixed(1)} ms`} />
                <MiniStat label="SKIN TEMP" value={`${user.baseline.tempC.toFixed(1)} °C`} />
                <MiniStat label="MEDIAN RT" value={`${user.baseline.rtMedianMs} ms`} />
              </View>
              <Text style={styles.timestamp}>
                Captured {new Date(user.baseline.capturedAt).toLocaleString()}
              </Text>
            </View>
          ) : (
            <Text style={styles.noData}>No baseline captured yet for this user.</Text>
          )}

          {/* Latest Test Result */}
          {latest && (
            <View style={styles.section}>
              <Text style={styles.sectionTitle}>LATEST RECOVERY TEST</Text>
              <ResultRow result={latest} user={user} />
            </View>
          )}

          {/* History */}
          {user.testResults.length > 1 && (
            <View style={styles.section}>
              <Text style={styles.sectionTitle}>TEST HISTORY ({user.testResults.length} sessions)</Text>
              {user.testResults.map((r, i) => (
                <View key={r.id}>
                  {i > 0 && <ResultRow result={r} user={user} compact />}
                </View>
              ))}
            </View>
          )}

          {/* Delete User Option */}
          <Pressable style={styles.deleteUserBtn} onPress={handleDelete}>
            <Text style={styles.deleteUserText}>Remove participant profile</Text>
          </Pressable>
        </View>
      )}
    </View>
  );
}

function ResultRow({ result, user, compact = false }: {
  result: TestResult;
  user: GymUser;
  compact?: boolean;
}) {
  const sensorError = !result.hrvDataValid;
  const rtThreshold = getRtThresholdForAge(user.age);
  const tempMargin  = getTempMarginForAge(user.age);

  let tempDeltaDisplay = '—';
  let tempDeltaVal = result.tempDeltaC;
  if (tempDeltaVal !== null) {
    if (tempDeltaVal > 5.0 && user.baseline) {
      tempDeltaVal = Number(Math.abs(tempDeltaVal - user.baseline.tempC).toFixed(1));
    } else {
      tempDeltaVal = Number(tempDeltaVal.toFixed(1));
    }
    tempDeltaDisplay = `Δ${tempDeltaVal}°C`;
  }

  // Dynamic pass flags according to age bracket & personal baseline
  const hrvPass = (result.rmssd !== null && user.baseline)
    ? (result.rmssd >= 0.9 * user.baseline.hrvRmssd)
    : result.hrvPass;

  const tempPass = (tempDeltaVal !== null)
    ? (tempDeltaVal <= tempMargin)
    : result.tempPass;

  const rtPass = (result.medianRtMs !== null)
    ? (result.medianRtMs < rtThreshold)
    : result.rtPass;

  const cleared = !sensorError && hrvPass && tempPass && rtPass;

  const verdictColor = sensorError ? colors.warn :
    cleared ? colors.ready : colors.notReady;
  const verdictText = sensorError ? 'DATA ERROR' :
    cleared ? `${user.name} is ready` : `${user.name} is not ready`;

  if (compact) {
    return (
      <View style={styles.compactRow}>
        <Text style={styles.timestamp}>{new Date(result.timestamp).toLocaleString()}</Text>
        <View style={[styles.smallBadge, { backgroundColor: verdictColor + '22' }]}>
          <Text style={[styles.smallBadgeText, { color: verdictColor }]}>{verdictText}</Text>
        </View>
      </View>
    );
  }

  return (
    <View style={styles.resultCard}>
      <Text style={[styles.verdict, { color: verdictColor }]}>{verdictText}</Text>
      <View style={styles.signalRow}>
        <SignalBadge label="HRV" value={result.rmssd != null ? `${result.rmssd.toFixed(1)}ms` : '—'} pass={hrvPass} invalid={sensorError} />
        <SignalBadge label="TEMP" value={tempDeltaDisplay} pass={tempPass} />
        <SignalBadge label="RT" value={result.medianRtMs != null ? `${result.medianRtMs}ms` : '—'} pass={rtPass} />
      </View>
      {!cleared && !sensorError && (
        <Text style={styles.retestTime}>Required recovery time: {result.minutesToClearance} min</Text>
      )}
      <Text style={styles.timestamp}>{new Date(result.timestamp).toLocaleString()}</Text>
    </View>
  );
}

function SignalBadge({ label, value, pass, invalid }: {
  label: string; value: string; pass: boolean; invalid?: boolean;
}) {
  const col = invalid ? colors.warn : pass ? colors.ready : colors.notReady;
  return (
    <View style={[sigStyles.box, { borderColor: col }]}>
      <Text style={[sigStyles.val, { color: col }]}>{value}</Text>
      <Text style={sigStyles.lbl}>{label}</Text>
      <Text style={[sigStyles.status, { color: col }]}>
        {invalid ? 'ERR' : pass ? 'PASS' : 'FAIL'}
      </Text>
    </View>
  );
}

function MiniStat({ label, value }: { label: string; value: string }) {
  return (
    <View style={styles.miniStat}>
      <Text style={styles.miniVal}>{value}</Text>
      <Text style={styles.miniLbl}>{label}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  empty:          { flex: 1, backgroundColor: colors.bg, alignItems: 'center', justifyContent: 'center', gap: 12, padding: 32 },
  emptyEmoji:     { fontSize: 48 },
  emptyTitle:     { fontSize: 20, fontWeight: '700', color: colors.text },
  emptySub:       { fontSize: 14, color: colors.textDim, textAlign: 'center' },
  card:           { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.border, overflow: 'hidden' },
  cardHeader:     { flexDirection: 'row', alignItems: 'center', padding: 14, gap: 10 },
  avatar:         { width: 40, height: 40, borderRadius: 20, backgroundColor: colors.surfaceHigh, alignItems: 'center', justifyContent: 'center' },
  avatarText:     { color: colors.gold, fontSize: 16, fontWeight: '900' },
  userName:       { fontSize: 15, fontWeight: '700', color: colors.text },
  userMeta:       { fontSize: 11, color: colors.textDim },
  verdictBadge:   { borderRadius: 8, paddingHorizontal: 8, paddingVertical: 4 },
  verdictBadgeText: { fontSize: 10, fontWeight: '900' },
  noTests:        { fontSize: 11, color: colors.textMuted },
  chevron:        { color: colors.textDim, fontSize: 12 },
  expanded:       { borderTopWidth: 1, borderTopColor: colors.border, padding: 14, gap: 14 },
  section:        { gap: 8 },
  sectionTitle:   { fontSize: 10, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  baselineRow:    { flexDirection: 'row', gap: 8 },
  miniStat:       { flex: 1, backgroundColor: colors.bg, borderRadius: 8, padding: 10, alignItems: 'center', gap: 2 },
  miniVal:        { fontSize: 15, fontWeight: '800', color: colors.gold },
  miniLbl:        { fontSize: 9, fontWeight: '700', color: colors.textDim },
  resultCard:     { backgroundColor: colors.bg, borderRadius: 10, padding: 12, gap: 8 },
  verdict:        { fontSize: 18, fontWeight: '900' },
  signalRow:      { flexDirection: 'row', gap: 8 },
  retestTime:     { fontSize: 12, color: colors.notReady, fontWeight: '600' },
  timestamp:      { fontSize: 11, color: colors.textMuted },
  compactRow:     { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', paddingVertical: 4 },
  smallBadge:     { borderRadius: 6, paddingHorizontal: 8, paddingVertical: 3 },
  smallBadgeText: { fontSize: 10, fontWeight: '800' },
  noData:         { color: colors.textMuted, fontSize: 13, fontStyle: 'italic' },
  deleteUserBtn:  { marginTop: 4, paddingVertical: 8, alignItems: 'center', borderWidth: 1, borderColor: colors.notReady + '44', borderRadius: 8 },
  deleteUserText: { color: colors.notReady, fontSize: 12, fontWeight: '600' },
});

const sigStyles = StyleSheet.create({
  box:    { flex: 1, borderRadius: 8, borderWidth: 1, padding: 8, alignItems: 'center', gap: 2 },
  val:    { fontSize: 13, fontWeight: '800' },
  lbl:    { fontSize: 9, color: colors.textDim, fontWeight: '600' },
  status: { fontSize: 10, fontWeight: '900' },
});
