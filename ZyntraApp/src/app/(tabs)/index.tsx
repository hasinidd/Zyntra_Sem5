import { useRouter } from 'expo-router';
import React, { useState } from 'react';
import {
  Alert, FlatList, Platform, Pressable, ScrollView,
  StyleSheet, Text, TextInput, View,
} from 'react-native';
import { useZyntra } from '../../services/ZyntraContext';
import { colors, radius } from '../../theme';

export default function UsersTab() {
  const router = useRouter();
  const { users, selectedUser, selectUser, addUser, deleteUser } = useZyntra();
  const [showForm, setShowForm] = useState(false);

  // Form state
  const [name, setName]       = useState('');
  const [age, setAge]         = useState('');
  const [height, setHeight]   = useState('');
  const [weight, setWeight]   = useState('');
  const [role, setRole]       = useState('');
  const [error, setError]     = useState('');

  const handleAdd = async () => {
    if (!name.trim()) { setError('Name is required'); return; }
    const ageN = parseInt(age);
    const hN   = parseFloat(height);
    const wN   = parseFloat(weight);
    if (!ageN || ageN < 10 || ageN > 100) { setError('Enter a valid age'); return; }
    if (!hN || hN < 100 || hN > 220)      { setError('Enter height in cm (100–220)'); return; }
    if (!wN || wN < 30  || wN > 200)      { setError('Enter weight in kg (30–200)'); return; }

    await addUser({
      name: name.trim(),
      age: ageN,
      heightCm: hN,
      weightKg: wN,
      role: role.trim() || 'Gym Member',
    });

    setName(''); setAge(''); setHeight(''); setWeight(''); setRole('');
    setError(''); setShowForm(false);
    router.push('/(tabs)/test');
  };

  const handleSelect = (userId: string) => {
    const user = users.find(u => u.id === userId);
    if (user) {
      selectUser(user);
      router.push('/(tabs)/test');
    }
  };

  const handleDelete = (userId: string, userName: string) => {
    if (Platform.OS === 'web') {
      if (typeof window !== 'undefined' && window.confirm(`Delete ${userName} and all recorded data?`)) {
        deleteUser(userId);
      }
    } else {
      Alert.alert(
        'Delete participant',
        `Remove ${userName} and all their test data?`,
        [
          { text: 'Cancel', style: 'cancel' },
          { text: 'Delete', style: 'destructive', onPress: () => deleteUser(userId) },
        ]
      );
    }
  };

  return (
    <View style={styles.screen}>
      {/* Header */}
      <View style={styles.headerRow}>
        <Text style={styles.title}>Gym Participants</Text>
        <Pressable style={styles.addBtn} onPress={() => setShowForm(v => !v)}>
          <Text style={styles.addBtnText}>{showForm ? '✕ Cancel' : '+ Add user'}</Text>
        </Pressable>
      </View>

      {/* Add user form */}
      {showForm && (
        <ScrollView style={styles.form} keyboardShouldPersistTaps="handled">
          <Text style={styles.formTitle}>New participant</Text>
          <TextInput style={styles.input} placeholder="Full name *" placeholderTextColor={colors.textMuted} value={name} onChangeText={setName} />
          <View style={styles.row}>
            <TextInput style={[styles.input, { flex: 1 }]} placeholder="Age" placeholderTextColor={colors.textMuted} value={age} onChangeText={setAge} keyboardType="number-pad" maxLength={3} />
            <TextInput style={[styles.input, { flex: 1 }]} placeholder="Height (cm)" placeholderTextColor={colors.textMuted} value={height} onChangeText={setHeight} keyboardType="decimal-pad" maxLength={5} />
            <TextInput style={[styles.input, { flex: 1 }]} placeholder="Weight (kg)" placeholderTextColor={colors.textMuted} value={weight} onChangeText={setWeight} keyboardType="decimal-pad" maxLength={5} />
          </View>
          <TextInput style={styles.input} placeholder="Role / occupation (optional)" placeholderTextColor={colors.textMuted} value={role} onChangeText={setRole} />
          {error ? <Text style={styles.error}>{error}</Text> : null}
          <Pressable style={styles.saveBtn} onPress={handleAdd}>
            <Text style={styles.saveBtnText}>Create participant</Text>
          </Pressable>
        </ScrollView>
      )}

      {/* User list */}
      <FlatList
        data={users}
        keyExtractor={u => u.id}
        contentContainerStyle={{ gap: 10, padding: 16, paddingTop: showForm ? 8 : 16 }}
        ListEmptyComponent={
          <View style={styles.empty}>
            <Text style={styles.emptyText}>No participants yet.</Text>
            <Text style={styles.emptySubText}>Tap + Add user to add your first gym participant.</Text>
          </View>
        }
        renderItem={({ item }) => (
          <Pressable style={[styles.card, selectedUser?.id === item.id && { borderColor: colors.gold }]}
            onPress={() => handleSelect(item.id)}>
            {/* Avatar */}
            <View style={styles.avatar}>
              <Text style={styles.avatarText}>{item.name.charAt(0).toUpperCase()}</Text>
            </View>
            {/* Info */}
            <View style={{ flex: 1 }}>
              <Text style={styles.userName}>{item.name}</Text>
              <Text style={styles.userMeta}>
                {item.age}y · {item.heightCm}cm · {item.weightKg}kg · {item.role}
              </Text>
              <View style={styles.statusRow}>
                <View style={[styles.pill, { backgroundColor: item.baseline ? colors.ready + '33' : colors.textMuted + '22' }]}>
                  <Text style={[styles.pillText, { color: item.baseline ? colors.ready : colors.textMuted }]}>
                    {item.baseline ? 'Baseline ✓' : 'No baseline'}
                  </Text>
                </View>
                {item.testResults.length > 0 && (
                  <View style={[styles.pill, { backgroundColor: colors.gold + '22' }]}>
                    <Text style={[styles.pillText, { color: colors.gold }]}>
                      {item.testResults.length} test{item.testResults.length > 1 ? 's' : ''}
                    </Text>
                  </View>
                )}
              </View>
            </View>
            {/* Actions */}
            <View style={styles.actions}>
              <Text style={styles.goText}>Test →</Text>
              <Pressable
                onPress={(e) => {
                  e.stopPropagation();
                  handleDelete(item.id, item.name);
                }}
                hitSlop={10}
              >
                <Text style={styles.deleteText}>Delete</Text>
              </Pressable>
            </View>
          </Pressable>
        )}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  screen:       { flex: 1, backgroundColor: colors.bg },
  headerRow:    { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', padding: 16, paddingBottom: 8 },
  title:        { fontSize: 22, fontWeight: '800', color: colors.text },
  addBtn:       { backgroundColor: colors.gold, borderRadius: 20, paddingHorizontal: 14, paddingVertical: 7 },
  addBtnText:   { color: '#0A0A0A', fontSize: 13, fontWeight: '700' },
  form:         { backgroundColor: colors.surface, borderRadius: radius.card, margin: 16, marginTop: 0, padding: 16, maxHeight: 300 },
  formTitle:    { fontSize: 15, fontWeight: '700', color: colors.text, marginBottom: 10 },
  row:          { flexDirection: 'row', gap: 8 },
  input:        { backgroundColor: colors.bg, borderRadius: radius.input, borderWidth: 1, borderColor: colors.border, color: colors.text, paddingHorizontal: 12, paddingVertical: 10, fontSize: 14, marginBottom: 8 },
  error:        { color: colors.notReady, fontSize: 12, marginBottom: 8 },
  saveBtn:      { backgroundColor: colors.gold, borderRadius: radius.button, paddingVertical: 12, alignItems: 'center' },
  saveBtnText:  { color: '#0A0A0A', fontSize: 15, fontWeight: '800' },
  empty:        { alignItems: 'center', paddingTop: 60, gap: 8 },
  emptyText:    { color: colors.text, fontSize: 18, fontWeight: '700' },
  emptySubText: { color: colors.textDim, fontSize: 14, textAlign: 'center' },
  card:         { backgroundColor: colors.surface, borderRadius: radius.card, borderWidth: 1, borderColor: colors.border, padding: 14, flexDirection: 'row', alignItems: 'center', gap: 12 },
  avatar:       { width: 44, height: 44, borderRadius: 22, backgroundColor: colors.surfaceHigh, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: colors.gold },
  avatarText:   { color: colors.gold, fontSize: 18, fontWeight: '900' },
  userName:     { fontSize: 16, fontWeight: '700', color: colors.text },
  userMeta:     { fontSize: 12, color: colors.textDim, marginTop: 2 },
  statusRow:    { flexDirection: 'row', gap: 6, marginTop: 6 },
  pill:         { borderRadius: 10, paddingHorizontal: 8, paddingVertical: 3 },
  pillText:     { fontSize: 11, fontWeight: '700' },
  actions:      { alignItems: 'flex-end', gap: 8 },
  goText:       { color: colors.gold, fontSize: 13, fontWeight: '700' },
  deleteText:   { color: colors.notReady, fontSize: 11, fontWeight: '600' },
});
