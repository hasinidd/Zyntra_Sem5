import React from 'react';
import {
  ActivityIndicator,
  Pressable,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import { useAuth } from '../../services/AuthContext';
import { colors, radius } from '../../theme';

export default function Login() {
  const { signIn, devSignIn, loading } = useAuth();

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

      {/* Sign-in card */}
      <View style={styles.card}>
        <Text style={styles.cardTitle}>Supervisor login</Text>
        <Text style={styles.cardSub}>
          Sign in with your Google account to access the Zyntra supervisor dashboard.
        </Text>

        <Pressable
          style={[styles.googleBtn, loading && styles.disabled]}
          onPress={signIn}
          disabled={loading}
        >
          {loading ? (
            <ActivityIndicator color={colors.bg} />
          ) : (
            <>
              {/* Simple G lettermark — no image dependency */}
              <View style={styles.gMark}>
                <Text style={styles.gText}>G</Text>
              </View>
              <Text style={styles.googleBtnText}>Continue with Google</Text>
            </>
          )}
        </Pressable>

        {/* Dev bypass — remove before production */}
        <Pressable style={styles.devBtn} onPress={devSignIn}>
          <Text style={styles.devBtnText}>Dev bypass (skip Google)</Text>
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
    padding: 28,
    justifyContent: 'center',
    gap: 32,
  },
  hero: { alignItems: 'center', gap: 12 },
  logoRing: {
    width: 80,
    height: 80,
    borderRadius: 40,
    borderWidth: 2,
    borderColor: colors.gold,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: colors.surface,
  },
  logoZ: { fontSize: 36, fontWeight: '900', color: colors.gold },
  wordmark: { fontSize: 34, fontWeight: '800', color: colors.text, letterSpacing: 1 },
  tagline: { fontSize: 13, color: colors.textDim, textAlign: 'center', lineHeight: 18 },

  card: {
    backgroundColor: colors.surface,
    borderRadius: radius.card,
    borderWidth: 1,
    borderColor: colors.border,
    padding: 24,
    gap: 16,
  },
  cardTitle: { fontSize: 20, fontWeight: '700', color: colors.text },
  cardSub: { fontSize: 13, color: colors.textDim, lineHeight: 18 },

  googleBtn: {
    backgroundColor: '#FFFFFF',
    borderRadius: radius.button,
    paddingVertical: 14,
    paddingHorizontal: 20,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    gap: 10,
  },
  disabled: { opacity: 0.5 },
  gMark: {
    width: 24,
    height: 24,
    borderRadius: 12,
    backgroundColor: '#4285F4',
    alignItems: 'center',
    justifyContent: 'center',
  },
  gText: { color: '#fff', fontSize: 13, fontWeight: '800' },
  googleBtnText: { color: '#1A1A1A', fontSize: 15, fontWeight: '700' },

  devBtn: {
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: radius.button,
    paddingVertical: 11,
    alignItems: 'center',
  },
  devBtnText: { color: colors.textDim, fontSize: 13 },

  footer: { color: colors.textMuted, fontSize: 11, textAlign: 'center' },
});
