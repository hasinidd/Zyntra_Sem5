import React, { useState } from 'react';
import {
  ActivityIndicator,
  Pressable,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';
import { useAuth } from '../../services/AuthContext';
import { colors, radius } from '../../theme';

export default function Login() {
  const { signIn, signUp, loading } = useAuth();

  const [mode, setMode] = useState<'signin' | 'signup'>('signin');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [name, setName] = useState('');
  const [error, setError] = useState<string | null>(null);

  const handleSubmit = async () => {
    setError(null);
    try {
      if (mode === 'signin') {
        if (!username || !password) {
          setError('Please enter your username and password.');
          return;
        }
        await signIn(username, password);
      } else {
        if (!username || !password || !name) {
          setError('Please fill in all registration fields.');
          return;
        }
        await signUp(username, password, name);
      }
    } catch (e: any) {
      setError(e?.message || 'Authentication failed');
    }
  };

  return (
    <View style={styles.screen}>
      {/* Logo / wordmark */}
      <View style={styles.hero}>
        <View style={styles.logoRing}>
          <Text style={styles.logoZ}>Z</Text>
        </View>
        <Text style={styles.wordmark}>Zyntra</Text>
        <Text style={styles.tagline}>Post-Break Physiological Readiness Clearance</Text>
      </View>

      {/* Authentication Card */}
      <View style={styles.card}>

        {/* Mode Switcher Tabs */}
        <View style={styles.tabContainer}>
          <Pressable
            style={[styles.tab, mode === 'signin' && styles.activeTab]}
            onPress={() => { setMode('signin'); setError(null); }}
          >
            <Text style={[styles.tabText, mode === 'signin' && styles.activeTabText]}>Sign In</Text>
          </Pressable>
          <Pressable
            style={[styles.tab, mode === 'signup' && styles.activeTab]}
            onPress={() => { setMode('signup'); setError(null); }}
          >
            <Text style={[styles.tabText, mode === 'signup' && styles.activeTabText]}>Create Account</Text>
          </Pressable>
        </View>

        {error && (
          <View style={styles.errorBox}>
            <Text style={styles.errorText}>{error}</Text>
          </View>
        )}

        {/* Full Name field (Only shown during Sign Up) */}
        {mode === 'signup' && (
          <View style={styles.field}>
            <Text style={styles.label}>FULL NAME</Text>
            <TextInput
              style={styles.input}
              placeholder="e.g. Hasini Dikkumbura"
              placeholderTextColor={colors.textMuted}
              value={name}
              onChangeText={setName}
            />
          </View>
        )}

        {/* Username field */}
        <View style={styles.field}>
          <Text style={styles.label}>USERNAME OR EMAIL</Text>
          <TextInput
            style={styles.input}
            placeholder="e.g. hasini"
            placeholderTextColor={colors.textMuted}
            autoCapitalize="none"
            value={username}
            onChangeText={setUsername}
          />
        </View>

        {/* Password field */}
        <View style={styles.field}>
          <Text style={styles.label}>PASSWORD</Text>
          <TextInput
            style={styles.input}
            placeholder="••••••••"
            placeholderTextColor={colors.textMuted}
            secureTextEntry
            value={password}
            onChangeText={setPassword}
          />
        </View>

        {/* Submit Button */}
        <Pressable
          style={[styles.submitBtn, loading && styles.disabled]}
          onPress={handleSubmit}
          disabled={loading}
        >
          {loading ? (
            <ActivityIndicator color="#0A0A0A" />
          ) : (
            <Text style={styles.submitBtnText}>
              {mode === 'signin' ? 'Sign In to Dashboard' : 'Create Account & Start'}
            </Text>
          )}
        </Pressable>

      </View>

      <Text style={styles.footer}>
        University of Moratuwa · CS3283 Embedded Systems Project
      </Text>
    </View>
  );
}

const styles = StyleSheet.create({
  screen: {
    flex: 1,
    backgroundColor: colors.bg,
    padding: 24,
    justifyContent: 'center',
    alignItems: 'center',
    gap: 24,
  },
  hero: { alignItems: 'center', gap: 10 },
  logoRing: {
    width: 72,
    height: 72,
    borderRadius: 36,
    borderWidth: 2,
    borderColor: colors.gold,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: colors.surface,
  },
  logoZ: { fontSize: 32, fontWeight: '900', color: colors.gold },
  wordmark: { fontSize: 30, fontWeight: '800', color: colors.text, letterSpacing: 1 },
  tagline: { fontSize: 12, color: colors.textDim, textAlign: 'center', lineHeight: 17 },

  card: {
    width: '100%',
    maxWidth: 420,
    backgroundColor: colors.surface,
    borderRadius: radius.card,
    borderWidth: 1,
    borderColor: colors.border,
    padding: 24,
    gap: 16,
  },

  tabContainer: {
    flexDirection: 'row',
    backgroundColor: colors.bg,
    borderRadius: 10,
    padding: 4,
    borderWidth: 1,
    borderColor: colors.border,
  },
  tab: {
    flex: 1,
    paddingVertical: 10,
    alignItems: 'center',
    borderRadius: 8,
  },
  activeTab: {
    backgroundColor: colors.surfaceHigh,
  },
  tabText: {
    fontSize: 13,
    fontWeight: '600',
    color: colors.textDim,
  },
  activeTabText: {
    color: colors.gold,
    fontWeight: '700',
  },

  errorBox: {
    backgroundColor: colors.notReady + '22',
    borderWidth: 1,
    borderColor: colors.notReady,
    borderRadius: 8,
    padding: 10,
  },
  errorText: {
    color: colors.notReady,
    fontSize: 12,
    fontWeight: '600',
    textAlign: 'center',
  },

  field: { gap: 6 },
  label: { fontSize: 10, fontWeight: '700', color: colors.textDim, letterSpacing: 0.8 },
  input: {
    backgroundColor: colors.bg,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: radius.button,
    paddingHorizontal: 14,
    paddingVertical: 12,
    color: colors.text,
    fontSize: 14,
  },

  submitBtn: {
    backgroundColor: colors.gold,
    borderRadius: radius.button,
    paddingVertical: 14,
    alignItems: 'center',
    justifyContent: 'center',
    marginTop: 4,
  },
  disabled: { opacity: 0.6 },
  submitBtnText: { color: '#0A0A0A', fontSize: 14, fontWeight: '800' },

  footer: { color: colors.textMuted, fontSize: 11, textAlign: 'center' },
});
