#pragma once

#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace SilKit::Services::PubSub;

namespace PubSubDemoCommon
{

inline PubSubSpec MakeAngleSpec()
{
    return {"angle", SilKit::Util::SerDes::MediaTypeData()};
}

inline PubSubSpec MakeForceSpec()
{
    return {"force", SilKit::Util::SerDes::MediaTypeData()};
}


inline PubSubSpec MakeXCurrentSpec()
{
    return {"x_current", SilKit::Util::SerDes::MediaTypeData()};
}

inline PubSubSpec MakeXDesiredSpec()
{
    return {"x_desired", SilKit::Util::SerDes::MediaTypeData()};
}



inline std::vector<uint8_t> SerializeDouble(double value)
{
    SilKit::Util::SerDes::Serializer s;
    s.Serialize(value);
    return s.ReleaseBuffer();
}

inline double DeserializeDouble(const std::vector<uint8_t>& buffer)
{
    SilKit::Util::SerDes::Deserializer d(buffer);
    return d.Deserialize<double>();
}

}