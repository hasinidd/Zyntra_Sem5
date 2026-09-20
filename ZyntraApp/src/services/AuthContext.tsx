import AsyncStorage from '@react-native-async-storage/async-storage';
import React, { createContext, useCallback, useContext, useEffect, useState } from 'react';
import { AuthUser } from './types';

const AUTH_STORAGE_KEY = '@zyntra_auth_user';
const USERS_STORAGE_KEY = '@zyntra_accounts';

interface AuthStore {
  user: AuthUser | null;
  loading: boolean;
  signIn: (username: string, password: string) => Promise<void>;
  signUp: (username: string, password: string, name: string) => Promise<void>;
  signOut: () => Promise<void>;
}

const Ctx = createContext<AuthStore | null>(null);

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [user, setUser] = useState<AuthUser | null>(null);
  const [loading, setLoading] = useState(true);

  // Restore stored session on startup
  useEffect(() => {
    AsyncStorage.getItem(AUTH_STORAGE_KEY).then(raw => {
      if (raw) {
        try {
          setUser(JSON.parse(raw));
        } catch {
          /* ignore error */
        }
      }
      setLoading(false);
    });
  }, []);

  // Sign In with username & password
  const signIn = useCallback(async (username: string, password: string) => {
    setLoading(true);
    try {
      const u = username.trim().toLowerCase();
      if (!u || !password) {
        throw new Error('Please fill in both username and password');
      }

      // Check registered accounts in storage
      const accountsRaw = await AsyncStorage.getItem(USERS_STORAGE_KEY);
      const accounts: Record<string, { password: string; name: string; email: string }> = accountsRaw
        ? JSON.parse(accountsRaw)
        : {};

      if (accounts[u]) {
        if (accounts[u].password !== password) {
          throw new Error('Incorrect password');
        }
        const authUser: AuthUser = {
          name: accounts[u].name || username,
          email: accounts[u].email || `${u}@zyntra.io`,
          photoUrl: null,
        };
        await AsyncStorage.setItem(AUTH_STORAGE_KEY, JSON.stringify(authUser));
        setUser(authUser);
      } else {
        // First-time fallback / automatic local account creation
        const authUser: AuthUser = {
          name: username,
          email: `${u}@zyntra.io`,
          photoUrl: null,
        };
        accounts[u] = { password, name: username, email: `${u}@zyntra.io` };
        await AsyncStorage.setItem(USERS_STORAGE_KEY, JSON.stringify(accounts));
        await AsyncStorage.setItem(AUTH_STORAGE_KEY, JSON.stringify(authUser));
        setUser(authUser);
      }
    } finally {
      setLoading(false);
    }
  }, []);

  // Sign Up / Create Account
  const signUp = useCallback(async (username: string, password: string, name: string) => {
    setLoading(true);
    try {
      const u = username.trim().toLowerCase();
      if (!u || !password || !name) {
        throw new Error('Please complete all registration fields');
      }

      const accountsRaw = await AsyncStorage.getItem(USERS_STORAGE_KEY);
      const accounts: Record<string, { password: string; name: string; email: string }> = accountsRaw
        ? JSON.parse(accountsRaw)
        : {};

      const authUser: AuthUser = {
        name: name.trim(),
        email: `${u}@zyntra.io`,
        photoUrl: null,
      };

      accounts[u] = { password, name: name.trim(), email: `${u}@zyntra.io` };
      await AsyncStorage.setItem(USERS_STORAGE_KEY, JSON.stringify(accounts));
      await AsyncStorage.setItem(AUTH_STORAGE_KEY, JSON.stringify(authUser));
      setUser(authUser);
    } finally {
      setLoading(false);
    }
  }, []);

  const signOut = useCallback(async () => {
    await AsyncStorage.removeItem(AUTH_STORAGE_KEY);
    setUser(null);
  }, []);

  return (
    <Ctx.Provider value={{ user, loading, signIn, signUp, signOut }}>
      {children}
    </Ctx.Provider>
  );
}

export function useAuth(): AuthStore {
  const ctx = useContext(Ctx);
  if (!ctx) throw new Error('useAuth must be inside <AuthProvider>');
  return ctx;
}