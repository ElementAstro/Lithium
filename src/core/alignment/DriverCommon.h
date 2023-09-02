/*!
 * \file DriverCommon.h
 *
 * \author Roger James
 * \date 28th January 2014
 *
 */

#pragma once

#include "hydrogenlogger.h"

namespace HYDROGEN
{
namespace AlignmentSubsystem
{
#define ASSDEBUG(msg) HYDROGEN::Logger::getInstance().print("alignmentSubsystem", DBG_ALIGNMENT, __FILE__, __LINE__, msg)
#define ASSDEBUGF(msg, ...) \
    HYDROGEN::Logger::getInstance().print("AlignmentSubsystem", DBG_ALIGNMENT, __FILE__, __LINE__, msg, __VA_ARGS__)

extern int DBG_ALIGNMENT;

} // namespace AlignmentSubsystem
} // namespace HYDROGEN
