# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

工时记录 (Daily Report) - A Tauri desktop application for tracking work hours.

## Tech Stack

- **Desktop Framework**: Tauri 2.2 (Rust backend + Web frontend)
- **Frontend**: React 18 + TypeScript + Vite
- **Build Tool**: Vite 5 (frontend dev server on port 1420)
- **Data**: Serde/JSON for persistence, UUID for session IDs

## Project Structure

```
/root/DailyReport/
├── src-tauri/           # Rust backend
│   ├── src/
│   │   ├── main.rs      # Tauri app entry point with command handlers
│   │   ├── commands.rs  # Tauri API commands (start/end shift, CRUD, export)
│   │   ├── models.rs    # Data models (WorkSession, DailyStatistics)
│   │   ├── storage.rs   # File-based session persistence
│   │   └── time_tracker.rs  # Time calculation utilities
│   ├── frontend/        # Built frontend assets (generated)
│   ├── build.rs         # Tauri build script
│   └── Cargo.toml
└── frontend/            # React frontend
    ├── src/
    │   ├── main.tsx     # React entry point
    │   ├── App.tsx      # Main application component
    │   ├── utils.ts     # Utility types and API declarations
    │   └── App.css      # Styles
    ├── index.html
    ├── vite.config.ts   # Vite config (output to ../src-tauri/frontend)
    └── package.json
```

## Architecture

- **Backend (Rust)**: Handles session storage in JSON files (organized by month in user data directory), provides Tauri commands for CRUD operations
- **Frontend (React)**: Manages UI state, calls Tauri API via `window.api`, displays sessions and statistics
- **Storage**: Sessions stored as JSON per month in platform-specific data directory (`com/daily-report/DailyReport/work_sessions/`)

## Common Commands

```bash
# Development: Start backend + frontend dev servers
cd src-tauri && cargo run

# Build frontend only
cd frontend && npm run build

# Lint/Rust tests
cd src-tauri && cargo clippy && cargo test
```

## API Commands (Tauri → Frontend)

- `startShift()` - Start a new work session
- `endShift(activity, workType)` - End current session
- `getSessions(date)` - Load sessions for a date
- `updateSession(date, sessionId, activity, workType)` - Edit session
- `deleteSession(date, sessionId)` - Delete session
- `exportData(date, format)` - Export to JSON/CSV
- `getTodayStatistics()` - Get today's total hours and session count
- `getAvailableDates()` - Get all dates with recorded sessions
