#ifndef MYTH_VIDEOOUT_NULL_H_
#define MYTH_VIDEOOUT_NULL_H_

// MythTV
#include "mythvideoout.h"

class MythVideoOutputNull : public MythVideoOutput
{
  public:
    static MythVideoOutputNull* Create(QSize VideoDim, QSize VideoDispDim, float VideoAspect, MythCodecID CodecID);
    static void GetRenderOptions(RenderOptions& Options);
   ~MythVideoOutputNull() override = default;

    bool Init(QSize VideoDim, QSize VideoDispDim,
              float Aspect, QRect DisplayVisibleRect, MythCodecID CodecID) override;
    void SetDeinterlacing(bool Enable, bool DoubleRate, MythDeintType Force = DEINT_NONE) override;

    void PrepareFrame (MythVideoFrame* Frame, FrameScanType Scan) override;
    void RenderFrame  (MythVideoFrame* Frame, FrameScanType Scan) override;
    void RenderOverlays (OSD& /*Osd*/) override { }
    void RenderEnd    () override { }
    void EndFrame     () override { }
    bool InputChanged(QSize VideoDim,          QSize VideoDispDim,
                      float        Aspect,     MythCodecID  CodecID,
                      bool&        AspectOnly,
                      int          ReferenceFrames, bool ForceChange) override;

  protected:
    MythVideoOutputNull() = default;

  private:
    Q_DISABLE_COPY(MythVideoOutputNull)
};
#endif
