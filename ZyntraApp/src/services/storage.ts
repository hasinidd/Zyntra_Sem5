import AsyncStorage from '@react-native-async-storage/async-storage';
import { GymUser } from './types';

const USERS_KEY = 'zyntra_gym_users';

export async function loadUsers(): Promise<GymUser[]> {
  try {
    const raw = await AsyncStorage.getItem(USERS_KEY);
    if (!raw) return [];
    return JSON.parse(raw) as GymUser[];
  } catch {
    return [];
  }
}

export async function saveUsers(users: GymUser[]): Promise<void> {
  try {
    await AsyncStorage.setItem(USERS_KEY, JSON.stringify(users));
  } catch (e) {
    console.error('[Storage] Failed to save users:', e);
  }
}

export async function addUser(user: GymUser): Promise<GymUser[]> {
  const users = await loadUsers();
  const updated = [...users, user];
  await saveUsers(updated);
  return updated;
}

export async function updateUser(updated: GymUser): Promise<GymUser[]> {
  const users = await loadUsers();
  const next = users.map(u => u.id === updated.id ? updated : u);
  await saveUsers(next);
  return next;
}

export async function deleteUser(id: string): Promise<GymUser[]> {
  const users = await loadUsers();
  const next = users.filter(u => u.id !== id);
  await saveUsers(next);
  return next;
}