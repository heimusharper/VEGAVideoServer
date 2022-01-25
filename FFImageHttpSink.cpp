#include "FFImageHttpSink.h"

FFImageHttpSink::~FFImageHttpSink()
{
    if (m_sink)
        delete m_sink;
}

FFImageHttpSink &FFImageHttpSink::instance()
{
    static FFImageHttpSink s;
    return s;
}

void FFImageHttpSink::create(const std::string &str)
{

    m_sink = new FFH264DecoderInstance(str);
}

Image *FFImageHttpSink::getImage()
{
    if (!m_sink)
        return nullptr;
    if (m_sink->lifetime() > 1000)
        return nullptr;
    AVPacket frame = m_sink->takeFrame();
    Image *image = new Image;
    image->image = new char[frame.size];
    image->size = frame.size;
    memcpy(image->image, frame.data, frame.size);
    av_packet_unref(&frame);

    try {
        Exiv2::Image::AutoPtr eximage =
                Exiv2::ImageFactory::open((Exiv2::byte *)image->image, image->size);
        if (eximage.get() == 0)
            return nullptr;
        eximage->setTypeSupported(1, Exiv2::mdExif);
        eximage->readMetadata();
        Exiv2::ExifData &exifData = eximage->exifData();

        {
            // GPS

            int lat = int(MavContext::instance().lat() * 10000000);
            int lon = int(MavContext::instance().lon() * 10000000);
            int alt = int(MavContext::instance().alt() * 1000);

            exifData["Exif.GPSInfo.GPSLatitudeRef"]  = (lat < 0) ? "S" : "N";
            exifData["Exif.GPSInfo.GPSLongitudeRef"] = (lon < 0) ? "W" : "E";
            exifData["Exif.GPSInfo.GPSMapDatum"]  = "WGS-84";
            exifData["Exif.GPSInfo.GPSVersionID"] = "2 0 0 0";
            exifData["Exif.Image.GPSTag"]         = 654;

            Exiv2::URationalValue::AutoPtr lat_r(new Exiv2::URationalValue);
            lat_r->value_.push_back(std::make_pair(std::abs(lat), 10000000));
            lat_r->value_.push_back(std::make_pair(0, 1));
            lat_r->value_.push_back(std::make_pair(0, 1));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude"), lat_r.get());

            Exiv2::URationalValue::AutoPtr lon_r(new Exiv2::URationalValue);
            lon_r->value_.push_back(std::make_pair(std::abs(lon), 10000000));
            lon_r->value_.push_back(std::make_pair(0, 1));
            lon_r->value_.push_back(std::make_pair(0, 1));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude"), lon_r.get());

            exifData["Exif.GPSInfo.GPSAltitudeRef"] = 0;
            Exiv2::URationalValue::AutoPtr alt_r(new Exiv2::URationalValue);

            alt_r->value_.push_back(std::make_pair(alt, 1000));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"), alt_r.get());
        }
        {
            // DJI

            // speeds

            exifData["Xmp.drone-dji.FlightXSpeed"] = 0;
            Exiv2::URationalValue::AutoPtr xspd_r(new Exiv2::URationalValue);
            xspd_r->value_.push_back(std::make_pair(int(MavContext::instance().speedX() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"), xspd_r.get());

            exifData["Xmp.drone-dji.FlightYSpeed"] = 0;
            Exiv2::URationalValue::AutoPtr yspd_r(new Exiv2::URationalValue);
            yspd_r->value_.push_back(std::make_pair(int(MavContext::instance().speedY() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Xmp.drone-dji.FlightYSpeed"), yspd_r.get());

            exifData["Xmp.drone-dji.FlightZSpeed"] = 0;
            Exiv2::URationalValue::AutoPtr zspd_r(new Exiv2::URationalValue);
            zspd_r->value_.push_back(std::make_pair(int(MavContext::instance().speedZ() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Xmp.drone-dji.FlightZSpeed"), zspd_r.get());


            // angles

            exifData["Xmp.drone-dji.FlightPitchDegree"] = 0;
            Exiv2::URationalValue::AutoPtr pdgr_r(new Exiv2::URationalValue);
            pdgr_r->value_.push_back(std::make_pair(int(MavContext::instance().pitch() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.FlightPitchDegree"), pdgr_r.get());

            exifData["Xmp.drone-dji.FlightYawDegree"] = 0;
            Exiv2::URationalValue::AutoPtr ydgr_r(new Exiv2::URationalValue);
            ydgr_r->value_.push_back(std::make_pair(int(MavContext::instance().yaw() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.FlightYawDegree"), ydgr_r.get());

            exifData["Xmp.drone-dji.FlightRollDegree"] = 0;
            Exiv2::URationalValue::AutoPtr rdgr_r(new Exiv2::URationalValue);
            rdgr_r->value_.push_back(std::make_pair(int(MavContext::instance().roll() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.FlightRollDegree"), rdgr_r.get());

            // gimbal

            exifData["Xmp.drone-dji.GimbalPitchDegree"] = 0;
            Exiv2::URationalValue::AutoPtr pgdgr_r(new Exiv2::URationalValue);
            pgdgr_r->value_.push_back(std::make_pair(int(MavContext::instance().gmbPitch() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GimbalPitchDegree"), pgdgr_r.get());

            exifData["Xmp.drone-dji.GimbalRollDegree"] = 0;
            Exiv2::URationalValue::AutoPtr rgdgr_r(new Exiv2::URationalValue);
            rgdgr_r->value_.push_back(std::make_pair(int(MavContext::instance().gmbRoll() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GimbalRollDegree"), rgdgr_r.get());

            exifData["Xmp.drone-dji.GimbalYawDegree"] = 0;
            Exiv2::URationalValue::AutoPtr ygdgr_r(new Exiv2::URationalValue);
            ygdgr_r->value_.push_back(std::make_pair(int(MavContext::instance().gmbYaw() * 100.), 100));
            exifData.add(Exiv2::ExifKey("Exif.GPSInfo.GimbalYawDegree"), ygdgr_r.get());

        }

        eximage->setExifData(exifData);
        eximage->writeMetadata();
    } catch (Exiv2::Error &e){
        std::cout << "Caught Exiv2 exception '" << e.what() << std::endl;
        return nullptr;
    }
    return image;
}

FFImageHttpSink::FFImageHttpSink()
{
}
