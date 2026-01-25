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
 * @file             Soft-UE.cc
 * @brief            200Gbps End-to-End Stress Test for Ultra Ethernet Protocol
 * @author           softuegroup@gmail.com
 * @version          3.0.0
 * @date             2025-12-16
 * @copyright        Apache License Version 2.0
 *
 * @details
 * Comprehensive stress test for Soft-UE Ultra Ethernet protocol stack at 200Gbps.
 * Features:
 * - High-throughput packet transmission at line rate
 * - Comprehensive statistics collection and analysis
 * - CSV output for data visualization
 * - Performance bottleneck identification
 * - Latency distribution analysis
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/soft-ue-module.h"
#include "ns3/packet.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>
#include <numeric>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SoftUeStressTest");

// 200Gbps = 200 * 1000 * 1000 * 1000 bits/s = 25 GB/s
static const uint64_t TARGET_RATE_BPS = 200000000000ULL; // 200 Gbps
static const uint64_t TARGET_RATE_GBPS = 200;

/**
 * @brief Configuration structure for stress test parameters
 */
struct StressTestConfig
{
    // Network configuration
    uint32_t nodeCount = 2;
    std::string networkBase = "10.1.1.0";
    std::string subnetMask = "255.255.255.0";
    uint16_t serverPort = 8000;

    // Protocol parameters
    uint16_t baseClientPid = 1001;
    uint16_t baseServerPid = 1001;
    uint16_t serverEndpointId = 8000;
    uint16_t clientEndpointId = 1001;
    uint32_t baseClientJobId = 12345;
    uint32_t baseServerJobId = 54321;
    uint64_t baseMemoryAddress = 0x1000;
    uint32_t addressIncrementStep = 1000;

    // Performance parameters - optimized for 200Gbps
    uint32_t packetSize = 9000;        // Jumbo frame for high throughput
    uint32_t packetCount = 100000;     // 100K packets for stress test
    Time sendInterval = NanoSeconds(1); // Minimum interval for max rate
    uint32_t maxPdcCount = 4096;       // Increased PDC count for high concurrency
    uint64_t targetRateBps = TARGET_RATE_BPS;

    // Timing parameters
    Time serverStartTime = MilliSeconds(1);
    Time clientStartTime = MilliSeconds(2);
    Time timeBuffer = MilliSeconds(10);
    Time extraEndTime = MilliSeconds(100);

    // Channel parameters (200Gbps)
    DataRate channelDataRate = DataRate("200Gbps");
    Time propagationDelay = NanoSeconds(100); // ~20m fiber at 5ns/m

    // Statistics parameters
    bool enableDetailedStats = true;
    std::string outputDir = "./stress_test_results/";
    uint32_t statsInterval = 1000;  // Record stats every N packets

    // Protocol overhead
    uint32_t headerOverhead = 42;
};

/**
 * @brief Per-packet statistics record
 */
struct PacketStats
{
    uint32_t packetId;
    Time sendTime;
    Time receiveTime;
    uint32_t packetSize;
    bool received;
    uint16_t pdcId;
};

/**
 * @brief Aggregated statistics
 */
struct AggregatedStats
{
    uint64_t totalPacketsSent = 0;
    uint64_t totalPacketsReceived = 0;
    uint64_t totalBytesSent = 0;
    uint64_t totalBytesReceived = 0;
    Time firstSendTime;
    Time lastSendTime;
    Time firstReceiveTime;
    Time lastReceiveTime;
    std::vector<double> latenciesNs;
    uint64_t droppedPackets = 0;
    uint64_t sesProcessed = 0;
    uint64_t pdsProcessed = 0;
    uint64_t pdcCreated = 0;
    uint64_t retransmissions = 0;

    // Time series data for throughput over time
    std::vector<std::pair<double, double>> throughputTimeSeries; // (time_ns, throughput_gbps)
    uint64_t lastSnapshotBytes = 0;
    Time lastSnapshotTime;

    double GetAverageLatencyNs() const {
        if (latenciesNs.empty()) return 0.0;
        return std::accumulate(latenciesNs.begin(), latenciesNs.end(), 0.0) / latenciesNs.size();
    }

    double GetP99LatencyNs() const {
        if (latenciesNs.empty()) return 0.0;
        std::vector<double> sorted = latenciesNs;
        std::sort(sorted.begin(), sorted.end());
        size_t idx = static_cast<size_t>(sorted.size() * 0.99);
        return sorted[std::min(idx, sorted.size() - 1)];
    }

    double GetP50LatencyNs() const {
        if (latenciesNs.empty()) return 0.0;
        std::vector<double> sorted = latenciesNs;
        std::sort(sorted.begin(), sorted.end());
        return sorted[sorted.size() / 2];
    }

    double GetMinLatencyNs() const {
        if (latenciesNs.empty()) return 0.0;
        return *std::min_element(latenciesNs.begin(), latenciesNs.end());
    }

    double GetMaxLatencyNs() const {
        if (latenciesNs.empty()) return 0.0;
        return *std::max_element(latenciesNs.begin(), latenciesNs.end());
    }

    double GetJitterNs() const {
        if (latenciesNs.size() < 2) return 0.0;
        double mean = GetAverageLatencyNs();
        double sq_sum = 0.0;
        for (double lat : latenciesNs) {
            sq_sum += (lat - mean) * (lat - mean);
        }
        return std::sqrt(sq_sum / latenciesNs.size());
    }

    double GetThroughputGbps() const {
        if (lastReceiveTime == firstSendTime) return 0.0;
        double durationNs = (lastReceiveTime - firstSendTime).GetNanoSeconds();
        if (durationNs <= 0) return 0.0;
        return (totalBytesReceived * 8.0) / durationNs;  // Gbps
    }

    double GetPacketRate() const {
        if (lastReceiveTime == firstSendTime) return 0.0;
        double durationSec = (lastReceiveTime - firstSendTime).GetSeconds();
        if (durationSec <= 0) return 0.0;
        return totalPacketsReceived / durationSec;
    }

    double GetLossRate() const {
        if (totalPacketsSent == 0) return 0.0;
        return 100.0 * (totalPacketsSent - totalPacketsReceived) / totalPacketsSent;
    }
};

// Global statistics collector
static AggregatedStats g_stats;
static std::vector<PacketStats> g_packetStats;
static std::map<uint32_t, Time> g_packetSendTimes;

/**
 * @brief Stress test application for high-throughput testing
 */
class StressTestApp : public Application
{
public:
    StressTestApp ();
    virtual ~StressTestApp ();

    void Setup (uint32_t packetSize, uint32_t numPackets, Address destination,
                uint16_t port, bool isServer = false);
    void SetConfiguration (const StressTestConfig& config);
    std::string GetStatistics () const;
    uint32_t GetPacketCount () const { return std::max(m_packetsSent, m_packetsReceived); }
    uint32_t GetSesProcessedCount () const { return m_sesProcessed; }
    uint32_t GetPdsProcessedCount () const { return m_pdsProcessed; }

private:
    virtual void StartApplication () override;
    virtual void StopApplication () override;
    void SendPacket ();
    void ScheduleSend ();
    bool HandleRead (Ptr<NetDevice> device, Ptr<const Packet> packet,
                    uint16_t protocolType, const Address& source);

    uint32_t m_packetSize;
    uint32_t m_numPackets;
    uint32_t m_packetsSent;
    uint32_t m_packetsReceived;
    uint32_t m_sesProcessed;
    uint32_t m_pdsProcessed;
    Address m_destination;
    uint16_t m_port;
    bool m_isServer;
    EventId m_sendEvent;
    Ptr<SesManager> m_sesManager;
    Ptr<PdsManager> m_pdsManager;

    uint64_t m_totalBytesSent;
    uint64_t m_totalBytesReceived;
    Time m_firstPacketTime;
    Time m_lastPacketTime;

    StressTestConfig m_config;
};

StressTestApp::StressTestApp ()
    : m_packetSize (0), m_numPackets (0), m_packetsSent (0), m_packetsReceived (0),
      m_sesProcessed (0), m_pdsProcessed (0), m_port (0), m_isServer (false),
      m_totalBytesSent (0), m_totalBytesReceived (0)
{
}

StressTestApp::~StressTestApp ()
{
}

void
StressTestApp::Setup (uint32_t packetSize, uint32_t numPackets,
                      Address destination, uint16_t port, bool isServer)
{
    m_packetSize = packetSize;
    m_numPackets = numPackets;
    m_destination = destination;
    m_port = port;
    m_isServer = isServer;
}

void
StressTestApp::SetConfiguration (const StressTestConfig& config)
{
    m_config = config;
}

void
StressTestApp::StartApplication ()
{
    NS_LOG_FUNCTION (this);

    if (GetNode ()->GetNDevices () > 0)
    {
        // Find SoftUeNetDevice by iterating through all devices
        for (uint32_t i = 0; i < GetNode ()->GetNDevices (); ++i)
        {
            Ptr<SoftUeNetDevice> device = GetNode ()->GetDevice (i)->GetObject<SoftUeNetDevice> ();
            if (device)
            {
                m_sesManager = device->GetSesManager ();
                m_pdsManager = device->GetPdsManager ();

                if (m_sesManager && m_pdsManager)
                {
                    m_sesManager->SetPdsManager (m_pdsManager);
                }

                if (m_isServer)
                {
                    device->SetReceiveCallback (MakeCallback (&StressTestApp::HandleRead, this));
                    NS_LOG_INFO ("Server ready for stress test");
                }
                break;
            }
        }
    }

    if (!m_isServer)
    {
        // Start burst sending for maximum throughput
        m_firstPacketTime = Simulator::Now ();
        g_stats.firstSendTime = m_firstPacketTime;
        ScheduleSend ();
        NS_LOG_INFO ("Client starting stress test: " << m_numPackets << " packets of "
                    << m_packetSize << " bytes");
    }
}

void
StressTestApp::StopApplication ()
{
    NS_LOG_FUNCTION (this);
    if (m_sendEvent.IsPending ())
    {
        Simulator::Cancel (m_sendEvent);
    }
}

void
StressTestApp::ScheduleSend ()
{
    if (m_packetsSent < m_numPackets)
    {
        // Calculate inter-packet gap for target rate
        // At 200Gbps with 9000-byte packets: interval = (9000*8) / 200e9 = 360 ns
        double packetBits = m_packetSize * 8.0;
        double intervalNs = (packetBits / m_config.targetRateBps) * 1e9;
        Time tNext = NanoSeconds(std::max(1.0, intervalNs));

        m_sendEvent = Simulator::Schedule (tNext, &StressTestApp::SendPacket, this);
    }
}

void
StressTestApp::SendPacket ()
{
    NS_LOG_FUNCTION (this << m_packetsSent);

    if (!m_sesManager || !m_pdsManager)
    {
        NS_LOG_ERROR ("Managers not initialized");
        return;
    }

    // Create packet
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    if (!packet)
    {
        NS_LOG_ERROR ("Failed to create packet");
        return;
    }

    // Create PDS header
    PDSHeader pdsHeader;
    pdsHeader.SetPdcId (m_packetsSent % m_config.maxPdcCount + 1);
    pdsHeader.SetSequenceNumber (m_packetsSent + 1);
    pdsHeader.SetSom (m_packetsSent == 0);
    pdsHeader.SetEom (m_packetsSent == m_numPackets - 1);
    packet->AddHeader (pdsHeader);

    // Create operation metadata
    Ptr<ExtendedOperationMetadata> extMetadata = Create<ExtendedOperationMetadata> ();
    extMetadata->op_type = OpType::SEND;
    extMetadata->s_pid_on_fep = m_config.baseClientPid + (m_packetsSent % 1000);
    extMetadata->t_pid_on_fep = (m_config.baseClientPid + 1000) + (m_packetsSent % 1000);
    extMetadata->job_id = m_config.baseClientJobId;
    extMetadata->messages_id = m_packetsSent + 1;
    extMetadata->payload.start_addr = m_config.baseMemoryAddress + m_packetsSent * m_config.addressIncrementStep;
    extMetadata->payload.length = m_packetSize;
    extMetadata->payload.imm_data = 0xDEADBEEF + m_packetsSent;
    extMetadata->use_optimized_header = false;
    extMetadata->has_imm_data = true;

    uint32_t srcNodeId = GetNode()->GetId () + 1;
    uint32_t dstNodeId = (srcNodeId == 1) ? 2 : 1;
    extMetadata->SetSourceEndpoint (srcNodeId, m_config.clientEndpointId);
    extMetadata->SetDestinationEndpoint (dstNodeId, m_config.serverEndpointId);

    // Process through SES
    bool sesProcessed = m_sesManager->ProcessSendRequest (extMetadata);

    // Send through device - find SoftUeNetDevice
    Ptr<SoftUeNetDevice> device;
    for (uint32_t i = 0; i < GetNode ()->GetNDevices (); ++i)
    {
        device = GetNode ()->GetDevice (i)->GetObject<SoftUeNetDevice> ();
        if (device) break;
    }
    if (device)
    {
        Time sendTime = Simulator::Now ();
        g_packetSendTimes[m_packetsSent] = sendTime;

        bool success = device->Send (packet, m_destination, 0x0800);
        if (success)
        {
            m_packetsSent++;
            if (sesProcessed) m_sesProcessed++;
            m_pdsProcessed++;
            m_totalBytesSent += packet->GetSize ();

            g_stats.totalPacketsSent++;
            g_stats.totalBytesSent += packet->GetSize ();
            g_stats.lastSendTime = sendTime;

            // Record packet stats periodically
            if (m_packetsSent % m_config.statsInterval == 0)
            {
                NS_LOG_INFO ("Progress: " << m_packetsSent << "/" << m_numPackets
                            << " (" << (100.0 * m_packetsSent / m_numPackets) << "%)");
            }
        }
    }

    ScheduleSend ();
}

bool
StressTestApp::HandleRead (Ptr<NetDevice> device, Ptr<const Packet> packet,
                          uint16_t protocolType, const Address& source)
{
    if (!packet) return false;

    Time receiveTime = Simulator::Now ();
    m_packetsReceived++;
    m_totalBytesReceived += packet->GetSize ();
    m_lastPacketTime = receiveTime;

    // Update global stats
    g_stats.totalPacketsReceived++;
    g_stats.totalBytesReceived += packet->GetSize ();
    g_stats.lastReceiveTime = receiveTime;

    if (g_stats.firstReceiveTime == Time(0))
    {
        g_stats.firstReceiveTime = receiveTime;
        g_stats.lastSnapshotTime = receiveTime;
        g_stats.lastSnapshotBytes = 0;
    }

    // Collect throughput time series every 100 packets
    if (g_stats.totalPacketsReceived % 100 == 0 && g_stats.lastSnapshotTime != Time(0))
    {
        double timeDeltaNs = (receiveTime - g_stats.lastSnapshotTime).GetNanoSeconds ();
        if (timeDeltaNs > 0)
        {
            uint64_t bytesDelta = g_stats.totalBytesReceived - g_stats.lastSnapshotBytes;
            double throughputGbps = (bytesDelta * 8.0) / timeDeltaNs;
            double timeNs = (receiveTime - g_stats.firstReceiveTime).GetNanoSeconds ();
            g_stats.throughputTimeSeries.push_back (std::make_pair(timeNs, throughputGbps));
            g_stats.lastSnapshotTime = receiveTime;
            g_stats.lastSnapshotBytes = g_stats.totalBytesReceived;
        }
    }

    // Parse header for packet ID
    Ptr<Packet> mutablePacket = packet->Copy ();
    PDSHeader pdsHeader;
    mutablePacket->RemoveHeader (pdsHeader);

    uint32_t packetId = pdsHeader.GetSequenceNumber () - 1;

    // Calculate latency
    auto it = g_packetSendTimes.find (packetId);
    if (it != g_packetSendTimes.end ())
    {
        double latencyNs = (receiveTime - it->second).GetNanoSeconds ();
        g_stats.latenciesNs.push_back (latencyNs);
    }

    m_pdsProcessed++;
    m_sesProcessed++;
    g_stats.pdsProcessed++;
    g_stats.sesProcessed++;

    return true;
}

std::string
StressTestApp::GetStatistics () const
{
    std::ostringstream oss;

    oss << "=== " << (m_isServer ? "Server" : "Client") << " Statistics ===\n"
        << "  Packets Sent: " << m_packetsSent << "\n"
        << "  Packets Received: " << m_packetsReceived << "\n"
        << "  Bytes Sent: " << m_totalBytesSent << "\n"
        << "  Bytes Received: " << m_totalBytesReceived << "\n"
        << "  SES Processed: " << m_sesProcessed << "\n"
        << "  PDS Processed: " << m_pdsProcessed << "\n";

    return oss.str ();
}

/**
 * @brief Write results to CSV files for visualization
 */
void WriteResultsToCSV(const StressTestConfig& config, const AggregatedStats& stats)
{
    // Create output directory
    std::string mkdirCmd = "mkdir -p " + config.outputDir;
    int ret = system(mkdirCmd.c_str());
    (void)ret;

    // Write summary statistics
    std::ofstream summaryFile(config.outputDir + "summary.csv");
    summaryFile << "Metric,Value,Unit\n"
                << "Target Rate," << TARGET_RATE_GBPS << ",Gbps\n"
                << "Achieved Throughput," << std::fixed << std::setprecision(3) << stats.GetThroughputGbps() << ",Gbps\n"
                << "Efficiency," << std::fixed << std::setprecision(2) << (stats.GetThroughputGbps() / TARGET_RATE_GBPS * 100) << ",%\n"
                << "Total Packets Sent," << stats.totalPacketsSent << ",packets\n"
                << "Total Packets Received," << stats.totalPacketsReceived << ",packets\n"
                << "Packet Loss Rate," << std::fixed << std::setprecision(4) << stats.GetLossRate() << ",%\n"
                << "Total Bytes Sent," << stats.totalBytesSent << ",bytes\n"
                << "Total Bytes Received," << stats.totalBytesReceived << ",bytes\n"
                << "Packet Rate," << std::fixed << std::setprecision(0) << stats.GetPacketRate() << ",pps\n"
                << "Average Latency," << std::fixed << std::setprecision(2) << stats.GetAverageLatencyNs() << ",ns\n"
                << "P50 Latency," << std::fixed << std::setprecision(2) << stats.GetP50LatencyNs() << ",ns\n"
                << "P99 Latency," << std::fixed << std::setprecision(2) << stats.GetP99LatencyNs() << ",ns\n"
                << "Min Latency," << std::fixed << std::setprecision(2) << stats.GetMinLatencyNs() << ",ns\n"
                << "Max Latency," << std::fixed << std::setprecision(2) << stats.GetMaxLatencyNs() << ",ns\n"
                << "Jitter," << std::fixed << std::setprecision(2) << stats.GetJitterNs() << ",ns\n"
                << "SES Processed," << stats.sesProcessed << ",operations\n"
                << "PDS Processed," << stats.pdsProcessed << ",operations\n";
    summaryFile.close();

    // Write latency distribution for histogram
    std::ofstream latencyFile(config.outputDir + "latency_distribution.csv");
    latencyFile << "Latency_ns\n";
    for (double lat : stats.latenciesNs)
    {
        latencyFile << std::fixed << std::setprecision(2) << lat << "\n";
    }
    latencyFile.close();

    // Write latency percentiles
    std::ofstream percentileFile(config.outputDir + "latency_percentiles.csv");
    percentileFile << "Percentile,Latency_ns\n";
    if (!stats.latenciesNs.empty())
    {
        std::vector<double> sorted = stats.latenciesNs;
        std::sort(sorted.begin(), sorted.end());
        for (int p = 0; p <= 100; p += 5)
        {
            size_t idx = static_cast<size_t>((sorted.size() - 1) * p / 100.0);
            percentileFile << p << "," << std::fixed << std::setprecision(2) << sorted[idx] << "\n";
        }
    }
    percentileFile.close();

    // Write throughput time series
    std::ofstream timeSeriesFile(config.outputDir + "throughput_timeseries.csv");
    timeSeriesFile << "Time_ns,Time_us,Throughput_Gbps\n";
    for (const auto& point : stats.throughputTimeSeries)
    {
        timeSeriesFile << std::fixed << std::setprecision(2)
                       << point.first << ","
                       << (point.first / 1000.0) << ","
                       << point.second << "\n";
    }
    timeSeriesFile.close();

    NS_LOG_INFO("Results written to " << config.outputDir);
}

/**
 * @brief Print comprehensive test results
 */
void PrintResults(const StressTestConfig& config, const AggregatedStats& stats)
{
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "           SOFT-UE 200Gbps STRESS TEST RESULTS\n";
    std::cout << std::string(80, '=') << "\n\n";

    // Configuration summary
    std::cout << "=== Test Configuration ===\n"
              << "  Target Rate: " << TARGET_RATE_GBPS << " Gbps\n"
              << "  Packet Size: " << config.packetSize << " bytes\n"
              << "  Total Packets: " << config.packetCount << "\n"
              << "  Max PDC Count: " << config.maxPdcCount << "\n"
              << "  Channel Delay: " << config.propagationDelay.GetNanoSeconds() << " ns\n\n";

    // Throughput results
    double achievedGbps = stats.GetThroughputGbps();
    double efficiency = (achievedGbps / TARGET_RATE_GBPS) * 100.0;

    std::cout << "=== Throughput Results ===\n"
              << "  Target Throughput:   " << TARGET_RATE_GBPS << " Gbps\n"
              << "  Achieved Throughput: " << std::fixed << std::setprecision(3) << achievedGbps << " Gbps\n"
              << "  Efficiency:          " << std::fixed << std::setprecision(2) << efficiency << "%\n"
              << "  Packet Rate:         " << std::fixed << std::setprecision(0) << stats.GetPacketRate() << " pps\n\n";

    // Packet statistics
    std::cout << "=== Packet Statistics ===\n"
              << "  Packets Sent:     " << stats.totalPacketsSent << "\n"
              << "  Packets Received: " << stats.totalPacketsReceived << "\n"
              << "  Packet Loss:      " << (stats.totalPacketsSent - stats.totalPacketsReceived) << " ("
              << std::fixed << std::setprecision(4) << stats.GetLossRate() << "%)\n"
              << "  Bytes Sent:       " << stats.totalBytesSent << " ("
              << std::fixed << std::setprecision(2) << (stats.totalBytesSent / 1e9) << " GB)\n"
              << "  Bytes Received:   " << stats.totalBytesReceived << " ("
              << std::fixed << std::setprecision(2) << (stats.totalBytesReceived / 1e9) << " GB)\n\n";

    // Latency analysis
    std::cout << "=== Latency Analysis ===\n"
              << "  Average Latency: " << std::fixed << std::setprecision(2) << stats.GetAverageLatencyNs() << " ns\n"
              << "  P50 Latency:     " << std::fixed << std::setprecision(2) << stats.GetP50LatencyNs() << " ns\n"
              << "  P99 Latency:     " << std::fixed << std::setprecision(2) << stats.GetP99LatencyNs() << " ns\n"
              << "  Min Latency:     " << std::fixed << std::setprecision(2) << stats.GetMinLatencyNs() << " ns\n"
              << "  Max Latency:     " << std::fixed << std::setprecision(2) << stats.GetMaxLatencyNs() << " ns\n"
              << "  Jitter:          " << std::fixed << std::setprecision(2) << stats.GetJitterNs() << " ns\n\n";

    // Protocol statistics
    std::cout << "=== Protocol Statistics ===\n"
              << "  SES Processed: " << stats.sesProcessed << "\n"
              << "  PDS Processed: " << stats.pdsProcessed << "\n\n";

    // Performance assessment
    std::cout << "=== Performance Assessment ===\n";
    if (efficiency >= 90.0)
    {
        std::cout << "  Status: EXCELLENT - Achieved >90% of target throughput\n";
    }
    else if (efficiency >= 70.0)
    {
        std::cout << "  Status: GOOD - Achieved 70-90% of target throughput\n";
    }
    else if (efficiency >= 50.0)
    {
        std::cout << "  Status: MODERATE - Achieved 50-70% of target throughput\n";
        std::cout << "  Recommendation: Consider optimizing PDC allocation or increasing burst size\n";
    }
    else
    {
        std::cout << "  Status: NEEDS IMPROVEMENT - Below 50% of target throughput\n";
        std::cout << "  Recommendations:\n";
        std::cout << "    - Review packet processing pipeline\n";
        std::cout << "    - Optimize SES/PDS layer interactions\n";
        std::cout << "    - Consider batching optimizations\n";
    }

    if (stats.GetLossRate() > 0.01)
    {
        std::cout << "  Warning: Packet loss detected (" << std::fixed << std::setprecision(4)
                  << stats.GetLossRate() << "%)\n";
    }

    if (stats.GetP99LatencyNs() > stats.GetAverageLatencyNs() * 10)
    {
        std::cout << "  Warning: High tail latency detected (P99/avg ratio > 10x)\n";
    }

    std::cout << "\n" << std::string(80, '=') << "\n";
}

int
main (int argc, char *argv[])
{
    // Initialize configuration
    StressTestConfig config;
    bool enableTracing = false;
    bool verbose = false;

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue ("packetSize", "Size of each packet in bytes", config.packetSize);
    cmd.AddValue ("numPackets", "Number of packets to send", config.packetCount);
    cmd.AddValue ("maxPdcCount", "Maximum PDC count per device", config.maxPdcCount);
    cmd.AddValue ("enableTracing", "Enable packet tracing", enableTracing);
    cmd.AddValue ("verbose", "Enable verbose logging", verbose);
    cmd.AddValue ("outputDir", "Output directory for results", config.outputDir);
    cmd.Parse (argc, argv);

    // Configure logging
    if (verbose)
    {
        LogComponentEnable ("SoftUeStressTest", LOG_LEVEL_INFO);
        LogComponentEnable ("SoftUeNetDevice", LOG_LEVEL_WARN);
        LogComponentEnable ("PdsManager", LOG_LEVEL_WARN);
        LogComponentEnable ("SesManager", LOG_LEVEL_WARN);
    }
    else
    {
        LogComponentEnable ("SoftUeStressTest", LOG_LEVEL_WARN);
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "   SOFT-UE 200Gbps END-TO-END STRESS TEST\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Configuration:\n"
              << "  Packet Size: " << config.packetSize << " bytes\n"
              << "  Packet Count: " << config.packetCount << "\n"
              << "  Target Rate: " << TARGET_RATE_GBPS << " Gbps\n"
              << "  Max PDC: " << config.maxPdcCount << "\n"
              << std::string(60, '-') << "\n";

    // Create nodes
    NodeContainer nodes;
    nodes.Create (config.nodeCount);

    // Install Soft-UE devices with optimized settings
    SoftUeHelper helper;
    helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (config.maxPdcCount));
    helper.SetDeviceAttribute ("EnableStatistics", BooleanValue (true));
    helper.SetDeviceAttribute ("MaxPacketSize", UintegerValue (65535)); // Jumbo frame support

    NetDeviceContainer devices = helper.Install (nodes);
    std::cout << "Installed " << devices.GetN () << " Soft-UE devices\n";

    // Set MTU for jumbo frames on all devices
    for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
        devices.Get (i)->SetMtu (65535);
    }

    // Configure channel parameters for 200Gbps
    Ptr<SoftUeNetDevice> dev0 = DynamicCast<SoftUeNetDevice> (devices.Get (0));
    if (dev0)
    {
        Ptr<SoftUeChannel> channel = DynamicCast<SoftUeChannel> (dev0->GetChannel ());
        if (channel)
        {
            channel->SetDataRate (config.channelDataRate);
            channel->SetDelay (config.propagationDelay);
            std::cout << "Channel configured: " << config.channelDataRate.GetBitRate() / 1e9
                      << " Gbps, " << config.propagationDelay.GetNanoSeconds() << " ns delay\n";
        }
    }

    // Get and initialize managers
    Ptr<SoftUeNetDevice> device0 = DynamicCast<SoftUeNetDevice> (devices.Get (0));
    Ptr<SoftUeNetDevice> device1 = DynamicCast<SoftUeNetDevice> (devices.Get (1));

    Ptr<PdsManager> pdsManager0 = device0->GetPdsManager ();
    Ptr<PdsManager> pdsManager1 = device1->GetPdsManager ();
    Ptr<SesManager> sesManager0 = device0->GetSesManager ();
    Ptr<SesManager> sesManager1 = device1->GetSesManager ();

    pdsManager0->Initialize ();
    pdsManager1->Initialize ();
    sesManager0->Initialize ();
    sesManager1->Initialize ();

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase (config.networkBase.c_str (), config.subnetMask.c_str ());
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    // Get server address
    Ptr<SoftUeNetDevice> serverDevice = DynamicCast<SoftUeNetDevice> (devices.Get (1));
    SoftUeConfig serverConfig = serverDevice->GetConfiguration ();
    Address serverAddress = serverConfig.address;

    // Calculate simulation time
    double packetBits = config.packetSize * 8.0;
    double intervalNs = (packetBits / config.targetRateBps) * 1e9;
    double totalTransmissionNs = config.packetCount * intervalNs;
    double transmissionSec = totalTransmissionNs / 1e9;
    // Ensure minimum simulation time of 1 second
    double simulationTime = std::max(1.0, config.clientStartTime.GetSeconds () +
                           transmissionSec +
                           config.extraEndTime.GetSeconds ());

    std::cout << "Calculated transmission time: " << transmissionSec * 1000 << " ms\n";
    std::cout << "Total simulation time: " << simulationTime << " seconds\n";

    // Create server application
    Ptr<StressTestApp> serverApp = CreateObject<StressTestApp> ();
    serverApp->SetConfiguration (config);
    serverApp->Setup (0, 0, Address (), config.serverPort, true);
    serverApp->SetStartTime (config.serverStartTime);
    serverApp->SetStopTime (Seconds (simulationTime));
    nodes.Get (1)->AddApplication (serverApp);

    // Create client application
    Ptr<StressTestApp> clientApp = CreateObject<StressTestApp> ();
    clientApp->SetConfiguration (config);
    clientApp->Setup (config.packetSize, config.packetCount, serverAddress, config.serverPort, false);
    clientApp->SetStartTime (config.clientStartTime);
    clientApp->SetStopTime (Seconds (simulationTime));
    nodes.Get (0)->AddApplication (clientApp);

    std::cout << "Starting simulation (estimated duration: "
              << std::fixed << std::setprecision(2) << simulationTime << " seconds)...\n";
    std::cout << std::string(60, '-') << "\n";

    // Run simulation
    auto startTime = std::chrono::high_resolution_clock::now();

    Simulator::Stop (Seconds (simulationTime));
    Simulator::Run ();

    auto endTime = std::chrono::high_resolution_clock::now();
    double wallClockSeconds = std::chrono::duration<double>(endTime - startTime).count();

    std::cout << "Simulation completed in " << std::fixed << std::setprecision(2)
              << wallClockSeconds << " seconds (wall clock)\n";

    // Print and save results
    PrintResults(config, g_stats);
    WriteResultsToCSV(config, g_stats);

    // Print individual app statistics
    std::cout << "\n" << clientApp->GetStatistics ();
    std::cout << serverApp->GetStatistics ();

    // PDS Manager statistics - get directly from devices to ensure consistency
    Ptr<PdsStatistics> pdsStats0 = device0->GetPdsManager ()->GetStatistics ();
    Ptr<PdsStatistics> pdsStats1 = device1->GetPdsManager ()->GetStatistics ();
    std::cout << "\nNode 0 PDS Statistics:\n" << pdsStats0->GetStatistics ();
    std::cout << "\nNode 1 PDS Statistics:\n" << pdsStats1->GetStatistics ();

    Simulator::Destroy ();

    // Return success if throughput efficiency > 50%
    double efficiency = g_stats.GetThroughputGbps() / TARGET_RATE_GBPS * 100.0;
    return (efficiency >= 50.0 && g_stats.GetLossRate() < 1.0) ? 0 : 1;
}
