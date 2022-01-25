#include "MavContext.h"

MavContext &MavContext::instance()
{
    static MavContext c;
    return c;
}

MavContext::MavContext()
{

}

uint32_t MavContext::timestamp() const
{
    return m_timestamp;
}

void MavContext::setTimestamp(uint32_t newTimestamp)
{
    m_timestamp = newTimestamp;
}

float MavContext::gmbYaw() const
{
    return m_gmbYaw;
}

void MavContext::setGmbYaw(float newGmbYaw)
{
    m_gmbYaw = newGmbYaw;
}

void MavContext::setGimbal(float r, float p, float y)
{
    m_gmbPitch = p;
    m_gmbRoll = r;
    m_gmbYaw = y;
}

float MavContext::gmbPitch() const
{
    return m_gmbPitch;
}

void MavContext::setGmbPitch(float newGmbPitch)
{
    m_gmbPitch = newGmbPitch;
}

float MavContext::gmbRoll() const
{
    return m_gmbRoll;
}

void MavContext::setGmbRoll(float newGmbRoll)
{
    m_gmbRoll = newGmbRoll;
}

double MavContext::alt() const
{
    return m_alt;
}

void MavContext::setAlt(double newAlt)
{
    m_alt = newAlt;
}

double MavContext::lon() const
{
    return m_lon;
}

void MavContext::setLon(double newLon)
{
    m_lon = newLon;
}

double MavContext::lat() const
{
    return m_lat;
}

void MavContext::setLat(double newLat)
{
    m_lat = newLat;
}

float MavContext::speedZ() const
{
    return m_speedZ;
}

void MavContext::setSpeedZ(float newSpeedZ)
{
    m_speedZ = newSpeedZ;
}

void MavContext::setSpeeds(float x, float y, float z)
{
    m_speedX = x;
    m_speedY = y;
    m_speedZ = z;
}

float MavContext::speedY() const
{
    return m_speedY;
}

void MavContext::setSpeedY(float newSpeedY)
{
    m_speedY = newSpeedY;
}

float MavContext::speedX() const
{
    return m_speedX;
}

void MavContext::setSpeedX(float newSpeedX)
{
    m_speedX = newSpeedX;
}

float MavContext::yaw() const
{
    return m_yaw;
}

void MavContext::setYaw(float newYaw)
{
    m_yaw = newYaw;
}

void MavContext::setAngles(float r, float p, float y)
{
    m_roll = r;
    m_pitch = p;
    m_yaw = y;
}

float MavContext::pitch() const
{
    return m_pitch;
}

void MavContext::setPitch(float newPitch)
{
    m_pitch = newPitch;
}

float MavContext::roll() const
{
    return m_roll;
}

void MavContext::setRoll(float newRoll)
{
    m_roll = newRoll;
}
