import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  server: {
    port: 1420,
    strictPort: true,
    origin: 'http://localhost:1420',
  },
  build: {
    outDir: '../src-tauri/frontend',
    emptyOutDir: true,
  },
});
