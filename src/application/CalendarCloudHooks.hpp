#pragma once

#include "stapik/sync/CloudSessionHooks.hpp"

// Cloud session hooks for the calendar: stored configuration, the "calendar.json" cloud slot,
// and a storage factory that reports a bad configuration instead of throwing.
[[nodiscard]] stapik::sync::CloudSessionHooks createCalendarCloudHooks();
