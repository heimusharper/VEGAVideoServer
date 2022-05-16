#ifndef MAVCONTEXT_H
#define MAVCONTEXT_H
#include "log.h"
#include <HeightSource.h>
#include <fmt/format.h>
#include <stdint.h>

class MavContext
{
public:

    // https://exiftool.org/TagNames/DJI.html

    static MavContext &instance();

    float roll() const;
    void setRoll(float newRoll);
    float pitch() const;
    void setPitch(float newPitch);
    float yaw() const;
    void setYaw(float newYaw);
    void setAngles(float r, float p, float y);

    float speedX() const;
    void setSpeedX(float newSpeedX);

    float speedY() const;
    void setSpeedY(float newSpeedY);

    float speedZ() const;
    void setSpeedZ(float newSpeedZ);
    void setSpeeds(float x, float y, float z);

    double lat() const;
    void setLat(double newLat);

    double lon() const;
    void setLon(double newLon);

    double alt() const;
    void setAlt(double newAlt);

    float gmbRoll() const;
    void setGmbRoll(float newGmbRoll);
    float gmbPitch() const;
    void setGmbPitch(float newGmbPitch);
    float gmbYaw() const;
    void setGmbYaw(float newGmbYaw);
    void setGimbal(float r, float p, float y);

    uint32_t timestamp() const;
    void setTimestamp(uint32_t newTimestamp);

    float homeAlt() const;
    void setHomeAlt(float newHomeAlt);

    float focalLength() const;
    void setFocalLength(float newFocalLength);

    float crop() const;
    void setCrop(float newCrop);

    float xResolution() const;
    void setXResolution(float newXResolution);

    float yResolution() const;
    void setYResolution(float newYResolution);

    int camOrientationEXIF() const;
    void setCamOrientationEXIF(int newCamOrientationEXIF);

    int zoom() const;
    void setZoom(int newZoom);

    int64_t time() const;
    void setTime(int64_t newTime);

    std::string header() const;
    std::string line(int64_t timeMS, struct tm time) const;

    int satelites() const;
    void setSatelites(int newSatelites);

private:
    MavContext();

    float m_roll = 0;
    float m_pitch = 0;
    float m_yaw = 0;

    float m_speedX = 0;
    float m_speedY = 0;
    float m_speedZ = 0;

    double m_lat = 0;
    double m_lon = 0;
    double m_alt = 0;

    float m_gmbRoll = 0;
    float m_gmbPitch = 0;
    float m_gmbYaw = 0;

    uint32_t m_timestamp = 0;
    int64_t m_time = 0;

    float m_homeAlt = 0;

    float m_focalLength = 50;
    float m_crop = 2;

    float m_xResolution = 0;
    float m_yResolution = 0;

    int m_camOrientationEXIF = 1;

    int m_zoom = 1;

    int m_satelites = 0;

};

#endif // MAVCONTEXT_H
