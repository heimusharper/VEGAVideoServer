#include "FFImageHttpSink.h"

FFImageHttpSink::FFImageHttpSink()
{
    Exiv2::XmpProperties::registerNs("http://ns.drone-dji.com/", "drone-dji");
}

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

void FFImageHttpSink::create(const std::string& str, bool sync, int w, int h)
{
    m_sink = new FFH264DecoderInstance(str, sync, w, h);
}

Image *FFImageHttpSink::getImage()
{
    if (!m_sink)
        return nullptr;
    if (m_sink->lifetime() > 1000)
        return nullptr;
    AVPacket* frame = m_sink->takeFrame();
    if (!frame)
        return nullptr;
    const float scaleFactor = m_sink->scaleFactor();
    Image *out = nullptr;
    try {
        Exiv2::Image::AutoPtr eximage =
                Exiv2::ImageFactory::open((Exiv2::byte *)frame->data, frame->size);
        // av_packet_unref(&frame);
        if (eximage.get() == 0) {
            return nullptr;
        }
#if EXIV2_TEST_VERSION(0,27,0)
        eximage->setTypeSupported(1, Exiv2::mdExif | Exiv2::mdXmp);
#endif
        eximage->readMetadata();

        Exiv2::ExifData &exifData = eximage->exifData();
        Exiv2::XmpData &xmpData = eximage->xmpData();
        {
            // GPS

            int lat = int(MavContext::instance().lat() * 10000000);
            int lon = int(MavContext::instance().lon() * 10000000);
            int32_t alt = geo::HeightSource::instance().at(
                        geo::Coords3D(MavContext::instance().lat(), MavContext::instance().lon()));

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
            // camera config
            Exiv2::URationalValue::AutoPtr focal(new Exiv2::URationalValue);
            focal->value_.push_back(std::make_pair(int(MavContext::instance().focalLength() * 100), 100));
            exifData.add(Exiv2::ExifKey("Exif.Photo.FocalLength"), focal.get());

            // 32 mm equivalent
            Exiv2::URationalValue::AutoPtr focal32(new Exiv2::URationalValue);
            focal32->value_.push_back(std::make_pair(int((MavContext::instance().focalLength() /
                                                          MavContext::instance().crop())  * 100), 100));
            exifData.add(Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"), focal32.get());

            // resolution
            Exiv2::URationalValue::AutoPtr xres(new Exiv2::URationalValue);
            xres->value_.push_back(std::make_pair(int(MavContext::instance().xResolution() * 100 * scaleFactor), 100));
            exifData.add(Exiv2::ExifKey("Exif.Photo.FocalPlaneXResolution"), xres.get());
            Exiv2::URationalValue::AutoPtr yres(new Exiv2::URationalValue);
            yres->value_.push_back(std::make_pair(int(MavContext::instance().yResolution() * 100 * scaleFactor), 100));
            exifData.add(Exiv2::ExifKey("Exif.Photo.FocalPlaneYResolution"), yres.get());
            Exiv2::UShortValue::AutoPtr resUnit(new Exiv2::UShortValue);
            resUnit->value_.push_back(3);
            exifData.add(Exiv2::ExifKey("Exif.Photo.FocalPlaneResolutionUnit"), resUnit.get());

            if (std::abs(scaleFactor - 1.f) < 0.000001) {
                const int w = m_sink->width();
                const int h = m_sink->height();
                // image dim
                Exiv2::ULongValue::AutoPtr width(new Exiv2::ULongValue);
                width->value_.push_back(w);
                exifData.add(Exiv2::ExifKey("Exif.Photo.PixelXDimension"), width.get());
                Exiv2::ULongValue::AutoPtr height(new Exiv2::ULongValue);
                height->value_.push_back(h);
                exifData.add(Exiv2::ExifKey("Exif.Photo.PixelYDimension"), height.get());
            }

            // orientation
            Exiv2::UShortValue::AutoPtr oritent(new Exiv2::UShortValue);
            oritent->value_.push_back(3);
            exifData.add(Exiv2::ExifKey("Exif.Image.Orientation"), oritent.get());

            // zoom
            Exiv2::URationalValue::AutoPtr zoom(new Exiv2::URationalValue);
            zoom->value_.push_back(std::make_pair(int(MavContext::instance().zoom()), 1));
            exifData.add(Exiv2::ExifKey("Exif.Photo.DigitalZoomRatio"), zoom.get());
        }
        {
            // DJI

            // speeds
            xmpData["Xmp.drone-dji.FlightXSpeed"] = MavContext::instance().speedX();
            xmpData["Xmp.drone-dji.FlightYSpeed"] = MavContext::instance().speedY();
            xmpData["Xmp.drone-dji.FlightZSpeed"] = MavContext::instance().speedZ();

            // angles
            xmpData["Xmp.drone-dji.FlightPitchDegree"] = MavContext::instance().pitch();
            xmpData["Xmp.drone-dji.FlightRollDegree"] = MavContext::instance().roll();
            xmpData["Xmp.drone-dji.FlightYawDegree"] = MavContext::instance().yaw();

            // gimbal
            xmpData["Xmp.drone-dji.GimbalPitchDegree"] = MavContext::instance().gmbPitch();
            xmpData["Xmp.drone-dji.GimbalRollDegree"] = MavContext::instance().gmbRoll();
            xmpData["Xmp.drone-dji.GimbalYawDegree"] = MavContext::instance().gmbYaw();

            eximage->setXmpData(xmpData);
        }

        eximage->setExifData(exifData);
        eximage->writeMetadata();

        int file_size = eximage->io().size();
        eximage->io().seek(0,Exiv2::BasicIo::beg);
        Exiv2::DataBuf buff = eximage->io().read(file_size);
        out = new Image;
        out->size = file_size;
        out->image = new char[file_size];
        memcpy(out->image, buff.pData_, file_size);
    } catch (Exiv2::Error &e){
        std::cout << "Caught Exiv2 exception '" << e.what() << std::endl;
    }
    av_packet_unref(frame);
    return out;
}
