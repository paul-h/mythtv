﻿#ifndef MYTH_VIDEOOUT_H_
#define MYTH_VIDEOOUT_H_

// Qt
#include <QSize>
#include <QRect>
#include <QString>
#include <QPoint>
#include <QMap>
#include <qwindowdefs.h>

// MythTV
#include "mythframe.h"
#include "videobuffers.h"
#include "mythcodecid.h"
#include "videoouttypes.h"
#include "mythvideobounds.h"
#include "mythdisplay.h"
#include "mythvideoprofile.h"
#include "mythvideocolourspace.h"
#include "mythavutil.h"
#include "mythdeinterlacer.h"

// Std
#include <memory>

class MythPlayer;
class OSD;
class AudioPlayer;
class MythRender;
class MythPainter;

using MythVideoProfilePtr = std::shared_ptr<MythVideoProfile>;

class MythVideoOutput : public MythVideoBounds
{
    Q_OBJECT

  public:
    static void GetRenderOptions(RenderOptions& Options, MythRender* Render);

    ~MythVideoOutput() override = default;

    virtual bool Init(QSize VideoDim, QSize VideoDispDim,
                      float VideoAspect, QRect WindowRect, MythCodecID CodecID);
    virtual void SetVideoFrameRate(float playback_fps);
    virtual void SetDeinterlacing(bool Enable, bool DoubleRate, MythDeintType Force = DEINT_NONE);
    virtual void PrepareFrame (MythVideoFrame* Frame, FrameScanType Scan = kScan_Ignore) = 0;
    virtual void RenderFrame  (MythVideoFrame* Frame, FrameScanType) = 0;
    virtual void RenderEnd    () = 0;
    virtual void EndFrame     () = 0;
    virtual bool InputChanged(QSize VideoDim, QSize VideoDispDim,
                              float VideoAspect, MythCodecID  CodecID,
                              bool& AspectChanged, int ReferenceFrames, bool ForceChange);
    virtual void GetOSDBounds(QRect& Total, QRect& Visible,
                              float& VisibleAspect, float& FontScaling,
                              float ThemeAspect) const;
    virtual void SetFramesPlayed(long long FramesPlayed);
    virtual long long GetFramesPlayed();
    bool         IsErrored() const;
    VideoErrorState GetError() const;

    void         SetPrebuffering(bool Normal);
    virtual void ClearAfterSeek();
    virtual int  ValidVideoFrames() const;
    int          FreeVideoFrames();
    bool         EnoughFreeFrames();
    bool         EnoughDecodedFrames();
    virtual MythVideoFrame* GetNextFreeFrame();
    virtual void ReleaseFrame(MythVideoFrame* Frame);
    virtual void DeLimboFrame(MythVideoFrame* Frame);
    virtual void StartDisplayingFrame();
    virtual void DoneDisplayingFrame(MythVideoFrame* Frame);
    virtual void DiscardFrame(MythVideoFrame* frame);
    virtual void DiscardFrames(bool KeyFrame, bool Flushed);
    virtual void CheckFrameStates() { }
    virtual MythVideoFrame* GetLastDecodedFrame();
    virtual MythVideoFrame* GetLastShownFrame();
    QString      GetFrameStatus() const;
    QRect        GetImageRect(QRect Rect, QRect* DisplayRect = nullptr);
    QRect        GetSafeRect();

    // These methods are only required by MythPlayerUI
    PictureAttributeSupported GetSupportedPictureAttributes();
    virtual void InitPictureAttributes () { }
    bool         HasSoftwareFrames     () const { return codec_sw_copy(m_videoCodecID); }
    virtual void RenderOverlays        (OSD& /*Osd*/) {}
    virtual void UpdatePauseFrame      (int64_t& /*DisplayTimecode*/,
                                        FrameScanType /*Scan*/ = kScan_Progressive) {}

  protected:
    MythVideoOutput();
    QRect        GetVisibleOSDBounds(float& VisibleAspect, float& FontScaling, float ThemeAspect) const;

    MythVideoColourSpace m_videoColourSpace;
    LetterBoxColour      m_dbLetterboxColour  { kLetterBoxColour_Black };
    MythCodecID          m_videoCodecID       { kCodec_NONE };
    int                  m_maxReferenceFrames { 16 };
    MythVideoProfilePtr  m_videoProfile       { nullptr };
    VideoBuffers         m_videoBuffers;
    VideoErrorState      m_errorState         { kError_None };
    long long            m_framesPlayed       { 0 };
    MythAVCopy           m_copyFrame;
    MythDeinterlacer     m_deinterlacer;
    const VideoFrameTypes* m_renderFormats    { &MythVideoFrame::kDefaultRenderFormats };
    bool                 m_deinterlacing      { false };
    bool                 m_deinterlacing2X    { false };
    MythDeintType        m_forcedDeinterlacer { DEINT_NONE };

  private:
    Q_DISABLE_COPY(MythVideoOutput)
};

#endif
