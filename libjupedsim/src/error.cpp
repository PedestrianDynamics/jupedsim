// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/error.h"

#include "ErrorMessage.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ErrorMessage
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* JPS_ErrorMessage_GetMessage(JPS_ErrorMessage handle)
{
    JPS_ErrorMessage_t* msg = reinterpret_cast<JPS_ErrorMessage_t*>(handle);
    return msg->message.c_str();
}

void JPS_ErrorMessage_Free(JPS_ErrorMessage handle)
{
    delete reinterpret_cast<JPS_ErrorMessage_t*>(handle);
}
