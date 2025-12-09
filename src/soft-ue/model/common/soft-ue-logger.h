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
 * @file             soft-ue-logger.h
 * @brief            Soft-UE Structured Logger
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-09
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains a structured logging system for Soft-UE components
 * that provides better readability and error tracking capabilities.
 */

#ifndef SOFT_UE_LOGGER_H
#define SOFT_UE_LOGGER_H

#include "ns3/log.h"
#include "ns3/simulator.h"
#include <sstream>
#include <fstream>
#include <string>

namespace ns3 {

/**
 * @class SoftUeLogger
 * @brief Structured logger for Soft-UE components
 *
 * This class provides a clean, structured logging interface that outputs
 * formatted messages to both ns-3 log system and a dedicated log file.
 */
class SoftUeLogger
{
public:
    /**
     * @brief Log levels for different types of messages
     */
    enum LogLevel {
        INFO = 0,      ///< Informational messages
        WARN = 1,      ///< Warning messages
        ERROR = 2,     ///< Error messages
        DEBUG = 3      ///< Debug messages (minimal output)
    };

    /**
     * @brief Get the singleton instance
     * @return Reference to the logger instance
     */
    static SoftUeLogger& GetInstance ();

    /**
     * @brief Log a message with specified level
     * @param level Log level
     * @param component Component name
     * @param message Message to log
     */
    void Log (LogLevel level, const std::string& component, const std::string& message);

    /**
     * @brief Log an informational message
     * @param component Component name
     * @param message Message to log
     */
    void LogInfo (const std::string& component, const std::string& message);

    /**
     * @brief Log a warning message
     * @param component Component name
     * @param message Message to log
     */
    void LogWarning (const std::string& component, const std::string& message);

    /**
     * @brief Log an error message
     * @param component Component name
     * @param message Message to log
     */
    void LogError (const std::string& component, const std::string& message);

    /**
     * @brief Set the output log file path
     * @param filePath Path to the log file
     */
    void SetLogFile (const std::string& filePath);

    /**
     * @brief Enable/disable file logging
     * @param enable True to enable file logging
     */
    void EnableFileLogging (bool enable);

    /**
     * @brief Get formatted timestamp string
     * @return Formatted timestamp
     */
    std::string GetTimestamp () const;

    /**
     * @brief Flush any pending log messages
     */
    void Flush ();

private:
    /**
     * @brief Private constructor for singleton
     */
    SoftUeLogger ();

    /**
     * @brief Destructor
     */
    ~SoftUeLogger ();

    /**
     * @brief Copy constructor (disabled)
     */
    SoftUeLogger (const SoftUeLogger&) = delete;

    /**
     * @brief Assignment operator (disabled)
     */
    SoftUeLogger& operator= (const SoftUeLogger&) = delete;

    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation
     */
    std::string LevelToString (LogLevel level) const;

    std::string m_logFilePath;       ///< Path to log file
    bool m_fileLoggingEnabled;       ///< File logging enabled flag
    mutable std::ofstream m_logFile;  ///< Log file stream
};

/**
 * @brief Convenience macros for logging
 */
#define SOFT_UE_LOG_INFO(component, message) \
    SoftUeLogger::GetInstance().LogInfo(component, message)

#define SOFT_UE_LOG_WARN(component, message) \
    SoftUeLogger::GetInstance().LogWarning(component, message)

#define SOFT_UE_LOG_ERROR(component, message) \
    SoftUeLogger::GetInstance().LogError(component, message)

} // namespace ns3

#endif /* SOFT_UE_LOGGER_H */