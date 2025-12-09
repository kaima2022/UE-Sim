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
 * @file             msn-entry.cc
 * @brief            Ultra Ethernet MSN (Message Sequence Number) Management Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-09
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the implementation of MSN entry and management classes
 * for tracking message sequence numbers in the Ultra Ethernet SES layer.
 */

#include "msn-entry.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include <sstream>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MsnEntry");
NS_OBJECT_ENSURE_REGISTERED (MsnEntry);
NS_OBJECT_ENSURE_REGISTERED (MsnTable);

// ============================================================================
// MsnEntry Implementation
// ============================================================================

TypeId
MsnEntry::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MsnEntry")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<MsnEntry> ();
  return tid;
}

TypeId
MsnEntry::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

MsnEntry::MsnEntry ()
  : m_jobId (0),
    m_pdcId (0),
    m_lastPsn (0),
    m_expectedLength (0),
    m_creationTime (Simulator::Now ()),
    m_receivedPacketCount (0),
    m_receivedLength (0),
    m_complete (false)
{
  NS_LOG_FUNCTION (this);
}

MsnEntry::MsnEntry (uint64_t jobId, uint32_t pdcId, uint64_t expectedLength, Time creationTime)
  : m_jobId (jobId),
    m_pdcId (pdcId),
    m_lastPsn (0),
    m_expectedLength (expectedLength),
    m_creationTime (creationTime),
    m_receivedPacketCount (0),
    m_receivedLength (0),
    m_complete (false)
{
  NS_LOG_FUNCTION (this << jobId << pdcId << expectedLength << creationTime);
}

MsnEntry::~MsnEntry ()
{
  NS_LOG_FUNCTION (this);
}

uint64_t
MsnEntry::GetJobId (void) const
{
  return m_jobId;
}

uint32_t
MsnEntry::GetPdcId (void) const
{
  return m_pdcId;
}

uint64_t
MsnEntry::GetLastPsn (void) const
{
  return m_lastPsn;
}

uint64_t
MsnEntry::GetExpectedLength (void) const
{
  return m_expectedLength;
}

Time
MsnEntry::GetCreationTime (void) const
{
  return m_creationTime;
}

bool
MsnEntry::IsComplete (void) const
{
  return m_complete;
}

uint32_t
MsnEntry::GetReceivedPacketCount (void) const
{
  return m_receivedPacketCount;
}

bool
MsnEntry::IsExpectedPsn (uint64_t psn) const
{
  // For the first packet, expect PSN = 0
  if (m_receivedPacketCount == 0)
    {
      return psn == 0;
    }
  // For subsequent packets, expect the next PSN
  return psn == m_lastPsn + 1;
}

bool
MsnEntry::UpdateWithPacket (uint64_t psn, uint32_t packetLength, bool isEom)
{
  NS_LOG_FUNCTION (this << psn << packetLength << isEom);

  // Check if PSN is expected
  if (!IsExpectedPsn (psn))
    {
      NS_LOG_WARN ("Unexpected PSN: " << psn << ", expected: " << (m_lastPsn + 1));
      return false;
    }

  // Update entry
  m_lastPsn = psn;
  m_receivedPacketCount++;
  m_receivedLength += packetLength;
  m_receivedPsns.insert (psn);

  // Check for completion
  if (isEom || (m_expectedLength > 0 && m_receivedLength >= m_expectedLength))
    {
      m_complete = true;
      NS_LOG_INFO ("MSN entry completed: JobId=" << m_jobId
                   << ", Packets=" << m_receivedPacketCount
                   << ", Bytes=" << m_receivedLength);
    }

  return true;
}

bool
MsnEntry::HasTimedOut (Time timeout) const
{
  return (Simulator::Now () - m_creationTime) > timeout;
}

std::set<uint64_t>
MsnEntry::GetMissingPsns (void) const
{
  std::set<uint64_t> missingPsns;

  if (m_receivedPacketCount == 0)
    {
      // If no packets received, we expect PSN 0
      missingPsns.insert (0);
      return missingPsns;
    }

  // Find gaps in the sequence
  for (uint64_t psn = 0; psn <= m_lastPsn; ++psn)
    {
      if (m_receivedPsns.find (psn) == m_receivedPsns.end ())
        {
          missingPsns.insert (psn);
        }
    }

  return missingPsns;
}

std::string
MsnEntry::ToString (void) const
{
  std::ostringstream oss;
  oss << "MsnEntry[JobId=" << m_jobId
      << ", PdcId=" << m_pdcId
      << ", LastPsn=" << m_lastPsn
      << ", ExpectedLength=" << m_expectedLength
      << ", ReceivedLength=" << m_receivedLength
      << ", Packets=" << m_receivedPacketCount
      << ", Complete=" << (m_complete ? "true" : "false")
      << "]";
  return oss.str ();
}

// ============================================================================
// MsnTable Implementation
// ============================================================================

TypeId
MsnTable::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MsnTable")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<MsnTable> ()
    .AddAttribute ("MaxSize",
                   "Maximum number of MSN entries",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&MsnTable::SetMaxSize, &MsnTable::GetMaxSize),
                   MakeUintegerChecker<size_t> ())
    .AddAttribute ("DefaultTimeout",
                   "Default timeout for MSN entries",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&MsnTable::m_defaultTimeout),
                   MakeTimeChecker ());
  return tid;
}

TypeId
MsnTable::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

MsnTable::MsnTable ()
  : m_maxSize (1000),
    m_autoCleanupEnabled (false),
    m_defaultTimeout (Seconds (30)),
    m_totalLookups (0),
    m_totalUpdates (0),
    m_totalRemovals (0)
{
  NS_LOG_FUNCTION (this);
}

MsnTable::~MsnTable ()
{
  NS_LOG_FUNCTION (this);
  Clear ();
  DisableAutoCleanup ();
}

bool
MsnTable::AddEntry (uint64_t jobId, uint32_t pdcId, uint64_t expectedLength)
{
  NS_LOG_FUNCTION (this << jobId << pdcId << expectedLength);

  // Check if entry already exists
  if (m_msnTable.find (jobId) != m_msnTable.end ())
    {
      NS_LOG_WARN ("MSN entry for JobId=" << jobId << " already exists");
      return false;
    }

  // Enforce maximum size
  if (m_msnTable.size () >= m_maxSize)
    {
      EnforceMaxSize ();
      if (m_msnTable.size () >= m_maxSize)
        {
          NS_LOG_ERROR ("MSN table is full, cannot add new entry");
          return false;
        }
    }

  // Create new entry
  Ptr<MsnEntry> entry = Create<MsnEntry> (jobId, pdcId, expectedLength, Simulator::Now ());
  m_msnTable[jobId] = entry;

  NS_LOG_INFO ("Added MSN entry: " << entry->ToString ());
  return true;
}

Ptr<MsnEntry>
MsnTable::FindEntry (uint64_t jobId) const
{
  NS_LOG_FUNCTION (this << jobId);

  m_totalLookups++;

  auto it = m_msnTable.find (jobId);
  if (it != m_msnTable.end ())
    {
      return it->second;
    }

  return nullptr;
}

bool
MsnTable::RemoveEntry (uint64_t jobId)
{
  NS_LOG_FUNCTION (this << jobId);

  auto it = m_msnTable.find (jobId);
  if (it != m_msnTable.end ())
    {
      NS_LOG_INFO ("Removing MSN entry: JobId=" << jobId);
      m_msnTable.erase (it);
      m_totalRemovals++;
      return true;
    }

  NS_LOG_WARN ("MSN entry for JobId=" << jobId << " not found");
  return false;
}

bool
MsnTable::UpdateEntry (uint64_t jobId, uint64_t psn, uint32_t packetLength, bool isEom, bool isFirstPacket)
{
  NS_LOG_FUNCTION (this << jobId << psn << packetLength << isEom << isFirstPacket);

  m_totalUpdates++;

  Ptr<MsnEntry> entry = FindEntry (jobId);
  if (!entry)
    {
      NS_LOG_WARN ("MSN entry for JobId=" << jobId << " not found for update");
      return false;
    }

  return entry->UpdateWithPacket (psn, packetLength, isEom);
}

bool
MsnTable::ValidatePsn (uint64_t jobId, uint64_t psn, uint64_t expectedLength,
                       uint32_t pdcId, bool isFirstPacket, bool isLastPacket)
{
  NS_LOG_FUNCTION (this << jobId << psn << expectedLength << pdcId << isFirstPacket << isLastPacket);

  Ptr<MsnEntry> entry = FindEntry (jobId);

  // If this is the first packet, create a new entry
  if (isFirstPacket && !entry)
    {
      NS_LOG_INFO ("Creating new MSN entry for first packet");
      return AddEntry (jobId, pdcId, expectedLength);
    }

  if (!entry)
    {
      NS_LOG_ERROR ("MSN entry validation failed: no entry found for JobId=" << jobId);
      return false;
    }

  // Validate PSN sequence
  if (!entry->IsExpectedPsn (psn))
    {
      NS_LOG_WARN ("PSN validation failed: expected " << (entry->GetLastPsn () + 1)
                   << ", got " << psn);
      return false;
    }

  return true;
}

uint32_t
MsnTable::CleanupTimedOutEntries (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);

  uint32_t cleanedCount = 0;
  auto it = m_msnTable.begin ();

  while (it != m_msnTable.end ())
    {
      Ptr<MsnEntry> entry = it->second;
      if (entry->HasTimedOut (timeout))
        {
          NS_LOG_INFO ("Cleaning up timed out MSN entry: JobId=" << entry->GetJobId ());
          it = m_msnTable.erase (it);
          cleanedCount++;
          m_totalRemovals++;
        }
      else
        {
          ++it;
        }
    }

  if (cleanedCount > 0)
    {
      NS_LOG_INFO ("Cleaned up " << cleanedCount << " timed out MSN entries");
    }

  return cleanedCount;
}

size_t
MsnTable::GetSize (void) const
{
  return m_msnTable.size ();
}

std::set<uint64_t>
MsnTable::GetAllJobIds (void) const
{
  std::set<uint64_t> jobIds;

  for (const auto& pair : m_msnTable)
    {
      jobIds.insert (pair.first);
    }

  return jobIds;
}

void
MsnTable::Clear (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Clearing MSN table with " << m_msnTable.size () << " entries");
  m_msnTable.clear ();
}

std::string
MsnTable::GetStatistics (void) const
{
  std::ostringstream oss;
  oss << "MsnTable Statistics:\n"
      << "  Size: " << m_msnTable.size () << "/" << m_maxSize << "\n"
      << "  Total Lookups: " << m_totalLookups << "\n"
      << "  Total Updates: " << m_totalUpdates << "\n"
      << "  Total Removals: " << m_totalRemovals << "\n"
      << "  Auto Cleanup: " << (m_autoCleanupEnabled ? "enabled" : "disabled");

  return oss.str ();
}

void
MsnTable::SetMaxSize (size_t maxSize)
{
  NS_LOG_FUNCTION (this << maxSize);
  m_maxSize = maxSize;
  EnforceMaxSize ();
}

size_t
MsnTable::GetMaxSize (void) const
{
  return m_maxSize;
}

void
MsnTable::EnableAutoCleanup (Time interval)
{
  NS_LOG_FUNCTION (this << interval);

  if (m_autoCleanupEnabled)
    {
      DisableAutoCleanup ();
    }

  m_autoCleanupEnabled = true;
  m_cleanupTimer.SetFunction (&MsnTable::DoAutoCleanup, this);
  m_cleanupTimer.Schedule (interval);

  NS_LOG_INFO ("Enabled MSN table auto cleanup with interval " << interval);
}

void
MsnTable::DisableAutoCleanup (void)
{
  NS_LOG_FUNCTION (this);

  if (m_cleanupTimer.IsRunning ())
    {
      m_cleanupTimer.Cancel ();
    }

  m_autoCleanupEnabled = false;
  NS_LOG_INFO ("Disabled MSN table auto cleanup");
}

void
MsnTable::DoAutoCleanup (void)
{
  NS_LOG_FUNCTION (this);

  uint32_t cleanedCount = CleanupTimedOutEntries (m_defaultTimeout);

  // Schedule next cleanup
  if (m_autoCleanupEnabled)
    {
      m_cleanupTimer.Schedule (m_defaultTimeout);
    }

  NS_LOG_DEBUG ("Auto cleanup completed, removed " << cleanedCount << " entries");
}

uint32_t
MsnTable::EnforceMaxSize (void)
{
  uint32_t removedCount = 0;

  while (m_msnTable.size () >= m_maxSize)
    {
      // Remove the oldest entry
      auto oldestIt = m_msnTable.begin ();
      Time oldestTime = Simulator::Now ();

      for (auto it = m_msnTable.begin (); it != m_msnTable.end (); ++it)
        {
          if (it->second->GetCreationTime () < oldestTime)
            {
              oldestTime = it->second->GetCreationTime ();
              oldestIt = it;
            }
        }

      NS_LOG_INFO ("Removing oldest MSN entry to enforce max size: JobId="
                  << oldestIt->second->GetJobId ());
      m_msnTable.erase (oldestIt);
      removedCount++;
      m_totalRemovals++;
    }

  return removedCount;
}

} // namespace ns3