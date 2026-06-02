#pragma once

#include <cstdio>

// ─── Logging ────────────────────────────────────────────────────────────────
void InitLog();
void CloseLog();
void Log(const wchar_t* fmt, ...);

// ─── Package Identity ───────────────────────────────────────────────────────
bool CheckPackageIdentity();

// ─── Limited Access Feature ─────────────────────────────────────────────────
bool UnlockLimitedAccessFeature();
