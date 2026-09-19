// Zyntra visual theme — dark charcoal + gold UI chrome, orange clearance states.
// Inspired by the 6-Star housekeeping dashboard reference.

export const colors = {
  // Base surfaces
  bg:           '#0A0A0A',   // near-black background
  surface:      '#141414',   // card background
  surfaceHigh:  '#1E1E1E',   // raised / input background
  border:       '#2A2A2A',   // hairline borders

  // Text
  text:         '#F0F0F0',   // primary
  textDim:      '#888888',   // secondary / labels
  textMuted:    '#444444',   // disabled / placeholders

  // Brand accents
  gold:         '#C9A84C',   // UI chrome — tabs, titles, labels (matches 6-Star gold)
  goldDim:      '#7A6028',   // inactive tab icons

  // Clearance signal colors (kept distinct from gold so verdicts pop)
  ready:        '#3DBE7B',   // PASS / READY
  notReady:     '#E05252',   // FAIL / NOT READY
  warn:         '#E8B04B',   // sensor error / caution

  // Status pills (matching the reference)
  pillBg:       '#2A2A2A',
};

export const radius = {
  card: 16,
  pill: 20,
  input: 10,
  button: 10,
};

export const typography = {
  screenTitle:  { fontSize: 26, fontWeight: '700' as const, color: '#F0F0F0' },
  cardTitle:    { fontSize: 16, fontWeight: '700' as const, color: '#F0F0F0' },
  label:        { fontSize: 11, fontWeight: '600' as const, color: '#888888', letterSpacing: 0.8 },
  body:         { fontSize: 14, color: '#F0F0F0' },
  bodyDim:      { fontSize: 13, color: '#888888' },
  stat:         { fontSize: 22, fontWeight: '800' as const, color: '#F0F0F0' },
  statLabel:    { fontSize: 10, fontWeight: '600' as const, color: '#888888', letterSpacing: 0.5 },
};
