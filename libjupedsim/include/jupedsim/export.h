/* Copyright © 2012-2023 Forschungszentrum Jülich GmbH */
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#ifdef _WIN32
#ifdef JUPEDSIM_API_EXPORTS
#define JUPEDSIM_API __declspec(dllexport)
#else
#define JUPEDSIM_API __declspec(dllimport)
#endif
#else
#define JUPEDSIM_API
#endif
