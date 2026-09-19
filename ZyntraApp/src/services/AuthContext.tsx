import React, { createContext, useCallback, useContext, useState } from 'react';
import { AuthUser } from './types';

interface AuthStore {
  user: AuthUser | null;
  loading: boolean;
  signIn: () => Promise<void>;
  signOut: () => void;
  devSignIn: () => void;
}

const Ctx = createContext<AuthStore | null>(null);

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [user, setUser] = useState<AuthUser | null>(null);
  const [loading] = useState(false);

  const signIn = useCallback(async () => {
    // Google sign-in requires rebuilt dev client — use devSignIn for now
    console.log('Google sign-in not available in this build — use Dev bypass');
  }, []);

  const signOut = useCallback(() => setUser(null), []);

  const devSignIn = useCallback(() => {
    setUser({
      name: 'Hasini (Dev)',
      email: 'hasini@zyntra.dev',
      photoUrl: null,
    });
  }, []);

  return (
    <Ctx.Provider value={{ user, loading, signIn, signOut, devSignIn }}>
      {children}
    </Ctx.Provider>
  );
}

export function useAuth(): AuthStore {
  const ctx = useContext(Ctx);
  if (!ctx) throw new Error('useAuth must be inside <AuthProvider>');
  return ctx;
}