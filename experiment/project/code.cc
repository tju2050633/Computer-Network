#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TrafficControlExample");

double simulationTime = 225.0; // 仿真时间 seconds

double
experiment(std::string protocal = "dsdv"， uint32_t packetSize = 100， double nodeSpeed = 1.0)
{
    /////////////////////////////////////////////////////////////
    // 配置仿真参数
    /////////////////////////////////////////////////////////////
    // std::cout << "配置仿真参数..." << std::endl;

    // 仿真参数（按论文的表1）
    std::string socketType = "ns3::UdpSocketFactory"; // UDP传输协议
    // double simAreaX = 900;                            // 仿真区域的X轴大小 units
    // double simAreaY = 100;                            // 仿真区域的Y轴大小 units
    uint32_t numMobileOBUs = 7;                       // 移动的OBU节点数量
    uint32_t numStationaryRSUs = 8;                   // 静止的RSU节点数量
    // double nodeSpeed = 1.0;                           // 节点的移动速度 units per second
    // uint32_t packetSize = 100;                        // 数据包大小 bytes
    // double txRange = 250.0; // 传输范围 units

    double dsdvPktGenRate = 7.36; // DSDV协议数据包生成速率 packets/sec
    double olsrPktGenRate = 2.92; // OLSR协议数据包生成速率 packets/sec
    double aodvPktGenRate = 4.53; // AODV协议数据包生成速率 packets/sec

    // 初始位置（按论文的图1）
    Vector obuPositions[7]; // OBU节点的位置坐标数组
    Vector rsuPositions[8]; // RSU节点的位置坐标数组

    obuPositions[0] = Vector(5， 0， 0);
    obuPositions[1] = Vector(-5， 0， 0);
    obuPositions[2] = Vector(10， -20， 0);
    obuPositions[3] = Vector(80， -20， 0);
    obuPositions[4] = Vector(-60， 40， 0);
    obuPositions[5] = Vector(130， 40， 0);
    obuPositions[6] = Vector(280， 80， 0);

    rsuPositions[0] = Vector(0， 0， 0);
    rsuPositions[1] = Vector(40， 0， 0);
    rsuPositions[2] = Vector(80， 0， 0);
    rsuPositions[3] = Vector(120， 0， 0);
    rsuPositions[4] = Vector(160， 0， 0);
    rsuPositions[5] = Vector(200， 0， 0);
    rsuPositions[6] = Vector(240， 0， 0);
    rsuPositions[7] = Vector(280， 0， 0);

    // 默认multi-lane unidirectional traffic
    // 默认Constant Data Rate (CDR)
    // 默认RTS/CTS disabled

    /////////////////////////////////////////////////////////////
    // 创建节点和配置节点属性
    /////////////////////////////////////////////////////////////
    // std::cout << "创建并配置节点..." << std::endl;

    NodeContainer mobileOBUs;
    NodeContainer stationaryRSUs;

    // 创建节点
    mobileOBUs.Create(numMobileOBUs);
    stationaryRSUs.Create(numStationaryRSUs);
    // std::cout << "创建了 " << mobileOBUs.GetN() << " 个移动 OBU 节点和 " << stationaryRSUs.GetN()
            //   << " 个静止 RSU 节点." << std::endl;

    // OBU移动属性、初始位置、初始速度
    MobilityHelper mobilityOBUs;
    mobilityOBUs.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobilityOBUs.Install(mobileOBUs);

    for (uint32_t i = 0; i < mobileOBUs.GetN(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> mobilityModel =
            mobileOBUs.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        mobilityModel->SetPosition(obuPositions[i]);
        mobilityModel->SetVelocity(Vector(nodeSpeed， 0.0， 0.0));
        // std::cout << "OBU 节点 " << i << " 初始位置: " << mobilityModel->GetPosition() << std::endl;
        // std::cout << "OBU 节点 " << i << " 初始速度: " << mobilityModel->GetVelocity() << std::endl;
    }

    // RSU静止属性、初始位置
    MobilityHelper mobilityRSUs;
    mobilityRSUs.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityRSUs.Install(stationaryRSUs);

    for (uint32_t i = 0; i < stationaryRSUs.GetN(); ++i)
    {
        Ptr<ConstantPositionMobilityModel> mobilityModel =
            stationaryRSUs.Get(i)->GetObject<ConstantPositionMobilityModel>();
        mobilityModel->SetPosition(rsuPositions[i]);
        // std::cout << "RSU 节点 " << i << " 初始位置: " << mobilityModel->GetPosition() << std::endl;
    }

    /////////////////////////////////////////////////////////////
    // 安装无线网络设备
    /////////////////////////////////////////////////////////////

    WifiHelper wifi; // WiFi助手类
    wifi.SetStandard(WIFI_STANDARD_80211p);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager"，
                                 "DataMode"，
                                 StringValue("OfdmRate6MbpsBW10MHz")，
                                 "ControlMode"，
                                 StringValue("OfdmRate6MbpsBW10MHz")，
                                 "NonUnicastMode"，
                                 StringValue("OfdmRate6MbpsBW10MHz"));

    YansWifiChannelHelper wifiChannel; // WiFi信道助手类
    wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper(); // WiFi物理层助手类
    wifiPhy.SetChannel(wifiChannel.Create());

    // 配置无线设备
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac"); // 配置为Adhoc模式

    NetDeviceContainer devicesOBU;
    NetDeviceContainer devicesRSU;
    devicesOBU = wifi.Install(wifiPhy， wifiMac， mobileOBUs); // 安装无线设备到
    devicesRSU = wifi.Install(wifiPhy， wifiMac， stationaryRSUs);

    // 合并设备
    NetDeviceContainer devices;
    devices.Add(devicesOBU);
    devices.Add(devicesRSU);

    /////////////////////////////////////////////////////////////
    // 安装网络协议栈
    /////////////////////////////////////////////////////////////
    // std::cout << "设置路由协议..." << std::endl;

    if (protocal == "dsdv")
    {
        // std::cout << "使用 DSDV 路由协议..." << std::endl;
        DsdvHelper protocol;
        InternetStackHelper stack;
        stack.SetRoutingHelper(protocol);
        stack.Install(mobileOBUs);
        stack.Install(stationaryRSUs);
    }
    else if (protocal == "olsr")
    {
        // std::cout << "使用 OLSR 路由协议..." << std::endl;
        OlsrHelper protocol;
        InternetStackHelper stack;
        stack.SetRoutingHelper(protocol);
        stack.Install(mobileOBUs);
        stack.Install(stationaryRSUs);
    }
    else if (protocal == "aodv")
    {
        // std::cout << "使用 AODV 路由协议..." << std::endl;
        AodvHelper protocol;
        InternetStackHelper stack;
        stack.SetRoutingHelper(protocol);
        stack.Install(mobileOBUs);
        stack.Install(stationaryRSUs);
    }
    else
    {
        // std::cout << "未知的路由协议: " << protocal << std::endl;
        return 1;
    }

    /////////////////////////////////////////////////////////////
    // 分配IPv4地址
    /////////////////////////////////////////////////////////////
    // std::cout << "分配IPv4地址..." << std::endl;

    Ipv4AddressHelper address;                                   // IPv4地址助手
    address.SetBase("10.1.1.0"， "255.255.255.0");                // 设置基础地址
    Ipv4InterfaceContainer interfaces = address.Assign(devices); // 分配地址给设备

    /////////////////////////////////////////////////////////////
    // 配置数据包接收应用
    /////////////////////////////////////////////////////////////
    // std::cout << "配置数据包接收应用..." << std::endl;

    uint16_t port = 7;                                                    // 端口号
    Address localAddress(InetSocketAddress(Ipv4Address::GetAny()， port)); // 本地地址
    PacketSinkHelper packetSinkHelper(socketType， localAddress);          // 数据包接收助手

    ApplicationContainer sinkMobileApps;
    ApplicationContainer sinkStationaryApps;
    for (uint32_t i = 0; i < mobileOBUs.GetN(); ++i)
    {
        sinkMobileApps.Add(packetSinkHelper.Install(mobileOBUs.Get(i)));
    }
    for (uint32_t i = 0; i < stationaryRSUs.GetN(); ++i)
    {
        sinkStationaryApps.Add(packetSinkHelper.Install(stationaryRSUs.Get(i)));
    }
    sinkMobileApps.Start(Seconds(0.0));
    sinkMobileApps.Stop(Seconds(simulationTime + 0.1));
    sinkStationaryApps.Start(Seconds(0.0));
    sinkStationaryApps.Stop(Seconds(simulationTime + 0.1));

    /////////////////////////////////////////////////////////////
    // 配置数据包生成发送应用
    /////////////////////////////////////////////////////////////
    // std::cout << "配置数据包生成发送应用..." << std::endl;

    uint32_t numMobileNodes = mobileOBUs.GetN();
    uint32_t numStationaryNodes = stationaryRSUs.GetN();
    uint32_t totalNodes = numMobileNodes + numStationaryNodes;

    ApplicationContainer onoffApps;

    // std::cout << "安装应用程序到移动节点..." << std::endl;
    for (uint32_t i = 0; i < numMobileNodes; ++i)
    {
        // 选择一个不同的节点作为目标
        uint32_t destIndex = (i + 1) % totalNodes;
        Ptr<Node> destNode = (destIndex < numMobileNodes)
                                 ? mobileOBUs.Get(destIndex)
                                 : stationaryRSUs.Get(destIndex - numMobileNodes);

        Ptr<Ipv4> destIpv4 = destNode->GetObject<Ipv4>();
        Ipv4Address destAddr = destIpv4->GetAddress(1， 0).GetLocal(); // 假设地址分配在接口1

        OnOffHelper onOff("ns3::UdpSocketFactory"， InetSocketAddress(destAddr， port));
        onOff.SetAttribute("OnTime"， StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onOff.SetAttribute("OffTime"， StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onOff.SetAttribute("PacketSize"， UintegerValue(packetSize));

        if (protocal == "dsdv")
        {
            onOff.SetAttribute("DataRate"，
                               DataRateValue(DataRate(dsdvPktGenRate * packetSize * 8)));
        }
        else if (protocal == "olsr")
        {
            onOff.SetAttribute("DataRate"，
                               DataRateValue(DataRate(olsrPktGenRate * packetSize * 8)));
        }
        else if (protocal == "aodv")
        {
            onOff.SetAttribute("DataRate"，
                               DataRateValue(DataRate(aodvPktGenRate * packetSize * 8)));
        }
        else
        {
            // std::cout << "未知的路由协议: " << protocal << std::endl;
            return 1;
        }

        onoffApps.Add(onOff.Install(mobileOBUs.Get(i)));
    }

    // std::cout << "安装应用程序到静止节点..." << std::endl;
    for (uint32_t i = 0; i < numStationaryNodes; ++i)
    {
        // 选择一个不同的节点作为目标
        uint32_t destIndex = (i + 1) % totalNodes;
        Ptr<Node> destNode = (destIndex < numMobileNodes)
                                 ? mobileOBUs.Get(destIndex)
                                 : stationaryRSUs.Get(destIndex - numMobileNodes);

        Ptr<Ipv4> destIpv4 = destNode->GetObject<Ipv4>();
        Ipv4Address destAddr = destIpv4->GetAddress(1， 0).GetLocal(); // 假设地址分配在接口1

        OnOffHelper onOff("ns3::UdpSocketFactory"， InetSocketAddress(destAddr， port));
        onOff.SetAttribute("OnTime"， StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onOff.SetAttribute("OffTime"， StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onOff.SetAttribute("PacketSize"， UintegerValue(packetSize));
        onOff.SetAttribute("DataRate"， DataRateValue(DataRate(dsdvPktGenRate * packetSize * 8)));

        onoffApps.Add(onOff.Install(stationaryRSUs.Get(i)));
    }

    onoffApps.Start(Seconds(1.0));
    onoffApps.Stop(Seconds(simulationTime + 0.1));

    /////////////////////////////////////////////////////////////
    // 配置流量监控器
    /////////////////////////////////////////////////////////////
    // std::cout << "配置流量监控器..." << std::endl;

    FlowMonitorHelper flowmon;                       // 流量监视器助手
    Ptr<FlowMonitor> monitor = flowmon.InstallAll(); // 安装流量监视器

    Simulator::Stop(Seconds(simulationTime + 5)); // 设置模拟器停止时间
    Simulator::Run();                             // 运行模拟器
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier()); // 获取流量分类器
    std::map<FlowId， FlowMonitor::FlowStats> stats = monitor->GetFlowStats(); // 获取流量统计信息

    /////////////////////////////////////////////////////////////
    // 流量监控统计
    /////////////////////////////////////////////////////////////

    // std::cout << std::endl << "*** 流量监控统计 ***" << std::endl;
    // std::cout << "  发送的数据包/字节:   " << stats[1].txPackets << " / " << stats[1].txBytes
    //           << std::endl;
    // std::cout << "  提供的负载: "
    //           << stats[1].txBytes * 8.0 /
    //                  (stats[1].timeLastTxPacket.GetSeconds() -
    //                   stats[1].timeFirstTxPacket.GetSeconds()) /
    //                  1000000
    //           << " Mbps" << std::endl;
    // std::cout << "  接收的数据包/字节:   " << stats[1].rxPackets << " / " << stats[1].rxBytes
    //           << std::endl;

    // // 队列丢弃
    // uint32_t packetsDroppedByQueueDisc = 0; // 由队列丢弃的数据包
    // uint64_t bytesDroppedByQueueDisc = 0;   // 由队列丢弃的字节
    // if (stats[1].packetsDropped.size() > Ipv4FlowProbe::DROP_QUEUE_DISC)
    // {
    //     packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    //     bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    // }
    // std::cout << "  由队列丢弃的数据包/字节:   " << packetsDroppedByQueueDisc << " / "
    //           << bytesDroppedByQueueDisc << std::endl;

    // // 网络设备丢弃
    // uint32_t packetsDroppedByNetDevice = 0; // 由网络设备丢弃的数据包
    // uint64_t bytesDroppedByNetDevice = 0;   // 由网络设备丢弃的字节
    // if (stats[1].packetsDropped.size() > Ipv4FlowProbe::DROP_QUEUE)
    // {
    //     packetsDroppedByNetDevice = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE];
    //     bytesDroppedByNetDevice = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE];
    // }
    // std::cout << "  由网络设备丢弃的数据包/字节:   " << packetsDroppedByNetDevice << " / "
    //           << bytesDroppedByNetDevice << std::endl;

    // 统计量
    // std::cout << "  吞吐量: "
            //   << stats[1].rxBytes * 8.0 /
            //          (stats[1].timeLastRxPacket.GetSeconds() -
            //           stats[1].timeFirstRxPacket.GetSeconds()) /
            //          1000000
            //   << " Mbps" << std::endl;
    // std::cout << "  平均延迟:   " << stats[1].delaySum.GetSeconds() / stats[1].rxPackets
            //   << std::endl;
    // std::cout << "  平均抖动:   " << stats[1].jitterSum.GetSeconds() / (stats[1].rxPackets - 1)
            //   << std::endl;
    auto dscpVec = classifier->GetDscpCounts(1); // 获取DSCP计数
    // for (auto p : dscpVec)
    // {
    //     std::cout << "  DSCP值:   0x" << std::hex << static_cast<uint32_t>(p.first) << std::dec
    //               << "  计数:   " << p.second << std::endl;
    // }

    Simulator::Destroy(); // 销毁模拟器

    // std::cout << std::endl << "*** 应用程序统计 ***" << std::endl;
    double sentBytes = stats[1].txBytes;
    double totalReceivedBytes = 0;
    for (uint32_t i = 0; i < sinkMobileApps.GetN(); ++i)
    {
        uint64_t receivedBytes = DynamicCast<PacketSink>(sinkMobileApps.Get(i))->GetTotalRx();
        totalReceivedBytes += receivedBytes;
    }
    for (uint32_t i = 0; i < sinkStationaryApps.GetN(); ++i)
    {
        uint64_t receivedBytes = DynamicCast<PacketSink>(sinkStationaryApps.Get(i))->GetTotalRx();
        totalReceivedBytes += receivedBytes;
    }
    // double thr = totalReceivedBytes * 8 / (simulationTime * 1000000.0); // Mbit/s
    // std::cout << "总接收的字节: " << totalReceivedBytes << std::endl;
    // std::cout << "平均接收的字节: " << totalReceivedBytes / sinkMobileApps.GetN() << std::endl;
    // std::cout << "平均有效吞吐量: " << thr << " Mbit/s" << std::endl;
    double PLR = (sentBytes - totalReceivedBytes / sinkMobileApps.GetN()) / sentBytes;
    // std::cout << "平均丢包率: " << PLR << std::endl;

    return PLR;
}

int
main(int argc， char* argv[])
{
    double PLRs[3 * 5 * 5];

    // DSDV
    int index = 0;
    for (int i = 100; i <= 500; i += 100) {
        for (double j = 0.5; j <= 2.5; j += 0.5) {
            PLRs[index] = experiment("dsdv"， i， j);
            std::cout << "dsdv " << i << " " << j << " = " << PLRs[index] << std::endl;
            index++;
        }
    }

    // OLSR
    for (int i = 100; i <= 500; i += 100) {
        for (double j = 0.5; j <= 2.5; j += 0.5) {
            PLRs[index] = experiment("olsr"， i， j);
            std::cout << "olsr " << i << " " << j << " = " << PLRs[index] << std::endl;
            index++;
        }
    }

    // AODV
    for (int i = 100; i <= 500; i += 100) {
        for (double j = 0.5; j <= 2.5; j += 0.5) {
            PLRs[index] = experiment("aodv"， i， j);
            std::cout << "aodv " << i << " " << j << " = " << PLRs[index] << std::endl;
            index++;
        }
    }

    return 0;
}
