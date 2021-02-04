#ifndef MYTHHDRVIDEOMETADATA_H
#define MYTHHDRVIDEOMETADATA_H

// MythTV
#include "mythhdr.h"

using MythHDRVideoPtr = std::shared_ptr<class MythHDRVideoMetadata>;

class MythHDRVideoMetadata : public MythHDRMetadata
{
  public:
    static void Populate(class MythVideoFrame* Frame, struct AVFrame* AvFrame);
    MythHDRVideoMetadata() = default;
    explicit MythHDRVideoMetadata(const MythHDRVideoMetadata& Other);

  protected:
    void Update(const struct AVMasteringDisplayMetadata* Display,
                const struct AVContentLightMetadata* Light);
};

#endif
