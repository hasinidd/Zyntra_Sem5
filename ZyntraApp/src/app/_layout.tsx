import { Stack, useRouter, useSegments } from 'expo-router';
import { StatusBar } from 'expo-status-bar';
import React, { useEffect } from 'react';
import { AuthProvider, useAuth } from '../services/AuthContext';
import { ZyntraProvider } from '../services/ZyntraContext';
import { colors } from '../theme';

// Auth guard — watches the user object and redirects accordingly.
function AuthGate() {
  const { user, loading } = useAuth();
  const segments = useSegments();
  const router = useRouter();

  useEffect(() => {
    if (loading) return;
    const inAuthGroup = segments[0] === '(auth)';
    if (!user && !inAuthGroup) {
      router.replace('/(auth)/login');
    } else if (user && inAuthGroup) {
      router.replace('/(tabs)');
    }
  }, [user, loading, segments, router]);

  return null;
}

export default function RootLayout() {
  return (
    <AuthProvider>
      <ZyntraProvider>
        <StatusBar style="light" />
        <AuthGate />
        <Stack screenOptions={{ headerShown: false, contentStyle: { backgroundColor: colors.bg } }}>
          <Stack.Screen name="(auth)" />
          <Stack.Screen name="(tabs)" />
          <Stack.Screen
            name="recovery"
            options={{
              headerShown: true,
              title: 'Recovery in Progress',
              headerStyle: { backgroundColor: colors.surface },
              headerTintColor: colors.text,
              headerTitleStyle: { fontWeight: '700' },
              contentStyle: { backgroundColor: colors.bg },
            }}
          />
          <Stack.Screen
            name="clearance"
            options={{
              headerShown: true,
              title: 'Clearance Verdict',
              headerBackVisible: false,
              headerStyle: { backgroundColor: colors.surface },
              headerTintColor: colors.text,
              headerTitleStyle: { fontWeight: '700' },
              contentStyle: { backgroundColor: colors.bg },
            }}
          />
        </Stack>
      </ZyntraProvider>
    </AuthProvider>
  );
}
