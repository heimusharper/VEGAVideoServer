#include "MavlinkParser.h"

MavlinkParser::MavlinkParser(int gcs_id) :
    GCS_ID(gcs_id)
{
    m_stop.store(false);
    m_ping = new std::thread(&MavlinkParser::ping, this);
}

MavlinkParser::~MavlinkParser()
{
    m_stop.store(true);
    if (m_ping->joinable())
        m_ping->join();
    delete m_ping;
}

void MavlinkParser::read(char *c, int n)
{
    m_lastRead = std::chrono::system_clock::now();
    mavlink_status_t status;
    mavlink_message_t msg;
    for (int i = 0; i < n; i++)
        if (mavlink_parse_char(DIFFERENT_CHANNEL, (uint8_t)c[i], &msg, &status) != 0)
            read(msg);
}

int MavlinkParser::lifetime() const
{
    const std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRead).count();
}

int MavlinkParser::write(char *buff, int max)
{
    std::lock_guard g(m_messageMutex);
    if (m_messageToSend.empty())
        return 0;
    mavlink_message_t msg = m_messageToSend.front();
    m_messageToSend.pop();
    assert(max >= MAVLINK_MAX_PACKET_LEN);
    uint16_t l = mavlink_msg_to_send_buffer((uint8_t*)buff, &msg);
    if (l > 0)
        return l;
    return 0;
}

void MavlinkParser::ping()
{
    while (!m_stop.load())
    {
        usleep(1000000);
        {
            mavlink_message_t msg_hb;
            mavlink_msg_heartbeat_pack_chan(
            GCS_ID, MAV_COMP_ID_MISSIONPLANNER, DIFFERENT_CHANNEL, &msg_hb, MAV_TYPE_ONBOARD_CONTROLLER,
                        MAV_AUTOPILOT_INVALID, 0, 0, 0);
            std::lock_guard g(m_messageMutex);
            m_messageToSend.push(msg_hb);

        }
    }
}

void MavlinkParser::read(const mavlink_message_t &msg)
{
    if (UAV_ID < 0)
    {
        if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
        {
            mavlink_heartbeat_t hrt;
            mavlink_msg_heartbeat_decode(&msg, &hrt);
            if (hrt.autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA ||
                    hrt.autopilot == MAV_AUTOPILOT_PX4)
            {
                UAV_ID = msg.sysid;
                UAV_COMPID = msg.compid;
                processIntervals();
            }
        }
    }
    else
    {
        switch (msg.msgid) {
        case MAVLINK_MSG_ID_HEARTBEAT:

            break;
        case MAVLINK_MSG_ID_ATTITUDE:
        {
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(&msg, &att);
            MavContext::instance().setAngles(static_cast<float>(att.roll / M_PI * 180.),
                                             static_cast<float>(att.pitch / M_PI * 180.),
                                             static_cast<float>(att.yaw / M_PI * 180.));
            break;
        }
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            mavlink_global_position_int_t pos;
            mavlink_msg_global_position_int_decode(&msg, &pos);
            MavContext::instance().setSpeeds(static_cast<float>(pos.vx) / 100.f,
                                             static_cast<float>(pos.vy) / 100.f,
                                             static_cast<float>(pos.vz) / 100.f);

            MavContext::instance().setLat(static_cast<double>(pos.lat) / 1.e7);
            MavContext::instance().setLon(static_cast<double>(pos.lon) / 1.e7);
            MavContext::instance().setAlt(static_cast<double>(pos.relative_alt) / 1000.);

            MavContext::instance().setTimestamp(pos.time_boot_ms);

            break;
        }
        case MAVLINK_MSG_ID_MOUNT_STATUS:
        {
            mavlink_mount_status_t mnt;
            mavlink_msg_mount_status_decode(&msg, &mnt);
            MavContext::instance().setGimbal(static_cast<float>(mnt.pointing_b) / 100.f,
                                             static_cast<float>(mnt.pointing_a) / 100.f,
                                             static_cast<float>(mnt.pointing_c) / 100.f);
            break;
        }
        default:
            break;
        }
    }
}

void MavlinkParser::processIntervals()
{
    std::list<std::pair<int, int>> msgInterval;
    msgInterval.push_back(std::make_pair(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 10));
    msgInterval.push_back(std::make_pair(MAVLINK_MSG_ID_ATTITUDE, 10));
    msgInterval.push_back(std::make_pair(MAVLINK_MSG_ID_MOUNT_STATUS, 10));

    for (const std::pair<int, int> &p : msgInterval) {
        int rate = 0;
        if (p.second < 0)
            rate = -1;
        else if (p.second == 0)
            rate = 0;
        else
            rate = (int)(1000000.f / (float)p.second);
        mavlink_message_t message;
        mavlink_msg_command_long_pack_chan(
        GCS_ID, 0, DIFFERENT_CHANNEL, &message, UAV_ID, UAV_COMPID, MAV_CMD_SET_MESSAGE_INTERVAL,
                    0, p.first, rate, 0, 0, 0, 0, 0);
        m_messageToSend.push(message);
    }
}
