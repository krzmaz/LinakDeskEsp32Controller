#pragma once

namespace LinakDesk {
class HeightSpeedData {
  public:
    HeightSpeedData(unsigned short heightRaw, short speedRaw) : mHeightRaw{heightRaw}, mSpeedRaw{speedRaw} {}
    unsigned short getRawHeight() const {return mHeightRaw;}
    short getSpeed() const {return mSpeedRaw;}
  private:
    unsigned short mHeightRaw;
    short mSpeedRaw;
};
} // namespace LinakDesk
