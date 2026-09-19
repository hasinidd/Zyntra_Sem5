import { useRouter } from 'expo-router';
import React, { useEffect, useRef, useState } from 'react';
import {
  ActivityIndicator, Pressable, ScrollView,
  StyleSheet, Text, TextInput, View,
} from 'react-native';
import { useZyntra } from '../../services/ZyntraContext';
import { DeviceState } from '../../services/types';
import { colors, radius } from '../../theme';

const STATE_LABEL: Record<DeviceState, string> = {
  DISCONNECTED: 'MQTT Offline',
  BASELINE:     'Capturing baseline on ESP32...',
  SHIFT:        'ESP32 Online — Ready',
  RECOVERY:     'Recovery test in progress...',
  CLEARANCE:    'Running reaction clearance test...',
  CLEARED:      'User is Ready',
  NOT_CLEARED:  'User is Not Ready',
};

const STATE_COLOR: Record<DeviceState, string> = {
  DISCONNECTED: colors.textMuted,
  BASELINE:     colors.gold,
  SHIFT:        colors.ready,
  RECOVERY:     colors.gold,
  CLEARANCE:    colors.gold,
  CLEARED:      colors.ready,
  NOT_CLEARED:  colors.notReady,
};

export default function TestTab() {
  const router = useRouter();
  const {
    deviceState, connecting,
    connect, disconnect, triggerBaseline, triggerBreak,
    selectedUser,
  } = useZyntra();

  const [brokerUrl, setBrokerUrl] = useState('wss://broker.hivemq.com:8884/mqtt');
  const prevState = useRef<DeviceState>(deviceState);

  // Navigation triggers
  useEffect(() => {
    if (prevState.current !== 'RECOVERY' && deviceState === 'RECOVERY') {
      router.push('/recovery');
    }
    if ((deviceState === 'CLEARED' || deviceState === 'NOT_CLEARED') &&
        prevState.current !== deviceState) {
      router.push('/clearance');
    }
    prevState.current = deviceState;
  }, [deviceState, router]);

  const connected = deviceState !== 'DISCONNECTED';

  if (!selectedUser) {
    return (
      <View style={styles.noUser}>
        <Text style={styles.noUserEmoji}>👤</Text>
        <Text style={styles.noUserTitle}>No user selected</Text>
        <Text style={styles.noUserSub}>Go to the Users tab, add or tap a user to start testing.</Text>
        <Pressable style={styles.goBtn} onPress={() => router.push('/(tabs)')}>
          <Text style={styles.goBtnText}>Go to Users</Text>
        </Pressable>
      </View>
    );
  }

  return (
    <ScrollView style={styles.screen} contentContainerStyle={styles.content}>

      {/* Selected User Header */}
      <View style={styles.userCard}>
        <View style={styles.avatar}>
          <Text style={styles.avatarText}>{selectedUser.name.charAt(0).toUpperCase()}</Text>
        </View>
        <View style={{ flex: 1 }}>
          <Text style={styles.userName}>{selectedUser.name}</Text>
          <Text style={styles.userMeta}>
            {selectedUser.age}y · {selectedUser.heightCm}cm · {selectedUser.weightKg}kg · {selectedUser.role}
          </Text>
        </View>
        <Pressable onPress={() => router.push('/(tabs)')}>
          <Text style={styles.changeText}>Switch user</Text>
        </Pressable>
      </View>

      {/* MQTT Broker & Wristband Connection */}
      <View style={styles.card}>
        <View style={styles.cardRow}>
          <Text style={styles.cardTitle}>ESP32 Wristband Link (MQTT)</Text>
          <View style={[styles.dot, { backgroundColor: connected ? colors.ready : colors.textMuted }]} />
        </View>

        <TextInput
          style={styles.input}
          placeholder="MQTT Broker WebSocket URL"
          placeholderTextColor={colors.textMuted}
          value={brokerUrl}
          onChangeText={setBrokerUrl}
          editable={!connected}
        />

        <Text style={[styles.stateText, { color: STATE_COLOR[deviceState] }]}>
          {STATE_LABEL[deviceState]}
        </Text>

        <Pressable
          style={[styles.btn, connected ? styles.btnGhost : styles.btnGold, connecting && styles.btnDisabled]}
          onPress={connected ? disconnect : () => connect(brokerUrl)}
          disabled={connecting}
        >
          {connecting ? (
            <ActivityIndicator color={colors.text} />
          ) : (
            <Text style={connected ? styles.btnGhostText : styles.btnGoldText}>
              {connected ? 'Disconnect MQTT' : 'Connect to MQTT Broker'}
            </Text>
          )}
        </Pressable>
      </View>

      {/* STEP 1: Baseline Test */}
      <View style={styles.card}>
        <Text style={styles.cardTitle}>Step 1 — Baseline Test</Text>
        <Text style={styles.cardSub}>
          Click the button below to start the resting baseline test on the wristband. The ESP32 will record resting HRV (RMSSD), skin temperature, and reaction time.
        </Text>

        {selectedUser.baseline ? (
          <View style={styles.baselineResult}>
            <Text style={styles.baselineTitle}>Baseline captured for {selectedUser.name} ✓</Text>
            <View style={styles.baselineRow}>
              <BaselineVal label="HRV" value={`${selectedUser.baseline.hrvRmssd.toFixed(1)} ms`} />
              <BaselineVal label="TEMP" value={`${selectedUser.baseline.tempC.toFixed(1)} °C`} />
              <BaselineVal label="RT" value={`${selectedUser.baseline.rtMedianMs} ms`} />
            </View>
            <Text style={styles.baselineTime}>
              Captured {new Date(selectedUser.baseline.capturedAt).toLocaleTimeString()}
            </Text>
          </View>
        ) : null}

        <Pressable
          style={[styles.btn, styles.btnGold, deviceState === 'BASELINE' && styles.btnDisabled]}
          disabled={deviceState === 'BASELINE'}
          onPress={triggerBaseline}
        >
          <Text style={styles.btnGoldText}>
            {deviceState === 'BASELINE' ? 'Baseline running...' :
             selectedUser.baseline ? 'Re-run Baseline Test' : 'Start the baseline test'}
          </Text>
        </Pressable>

        {deviceState === 'BASELINE' && (
          <Text style={styles.hint}>Baseline in progress — ask {selectedUser.name} to hold still with wristband on.</Text>
        )}
      </View>

      {/* STEP 2: Recovery Test */}
      <View style={[styles.card, !selectedUser.baseline && { opacity: 0.5 }]}>
        <Text style={styles.cardTitle}>Step 2 — Recovery Test</Text>
        <Text style={styles.cardSub}>
          Perform this test during or after user workout/break. The wristband will monitor live vitals, compare them to the captured baseline, run the reaction test, and determine readiness.
        </Text>

        <Pressable
          style={[styles.btn, styles.btnGold,
            (!selectedUser.baseline || deviceState === 'RECOVERY') && styles.btnDisabled]}
          disabled={!selectedUser.baseline || deviceState === 'RECOVERY'}
          onPress={triggerBreak}
        >
          <Text style={styles.btnGoldText}>Start recovery test</Text>
        </Pressable>

        {!selectedUser.baseline && (
          <Text style={styles.hint}>Complete the baseline test first.</Text>
        )}
        {deviceState === 'RECOVERY' && (
          <Pressable style={styles.viewLive} onPress={() => router.push('/recovery')}>
            <Text style={styles.viewLiveText}>View live recovery →</Text>
          </Pressable>
        )}
      </View>

    </ScrollView>
  );
}

function BaselineVal({ label, value }: { label: string; value: string }) {
  return (
    <View style={bStyles.box}>
      <Text style={bStyles.val}>{value}</Text>
      <Text style={bStyles.lbl}>{label}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  screen:          { flex: 1, backgroundColor: colors.bg },
  content:         { padding: 16, gap: 14 },
  noUser:          { flex: 1, backgroundColor: colors.bg, alignItems: 'center', justifyContent: 'center', padding: 32, gap: 12 },
  noUserEmoji:     { fontSize: 48 },
  noUserTitle:     { fontSize: 20, fontWeight: '700', color: colors.text },
  noUserSub:       { fontSize: 14, color: colors.textDim, textAlign: 'center', lineHeight: 20 },
  goBtn:           { backgroundColor: colors.gold, borderRadius: radius.button, paddingVertical: 12, paddingHorizontal: 24, marginTop: 8 },
  goBtnText:       { color: '#0A0A0A', fontSize: 15, fontWeight: '800' },
  userCard:        { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.gold, padding: 14, flexDirection: 'row', alignItems: 'center', gap: 12 },
  avatar:          { width: 44, height: 44, borderRadius: 22, backgroundColor: colors.surfaceHigh, alignItems: 'center', justifyContent: 'center' },
  avatarText:      { color: colors.gold, fontSize: 18, fontWeight: '900' },
  userName:        { fontSize: 16, fontWeight: '700', color: colors.text },
  userMeta:        { fontSize: 12, color: colors.textDim },
  changeText:      { color: colors.gold, fontSize: 13, fontWeight: '600' },
  card:            { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.border, padding: 18, gap: 10 },
  cardRow:         { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center' },
  dot:             { width: 10, height: 10, borderRadius: 5 },
  cardTitle:       { fontSize: 16, fontWeight: '700', color: colors.text },
  cardSub:         { fontSize: 13, color: colors.textDim, lineHeight: 18 },
  input:           { backgroundColor: colors.bg, borderRadius: radius.input, borderWidth: 1, borderColor: colors.border, color: colors.text, paddingHorizontal: 12, paddingVertical: 10, fontSize: 13 },
  stateText:       { fontSize: 18, fontWeight: '800' },
  btn:             { borderRadius: radius.button, paddingVertical: 13, alignItems: 'center' },
  btnGold:         { backgroundColor: colors.gold },
  btnGhost:        { borderWidth: 1, borderColor: colors.border },
  btnDisabled:     { opacity: 0.35 },
  btnGoldText:     { color: '#0A0A0A', fontSize: 15, fontWeight: '800' },
  btnGhostText:    { color: colors.text, fontSize: 15, fontWeight: '600' },
  hint:            { fontSize: 12, color: colors.textDim, fontStyle: 'italic' },
  baselineResult:  { backgroundColor: colors.bg, borderRadius: 10, padding: 12, gap: 6 },
  baselineTitle:   { fontSize: 13, fontWeight: '700', color: colors.ready },
  baselineRow:     { flexDirection: 'row', gap: 8 },
  baselineTime:    { fontSize: 11, color: colors.textMuted },
  viewLive:        {},
  viewLiveText:    { color: colors.gold, fontSize: 14, fontWeight: '600' },
});

const bStyles = StyleSheet.create({
  box: { flex: 1, backgroundColor: colors.surface, borderRadius: 8, padding: 10, alignItems: 'center', gap: 2 },
  val: { fontSize: 15, fontWeight: '800', color: colors.gold },
  lbl: { fontSize: 9, fontWeight: '700', color: colors.textDim, letterSpacing: 0.5 },
});
