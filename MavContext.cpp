#include "MavContext.h"

MavContext &MavContext::instance()
{
    static MavContext c;
    return c;
}

MavContext::MavContext()
{

}

int MavContext::satelites() const
{
    return m_satelites;
}

void MavContext::setSatelites(int newSatelites)
{
    m_satelites = newSatelites;
}

int64_t MavContext::time() const
{
    return m_time;
}

void MavContext::setTime(int64_t newTime)
{
    m_time = newTime;
}

std::string MavContext::header() const
{
    return fmt::format("Ширина сенсора: {:.2f}\r\n"
                       "Высота сенсора: {:.2f}\r\n"
                       "Фокусное расстояние: {:.2f}\r\n"
                       "Кроп фактор матрицы: {:.2f}\r\n"
                       "Время (миллисекунды)\tДата время (UTC)\tШирота\tДолгота\tВысота от точки старта (метры)\tВысота от мирового океана (метры)\tСкорость (м/с)\tКоличество спутников\tСкорость по оси X (м/с)\tСкорость по оси Y (м/с)\tСкорость по оси Z(м/с)\tКурс БВС(град.)\tТангаж БВС(град.)\tКрен БВС(град.)\tВидео (N в сессии)\tКурс подвеса(град.)\tТангаж подвеса(град.)\tZoom\r\n"
                       "======\r\n",
                       m_xResolution,
                       m_yResolution,
                       m_focalLength,
                       m_crop
               );
}

std::string MavContext::line(int64_t timeMS, tm time) const
{
    LOG->debug("INF");
    return fmt::format("{}\t{}-{}-{} {}:{}:{}\t{:.8f}\t{:.8f}\t{:.3f}\t{:.3f}\t{:.3f}\t{}\t{:.3f}\t{:.3f}\t{:.3f}\t{:.1f}\t{:.1f}\t{:.1f}\t0\t{:.1f}\t{:.1f}\t{}\r\n",
                       timeMS,
                       time.tm_year,
                       time.tm_mon,
                       time.tm_mday,
                       time.tm_hour,
                       time.tm_min,
                       time.tm_sec,
                       m_lat,
                       m_lon,
                       m_alt, // agl
                       m_alt + m_homeAlt, // msl
                       std::sqrt(m_speedX * m_speedX + m_speedY * m_speedY),
                       m_satelites,
                       m_speedX,
                       m_speedY,
                       m_speedZ,
                       m_yaw,
                       m_pitch,
                       m_roll,
                       m_gmbYaw,
                       m_gmbRoll,
                       m_zoom);
}

int MavContext::zoom() const
{
    return m_zoom;
}

void MavContext::setZoom(int newZoom)
{
    m_zoom = newZoom;
}

int MavContext::camOrientationEXIF() const
{
    return m_camOrientationEXIF;
}

void MavContext::setCamOrientationEXIF(int newCamOrientationEXIF)
{
    m_camOrientationEXIF = newCamOrientationEXIF;
}

float MavContext::yResolution() const
{
    return m_yResolution;
}

void MavContext::setYResolution(float newYResolution)
{
    m_yResolution = newYResolution;
}

float MavContext::xResolution() const
{
    return m_xResolution;
}

void MavContext::setXResolution(float newXResolution)
{
    m_xResolution = newXResolution;
}

float MavContext::crop() const
{
    return m_crop;
}

void MavContext::setCrop(float newCrop)
{
    m_crop = newCrop;
}

float MavContext::focalLength() const
{
    return m_focalLength;
}

void MavContext::setFocalLength(float newFocalLength)
{
    m_focalLength = newFocalLength;
}

void MavContext::setHomeAlt(float newHomeAlt)
{
    m_homeAlt = newHomeAlt;
    geo::HeightSource::instance().setDefaultHeight(m_homeAlt);
}

float MavContext::homeAlt() const
{
    return m_homeAlt;
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
