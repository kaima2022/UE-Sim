/*******************************************************************************
 * Copyright 2025 Soft UE Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

/**
 * @file             soft-ue-logger.cc
 * @brief            Soft-UE Structured Logger Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-09
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the implementation of the structured logging system
 * for Soft-UE components with improved readability and error tracking.
 */

#include "soft-ue-logger.h"
#include "ns3/simulator.h"
#include <iomanip>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SoftUeLogger");

SoftUeLogger::SoftUeLogger ()
    : m_logFilePath ("log/soft-ue-optimized.log"),
      m_fileLoggingEnabled (false)
{
    NS_LOG_FUNCTION (this);
}

SoftUeLogger::~SoftUeLogger ()
{
    NS_LOG_FUNCTION (this);
    if (m_logFile.is_open ())
    {
        m_logFile.close ();
    }
}

SoftUeLogger&
SoftUeLogger::GetInstance ()
{
    static SoftUeLogger instance;
    return instance;
}

void
SoftUeLogger::Log (LogLevel level, const std::string& component, const std::string& message)
{
    std::string levelStr = LevelToString (level);
    std::string timestamp = GetTimestamp ();

    // Format: [TIMESTAMP] [LEVEL] COMPONENT: MESSAGE
    std::ostringstream oss;
    oss << "[" << timestamp << "] [" << levelStr << "] " << component << ": " << message;

    std::string formattedMsg = oss.str ();

    // Always log to ns-3 system (with appropriate level)
    switch (level)
    {
        case INFO:
        case DEBUG:
            NS_LOG_INFO (formattedMsg);
            break;
        case WARN:
            NS_LOG_WARN (formattedMsg);
            break;
        case ERROR:
            NS_LOG_ERROR (formattedMsg);
            break;
    }

    // Log to file if enabled
    if (m_fileLoggingEnabled)
    {
        if (!m_logFile.is_open ())
        {
            m_logFile.open (m_logFilePath, std::ios::app);
        }
        if (m_logFile.is_open ())
        {
            m_logFile << formattedMsg << std::endl;
            m_logFile.flush (); // Ensure immediate write for debugging
        }
    }
}

void
SoftUeLogger::LogInfo (const std::string& component, const std::string& message)
{
    Log (INFO, component, message);
}

void
SoftUeLogger::LogWarning (const std::string& component, const std::string& message)
{
    Log (WARN, component, message);
}

void
SoftUeLogger::LogError (const std::string& component, const std::string& message)
{
    Log (ERROR, component, message);
}

void
SoftUeLogger::SetLogFile (const std::string& filePath)
{
    m_logFilePath = filePath;

    // Close existing file if open
    if (m_logFile.is_open ())
    {
        m_logFile.close ();
    }
}

void
SoftUeLogger::EnableFileLogging (bool enable)
{
    m_fileLoggingEnabled = enable;

    if (!enable && m_logFile.is_open ())
    {
        m_logFile.close ();
    }
}

std::string
SoftUeLogger::GetTimestamp () const
{
    std::ostringstream oss;
    Time now = Simulator::Now ();

    // Format: [+ss.ssssss] - padded to 12 characters for alignment
    oss << "[+";
    oss << std::fixed << std::setprecision (6) << std::setw (8) << std::setfill ('0') << now.GetSeconds ();
    oss << "]";

    return oss.str ();
}

void
SoftUeLogger::Flush ()
{
    if (m_logFile.is_open ())
    {
        m_logFile.flush ();
    }
}

std::string
SoftUeLogger::LevelToString (LogLevel level) const
{
    switch (level)
    {
        case INFO:   return "INFO";
        case WARN:   return "WARN";
        case ERROR:  return "ERROR";
        case DEBUG:  return "DEBUG";
        default:     return "UNKNOWN";
    }
}

} // namespace ns3